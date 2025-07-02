#include <citro2d.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <3ds.h>
#include <3ds/types.h>
#include <3ds/synchronization.h>
#include <cwav.h>
#include <ncsnd.h>
#include <curl/curl.h>
#include <malloc.h>
#include <jansson.h>
#include <unistd.h>
#include "zappka_totp_qr.h"
#include "logs.h"
#include "request.h"
#include "zappka_api_processing.h"
#include "buttons.h"
#include "cwav_shit.h"

#define MAX_SPRITES   1
#define SCREEN_WIDTH  400
#define SCREEN_HEIGHT 240
#define fmin(a, b) ((a) < (b) ? (a) : (b))
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define SOC_ALIGN       0x1000
#define SOC_BUFFERSIZE  0x300000

C2D_TextBuf totpBuf = NULL;
C2D_Text g_totpText[5];  // Just declared, will be initialized when needed
extern time_t czas_wygasniecia;
extern bool logplz;
extern float text_w, text_h;
extern float max_scroll;
size_t totalsajz = 0;
extern bool readingoferta;
bool day = true;
bool cpu_debug = false;
bool citra_machen = false;
uint8_t* obrazek = NULL;
char *zabkazonefeed = NULL;
extern bool json_done;
bool themeon;
int currenttheme;
extern CWAVInfo cwavList[99];
extern int cwavCount;
bool internet_available = false;
static Thread internet_thread;
static bool internet_thread_running = true;
char combinedText[128];
float speed = 20.0f; // High speed for fast reset
int selectioncodelol;
extern Button buttonsy[100];
int selectionthemelol;
extern bool przycskmachen;
bool touchoferta = true;
extern const char *usernan;
extern bool dawajploy;
extern bool canredeem;
extern bool ofertanow;
extern bool zonefeeddone;
extern bool categoryfeeddone;
extern const char* secrettotpglobal;
bool generatingQR = false;
FILE *fptr;
static u32 *SOC_buffer = NULL;
extern const char *id_tokenk;
const char *numertelefonen = NULL;
extern const char *refreshtoken;
int Scene;
bool VOICEACT;
extern const char *userajd;
extern const char *usernan;
const char *nejmenmachen;
int amountzappsy;
extern bool youfuckedup;
extern char zappsystr[64];
float currentY = -400.0f;
float currentY2 = -400.0f;
float timer = 0.0f;
float timer2 = 255.0f;
float timer3 = 0.0f;
float x = 0.0f;
float y = 0.0f;
float startY = -400.0f;
float endY = 0.0f;
float duration = 7.0f;
float elapsed = 0.0f;
float deltaTime = 0.1f;
char themes[100][100];
struct memory {
  char *response;
  size_t size;
};
float easeHop(float t, float start, float end, float duration) {
    float s = 1.70158f * 1.5f; // overshoot factor
    t /= duration;
    t -= 1.0f;
    return (end - start) * (t * t * ((s + 1) * t + s) + 1.0f) + start;
}



void createDirectory(const char* dirPath) {
    FS_Path fsPath = fsMakePath(PATH_ASCII, dirPath);
    FS_Archive sdmcArchive;
    Result rc = FSUSER_OpenArchive(&sdmcArchive, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""));
    if (R_FAILED(rc)) {
        printf("Failed to open SDMC archive: 0x%08lX\n", rc);
        return;
    }
    rc = FSUSER_CreateDirectory(sdmcArchive, fsPath, 0);
    if (R_FAILED(rc)) {
        printf("Failed to create directory '%s': 0x%08lX\n", dirPath, rc);
    } else {
        printf("Directory '%s' created successfully.\n", dirPath);
    }
    FSUSER_CloseArchive(sdmcArchive);
}
C2D_SpriteSheet zaba_frames;
float zaba_x = 0.0f;
int zaba_dir = -1;
u64 lastFrameTime = 0;
int currentFrame = 0;

void zaba(float scale) {
	u64 now = osGetTime();

	// Animate
	if (now - lastFrameTime >= 1000 / 12) {
		currentFrame = (currentFrame + 1) % 3;
		lastFrameTime = now;
	}

	// Get current frame image
	C2D_Image frameImage = C2D_SpriteSheetGetImage(zaba_frames, currentFrame);
	float imageWidth = frameImage.subtex->width;

	// Move
	zaba_x += 1.0f * zaba_dir;

	// Clamp and flip
	if (zaba_x > 500 - (imageWidth * scale)) {
		zaba_x = 500 - (imageWidth * scale);
		zaba_dir = -1;
	} else if (zaba_x < -200) {
		zaba_x = -200;
		zaba_dir = 1;
	}

	// Flip logic
	float drawX = zaba_x;
	float scaleX = scale;

	if (zaba_dir == -1) {
		scaleX = -scale;
		drawX += imageWidth * scale; // correct mirrored anchor
	}

	// Draw
	C2D_DrawImageAtRotated(
		frameImage,
		drawX, 218,
		1.0f,
		0.0f,
		NULL,
		-scaleX, scale
	);
}

void drawShadowedText(C2D_Text* text, float x, float y, float depth, float scaleX, float scaleY, u32 color, u32 shadowColor) {
    static const float shadowOffsets[4][2] = {
        {0.0f, 1.8f},
        {0.0f, -0.7f},
        {-1.7f, 0.0f},
        {1.8f, 0.0f}
    };

    for (int i = 0; i < 4; i++) {
        C2D_DrawText(text, C2D_AlignCenter | C2D_WithColor,
                     x + shadowOffsets[i][0], y + shadowOffsets[i][1],
                     depth, scaleX, scaleY, shadowColor);
    }

    C2D_DrawText(text, C2D_AlignCenter | C2D_WithColor, x, y, depth, scaleX, scaleY, color);
}


void drawShadowedTextWrapped(C2D_Text* text, float x, float y, float depth, float scaleX, float scaleY, u32 color, u32 shadowColor) {
    static const float shadowOffsets[4][2] = {
        {0.0f, 1.8f},
        {0.0f, -0.7f},
        {-1.7f, 0.0f},
        {1.8f, 0.0f}
    };

    for (int i = 0; i < 4; i++) {
        C2D_DrawText(text, C2D_AlignCenter | C2D_WithColor,
                     x + shadowOffsets[i][0], y + shadowOffsets[i][1],
                     depth, scaleX, scaleY, shadowColor, 300.0f);
    }

    C2D_DrawText(text, C2D_AlignCenter | C2D_WithColor, x, y, depth, scaleX, scaleY, color, 300.0f);
}
void drawShadowedText_noncentered(C2D_Text* text, float x, float y, float depth, float scaleX, float scaleY, u32 color, u32 shadowColor) {
    static const float shadowOffsets[4][2] = {
        {0.0f, 1.8f},
        {0.0f, -0.7f},
        {-1.7f, 0.0f},
        {1.8f, 0.0f}
    };

    for (int i = 0; i < 4; i++) {
        C2D_DrawText(text, C2D_WithColor,
                     x + shadowOffsets[i][0], y + shadowOffsets[i][1],
                     depth, scaleX, scaleY, shadowColor);
    }

    C2D_DrawText(text, C2D_WithColor, x, y, depth, scaleX, scaleY, color);
}
typedef struct {
    C2D_Sprite spr;
    float dx, dy;
} Sprite;

C2D_TextBuf g_staticBuf;
extern C2D_TextBuf kupon_text_Buf;
C2D_Text g_staticText[100];
C2D_TextBuf themeBuf;
C2D_Text themeText[100];
extern C2D_Text g_kuponText[100];
C2D_Font font[1];
static C2D_SpriteSheet background_top, background_down, logo, buttons, settings, scan, points, coupons, qrframe, stat, zappbar, couponenbuttonen, goback, act_buttons, deact_buttons, more_b, logout_buttons, themename_border, too_less;
C2D_Image bgtop, bgdown, logo3ds, buttonsmol, buttonmed, buttonbeeg;
C2D_Image scan_button, scan_pressed, coupons_button, coupons_pressed;
C2D_Image points_button, points_pressed, settings_button, settings_pressed;
C2D_Image qr_framen_machen, statusbaren, zappsybaren;
C2D_Image couponbutton, couponbutton_pressed;
C2D_Image gobackplz;
C2D_Image act_button, act_pressed;
C2D_Image deact_button, deact_pressed;
C2D_Image logout_button, logout_pressed;
C2D_Image more_button, themeborder, ch_theme, za_malo;

static Sprite sprites[MAX_SPRITES];
static size_t numSprites = MAX_SPRITES / 2;
bool isScrolling;
bool isLogged;
float easeInQuad(float t, float start, float end, float duration) {
    t /= duration;
    return start + (end - start) * (t * t);
}
float easeOutQuad(float t, float start, float end, float duration) {
    t /= duration;
    return start + (end - start) * (1 - (1 - t) * (1 - t));
}
static size_t cb(void *data, size_t size, size_t nmemb, void *clientp)
{
  size_t realsize = size * nmemb;
  struct memory *mem = (struct memory *)clientp;

  char *ptr = realloc(mem->response, mem->size + realsize + 1);
  if(ptr == NULL)
    return 0;  /* out of memory! */

  mem->response = ptr;
  memcpy(&(mem->response[mem->size]), data, realsize);
  mem->size += realsize;
  mem->response[mem->size] = 0;

  return realsize;
}

void synerise_tokenauth() {
    CURL *curl;
    CURLcode res;

    json_t *json = json_object();
    if (!json) {
        fprintf(stderr, "Failed to create JSON object\n");
        return;
    }
	char device_id[17];
	for (int i = 0; i < 16; i++) {
		sprintf(device_id + i, "%x", rand() % 16);
	}
	device_id[16] = '\0';
	srand(time(NULL)); // Or use better RNG if needed
	char uuid[37];     // 36 chars + null terminator
	generate_uuid_v4(uuid);
    json_object_set_new(json, "apiKey", json_string("2329c8ce-0278-49e2-9d99-5ee70186b5dd"));
    json_object_set_new(json, "uuid", json_string(uuid));
    json_object_set_new(json, "deviceID", json_string(device_id));

    char *data = json_dumps(json, JSON_COMPACT);

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl) {
        struct curl_slist *headers = NULL;

        log_to_file("Starting request to URL: https://zabka-snrs.zabka.pl/sauth/v3/auth/login/client/anonymous");
        log_to_file("Request Data: %s", data);
        curl_easy_setopt(curl, CURLOPT_URL, "https://zabka-snrs.zabka.pl/sauth/v3/auth/login/client/anonymous");


        headers = curl_slist_append(headers, "Content-Type: application/json");
		headers = curl_slist_append(headers, "User-Agent: Synerise Android SDK 6.3.0 pl.zabka.apb2c");
        char auth_header[2048];
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);


        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
		curl_easy_setopt(curl, CURLOPT_CAINFO, "romfs:/cacert.pem");
        curl_easy_setopt(curl, CURLOPT_STDERR, fopen(LOG_FILE, "a"));
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);


        res = curl_easy_perform(curl);


        if (res != CURLE_OK) {
            log_to_file("curl_easy_perform() failed: %s", curl_easy_strerror(res));
        } else {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            log_to_file("Request successful! Response Code: %ld", response_code);
            log_to_file("Response: %s", response);
        }


        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }


    curl_global_cleanup();


    free(data);
    json_decref(json);
}

void send_verification_code(const char *nrtel, const char *id_token) {
    CURL *curl;
    CURLcode res;

    json_t *json = json_object();
    if (!json) {
        fprintf(stderr, "Failed to create JSON object\n");
        return;
    }
	log_to_file("token: %s", id_token);
    json_t *variables = json_object();
    json_t *input = json_object();
    json_t *phone_number = json_object();

    json_object_set_new(phone_number, "countryCode", json_string("48"));
    json_object_set_new(phone_number, "nationalNumber", json_string(nrtel));
    json_object_set_new(input, "phoneNumber", phone_number);
    json_object_set_new(variables, "input", input);

    json_object_set_new(json, "operationName", json_string("SendVerificationCode"));
    json_object_set_new(json, "query", json_string("mutation SendVerificationCode($input: SendVerificationCodeInput!) { sendVerificationCode(input: $input) { retryAfterSeconds } }"));
    json_object_set_new(json, "variables", variables);


    char *data = json_dumps(json, JSON_COMPACT);

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl) {
        struct curl_slist *headers = NULL;

        log_to_file("Starting request to URL: https://super-account.spapp.zabka.pl/");
        log_to_file("Request Data: %s", data);
        curl_easy_setopt(curl, CURLOPT_URL, "https://super-account.spapp.zabka.pl/");


        headers = curl_slist_append(headers, "Content-Type: application/json");
        char auth_header[2048];
        snprintf(auth_header, sizeof(auth_header), "Authorization: %s", id_token);
        headers = curl_slist_append(headers, auth_header);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);


        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
		curl_easy_setopt(curl, CURLOPT_CAINFO, "romfs:/cacert.pem");
        curl_easy_setopt(curl, CURLOPT_STDERR, fopen(LOG_FILE, "a"));
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);


        res = curl_easy_perform(curl);


        if (res != CURLE_OK) {
            log_to_file("curl_easy_perform() failed: %s", curl_easy_strerror(res));
        } else {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            log_to_file("Request successful! Response Code: %ld", response_code);
            log_to_file("Response: %s", response);
        }


        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }


    curl_global_cleanup();


    free(data);
    json_decref(json);
}



bool is_network_connected() {
    Result res = acInit();
    if (R_FAILED(res)) {
        log_to_file("[Żappka3DS] acInit() failed: 0x%08lX\n", res);
        return false;
    }

    u32 status;
    bool connected = false;

    Result getStatusRes = ACU_GetStatus(&status);
    if (R_SUCCEEDED(getStatusRes)) {
        connected = (status == 3);
        log_to_file("[Żappka3DS] ACU_GetStatus: %lu (connected: %s)\n", status, connected ? "yes" : "no");
    } else {
        log_to_file("[Żappka3DS] ACU_GetStatus failed: 0x%08lX\n", getStatusRes);
    }

    acExit();

    return connected;
}


void check_internet() {
	if (is_network_connected()) {
		internet_available = (response_code == 200);
	} else {
		internet_available = false;
	}
}
volatile bool pausedForSleep = false;

void aptHookCallback(APT_HookType hook, void* param) {
    switch (hook) {
        case APTHOOK_ONSUSPEND:
            pausedForSleep = true;
            break;
        case APTHOOK_ONRESTORE:
            pausedForSleep = false;
            break;
        default:
            break;
    }
}
void internet_check_thread(void* arg) {
    while (internet_thread_running) {
        if (!pausedForSleep) {
            check_internet();
            schizofrenia = !internet_available;
        }
        svcSleepThread(4 * 1000000000LL);
    }
}
void start_internet_thread() {
    internet_thread = threadCreate(internet_check_thread, NULL, 32 * 1024, 0x30, -2, false);
}

void send_ver_code_preauth() {

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
	headers = curl_slist_append(headers, "X-Android-Package: pl.zabka.apb2c");
	headers = curl_slist_append(headers, "X-Android-Cert: FAB089D9E5B41002F29848FC8034A391EE177077");
	headers = curl_slist_append(headers, "User-Agent: Dalvik/2.1.0 (Linux; U; Android 13; 22011119UY Build/TP1A.220624.014)");

    json_t *json = json_object();
    json_object_set_new(json, "clientType", json_string("CLIENT_TYPE_ANDROID"));
    char *data = json_dumps(json, JSON_COMPACT);

    const char *url = "https://www.googleapis.com/identitytoolkit/v3/relyingparty/signupNewUser?key=AIzaSyDe2Fgxn_8HJ6NrtJtp69YqXwocutAoa9Q";

    refresh_data(url, data, headers);

    json_t *root = json_loads(global_response.data, 0, NULL);
    if (root) {
        json_t *id_token_json = json_object_get(root, "idToken");
        id_tokenk = strdup(json_string_value(id_token_json));
        json_decref(root);
    }


    free(data);
    json_decref(json);
	
	root = json_object();
	json_object_set_new(root, "idToken", json_string(id_tokenk));
	char *json_data = json_dumps(root, JSON_COMPACT);

	refresh_data("https://www.googleapis.com/identitytoolkit/v3/relyingparty/getAccountInfo?key=AIzaSyDe2Fgxn_8HJ6NrtJtp69YqXwocutAoa9Q", json_data, headers);
	
    curl_slist_free_all(headers);
	free(json_data);
	json_decref(root);
}
void rebuild_buffer() {
	C2D_TextBufClear(g_staticBuf);
    C2D_TextFontParse(&g_staticText[0], font[0], g_staticBuf, "Wciśnij A.");
    C2D_TextOptimize(&g_staticText[0]);
	C2D_TextFontParse(&g_staticText[1], font[0], g_staticBuf, "Ładowanie...");
	C2D_TextOptimize(&g_staticText[1]);
	C2D_TextFontParse(&g_staticText[2], font[0], g_staticBuf, "Nie wykryto danych konta Żappka.\nWciśnij A by kontynuować");
	C2D_TextOptimize(&g_staticText[2]);
	C2D_TextFontParse(&g_staticText[3], font[0], g_staticBuf, "Tak");
	C2D_TextFontParse(&g_staticText[4], font[0], g_staticBuf, "Nie");
	C2D_TextFontParse(&g_staticText[5], font[0], g_staticBuf, "Wprowadź numer telefonu.");
	C2D_TextOptimize(&g_staticText[5]);
	C2D_TextFontParse(&g_staticText[6], font[0], g_staticBuf, "Wprowadź kod SMS.");
	C2D_TextOptimize(&g_staticText[6]);
	C2D_TextFontParse(&g_staticText[7], font[0], g_staticBuf, combinedText);
    C2D_TextFontParse(&g_staticText[8], font[0], g_staticBuf, "Twoje Żappsy");
    C2D_TextOptimize(&g_staticText[8]);
	C2D_TextFontParse(&g_staticText[9], font[0], g_staticBuf, zappsystr);
	C2D_TextOptimize(&g_staticText[9]);
    C2D_TextFontParse(&g_staticText[10], font[0], g_staticBuf, "B - Powrót");
    C2D_TextOptimize(&g_staticText[10]);
    C2D_TextFontParse(&g_staticText[11], font[0], g_staticBuf, "Brak internetu :(");
    C2D_TextOptimize(&g_staticText[11]);
	C2D_TextFontParse(&g_staticText[12], font[0], g_staticBuf, "Kupony");
    C2D_TextOptimize(&g_staticText[12]);
	C2D_TextFontParse(&g_staticText[13], font[0], g_staticBuf, "Gotowe :)");
    C2D_TextOptimize(&g_staticText[13]);
	C2D_TextFontParse(&g_staticText[14], font[0], g_staticBuf, "Opcje");
    C2D_TextOptimize(&g_staticText[14]);
	C2D_TextFontParse(&g_staticText[15], font[0], g_staticBuf, "Motyw:");
    C2D_TextOptimize(&g_staticText[15]);
	C2D_TextFontParse(&g_staticText[16], font[0], g_staticBuf, "(Zrestartuj aplikacje by zapisać zmiany)");
    C2D_TextOptimize(&g_staticText[16]);
	C2D_TextFontParse(&g_staticText[17], font[0], g_staticBuf, "VA");
    C2D_TextOptimize(&g_staticText[17]);
}
void process_json_response() {
    if (global_response.data) {
        log_to_file("Processing JSON response...");
        json_t *response_root = json_loads(global_response.data, 0, NULL);
        if (response_root) {
            json_t *data = json_object_get(response_root, "data");
            if (data) {
                json_t *loyalty = json_object_get(data, "loyaltyProgram");
                if (loyalty) {
                    json_t *punkty = json_object_get(loyalty, "points");
                    if (punkty) {
                        amountzappsy = json_integer_value(punkty);
                        sprintf(zappsystr, "%d", amountzappsy);
                        log_to_file("Points: %d", amountzappsy);
                    } else {
                        log_to_file("ERROR: 'points' not found in the loyaltyProgram.");
                    }
                } else {
                    log_to_file("ERROR: 'loyaltyProgram' not found in the data.");
                }
            } else {
                log_to_file("ERROR: 'data' not found in the response.");
            }

            json_decref(response_root);
        } else {
            log_to_file("ERROR: Failed to parse JSON response.");
        }
    } else {
        log_to_file("ERROR: No response data available to process.");
    }
}
extern char tileNames[100][256];
extern int tileCount;


void chuj() {
	log_to_file("chuj..");
}

void ping() {
	refresh_data("https://www.google.com/", "", NULL);
}

void kupony() {
	if (is_network_connected()) {
		buttonsy[4] = (Button){0};
		if (kuponobraz != NULL) {
			C2D_SpriteSheetFree(kuponobraz);
		}
		ofertanow = false;
		obrazekdone = false;
		offermachen = false;
		touchoferta = true;
		requestdone = false;
		loadingshit = true;
		przycskmachen = false;
		categoryfeeddone = true;
		dawajploy = false;
		categoryornah = false;
		zonefeeddone = false;
		json_done = false;
		timer = 0.0f;
		Scene = 14;
		startY = 0.0f;
		elapsed = 0.0f;
		endY = -400.0f;

		if (sfx->numChannels == 2) {
			cwavPlay(sfx, 0, 1);
		} else {
			cwavPlay(sfx, 0, -1);
		}
		log_to_file("Updating Ploy Zones...");
		updateploy(id_tokenk, refreshtoken);
	}
}


void zappsy() {
	if (is_network_connected()) {
		przycskmachen = false;
		json_done = false;
		timer = 0.0f;
		Scene = 12;
		startY = 0.0f;
		elapsed = 0.0f;
		endY = -400.0f;

		if (sfx->numChannels == 2) {
			cwavPlay(sfx, 0, 1);
		} else {
			cwavPlay(sfx, 0, -1);
		}
		updatezappsy(id_tokenk, refreshtoken);
	}

}

void opcje() {
    przycskmachen = false;
	ofertanow = false;
	obrazekdone = false;
	offermachen = false;
	touchoferta = true;
	requestdone = false;
	loadingshit = true;
	przycskmachen = false;
	categoryfeeddone = true;
	dawajploy = false;
	categoryornah = false;
	zonefeeddone = false;
	json_done = false;
	timer = 0.0f;
    Scene = 16;
	startY = 0.0f;
	elapsed = 0.0f;
	endY = -400.0f;

	if (sfx->numChannels == 2) {
		cwavPlay(sfx, 0, 1);
	} else {
		cwavPlay(sfx, 0, -1);
	}
}
void executeButtonFunction(int buttonIndex) {
    if (buttonIndex >= 0 && buttonIndex < 100 && buttonsy[buttonIndex].onClick != NULL) {
        buttonsy[buttonIndex].onClick();
    } else {
        log_to_file("Invalid button index or function not assigned!\n");
    }
}

static bool running = true;
extern bool czasfuckup;
void generate_qrcode() {
	if (isLogged) {
		int otp = compute_magic_number(secrettotpglobal);
		doBasicDemo(&qrImage, otp, userajd);
	}

}
void kodQR() {
	przycskmachen = false;
	timer = 0.0f;
    Scene = 10;
	startY = 0.0f;
	elapsed = 0.0f;
	endY = -400.0f;

	if (sfx->numChannels == 2) {
		cwavPlay(sfx, 0, 1);
	} else {
		cwavPlay(sfx, 0, -1);
	}
	generate_qrcode();

}
void playSoundOncePerScene(int sceneID, int soundIndex) {
    static int lastScenePlayed = -1;

    if (lastScenePlayed != sceneID) {
        CWAV* sfx_va = cwavList[soundIndex].cwav;
        cwavPlay(sfx_va, 0, -1);
        lastScenePlayed = sceneID;
    }
}


void wyloguj() {
	remove("/3ds/data.json");
	przycskmachen = false;
	timer = 0.0f;
    Scene = 17;
	startY = 0.0f;
	elapsed = 0.0f;
	endY = -400.0f;

	if (sfx->numChannels == 2) {
		cwavPlay(sfx, 0, 1);
	} else {
		cwavPlay(sfx, 0, -1);
	}
	CWAV* bgm = cwavList[1].cwav;
	CWAV* menu = day ? cwavList[3].cwav : cwavList[5].cwav;
	cwavStop(menu, 0, 1);
	if (bgm->numChannels == 2) {
		cwavPlay(bgm, 0, 1);
	} else {
		cwavPlay(bgm, 0, -1);

	}
}
void qr_thread(void* arg) {
    while (running) {
        generate_qrcode();
        svcSleepThread(2 * 1000000000LL);
    }
}
int main(int argc, char* argv[]) {
	cwavEnvMode_t mode = CWAV_ENV_DSP;
	cwavUseEnvironment(mode);
    romfsInit();
	cfguInit();
    gfxInitDefault();
	PrintConsole topConsole;
    consoleInit(GFX_TOP, &topConsole);
	aptHookCookie aptCookie;
	aptHook(&aptCookie, aptHookCallback, NULL);
    ndspInit();
	json_t *jsonfl;
    Result ret;
	init_logger();
	logplz = true;
    SOC_buffer = (u32*)memalign(SOC_ALIGN, SOC_BUFFERSIZE);
	if(SOC_buffer == NULL) {
		printf("memalign: failed to allocate\n");
	}
	if ((ret = socInit(SOC_buffer, SOC_BUFFERSIZE)) != 0) {
    	printf("socInit: 0x%08X\n", (unsigned int)ret);
	}
	totpBuf = C2D_TextBufNew(128);
	C2D_TextBuf memBuf = C2D_TextBufNew(128);
	C2D_Text memtext[100];
	start_request_thread();
	svcSleepThread(100000000);
	log_to_file("[Żappka3DS] Sprawdzam dane.\n");
	if (access("/3ds/opcje.json", F_OK) == 0){
		json_t *jsonfl = json_load_file("/3ds/opcje.json", 0, NULL);
		json_t *czyjest = json_object_get(jsonfl, "va");
		VOICEACT = json_boolean_value(czyjest);
		json_decref(jsonfl);
	} else {
		VOICEACT = true;
		json_t *oproot = json_object();
		json_object_set_new(oproot, "va", json_boolean(VOICEACT));
		json_dump_file(oproot, "/3ds/opcje.json", JSON_COMPACT);
		json_decref(oproot);
	}
	if (access("/3ds/data.json", F_OK) == 0) {
		isLogged = true;
		log_to_file("[Żappka3DS] Zalogowany.\n");
	    jsonfl = json_load_file("/3ds/data.json", 0, NULL);
		json_t *tempajd = json_object_get(jsonfl, "user_id");
		json_t *nwmsecret = json_object_get(jsonfl, "hex_secret");
		json_t *nwmsecretnano = json_object_get(jsonfl, "pay_secret");
		json_t *nejmen = json_object_get(jsonfl, "name");
		json_t *refren = json_object_get(jsonfl, "refresh");
		json_t *ajdentokenen = json_object_get(jsonfl, "token");
		json_t *has_nano = json_object_get(jsonfl, "has_nano");
		if (!tempajd || !nwmsecret || !nwmsecretnano || !nejmen || !has_nano) {
			log_to_file("[Żappka3DS] Brakuje danych, usuwam.\n");
			isLogged = false;
			json_decref(jsonfl);
			remove("/3ds/data.json");
		} else {
			refreshtoken = json_string_value(refren);
			id_tokenk = json_string_value(ajdentokenen);
			nejmenmachen = json_string_value(nejmen);

			userajd = json_string_value(tempajd);
			usernan = json_string_value(has_nano);
			if (strcmp(usernan, "no") == 0) {
				secrettotpglobal = json_string_value(nwmsecret);
			} else {
				secrettotpglobal = json_string_value(nwmsecretnano);
			}

			snprintf(combinedText, sizeof(combinedText), "Witaj %s!", nejmenmachen);
			int otpen = compute_magic_number(secrettotpglobal);
			doBasicDemo(&qrImage, otpen, userajd);
			if (!citra_machen) {
				if (is_network_connected()) {
					log_to_file("[Żappka3DS] Sprawdzam token.\n");
					sprawdzajtokenasa(id_tokenk, refreshtoken);
				}

			} else {
				sprawdzajtokenasa(id_tokenk, refreshtoken);
				json_t *czroot = json_object();
				time_t serw = snrs_czas();
				json_object_set_new(czroot, "onlineczas", json_integer(serw));
				json_object_set_new(czroot, "localczas", json_integer(time(NULL)));
				json_dump_file(czroot, "/3ds/czas.json", JSON_COMPACT);
				json_decref(czroot);
			}
		}

	} else {
		isLogged = false;
		log_to_file("[Żappka3DS] Niezalogowany :p .\n");
	}
    char* url;
    char* body;

	Thread thread = threadCreate(qr_thread, NULL, 32 * 1024, 0x30, -2, false);
	if (!citra_machen){
		start_internet_thread();
	}
	consoleClear();
	bool time_broken = false;
	log_to_file("[Żappka3DS] Sprawdzam czy czas konsoli jest poprawny.\n");
	if (access("/3ds/data.json", F_OK) != 0) {
		time_broken = refresh_data("https://zabka-snrs.zabka.pl/v4/server/time", "", NULL);
	}
	if (time_broken) {
		log_to_file("[Żappka3DS] Czas jest zjebany, napraw plz.\n");
		const char* msg = "Zle ustawienia czasu!";
		const char* msg2 = "W Rosalina Menu zrob:";
		const char* msg3 = "Misc. Settings > Set Time via NTP";
		int screenWidth = topConsole.windowWidth;
		int screenHeight = topConsole.windowHeight;
		int x = (screenWidth - strlen(msg)) / 2;
		int x2 = (screenWidth - strlen(msg2)) / 2;
		int x3 = (screenWidth - strlen(msg3)) / 2;
		int y = screenHeight / 2;
		printf("\x1b[%d;%dH%s", y, x, msg);  // ANSI escape to move cursor to (y, x)
		printf("\x1b[%d;%dH%s", y+1, x2, msg2);  // ANSI escape to move cursor to (y, x)
		printf("\x1b[%d;%dH%s", y+2, x3, msg3);  // ANSI escape to move cursor to (y, x)
		//printf("Pobieranie z serwerów...");
		sleep(5);
	}
	json_t *czroot = NULL;
	json_error_t error;
	if (!czasfuckup) {
		if (access("/3ds/czas.json", F_OK) == 0) {
			log_to_file("[Żappka3DS] Aktualizuje czas.\n");
			czroot = json_load_file("/3ds/czas.json", 0, &error);
			if (!czroot) {
				//printf("Failed to load czas.json: %s\n", error.text);
				czroot = json_object();
			}
		} else {
			log_to_file("[Żappka3DS] Brak danych czasu, pobieram.\n");
			czroot = json_object();
		}

		if (!czroot) {
			printf("Failed to create or load JSON object.\n");
		}

		if (is_network_connected()) {
			log_to_file("[Żappka3DS] Jest Internet, pobieram czas z sieci.\n");
			time_t serw = snrs_czas();
			json_object_set_new(czroot, "onlineczas", json_integer(serw));
		}
		log_to_file("[Żappka3DS] Pobieram czas lokalny.\n");
		json_object_set_new(czroot, "localczas", json_integer(time(NULL)));

		if (json_dump_file(czroot, "/3ds/czas.json", JSON_COMPACT) != 0) {
			log_to_file("[Żappka3DS] Z jakiegoś powodu czas sie nie zapisał.\n");
			printf("Failed to write czas.json\n");
		}
	}

	json_decref(czroot);
	consoleClear();

    gfxExit();
    gfxInitDefault();


    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();
    C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    C3D_RenderTarget* bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

	static SwkbdState swkbd;
	static char mybuf[60];
	static char mybuf2[60];
	static SwkbdStatusData swkbdStatus;
	static SwkbdLearningData swkbdLearning;
	SwkbdButton button = SWKBD_BUTTON_NONE;
	bool didit = false;
	bool swkbdTriggered = false;
    Scene = 0;
	bool sceneStarted = false;
	bool rei = false;
	bool chiyoko = true;
	przycskmachen = true;
    g_staticBuf = C2D_TextBufNew(256);
	kupon_text_Buf = C2D_TextBufNew(512);
	themeBuf = C2D_TextBufNew(512);
    font[0] = C2D_FontLoad("romfs:/bold.bcfnt");
	strncpy(themes[0], "Domyślny", 99);
	themes[0][99] = '\0';
	int maxThemes;
	u32 themeoutColor;
	u32 themeBaseColor;
    time_t rawtime;
    struct tm *timeinfo;

    // Get current time in seconds since epoch
    rawtime = time(NULL);

    // Convert to local time
    timeinfo = localtime(&rawtime);
	if (timeinfo->tm_hour > 18 || timeinfo->tm_hour < 6) {
		day = false;
	} else {
		day = true;
	}
	if (access("/3ds/zappkathemes/current_theme.json", F_OK) == 0) {
		json_t *rootenmach = json_load_file("/3ds/zappkathemes/current_theme.json", 0, NULL);
		json_t *currentthemene = json_object_get(rootenmach, "currenttheme");
		currenttheme = json_integer_value(currentthemene);
		selectionthemelol = json_integer_value(currentthemene);
	} else {
		createDirectory("/3ds/zappkathemes/");
		FILE *stworzplkplz = fopen("/3ds/zappkathemes/current_theme.json", "w");
		fprintf(stworzplkplz, "{\"currenttheme\":0}");
		currenttheme = 0;
		selectionthemelol = 0;
		fclose(stworzplkplz);
	}
	if (currenttheme == 0){
		themeon = false;
	} else {
		themeon = true;
	}
	if (access("/3ds/zappkathemes/themelst.json", F_OK) == 0) {
		jsonfl = json_load_file("/3ds/zappkathemes/themelst.json", 0, NULL);
		json_t *themesy = json_object_get(jsonfl, "themes");
		size_t index;
		json_t *value;
		json_array_foreach(themesy, index, value) {
			if (json_is_string(value)) {
				strncpy(themes[index + 1], json_string_value(value), 99);
				themes[index + 1][99] = '\0';
				log_to_file("Theme %zu: %s\n", index, json_string_value(value));
				C2D_TextFontParse(&themeText[index + 1], font[0], themeBuf, themes[index + 1]);
				C2D_TextOptimize(&themeText[index + 1]);
			}
		}
		maxThemes = index;
		if (currenttheme != 0) {
			char path[128];
			snprintf(path, sizeof(path), "/3ds/zappkathemes/%s/colors.json", themes[currenttheme]);
			json_t *root = json_load_file(path, 0, NULL);
			json_t *color = json_object_get(root, "outlinecolor");
			json_t *color2 = json_object_get(root, "basecolor");

			uint8_t r = json_integer_value(json_object_get(color, "r"));
			uint8_t g = json_integer_value(json_object_get(color, "g"));
			uint8_t b = json_integer_value(json_object_get(color, "b"));
			uint8_t a = json_integer_value(json_object_get(color, "a"));
			uint8_t r2 = json_integer_value(json_object_get(color2, "r"));
			uint8_t g2 = json_integer_value(json_object_get(color2, "g"));
			uint8_t b2 = json_integer_value(json_object_get(color2, "b"));
			uint8_t a2 = json_integer_value(json_object_get(color2, "a"));
			themeoutColor = C2D_Color32(r, g, b, a);
			themeBaseColor = C2D_Color32(r2, g2, b2, a2);
		}
	}
	C2D_SpriteSheet scrollbarsheet;
	C2D_Image scrollbar;
	C2D_TextFontParse(&themeText[0], font[0], themeBuf, themes[0]);
    C2D_TextOptimize(&themeText[0]);
	C2D_SpriteSheet splash = C2D_SpriteSheetLoad("romfs:/gfx/splash.t3x");
	C2D_Image splash1 = C2D_SpriteSheetGetImage(splash, 0);
	C2D_Image splash2 = C2D_SpriteSheetGetImage(splash, 1);
    if (!themeon) {
		scrollbarsheet = C2D_SpriteSheetLoad("romfs:/gfx/usestylus.t3x");
		background_top = C2D_SpriteSheetLoad("romfs:/gfx/bg.t3x");
		background_down = C2D_SpriteSheetLoad("romfs:/gfx/bottombg.t3x");
		logo = C2D_SpriteSheetLoad("romfs:/gfx/logo.t3x");
		points = C2D_SpriteSheetLoad("romfs:/gfx/points.t3x");
		too_less = C2D_SpriteSheetLoad("romfs:/gfx/za_malo.t3x");
		scan = C2D_SpriteSheetLoad("romfs:/gfx/scan.t3x");
		coupons = C2D_SpriteSheetLoad("romfs:/gfx/coupons.t3x");
		settings = C2D_SpriteSheetLoad("romfs:/gfx/settings.t3x");
		qrframe = C2D_SpriteSheetLoad("romfs:/gfx/qrframe.t3x");
		stat = C2D_SpriteSheetLoad("romfs:/gfx/status_bar.t3x");
		zappbar = C2D_SpriteSheetLoad("romfs:/gfx/zappsy_bar.t3x");
		couponenbuttonen = C2D_SpriteSheetLoad("romfs:/gfx/coupon_button_machen.t3x");
		goback = C2D_SpriteSheetLoad("romfs:/gfx/gobackplz.t3x");
		act_buttons = C2D_SpriteSheetLoad("romfs:/gfx/act_buttons.t3x");
		deact_buttons = C2D_SpriteSheetLoad("romfs:/gfx/deact_buttons.t3x");
		logout_buttons = C2D_SpriteSheetLoad("romfs:/gfx/logout_buttons.t3x");
		more_b = C2D_SpriteSheetLoad("romfs:/gfx/more.t3x");
		themename_border = C2D_SpriteSheetLoad("romfs:/gfx/themename_border.t3x");
		zaba_frames = C2D_SpriteSheetLoad("romfs:/gfx/zaba_anim.t3x");
		
		scrollbar = C2D_SpriteSheetGetImage(scrollbarsheet, 0);
		bgtop = C2D_SpriteSheetGetImage(background_top, 0);
		bgdown = C2D_SpriteSheetGetImage(background_down, 0);
		logo3ds = C2D_SpriteSheetGetImage(logo, 0);
		scan_button = C2D_SpriteSheetGetImage(scan, 0);
		scan_pressed = C2D_SpriteSheetGetImage(scan, 1);
		coupons_button = C2D_SpriteSheetGetImage(coupons, 0);
		coupons_pressed = C2D_SpriteSheetGetImage(coupons, 1);
		za_malo = C2D_SpriteSheetGetImage(too_less, 0);
		points_button = C2D_SpriteSheetGetImage(points, 0);
		points_pressed = C2D_SpriteSheetGetImage(points, 1);
		settings_button = C2D_SpriteSheetGetImage(settings, 0);
		settings_pressed = C2D_SpriteSheetGetImage(settings, 1);
		qr_framen_machen = C2D_SpriteSheetGetImage(qrframe, 0);
		statusbaren = C2D_SpriteSheetGetImage(stat, 0);
		zappsybaren = C2D_SpriteSheetGetImage(zappbar, 0);
		couponbutton = C2D_SpriteSheetGetImage(couponenbuttonen, 0);
		couponbutton_pressed = C2D_SpriteSheetGetImage(couponenbuttonen, 1);
		gobackplz = C2D_SpriteSheetGetImage(goback, 0);
		act_button = C2D_SpriteSheetGetImage(act_buttons, 0);
		act_pressed = C2D_SpriteSheetGetImage(act_buttons, 1);
		deact_button = C2D_SpriteSheetGetImage(deact_buttons, 0);
		deact_pressed = C2D_SpriteSheetGetImage(deact_buttons, 1);
		logout_button = C2D_SpriteSheetGetImage(logout_buttons, 0);
		logout_pressed = C2D_SpriteSheetGetImage(logout_buttons, 1);
		more_button = C2D_SpriteSheetGetImage(more_b, 0);
		themeborder = C2D_SpriteSheetGetImage(themename_border, 0);
		ch_theme = C2D_SpriteSheetGetImage(themename_border, 1);
	} else {
		char path[128];
		snprintf(path, sizeof(path), "%s", themes[currenttheme]);

		char fullpath[256];

		#define LOAD_SPRITE(var, filename) \
			snprintf(fullpath, sizeof(fullpath), "/3ds/zappkathemes/%s/%s", path, filename); \
			var = C2D_SpriteSheetLoad(fullpath);

		LOAD_SPRITE(scrollbarsheet, "usestylus.t3x");
		LOAD_SPRITE(background_top, "bg.t3x");
		LOAD_SPRITE(background_down, "bottombg.t3x");
		LOAD_SPRITE(logo, "logo.t3x");
		LOAD_SPRITE(too_less, "za_malo.t3x");
		LOAD_SPRITE(points, "points.t3x");
		LOAD_SPRITE(scan, "scan.t3x");
		LOAD_SPRITE(coupons, "coupons.t3x");
		LOAD_SPRITE(settings, "settings.t3x");
		LOAD_SPRITE(qrframe, "qrframe.t3x");
		LOAD_SPRITE(stat, "status_bar.t3x");
		LOAD_SPRITE(zappbar, "zappsy_bar.t3x");
		LOAD_SPRITE(couponenbuttonen, "coupon_button_machen.t3x");
		LOAD_SPRITE(goback, "gobackplz.t3x");
		LOAD_SPRITE(act_buttons, "act_buttons.t3x");
		LOAD_SPRITE(deact_buttons, "deact_buttons.t3x");
		LOAD_SPRITE(logout_buttons, "logout_buttons.t3x");
		LOAD_SPRITE(more_b, "more.t3x");
		LOAD_SPRITE(themename_border, "themename_border.t3x");
		LOAD_SPRITE(zaba_frames, "zaba_anim.t3x");

		scrollbar = C2D_SpriteSheetGetImage(scrollbarsheet, 0);
		bgtop = C2D_SpriteSheetGetImage(background_top, 0);
		bgdown = C2D_SpriteSheetGetImage(background_down, 0);
		logo3ds = C2D_SpriteSheetGetImage(logo, 0);
		scan_button = C2D_SpriteSheetGetImage(scan, 0);
		scan_pressed = C2D_SpriteSheetGetImage(scan, 1);
		coupons_button = C2D_SpriteSheetGetImage(coupons, 0);
		coupons_pressed = C2D_SpriteSheetGetImage(coupons, 1);
		za_malo = C2D_SpriteSheetGetImage(too_less, 0);
		points_button = C2D_SpriteSheetGetImage(points, 0);
		points_pressed = C2D_SpriteSheetGetImage(points, 1);
		settings_button = C2D_SpriteSheetGetImage(settings, 0);
		settings_pressed = C2D_SpriteSheetGetImage(settings, 1);
		qr_framen_machen = C2D_SpriteSheetGetImage(qrframe, 0);
		statusbaren = C2D_SpriteSheetGetImage(stat, 0);
		zappsybaren = C2D_SpriteSheetGetImage(zappbar, 0);
		couponbutton = C2D_SpriteSheetGetImage(couponenbuttonen, 0);
		couponbutton_pressed = C2D_SpriteSheetGetImage(couponenbuttonen, 1);
		gobackplz = C2D_SpriteSheetGetImage(goback, 0);
		act_button = C2D_SpriteSheetGetImage(act_buttons, 0);
		act_pressed = C2D_SpriteSheetGetImage(act_buttons, 1);
		deact_button = C2D_SpriteSheetGetImage(deact_buttons, 0);
		deact_pressed = C2D_SpriteSheetGetImage(deact_buttons, 1);
		logout_button = C2D_SpriteSheetGetImage(logout_buttons, 0);
		logout_pressed = C2D_SpriteSheetGetImage(logout_buttons, 1);
		more_button = C2D_SpriteSheetGetImage(more_b, 0);
		themeborder = C2D_SpriteSheetGetImage(themename_border, 0);
		ch_theme = C2D_SpriteSheetGetImage(themename_border, 1);

	}

    buttonsy[0] = (Button){0, 10, 138, 105, coupons_button, coupons_pressed, false, 5, 10, 12, 14, 16, 0.75f, kupony};
	buttonsy[1] = (Button){0, 130, 138, 105, points_button, points_pressed, false, 5, 10, 12, 14, 16, 0.75f, zappsy};
	buttonsy[2] = (Button){0, 10, 193, 172, scan_button, scan_pressed, false, 5, 10, 12, 14, 16, 0.75f, kodQR};
	buttonsy[3] = (Button){0, 185, 193, 51, settings_button, settings_pressed, false, 5, 10, 12, 14, 16, 0.75f, opcje};
	buttonsy[97] = (Button){0, 185, 193, 51, logout_button, logout_pressed, false, 16, 17, 23, 23, 23, 0.75f, wyloguj};
	populateCwavList();
    isScrolling = false;
    C2D_TextFontParse(&g_staticText[0], font[0], g_staticBuf, "Wciśnij A.");
    C2D_TextOptimize(&g_staticText[0]);
	C2D_TextFontParse(&g_staticText[1], font[0], g_staticBuf, "Ładowanie...");
	C2D_TextOptimize(&g_staticText[1]);
	C2D_TextFontParse(&g_staticText[2], font[0], g_staticBuf, "Nie wykryto danych konta Żappka.\nWciśnij A by kontynuować");
	C2D_TextOptimize(&g_staticText[2]);
	C2D_TextFontParse(&g_staticText[3], font[0], g_staticBuf, "Tak");
	C2D_TextFontParse(&g_staticText[4], font[0], g_staticBuf, "Nie");
	C2D_TextFontParse(&g_staticText[5], font[0], g_staticBuf, "Wprowadź numer telefonu.");
	C2D_TextOptimize(&g_staticText[5]);
	C2D_TextFontParse(&g_staticText[6], font[0], g_staticBuf, "Wprowadź kod SMS.");
	C2D_TextOptimize(&g_staticText[6]);
	C2D_TextFontParse(&g_staticText[7], font[0], g_staticBuf, combinedText);
    C2D_TextFontParse(&g_staticText[8], font[0], g_staticBuf, "Twoje Żappsy");
    C2D_TextOptimize(&g_staticText[8]);
    C2D_TextFontParse(&g_staticText[10], font[0], g_staticBuf, "B - Powrót");
    C2D_TextOptimize(&g_staticText[10]);
    C2D_TextFontParse(&g_staticText[11], font[0], g_staticBuf, "Brak internetu :(");
    C2D_TextOptimize(&g_staticText[11]);
	C2D_TextFontParse(&g_staticText[12], font[0], g_staticBuf, "Kupony");
    C2D_TextOptimize(&g_staticText[12]);
	C2D_TextFontParse(&g_staticText[13], font[0], g_staticBuf, "Gotowe :)");
    C2D_TextOptimize(&g_staticText[13]);
	C2D_TextFontParse(&g_staticText[14], font[0], g_staticBuf, "Opcje");
    C2D_TextOptimize(&g_staticText[14]);
	C2D_TextFontParse(&g_staticText[15], font[0], g_staticBuf, "Motyw:");
    C2D_TextOptimize(&g_staticText[15]);
	C2D_TextFontParse(&g_staticText[16], font[0], g_staticBuf, "(Zrestartuj aplikacje by zapisać zmiany)");
    C2D_TextOptimize(&g_staticText[16]);
	C2D_TextFontParse(&g_staticText[17], font[0], g_staticBuf, "VA");
    C2D_TextOptimize(&g_staticText[17]);
	CWAV* bgm = cwavList[1].cwav;
	CWAV* loginbgm = cwavList[2].cwav;
	CWAV* menu = day ? cwavList[3].cwav : cwavList[5].cwav;
	CWAV* splashb = cwavList[4].cwav;

	if (!day) {
		menu->volume = 0.4f;
	} else {
		menu->volume = 0.45f;
	}

	int textOffset = 0;
    int lastTouchY = -1;
	float scrollVelocity = 0.0f;
	float friction = 0.9f;
	int prevTouchX = -1;
	bool isDragging = false;
	int dragStartX = -1;
	int dragStartY = -1;
	const int SCROLL_THRESHOLD = 10;
	bool splashPlayed = false;
	u64 splashStartTime = 0;
	bool splashDone = false;
	float splashTimer = 0.0f;
	float splashY = 240.0f;       // Start below screen
	float splashHopTime = 0.0f;
	const float splashHopDuration = 0.8f; // Total hop time
	int transpar = 255;
	int transpar2 = 0;
	logplz = false;
    while(aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
		u32 kHeld = hidKeysHeld();
        if (kDown & KEY_START) {
            break;
        }
		if (czasfuckup) {
			break;
		}
        touchPosition touch;
        hidTouchRead(&touch);
		sfx = cwavList[0].cwav;
		if (przycskmachen) {
			if (kHeld & KEY_TOUCH) {
				if (dragStartX == -1 || dragStartY == -1) {
					dragStartX = touch.px;
					dragStartY = touch.py;
					isDragging = false;
					scrollVelocity = 0.0f;
				} else {
					int dx = abs(touch.px - dragStartX);
					int dy = abs(touch.py - dragStartY);
					if (dx > SCROLL_THRESHOLD || dy > SCROLL_THRESHOLD) {
						isDragging = true;
						for (int i = 0; i < 100; i++) {
							buttonsy[i].isPressed = false;
						}
					}
				}

				if (isDragging) {
					int currentTouch = readingoferta ? touch.py : touch.px;
					if (lastTouchY >= 0) {
						int delta = currentTouch - lastTouchY;
						textOffset -= delta;
						if (textOffset < 0) textOffset = 0;
						if (textOffset > max_scroll) textOffset = max_scroll;

						scrollVelocity = (float)delta;
					}
					lastTouchY = currentTouch;
					prevTouchX = touch.px;
				} else {
					for (int i = 0; i < 100; i++) {
						if (buttonsy[i].scene != Scene && buttonsy[i].scene2 != Scene) continue;
						if (isButtonPressed(&buttonsy[i], touch, Scene)) {
							buttonsy[i].isPressed = true;
							break;
						}
					}
				}
			} else {
				if (!isDragging) {
					for (int i = 0; i < 100; i++) {
						if (buttonsy[i].scene != Scene && buttonsy[i].scene2 != Scene) continue;
						if (buttonsy[i].isPressed) {
							buttonsy[i].isPressed = false;
							selectioncodelol = i;
							executeButtonFunction(i);
							break;
						}
					}
				}
				lastTouchY = -7;
				dragStartX = -1;
				dragStartY = -1;
				isDragging = false;
				prevTouchX = -1;
			}
		}
		if (!(kHeld & KEY_TOUCH) && fabs(scrollVelocity) > 0.1f) {
			textOffset -= scrollVelocity;
			if (textOffset < 0) textOffset = 0;
			if (textOffset > max_scroll) textOffset = max_scroll;

			scrollVelocity *= friction;
			if (fabs(scrollVelocity) < 0.1f) {
				scrollVelocity = 0.0f;
			}
		}

		if (kDown & KEY_L) {
			if (Scene == 0){
				cpu_debug = true;
				logplz = true;
			}
		}
		if (kDown & KEY_R) {
			VOICEACT = !VOICEACT;
		}
        if (kDown & KEY_A) {
            if (Scene == 1 & timer > 40) {

				cwavStop(bgm, 0, 1);
				if (sfx->numChannels == 2) {
					cwavPlay(sfx, 0, 1);
				} else {
					cwavPlay(sfx, 0, -1);
				}
                startY = 0.0f;
				timer = 0.0f;
                endY = -400.0f;
                elapsed = 0;
				if (!isLogged) {
					Scene = 2;
				} else {
					Scene = 5;
				}
            } else if (Scene == 2 & timer > 119) {

				cwavStop(bgm, 0, 1);
                startY = 0.0f;
				timer = 0.0f;
                endY = -400.0f;
                elapsed = 0;
                Scene = 3;
				if (sfx->numChannels == 2) {
					cwavPlay(sfx, 0, 1);
				} else {
					cwavPlay(sfx, 0, -1);
				}

				cwavPlay(loginbgm, 0, 1);
            }
        }
        if (kDown & KEY_B) {
			if (Scene == 10 & timer > 40) {
				przycskmachen = true;

				if (sfx->numChannels == 2) {
					cwavPlay(sfx, 0, 1);
				} else {
					cwavPlay(sfx, 0, -1);
				}
				startY = 0.0f;
				timer = 0.0f;
				endY = -400.0f;
				elapsed = 0;
				Scene = 11;
			}
            if (Scene == 12 & timer > 40) {
				przycskmachen = true;

				if (sfx->numChannels == 2) {
					cwavPlay(sfx, 0, 1);
				} else {
					cwavPlay(sfx, 0, -1);
				}
                startY = 0.0f;
				timer = 0.0f;
                endY = -400.0f;
                elapsed = 0;
				Scene = 13;
            }
            if (Scene == 14 & timer > 40) {
				przycskmachen = true;

				if (sfx->numChannels == 2) {
					cwavPlay(sfx, 0, 1);
				} else {
					cwavPlay(sfx, 0, -1);
				}
                startY = 0.0f;
				timer = 0.0f;
                endY = -400.0f;
                elapsed = 0;
				Scene = 15;
				readingoferta = false;
            }
            if (Scene == 16 & timer > 40) {
				przycskmachen = true;

				if (sfx->numChannels == 2) {
					cwavPlay(sfx, 0, 1);
				} else {
					cwavPlay(sfx, 0, -1);
				}
                startY = 0.0f;
				timer = 0.0f;
                endY = -400.0f;
                elapsed = 0;
				Scene = 17;
				json_t *rootenmach = json_load_file("/3ds/zappkathemes/current_theme.json", 0, NULL);
				json_object_set_new(rootenmach, "currenttheme", json_integer(selectionthemelol));
				json_dump_file(rootenmach, "/3ds/zappkathemes/current_theme.json", JSON_INDENT(4));
            }
        }
		if (kDown & KEY_DLEFT) {
			if (Scene == 16 & timer > 40) {
				if (selectionthemelol == 0){
					selectionthemelol = maxThemes;
				} else {
					selectionthemelol--;

				}
			}
		}
		if (kDown & KEY_DRIGHT) {
			if (Scene == 16 & timer > 40) {
				if (selectionthemelol == maxThemes){
					selectionthemelol = 0;
				} else {
					selectionthemelol++;
				}
			}
		}


        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		if (Scene == 0) {
			if (!splashDone) {
				float dt = 1.0f / 60.0f;
				splashTimer += dt;

				if (!splashPlayed) {
					cwavPlay(splashb, 0, 1);
					splashPlayed = true;
				}

				// Only increment once here
				if (splashHopTime < splashHopDuration) {
					splashHopTime += dt;
					splashY = easeHop(splashHopTime, 100.0f, 0.0f, splashHopDuration);
				} else {
					splashY = 0.0f;
				}

				if (splashTimer < 4.6f) {
					C2D_TargetClear(top, C2D_Color32(0, 0, 0, 255));
					C2D_SceneBegin(top);
					C2D_DrawImageAt(splash1, 0.0f, splashY, 0.0f, NULL, 1.0f, 1.0f);
					if (splashTimer > 1.2f) {
						C2D_DrawImageAt(splash2, 0.0f, splashY, 0.0f, NULL, 1.0f, 1.0f);
					}
					if (splashTimer > 1.2f && transpar != 0) {
						transpar -= 5;
						C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0xff, 0xff, 0xff, transpar));
					}
					if (splashTimer > 3.5f) {
						if (transpar2 != 255){
							transpar2 += 5;
						}
						C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, transpar2));
					}
					if (schizofrenia) {
						if (!themeon) {
							drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x29, 0x6e, 0x44, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
						} else {
							drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x74, 0x1d, 0x4a, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
						}
					}
					C2D_TargetClear(bottom, C2D_Color32(0, 0, 0, 255));
					C2D_SceneBegin(bottom);
				} else {
					splashDone = true;
				}
			} else {
				if (bgm->numChannels == 2) {
					cwavPlay(bgm, 0, 1);
				} else {
					cwavPlay(bgm, 0, -1);

				}
				Scene = 1;
			}
		} else if (Scene == 1) {
            timer += 0.2f ;
            if (timer > 20.0f) {
                isScrolling = true;
            }

            C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(top);
            if (y > -40.0f) {
                x -= 0.5f;
                y -= 0.5f;
            } else {
                x = 0.0f;
                y = 0.0f;
            }

            if (isScrolling) {
                if (elapsed < duration) {
                    currentY = easeOutQuad(elapsed, startY, endY, duration);
                    elapsed += deltaTime;
                }
            }

            C2D_DrawImageAt(bgtop, x, y, 0.0f, NULL, 1.0f, 1.0f);
            C2D_DrawImageAt(logo3ds, 0.0f, currentY, 0.0f, NULL, 1.0f, 1.0f);
			if (VOICEACT) {
				playSoundOncePerScene(1, 17);
			}
			
			if (schizofrenia) {
				if (!themeon) {
					drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x29, 0x6e, 0x44, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				} else {
					drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x74, 0x1d, 0x4a, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				}
			}
			zaba(0.25f);
            C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(bottom);

            C2D_DrawImageAt(bgdown, x, y, 0.0f, NULL, 1.0f, 1.0f);
			for (int i = 0; i < 100; i++) {
				drawButton(&buttonsy[i], Scene);
			}
			if (!themeon) {
				drawShadowedText(&g_staticText[0], 160.0f, -currentY + 100.0f, 0.5f, 1.5f, 1.5f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
			} else {
				drawShadowedText(&g_staticText[0], 160.0f, -currentY + 100.0f, 0.5f, 1.5f, 1.5f, themeBaseColor, themeoutColor);
			}
        } else if (Scene == 2) {
			if (timer2 > 0.0f) {
                timer2 -= 7.0f;
            }
			if (timer < 70) {
				timer += 1.0f;
			} else if (timer < 100) {
				timer += 1.0f;
				timer3 += 4.0f;

			} else if (timer < 120) {
				timer += 1.0f;
                startY = -400.0f;
                endY = 0.0f;
                elapsed = 0;
			}


            C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(top);
            if (y > -40.0f) {
                x -= 0.5f;
                y -= 0.5f;
            } else {
                x = 0.0f;
                y = 0.0f;
            }

            if (isScrolling) {
                if (elapsed < duration) {
					if (timer < 100) {
						currentY = easeInQuad(elapsed, startY, endY, duration);
					}
					currentY2 = easeOutQuad(elapsed, startY, endY, duration);
                    elapsed += deltaTime;
                }
            }

            C2D_DrawImageAt(bgtop, x, y, 0.0f, NULL, 1.0f, 1.0f);
            C2D_DrawImageAt(logo3ds, 0.0f, currentY, 0.0f, NULL, 1.0f, 1.0f);
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0xff, 0xff, 0xff, timer2));
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));
			if (VOICEACT) {
				playSoundOncePerScene(2, 6);
			}
			if (timer > 80) {
				if (!themeon){
					drawShadowedText(&g_staticText[2], 200.0f, currentY2 + 90.0f, 0.5f, 1.0f, 1.0f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				} else {
					drawShadowedText(&g_staticText[2], 200.0f, currentY2 + 90.0f, 0.5f, 1.0f, 1.0f, themeBaseColor, themeoutColor);
				}
			}
			if (schizofrenia) {
				if (!themeon) {
					drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x29, 0x6e, 0x44, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				} else {
					drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x74, 0x1d, 0x4a, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				}
			}
			zaba(0.25f);
            C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(bottom);

            C2D_DrawImageAt(bgdown, x, y, 0.0f, NULL, 1.0f, 1.0f);
			for (int i = 0; i < 100; i++) {
				drawButton(&buttonsy[i], Scene);
			}
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0xff, 0xff, 0xff, timer2));
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));

		} else if (Scene == 3) {
			if (timer < 70) {
				timer += 1.0f;
			} else if (timer < 120) {
				timer += 1.0f;
                startY = -400.0f;
				elapsed = 0.0f;
                endY = 0.0f;
			} else if (timer < 209) {
				timer += 1.0f;
			} else if (timer > 208 && !swkbdTriggered) {
				swkbdTriggered = true;
				didit = true;
				swkbdInit(&swkbd, SWKBD_TYPE_NUMPAD, 1, 9);
				swkbdSetPasswordMode(&swkbd, SWKBD_PASSWORD_HIDE_DELAY);
				swkbdSetValidation(&swkbd, SWKBD_ANYTHING, 0, 0);
				swkbdSetFeatures(&swkbd, SWKBD_FIXED_WIDTH);
				button = swkbdInputText(&swkbd, mybuf, sizeof(mybuf));
				numertelefonen = mybuf;
				synerise_tokenauth();
				send_ver_code_preauth();
				send_verification_code(mybuf, id_tokenk);
				timer = 0.0f;
				startY = 0.0f;
				elapsed = 0.0f;
                endY = -400.0f;
				swkbdTriggered = false;
				Scene = 4;
			}

            C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(top);
            if (y > -40.0f) {
                x -= 0.5f;
                y -= 0.5f;
            } else {
                x = 0.0f;
                y = 0.0f;
            }

            if (isScrolling) {
                if (elapsed < duration) {
					if (timer > 80) {
						currentY = easeOutQuad(elapsed, startY, endY, duration);
					}
					if (timer < 80) {
						currentY2 = easeInQuad(elapsed, startY, endY, duration);
					}
                    elapsed += deltaTime;
                }
            }

            C2D_DrawImageAt(bgtop, x, y, 0.0f, NULL, 1.0f, 1.0f);
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));
			if (timer > 80) {
				if (VOICEACT) {
					playSoundOncePerScene(3, 16);
				}
				if (!themeon){
					drawShadowedText(&g_staticText[5], 200.0f, currentY + 90.0f, 0.5f, 1.0f, 1.0f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				} else {
					drawShadowedText(&g_staticText[5], 200.0f, currentY + 90.0f, 0.5f, 1.0f, 1.0f, themeBaseColor, themeoutColor);
				}
			} else {
				if (!themeon){
					drawShadowedText(&g_staticText[2], 200.0f, currentY2 + 90.0f, 0.5f, 1.0f, 1.0f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				} else {
					drawShadowedText(&g_staticText[2], 200.0f, currentY2 + 90.0f, 0.5f, 1.0f, 1.0f, themeBaseColor, themeoutColor);
				}
			}
			if (schizofrenia) {
				if (!themeon) {
					drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x29, 0x6e, 0x44, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				} else {
					drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x74, 0x1d, 0x4a, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				}
			}
			
			zaba(0.25f);
            C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(bottom);

            C2D_DrawImageAt(bgdown, x, y, 0.0f, NULL, 1.0f, 1.0f);
			for (int i = 0; i < 100; i++) {
				drawButton(&buttonsy[i], Scene);
			}

			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));


		} else if (Scene == 4) {
			if (timer < 70) {
				timer += 1.0f;
			} else if (timer < 120) {
				timer += 1.0f;
                startY = -400.0f;
				elapsed = 0.0f;
                endY = 0.0f;
			} else if (timer < 209) {
				timer += 1.0f;
			} else if (timer > 208 && !swkbdTriggered) {
				swkbdTriggered = true;
				didit = true;
				swkbdInit(&swkbd, SWKBD_TYPE_NUMPAD, 1, 6);
				swkbdSetPasswordMode(&swkbd, SWKBD_PASSWORD_HIDE_DELAY);
				swkbdSetValidation(&swkbd, SWKBD_ANYTHING, 0, 0);
				swkbdSetFeatures(&swkbd, SWKBD_FIXED_WIDTH);
				button = swkbdInputText(&swkbd, mybuf2, sizeof(mybuf2));
				login_flow(numertelefonen, mybuf2);
				timer = 0.0f;
				startY = 0.0f;
				elapsed = 0.0f;
                endY = -400.0f;
				swkbdTriggered = false;
				Scene = 6;
			}

            C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(top);
            if (y > -40.0f) {
                x -= 0.5f;
                y -= 0.5f;
            } else {
                x = 0.0f;
                y = 0.0f;
            }

            if (isScrolling) {
                if (elapsed < duration) {
					if (timer > 80) {
						currentY = easeOutQuad(elapsed, startY, endY, duration);
					}
					if (timer < 80) {
						currentY2 = easeInQuad(elapsed, startY, endY, duration);
					}
                    elapsed += deltaTime;
                }
            }

            C2D_DrawImageAt(bgtop, x, y, 0.0f, NULL, 1.0f, 1.0f);
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));
			if (timer > 80) {
				if (VOICEACT) {
					playSoundOncePerScene(4, 14);
				}
				if (!themeon){
					drawShadowedText(&g_staticText[6], 200.0f, currentY + 90.0f, 0.5f, 1.0f, 1.0f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				} else {
					drawShadowedText(&g_staticText[6], 200.0f, currentY + 90.0f, 0.5f, 1.0f, 1.0f, themeBaseColor, themeoutColor);
				}
			} else {
				if (!themeon){
					drawShadowedText(&g_staticText[5], 200.0f, currentY2 + 90.0f, 0.5f, 1.0f, 1.0f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				} else {
					drawShadowedText(&g_staticText[5], 200.0f, currentY2 + 90.0f, 0.5f, 1.0f, 1.0f, themeBaseColor, themeoutColor);
				}
			}
			if (schizofrenia) {
				if (!themeon) {
					drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x29, 0x6e, 0x44, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				} else {
					drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x74, 0x1d, 0x4a, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				}
			}
			zaba(0.25f);
            C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(bottom);

            C2D_DrawImageAt(bgdown, x, y, 0.0f, NULL, 1.0f, 1.0f);
			for (int i = 0; i < 100; i++) {
				drawButton(&buttonsy[i], Scene);
			}

			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));

		} else if (Scene == 5) {
			if (!sceneStarted) {
				cwavStop(loginbgm, 0, 1);
				cwavPlay(menu, 0, 1);
				sceneStarted = true;
				isLogged = true;
			}

			if (timer2 > 0.0f) {
                timer2 -= 7.0f;
            }
			if (timer < 70) {
				timer += 1.0f;
			} else if (timer < 120) {
				timer += 1.0f;
				timer3 -= 2.0f;
                startY = -400.0f;
				elapsed = 0.0f;
                endY = 0.0f;
				// if (!schizofrenia) {
					// generatingQR = false;
				// } else {
					// Scene = 10;
					// generatingQR = true;
				// }
			} else if (timer < 209) {
				timer += 1.0f;
			} else if (timer > 208) {
				// if (schizofrenia) {
					// timer = 0.0f;
					// generatingQR = true;
					// startY = 0.0f;
					// elapsed = 0.0f;
					// endY = -400.0f;
					// Scene = 10;
				// }
			}


            C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(top);
            if (y > -40.0f) {
                x -= 0.5f;
                y -= 0.5f;
            } else {
                x = 0.0f;
                y = 0.0f;
            }

            if (isScrolling) {
                if (elapsed < duration) {
					if (timer < 120) {
						currentY = easeInQuad(elapsed, startY, endY, duration);
						buttonsy[0].x = -400;
						buttonsy[1].x = -400;
						buttonsy[2].x = 400;
						buttonsy[3].x = 400;

					}
					if (timer > 120) {
						currentY = easeOutQuad(elapsed, startY, endY, duration);
						buttonsy[0].x = currentY;
						buttonsy[1].x = currentY;
						buttonsy[2].x = -currentY + 140;
						buttonsy[3].x = -currentY + 140;
					}
                    elapsed += deltaTime;
                }
            }

            C2D_DrawImageAt(bgtop, x, y, 0.0f, NULL, 1.0f, 1.0f);
            C2D_DrawImageAt(logo3ds, 0.0f, currentY, 0.0f, NULL, 1.0f, 1.0f);
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0xff, 0xff, 0xff, timer2));
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));
			if (timer > 90) {
				C2D_DrawImageAt(statusbaren, 0.0f, -currentY, 0.0f, NULL, 1.0f, 1.0f);
				if (!themeon) {
					if (!loadingshit) {
						drawShadowedText_noncentered(&g_staticText[7], 180.0f, -currentY + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
					} else {
						drawShadowedText_noncentered(&g_staticText[1], 180.0f, -currentY + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
					}
				} else {
					if (!loadingshit) {
						drawShadowedText_noncentered(&g_staticText[7], 180.0f, -currentY + 215.0f, 0.5f, 0.8f, 0.8f, themeBaseColor, themeoutColor);
					} else {
						drawShadowedText_noncentered(&g_staticText[1], 180.0f, -currentY + 215.0f, 0.5f, 0.8f, 0.8f, themeBaseColor, themeoutColor);
					}
				}
			}
			if (schizofrenia) {
				if (!themeon) {
					drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x29, 0x6e, 0x44, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				} else {
					drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x74, 0x1d, 0x4a, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				}
			}
			zaba(0.25f);
            C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(bottom);

            C2D_DrawImageAt(bgdown, x, y, 0.0f, NULL, 1.0f, 1.0f);
			for (int i = 0; i < 100; i++) {
				drawButton(&buttonsy[i], Scene);
			}
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0xff, 0xff, 0xff, timer2));
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));

		} else if (Scene == 6) {
			if (timer < 70) {
				timer += 1.0f;
			} else if (timer < 120) {
                startY = -400.0f;
				elapsed = 0.0f;
                endY = 0.0f;
				if (!youfuckedup) {
					Scene = 5;
				} else {
					Scene = 3;
				}
			} else if (timer < 209) {
				timer += 1.0f;
			}

            C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(top);
            if (y > -40.0f) {
                x -= 0.5f;
                y -= 0.5f;
            } else {
                x = 0.0f;
                y = 0.0f;
            }

            if (isScrolling) {
                if (elapsed < duration) {
					if (timer > 80) {
						currentY = easeOutQuad(elapsed, startY, endY, duration);

					}
					if (timer < 80) {
						currentY2 = easeInQuad(elapsed, startY, endY, duration);
					}
                    elapsed += deltaTime;
                }
            }

            C2D_DrawImageAt(bgtop, x, y, 0.0f, NULL, 1.0f, 1.0f);
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));
			if (timer > 80) {
				Scene = 7;
			} else {
				if (!themeon){
					drawShadowedText(&g_staticText[6], 200.0f, currentY2 + 90.0f, 0.5f, 1.0f, 1.0f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				} else {
					drawShadowedText(&g_staticText[6], 200.0f, currentY2 + 90.0f, 0.5f, 1.0f, 1.0f, themeBaseColor, themeoutColor);
				}
			}
			if (schizofrenia) {
				if (!themeon) {
					drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x29, 0x6e, 0x44, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				} else {
					drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x74, 0x1d, 0x4a, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				}
			}
			zaba(0.25f);
            C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(bottom);

            C2D_DrawImageAt(bgdown, x, y, 0.0f, NULL, 1.0f, 1.0f);
			for (int i = 0; i < 100; i++) {
				drawButton(&buttonsy[i], Scene);
			}

			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));

		} else if (Scene == 7) {
            C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(top);
            if (y > -40.0f) {
                x -= 0.5f;
                y -= 0.5f;
				if (timer3 >= 0) {
					timer3 -= 4.0f;
				}
            } else {
                x = 0.0f;
                y = 0.0f;
            }

            if (isScrolling) {
                if (elapsed < duration) {
					if (timer > 80) {
						currentY = easeOutQuad(elapsed, startY, endY, duration);
					}
					if (timer < 80) {
						currentY2 = easeInQuad(elapsed, startY, endY, duration);
					}
                    elapsed += deltaTime;
                }
            }

            C2D_DrawImageAt(bgtop, x, y, 0.0f, NULL, 1.0f, 1.0f);
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));
			if (schizofrenia) {
				if (!themeon) {
					drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x29, 0x6e, 0x44, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				} else {
					drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x74, 0x1d, 0x4a, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				}
			}
			zaba(0.25f);
            C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(bottom);

            C2D_DrawImageAt(bgdown, x, y, 0.0f, NULL, 1.0f, 1.0f);
			for (int i = 0; i < 100; i++) {
				drawButton(&buttonsy[i], Scene);
			}
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));

		} else if (Scene == 8) {
            C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(top);
            if (y > -40.0f) {
                x -= 0.5f;
                y -= 0.5f;
				if (timer3 >= 0) {
					timer3 -= 4.0f;
				}
            } else {
                x = 0.0f;
                y = 0.0f;
            }

            if (isScrolling) {
                if (elapsed < duration) {
					if (timer > 80) {
						currentY = easeOutQuad(elapsed, startY, endY, duration);
					}
					if (timer < 80) {
						currentY2 = easeInQuad(elapsed, startY, endY, duration);
					}
                    elapsed += deltaTime;
                }
            }

            C2D_DrawImageAt(bgtop, x, y, 0.0f, NULL, 1.0f, 1.0f);
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));
			if (schizofrenia) {
				if (!themeon) {
					drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x29, 0x6e, 0x44, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				} else {
					drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x74, 0x1d, 0x4a, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				}
			}
			zaba(0.25f);
            C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(bottom);

            C2D_DrawImageAt(bgdown, x, y, 0.0f, NULL, 1.0f, 1.0f);
			for (int i = 0; i < 100; i++) {
				drawButton(&buttonsy[i], Scene);
			}
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));

		} else if (Scene == 10) {
			if (timer2 > 0.0f) {
                timer2 -= 7.0f;
            }
			if (timer < 70) {
				timer += 1.0f;
				generatingQR = true;
			} else if (timer < 120) {
				timer += 1.0f;
				timer3 -= 2.0f;
                startY = -400.0f;
				elapsed = 0.0f;
                endY = 0.0f;

			} else if (timer < 209) {
				timer += 1.0f;

			}


            C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(top);
            if (y > -40.0f) {
                x -= 0.5f;
                y -= 0.5f;
            } else {
                x = 0.0f;
                y = 0.0f;
            }

            if (isScrolling) {
                if (elapsed < duration) {
					if (timer < 120) {
						currentY = easeInQuad(elapsed, startY, endY, 6.5f);
						buttonsy[0].x = currentY;
						buttonsy[1].x = currentY;
						buttonsy[2].x = -currentY + 140;
						buttonsy[3].x = -currentY + 140;

					}
					if (timer > 120) {
						currentY = easeOutQuad(elapsed, startY, endY, 7.0f);
					}
                    elapsed += deltaTime;
                }
            }

            C2D_DrawImageAt(bgtop, x, y, 0.0f, NULL, 1.0f, 1.0f);
			if (timer < 120) {
				if (VOICEACT) {
					playSoundOncePerScene(10, 12);
				}
				C2D_DrawImageAt(logo3ds, 0.0f, currentY, 0.0f, NULL, 1.0f, 1.0f);
				C2D_DrawImageAt(statusbaren, 0.0f, -currentY, 0.0f, NULL, 1.0f, 1.0f);
				if (!themeon) {
					drawShadowedText_noncentered(&g_staticText[7], 180.0f, -currentY + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				} else {
					drawShadowedText_noncentered(&g_staticText[7], 180.0f, -currentY + 215.0f, 0.5f, 0.8f, 0.8f, themeBaseColor, themeoutColor);
				}
			} else if (timer > 120) {
				C2D_DrawImageAt(qr_framen_machen, 0.0f, currentY, 0.0f, NULL, 1.0f, 1.0f);
				if (qrImage.tex && qrImage.subtex) {
					C2D_DrawImageAt(qrImage, 112.0f, currentY + 27, 0.0f, NULL, 1.0f, 1.0f);
				}
			}
			if (schizofrenia) {
				if (!themeon) {
					drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x29, 0x6e, 0x44, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				} else {
					drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x74, 0x1d, 0x4a, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				}
			}
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0xff, 0xff, 0xff, timer2));
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));
			zaba(0.25f);
            C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(bottom);

            C2D_DrawImageAt(bgdown, x, y, 0.0f, NULL, 1.0f, 1.0f);
			C2D_DrawImageAt(gobackplz, currentY + 5.0f, 210.0f, 0.0f, NULL, 1.0f, 1.0f);
			for (int i = 0; i < 100; i++) {
				drawButton(&buttonsy[i], Scene);
			}
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0xff, 0xff, 0xff, timer2));
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));


		} else if (Scene == 11) {
			if (timer2 > 0.0f) {
                timer2 -= 7.0f;
            }
			if (timer < 70) {
				timer += 1.0f;
			} else if (timer < 120) {
				timer += 1.0f;
				timer3 -= 2.0f;
                startY = -400.0f;
				elapsed = 0.0f;
                endY = 0.0f;
			} else if (timer < 209) {
				timer += 1.0f;

			}


            C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(top);
            if (y > -40.0f) {
                x -= 0.5f;
                y -= 0.5f;
            } else {
                x = 0.0f;
                y = 0.0f;
            }

            if (isScrolling) {
                if (elapsed < duration) {
					if (timer < 120) {
						currentY = easeInQuad(elapsed, startY, endY, duration);
						buttonsy[0].x = currentY;
						buttonsy[1].x = currentY;
						buttonsy[2].x = -currentY + 140;
						buttonsy[3].x = -currentY + 140;

					}
					if (timer > 120) {
						currentY = easeOutQuad(elapsed, startY, endY, duration);
					}
                    elapsed += deltaTime;
                }
            }

            C2D_DrawImageAt(bgtop, x, y, 0.0f, NULL, 1.0f, 1.0f);
			if (timer < 120) {
				C2D_DrawImageAt(qr_framen_machen, 0.0f, currentY, 0.0f, NULL, 1.0f, 1.0f);
				if (qrImage.tex && qrImage.subtex) {
					C2D_DrawImageAt(qrImage, 112.0f, currentY + 27, 0.0f, NULL, 1.0f, 1.0f);
				}
			} else if (timer > 120) {
				timer = 80;
				timer2 = 0;
				elapsed = 0;
				Scene = 5;
			}
			if (schizofrenia) {
				if (!themeon) {
					drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x29, 0x6e, 0x44, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				} else {
					drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x74, 0x1d, 0x4a, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				}
			}
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0xff, 0xff, 0xff, timer2));
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));
			zaba(0.25f);
            C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(bottom);

            C2D_DrawImageAt(bgdown, x, y, 0.0f, NULL, 1.0f, 1.0f);
			for (int i = 0; i < 100; i++) {
				drawButton(&buttonsy[i], Scene);
			}
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0xff, 0xff, 0xff, timer2));
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));
			C2D_DrawImageAt(gobackplz, currentY + 5.0f, 210.0f, 0.0f, NULL, 1.0f, 1.0f);

		} else if (Scene == 12) {
			if (timer2 > 0.0f) {
                timer2 -= 7.0f;
            }
			if (timer < 70) {
				timer += 1.0f;
			} else if (timer < 120) {
				timer += 1.0f;
				timer3 -= 2.0f;
                startY = -400.0f;
				elapsed = 0.0f;
                endY = 0.0f;

			} else if (timer < 209) {
				timer += 1.0f;

			}


            C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(top);
            if (y > -40.0f) {
                x -= 0.5f;
                y -= 0.5f;
            } else {
                x = 0.0f;
                y = 0.0f;
            }

            if (isScrolling) {
                if (elapsed < duration) {
					if (timer < 120) {
						currentY = easeInQuad(elapsed, startY, endY, duration);
						buttonsy[0].x = currentY;
						buttonsy[1].x = currentY;
						buttonsy[2].x = -currentY + 140;
						buttonsy[3].x = -currentY + 140;

					}
					if (timer > 120) {
						currentY = easeOutQuad(elapsed, startY, endY, duration);
					}
                    elapsed += deltaTime;
                }
            }

            C2D_DrawImageAt(bgtop, x, y, 0.0f, NULL, 1.0f, 1.0f);
			if (timer < 120) {
				C2D_DrawImageAt(logo3ds, 0.0f, currentY, 0.0f, NULL, 1.0f, 1.0f);
				C2D_DrawImageAt(statusbaren, 0.0f, -currentY, 0.0f, NULL, 1.0f, 1.0f);
				if (!themeon) {
					if (!loadingshit) {
						drawShadowedText_noncentered(&g_staticText[7], 180.0f, -currentY + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
					} else {
						drawShadowedText_noncentered(&g_staticText[1], 180.0f, -currentY + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
					}
				} else {
					if (!loadingshit) {
						drawShadowedText_noncentered(&g_staticText[7], 180.0f, -currentY + 215.0f, 0.5f, 0.8f, 0.8f, themeBaseColor, themeoutColor);
					} else {
						drawShadowedText_noncentered(&g_staticText[1], 180.0f, -currentY + 215.0f, 0.5f, 0.8f, 0.8f, themeBaseColor, themeoutColor);
					}
				}
			} else if (timer > 120) {

			}
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0xff, 0xff, 0xff, timer2));
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));
			if (timer > 120) {
				loadingshit = false;
				if (!themeon) {
					drawShadowedText(&g_staticText[8], 200.0f, currentY + 80, 0.5f, 2.5f, 2.5f, C2D_Color32(0x29, 0x6e, 0x44, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				} else {
					drawShadowedText(&g_staticText[8], 200.0f, currentY + 80, 0.5f, 2.5f, 2.5f, C2D_Color32(0x74, 0x1d, 0x4a, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				}
			}
			if (schizofrenia) {
				if (!themeon) {
					drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x29, 0x6e, 0x44, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				} else {
					drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x74, 0x1d, 0x4a, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				}
			}
			zaba(0.25f);
            C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(bottom);

            C2D_DrawImageAt(bgdown, x, y, 0.0f, NULL, 1.0f, 1.0f);
			C2D_DrawImageAt(gobackplz, currentY + 5.0f, 210.0f, 0.0f, NULL, 1.0f, 1.0f);
			for (int i = 0; i < 100; i++) {
				drawButton(&buttonsy[i], Scene);
			}
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0xff, 0xff, 0xff, timer2));
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));
			if (timer > 120) {
				C2D_DrawImageAt(zappsybaren, 0.0f, -currentY, 0.0f, NULL, 1.0f, 1.0f);

				if (json_done) {
					drawShadowedText(&g_staticText[9], 160.0f, -currentY + 80, 0.5f, 2.3f, 2.3f, C2D_Color32(0xff, 0xff, 0xff, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				} else {
					drawShadowedText(&g_staticText[1], 160.0f, -currentY + 93, 0.5f, 1.6f, 1.6f, C2D_Color32(0xff, 0xff, 0xff, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
					if (global_response.data && strstr(global_response.data, "points") != NULL) {
						process_json_response();
						rebuild_buffer();
						json_done = true;
					}
				}
			}

		} else if (Scene == 13) {
			if (timer2 > 0.0f) {
                timer2 -= 7.0f;
            }
			if (timer < 70) {
				timer += 1.0f;
			} else if (timer < 120) {
				timer += 1.0f;
				timer3 -= 2.0f;
                startY = -400.0f;
				elapsed = 0.0f;
                endY = 0.0f;
			} else if (timer < 209) {
				timer += 1.0f;

			}


            C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(top);
            if (y > -40.0f) {
                x -= 0.5f;
                y -= 0.5f;
            } else {
                x = 0.0f;
                y = 0.0f;
            }

            if (isScrolling) {
                if (elapsed < duration) {
					if (timer < 120) {
						currentY = easeInQuad(elapsed, startY, endY, duration);
						buttonsy[0].x = currentY;
						buttonsy[1].x = currentY;
						buttonsy[2].x = -currentY + 140;
						buttonsy[3].x = -currentY + 140;

					}
					if (timer > 120) {
						currentY = easeOutQuad(elapsed, startY, endY, duration);
					}
                    elapsed += deltaTime;
                }
            }

            C2D_DrawImageAt(bgtop, x, y, 0.0f, NULL, 1.0f, 1.0f);
			if (timer < 120) {
				if (!themeon) {
					drawShadowedText(&g_staticText[8], 200.0f, currentY + 80, 0.5f, 2.5f, 2.5f, C2D_Color32(0x29, 0x6e, 0x44, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				} else {
					drawShadowedText(&g_staticText[8], 200.0f, currentY + 80, 0.5f, 2.5f, 2.5f, C2D_Color32(0x74, 0x1d, 0x4a, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				}
			} else if (timer > 120) {
				timer = 80;
				timer2 = 0;
				elapsed = 0;
				Scene = 5;
			}
			if (schizofrenia) {
				if (!themeon) {
					drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x29, 0x6e, 0x44, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				} else {
					drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x74, 0x1d, 0x4a, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				}
			}
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0xff, 0xff, 0xff, timer2));
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));
			zaba(0.25f);
            C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(bottom);

            C2D_DrawImageAt(bgdown, x, y, 0.0f, NULL, 1.0f, 1.0f);
			C2D_DrawImageAt(gobackplz, currentY + 5.0f, 210.0f, 0.0f, NULL, 1.0f, 1.0f);
			for (int i = 0; i < 100; i++) {
				drawButton(&buttonsy[i], Scene);
			}

			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0xff, 0xff, 0xff, timer2));
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));
			if (timer < 120) {
				C2D_DrawImageAt(zappsybaren, 0.0f, -currentY, 0.0f, NULL, 1.0f, 1.0f);
				drawShadowedText(&g_staticText[9], 160.0f, -currentY + 80, 0.5f, 2.3f, 2.3f, C2D_Color32(0xff, 0xff, 0xff, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
			}

		} else if (Scene == 14) {
			if (timer2 > 0.0f) {
                timer2 -= 7.0f;
            }
			if (timer < 70) {
				timer += 1.0f;
			} else if (timer < 120) {
				timer += 1.0f;
				timer3 -= 2.0f;
                startY = -400.0f;
				elapsed = 0.0f;
                endY = 0.0f;

			} else if (timer < 209) {
				timer += 1.0f;

			}


            C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(top);
            if (y > -40.0f) {
                x -= 0.5f;
                y -= 0.5f;
            } else {
                x = 0.0f;
                y = 0.0f;
            }

            if (isScrolling) {
                if (elapsed < duration) {
					if (timer < 120) {
						currentY = easeInQuad(elapsed, startY, endY, duration);
						buttonsy[0].x = currentY;
						buttonsy[1].x = currentY;
						buttonsy[2].x = -currentY + 140;
						buttonsy[3].x = -currentY + 140;

					}
					if (timer > 120) {

						currentY = easeOutQuad(elapsed, startY, endY, duration);
						buttonsy[4].x = 40;
						buttonsy[4].y = -currentY + 70 + text_h - textOffset;
					}
                    elapsed += deltaTime;
                } else {
					buttonsy[4].y = 70 + text_h - textOffset;
				}
            }

            C2D_DrawImageAt(bgtop, x, y, 0.0f, NULL, 1.0f, 1.0f);
			if (timer < 120) {
				C2D_DrawImageAt(logo3ds, 0.0f, currentY, 0.0f, NULL, 1.0f, 1.0f);
				C2D_DrawImageAt(statusbaren, 0.0f, -currentY, 0.0f, NULL, 1.0f, 1.0f);
				if (!themeon) {
					if (!loadingshit) {
						drawShadowedText_noncentered(&g_staticText[7], 180.0f, -currentY + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
					} else {
						drawShadowedText_noncentered(&g_staticText[1], 180.0f, -currentY + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
					}
				} else {
					if (!loadingshit) {
						drawShadowedText_noncentered(&g_staticText[7], 180.0f, -currentY + 215.0f, 0.5f, 0.8f, 0.8f, themeBaseColor, themeoutColor);
					} else {
						drawShadowedText_noncentered(&g_staticText[1], 180.0f, -currentY + 215.0f, 0.5f, 0.8f, 0.8f, themeBaseColor, themeoutColor);
					}
				}
			} else if (timer > 120) {

			}
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0xff, 0xff, 0xff, timer2));
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));
			if (timer > 120) {
				loadingshit = false;
				if (!obrazekdone) {
					if (!themeon) {
						drawShadowedText(&g_staticText[12], 200.0f, currentY + 80, 0.5f, 2.5f, 2.5f, C2D_Color32(0x29, 0x6e, 0x44, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
					} else {
						drawShadowedText(&g_staticText[12], 200.0f, currentY + 80, 0.5f, 2.5f, 2.5f, C2D_Color32(0x74, 0x1d, 0x4a, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
					}
				}
			}
			if (json_done && obrazekdone) {
				float screenWidth = 400.0f;
				float screenHeight = 240.0f;
				float imageWidth = kuponkurwa.tex->width;
				float imageHeight = kuponkurwa.tex->height;

				float scaleX = (screenWidth / imageWidth) * 0.85f;
				float scaleY = (screenHeight / imageHeight) * 0.85f;
				float scale = (scaleX < scaleY) ? scaleX : scaleY;

				C2D_DrawParams params = {0};
				params.pos.x = (screenWidth - imageWidth * scale) / 2.0f;
				params.pos.y = (screenHeight - imageHeight * scale) / 2.0f + currentY;
				params.pos.w = imageWidth * scale;
				params.pos.h = imageHeight * scale;
				params.center.x = 0.0f;
				params.center.y = 0.0f;
				params.angle = 0.0f;
				params.depth = 0.0f;

				C2D_ImageTint shadowTint;
				C2D_SetImageTint(&shadowTint, C2D_TopLeft,     C2D_Color32(0, 0, 0, 128), 1.0f);
				C2D_SetImageTint(&shadowTint, C2D_TopRight,    C2D_Color32(0, 0, 0, 128), 1.0f);
				C2D_SetImageTint(&shadowTint, C2D_BotLeft,  C2D_Color32(0, 0, 0, 128), 1.0f);
				C2D_SetImageTint(&shadowTint, C2D_BotRight, C2D_Color32(0, 0, 0, 128), 1.0f);

				C2D_DrawParams shadowParams = params;
				shadowParams.pos.x += 5.0f;
				shadowParams.pos.y += 5.0f;
				C2D_DrawImage(kuponkurwa, &shadowParams, &shadowTint);

				C2D_DrawImage(kuponkurwa, &params, NULL);
			}
			if (schizofrenia) {
				if (!themeon) {
					drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x29, 0x6e, 0x44, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				} else {
					drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x74, 0x1d, 0x4a, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				}
			}
			zaba(0.25f);
            C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(bottom);

            C2D_DrawImageAt(bgdown, x, y, 0.0f, NULL, 1.0f, 1.0f);
			C2D_DrawImageAt(gobackplz, currentY + 5.0f, 210.0f, 0.0f, NULL, 1.0f, 1.0f);
			for (int i = 0; i < 100; i++) {
				drawButton(&buttonsy[i], Scene);
			}

			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0xff, 0xff, 0xff, timer2));
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));
			if (timer > 120) {
				if (json_done) {
					if (!ofertanow) {
						for (int i = 0; i < tileCount; i++) {
							float buttonX = 5.0f + (i * 142.0f) - textOffset;
							float buttonY = currentY + 25.0f;
							buttonsy[i + 5].x = buttonX;
							buttonsy[i + 5].y = buttonY;
							float textX = buttonX + 65.0f;
							float textY = buttonY + 140.0f;
							if (!themeon){
								drawShadowedText(&g_kuponText[i], textX, textY, 0.5f, 0.85f, 0.85f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
							} else {
								drawShadowedText(&g_kuponText[i], textX, textY, 0.5f, 0.85f, 0.85f, themeBaseColor, themeoutColor);
							}
						}
					} else {
						float textX = 160.0f;
						float textY = currentY + 25.0f + 30.0f;
						C2D_DrawImageAt(scrollbar, -currentY + 287.0f, 12.5f, 0.0f, NULL, 1.0f, 1.0f);
						if (!themeon){
							drawShadowedTextWrapped(&g_kuponText[0], textX, textY - textOffset, 0.5f, 0.6f, 0.6f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
							drawShadowedTextWrapped(&g_kuponText[1], textX, textY - 50.0f - textOffset, 0.5f, 1.5f, 1.5f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
						} else {
							drawShadowedTextWrapped(&g_kuponText[0], textX, textY - textOffset, 0.5f, 0.6f, 0.6f, themeBaseColor, themeoutColor);
							drawShadowedTextWrapped(&g_kuponText[1], textX, textY - 50.0f - textOffset, 0.5f, 1.5f, 1.5f, themeBaseColor, themeoutColor);
						}
						if (!canredeem) {
							 C2D_DrawImageAt(za_malo, 40.0f, -currentY + 140, 0.0f, NULL, 1.0f, 1.0f);
						}
						if (aktywacja_done) {
							if (!themeon){
								drawShadowedTextWrapped(&g_staticText[13], textX, textY + 50.0f + text_h - textOffset, 0.5f, 1.5f, 1.5f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
							} else {
								drawShadowedTextWrapped(&g_staticText[13], textX, textY + 50.0f + text_h - textOffset, 0.5f, 1.5f, 1.5f, themeBaseColor, themeoutColor);
							}
						}
					}

				} else {
					if (!themeon) {
						drawShadowedText(&g_staticText[1], 160.0f, -currentY + 93, 0.5f, 1.6f, 1.6f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
					} else {
						drawShadowedText(&g_staticText[1], 160.0f, -currentY + 93, 0.5f, 1.6f, 1.6f, themeBaseColor, themeoutColor);
					}
					if (!loadingshit) {
						textOffset = 0;
						if (!offermachen) {
							if (!zonefeeddone) {
								if (requestdone) {
									if (VOICEACT) {
										playSoundOncePerScene(14, 9);
									}
									process_kupony();
									json_done = true;
									loadingshit = true;
								}
							}
							if (!categoryfeeddone) {
								if (requestdone) {
									process_category();
									log_to_file("dupa");
									json_done = true;
								}
							}
						} else {
							if (requestdone) {
								textOffset = 0;
								readingoferta = true;
								process_ofertamachen();
								touchoferta = false;
								log_to_file("dupa");
								json_done = true;
							}

						}
					}
				}
			}

		} else if (Scene == 15) {
			if (timer2 > 0.0f) {
                timer2 -= 7.0f;
            }
			if (timer < 70) {
				timer += 1.0f;
			} else if (timer < 120) {
				timer += 1.0f;
				timer3 -= 2.0f;
                startY = -400.0f;
				elapsed = 0.0f;
                endY = 0.0f;
			} else if (timer < 209) {
				timer += 1.0f;

			}


            C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(top);
            if (y > -40.0f) {
                x -= 0.5f;
                y -= 0.5f;
            } else {
                x = 0.0f;
                y = 0.0f;
            }

            if (isScrolling) {
                if (elapsed < duration) {
					if (timer < 120) {
						currentY = easeInQuad(elapsed, startY, endY, duration);
						buttonsy[0].x = currentY;
						buttonsy[1].x = currentY;
						buttonsy[2].x = -currentY + 140;
						buttonsy[3].x = -currentY + 140;
						buttonsy[4].x = 40;
						buttonsy[4].y = -currentY + 70 + text_h - textOffset;

					}
					if (timer > 120) {
						currentY = easeOutQuad(elapsed, startY, endY, duration);
					}
                    elapsed += deltaTime;
                }
            }

            C2D_DrawImageAt(bgtop, x, y, 0.0f, NULL, 1.0f, 1.0f);
			if (timer > 120) {
				timer = 80;
				timer2 = 0;
				elapsed = 0;
				Scene = 5;
				removeButtonEntries(70);
			}
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0xff, 0xff, 0xff, timer2));
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));
			if (timer > 120) {
				loadingshit = false;
				if (!obrazekdone) {
					if (!themeon) {
						drawShadowedText(&g_staticText[12], 200.0f, currentY + 80, 0.5f, 2.5f, 2.5f, C2D_Color32(0x29, 0x6e, 0x44, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
					} else {
						drawShadowedText(&g_staticText[12], 200.0f, currentY + 80, 0.5f, 2.5f, 2.5f, C2D_Color32(0x74, 0x1d, 0x4a, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
					}
				}
			}
			if (json_done && obrazekdone) {
				float screenWidth = 400.0f;
				float screenHeight = 240.0f;
				float imageWidth = kuponkurwa.tex->width;
				float imageHeight = kuponkurwa.tex->height;

				float scaleX = (screenWidth / imageWidth) * 0.85f;
				float scaleY = (screenHeight / imageHeight) * 0.85f;
				float scale = (scaleX < scaleY) ? scaleX : scaleY;

				C2D_DrawParams params = {0};
				params.pos.x = (screenWidth - imageWidth * scale) / 2.0f;
				params.pos.y = (screenHeight - imageHeight * scale) / 2.0f + currentY;
				params.pos.w = imageWidth * scale;
				params.pos.h = imageHeight * scale;
				params.center.x = 0.0f;
				params.center.y = 0.0f;
				params.angle = 0.0f;
				params.depth = 0.0f;

				C2D_ImageTint shadowTint;
				C2D_SetImageTint(&shadowTint, C2D_TopLeft,     C2D_Color32(0, 0, 0, 128), 1.0f);
				C2D_SetImageTint(&shadowTint, C2D_TopRight,    C2D_Color32(0, 0, 0, 128), 1.0f);
				C2D_SetImageTint(&shadowTint, C2D_BotLeft,  C2D_Color32(0, 0, 0, 128), 1.0f);
				C2D_SetImageTint(&shadowTint, C2D_BotRight, C2D_Color32(0, 0, 0, 128), 1.0f);

				C2D_DrawParams shadowParams = params;
				shadowParams.pos.x += 5.0f;
				shadowParams.pos.y += 5.0f;
				C2D_DrawImage(kuponkurwa, &shadowParams, &shadowTint);

				C2D_DrawImage(kuponkurwa, &params, NULL);
			}
			zaba(0.25f);
            C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(bottom);

            C2D_DrawImageAt(bgdown, x, y, 0.0f, NULL, 1.0f, 1.0f);
			C2D_DrawImageAt(gobackplz, currentY + 5.0f, 210.0f, 0.0f, NULL, 1.0f, 1.0f);
			for (int i = 0; i < 100; i++) {
				drawButton(&buttonsy[i], Scene);
			}
			if (schizofrenia) {
				if (!themeon) {
					drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x29, 0x6e, 0x44, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				} else {
					drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x74, 0x1d, 0x4a, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				}
			}
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0xff, 0xff, 0xff, timer2));
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));
			if (timer < 120) {
				if (json_done) {
					if (!ofertanow) {
						for (int i = 0; i < tileCount; i++) {
							float buttonX = 5.0f + (i * 142.0f) - textOffset;
							float buttonY = currentY + 25.0f;
							buttonsy[i + 5].x = buttonX;
							buttonsy[i + 5].y = buttonY;
							float textX = buttonX + 65.0f;
							float textY = buttonY + 140.0f;
							if (!themeon){
								drawShadowedText(&g_kuponText[i], textX, textY, 0.5f, 0.85f, 0.85f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
							} else {
								drawShadowedText(&g_kuponText[i], textX, textY, 0.5f, 0.85f, 0.85f, themeBaseColor, themeoutColor);
							}
						}
					} else {
						float textX = 160.0f;
						float textY = currentY + 25.0f + 30.0f;
						C2D_DrawImageAt(scrollbar, -currentY + 287.0f, 12.5f, 0.0f, NULL, 1.0f, 1.0f);
						if (!themeon){
							drawShadowedTextWrapped(&g_kuponText[0], textX, textY - textOffset, 0.5f, 0.6f, 0.6f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
							drawShadowedTextWrapped(&g_kuponText[1], textX, textY - 50.0f - textOffset, 0.5f, 1.5f, 1.5f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
						} else {
							drawShadowedTextWrapped(&g_kuponText[0], textX, textY - textOffset, 0.5f, 0.6f, 0.6f, themeBaseColor, themeoutColor);
							drawShadowedTextWrapped(&g_kuponText[1], textX, textY - 50.0f - textOffset, 0.5f, 1.5f, 1.5f, themeBaseColor, themeoutColor);
						}
						if (textOffset > 0) {
							textOffset -= speed * 0.4f;
							if (textOffset < 0) textOffset = 0;
						}
						if (!canredeem) {
							 C2D_DrawImageAt(za_malo, 40.0f, -currentY + 140 + text_h, 0.0f, NULL, 1.0f, 1.0f);
						}
						if (aktywacja_done) {
							if (!themeon){
								drawShadowedTextWrapped(&g_staticText[13], textX, textY + 50.0f + text_h - textOffset, 0.5f, 1.5f, 1.5f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
							} else {
								drawShadowedTextWrapped(&g_staticText[13], textX, textY + 50.0f + text_h - textOffset, 0.5f, 1.5f, 1.5f, themeBaseColor, themeoutColor);
							}
						}
					}

				} else {
					drawShadowedText(&g_staticText[1], 160.0f, -currentY + 93, 0.5f, 1.6f, 1.6f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
					if (!loadingshit) {
						if (!offermachen) {
							if (!zonefeeddone) {
								if (requestdone) {
									process_kupony();
									json_done = true;
									loadingshit = true;
								}
							}
							if (!categoryfeeddone) {
								if (requestdone) {
									process_category();
									log_to_file("dupa");
									json_done = true;
								}
							}
						} else {
							if (requestdone) {
								process_ofertamachen();
								touchoferta = false;
								log_to_file("dupa");
								json_done = true;
							}

						}
					}
				}
			}
		} else if (Scene == 16) {
			if (timer2 > 0.0f) {
                timer2 -= 7.0f;
            }
			if (timer < 70) {
				timer += 1.0f;
			} else if (timer < 120) {
				timer += 1.0f;
				timer3 -= 2.0f;
                startY = -400.0f;
				elapsed = 0.0f;
                endY = 0.0f;

			} else if (timer < 209) {
				timer += 1.0f;

			}


            C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(top);
            if (y > -40.0f) {
                x -= 0.5f;
                y -= 0.5f;
            } else {
                x = 0.0f;
                y = 0.0f;
            }

            if (isScrolling) {
                if (elapsed < duration) {
					if (timer < 120) {
						currentY = easeInQuad(elapsed, startY, endY, duration);
						buttonsy[0].x = currentY;
						buttonsy[1].x = currentY;
						buttonsy[2].x = -currentY + 140;
						buttonsy[3].x = -currentY + 140;

					}
					if (timer > 120) {
						currentY = easeOutQuad(elapsed, startY, endY, duration);
						buttonsy[97].x = 70;
						przycskmachen = true;
						buttonsy[97].y = -currentY + 20;
					}
                    elapsed += deltaTime;
                }
            }

            C2D_DrawImageAt(bgtop, x, y, 0.0f, NULL, 1.0f, 1.0f);
			if (timer < 120) {
				C2D_DrawImageAt(logo3ds, 0.0f, currentY, 0.0f, NULL, 1.0f, 1.0f);
				C2D_DrawImageAt(statusbaren, 0.0f, -currentY, 0.0f, NULL, 1.0f, 1.0f);
				if (!themeon) {
					if (!loadingshit) {
						drawShadowedText_noncentered(&g_staticText[7], 180.0f, -currentY + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
					} else {
						drawShadowedText_noncentered(&g_staticText[1], 180.0f, -currentY + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
					}
				} else {
					if (!loadingshit) {
						drawShadowedText_noncentered(&g_staticText[7], 180.0f, -currentY + 215.0f, 0.5f, 0.8f, 0.8f, themeBaseColor, themeoutColor);
					} else {
						drawShadowedText_noncentered(&g_staticText[1], 180.0f, -currentY + 215.0f, 0.5f, 0.8f, 0.8f, themeBaseColor, themeoutColor);
					}
				}
			} else if (timer > 120) {

			}
			if (schizofrenia) {
				if (!themeon) {
					drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x29, 0x6e, 0x44, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				} else {
					drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x74, 0x1d, 0x4a, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				}
			}
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0xff, 0xff, 0xff, timer2));
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));
			if (timer > 120) {
				loadingshit = false;
				if (!themeon) {
					drawShadowedText(&g_staticText[14], 200.0f, currentY + 80, 0.5f, 2.5f, 2.5f, C2D_Color32(0x29, 0x6e, 0x44, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				} else {
					drawShadowedText(&g_staticText[14], 200.0f, currentY + 80, 0.5f, 2.5f, 2.5f, themeBaseColor, themeoutColor);
				}
			}
			zaba(0.25f);
            C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(bottom);

            C2D_DrawImageAt(bgdown, x, y, 0.0f, NULL, 1.0f, 1.0f);
			C2D_DrawImageAt(gobackplz, currentY + 5.0f, 210.0f, 0.0f, NULL, 1.0f, 1.0f);
			for (int i = 0; i < 96; i++) {
				drawButton(&buttonsy[i], Scene);
			}
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0xff, 0xff, 0xff, timer2));
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));
			if (timer > 120) {
				for (int i = 97; i < 100; i++) {
					drawButton(&buttonsy[i], Scene);
				}
				C2D_DrawImageAt(themeborder, 38.0f, -currentY + 100, 0.0f, NULL, 1.0f, 1.0f);
				if (!themeon) {
					drawShadowedText(&g_staticText[15], 160.0f, -currentY + 70, 0.5f, 1.0f, 1.0f, C2D_Color32(0x29, 0x6e, 0x44, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
					drawShadowedText(&themeText[selectionthemelol], 160.0f, -currentY + 115, 0.5f, 1.5f, 1.5f, C2D_Color32(0x29, 0x6e, 0x44, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
					drawShadowedText(&g_staticText[16], 160.0f, -currentY + 163, 0.5f, 0.5f, 0.5f, C2D_Color32(0x29, 0x6e, 0x44, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				} else {
					drawShadowedText(&g_staticText[15], 160.0f, -currentY + 70, 0.5f, 1.0f, 1.0f, themeBaseColor, themeoutColor);
					drawShadowedText(&themeText[selectionthemelol], 160.0f, -currentY + 115, 0.5f, 1.5f, 1.5f, themeBaseColor, themeoutColor);
					drawShadowedText(&g_staticText[16], 160.0f, -currentY + 163, 0.5f, 0.5f, 0.5f, themeBaseColor, themeoutColor);
				}
				C2D_DrawImageAt(ch_theme, 38.0f, -currentY + 185, 0.0f, NULL, 0.6f, 0.6f);
			}


		} else if (Scene == 17) {
			if (timer2 > 0.0f) {
                timer2 -= 7.0f;
            }
			if (timer < 70) {
				timer += 1.0f;
			} else if (timer < 120) {
				timer += 1.0f;
				timer3 -= 2.0f;
                startY = -400.0f;
				elapsed = 0.0f;
                endY = 0.0f;
			} else if (timer < 209) {
				timer += 1.0f;

			}


            C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(top);
            if (y > -40.0f) {
                x -= 0.5f;
                y -= 0.5f;
            } else {
                x = 0.0f;
                y = 0.0f;
            }

            if (isScrolling) {
                if (elapsed < duration) {
					if (timer < 120) {
						currentY = easeInQuad(elapsed, startY, endY, duration);
						buttonsy[0].x = currentY;
						buttonsy[1].x = currentY;
						buttonsy[2].x = -currentY + 140;
						buttonsy[3].x = -currentY + 140;
						buttonsy[97].x = 70;
						buttonsy[97].y = -currentY + 20;

					}
					if (timer > 120) {
						currentY = easeOutQuad(elapsed, startY, endY, duration);
					}
                    elapsed += deltaTime;
                }
            }

            C2D_DrawImageAt(bgtop, x, y, 0.0f, NULL, 1.0f, 1.0f);
			if (timer < 120) {
				if (!themeon) {
					drawShadowedText(&g_staticText[14], 200.0f, currentY + 80, 0.5f, 2.5f, 2.5f, C2D_Color32(0x29, 0x6e, 0x44, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				} else {
					drawShadowedText(&g_staticText[14], 200.0f, currentY + 80, 0.5f, 2.5f, 2.5f, themeBaseColor, themeoutColor);
				}
			} else if (timer > 120) {
				timer = 80;
				timer2 = 0;
				elapsed = 0;
				if (access("/3ds/data.json", F_OK) == 0) {
					Scene = 5;
				} else {
					Scene = 1;
					isLogged = false;
					timer2 = 255.0f;
					timer3 = 0.0f;
				}
			}
			if (schizofrenia) {
				if (!themeon) {
					drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x29, 0x6e, 0x44, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				} else {
					drawShadowedText_noncentered(&g_staticText[11], 0.0f, 0 + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x74, 0x1d, 0x4a, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				}
			}
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0xff, 0xff, 0xff, timer2));
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));
			zaba(0.25f);
            C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(bottom);

            C2D_DrawImageAt(bgdown, x, y, 0.0f, NULL, 1.0f, 1.0f);
			C2D_DrawImageAt(gobackplz, currentY + 5.0f, 210.0f, 0.0f, NULL, 1.0f, 1.0f);
			for (int i = 0; i < 100; i++) {
				drawButton(&buttonsy[i], Scene);
			}
			C2D_DrawImageAt(themeborder, 38.0f, -currentY + 100, 0.0f, NULL, 1.0f, 1.0f);
			if (!themeon) {
				drawShadowedText(&g_staticText[15], 160.0f, -currentY + 70, 0.5f, 1.0f, 1.0f, C2D_Color32(0x29, 0x6e, 0x44, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				drawShadowedText(&themeText[selectionthemelol], 160.0f, -currentY + 115, 0.5f, 1.5f, 1.5f, C2D_Color32(0x29, 0x6e, 0x44, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				drawShadowedText(&g_staticText[16], 160.0f, -currentY + 163, 0.5f, 0.5f, 0.5f, C2D_Color32(0x29, 0x6e, 0x44, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
			} else {
				drawShadowedText(&g_staticText[15], 160.0f, -currentY + 70, 0.5f, 1.0f, 1.0f, themeBaseColor, themeoutColor);
				drawShadowedText(&themeText[selectionthemelol], 160.0f, -currentY + 115, 0.5f, 1.5f, 1.5f, themeBaseColor, themeoutColor);
				drawShadowedText(&g_staticText[16], 160.0f, -currentY + 163, 0.5f, 0.5f, 0.5f, themeBaseColor, themeoutColor);
			}
			C2D_DrawImageAt(ch_theme, 38.0f, -currentY + 185, 0.0f, NULL, 0.6f, 0.6f);

			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0xff, 0xff, 0xff, timer2));
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));
			if (timer > 120) {

			}

		}
		if (cpu_debug) {
			C2D_SceneBegin(top);
			float cpuStartX = 20.0f;
			float cpuStartY = 20.0f;
			float cpuMaxLength = 260.0f;
			float cpuUsage = C3D_GetProcessingTime() * 6.0f;
			if (cpuUsage > 100.0f) cpuUsage = 100.0f;
			float cpuLineLength = (cpuUsage / 100.0f) * cpuMaxLength;
			C2D_DrawLine(cpuStartX, cpuStartY, C2D_Color32(255, 255, 0, 255), cpuStartX + cpuLineLength, cpuStartY, C2D_Color32(255, 255, 0, 255), 5.0f, 0);
			C2D_TextBufClear(memBuf);
			char memeText[64];
			snprintf(memeText, sizeof(memeText), "CPU: %.2f%%", cpuUsage);
			C2D_TextParse(&memtext[0], memBuf, memeText);
			C2D_TextOptimize(&memtext[0]);
			C2D_DrawText(&memtext[0], C2D_AlignLeft | C2D_WithColor, 20, 25, 0.4f, 0.4f, 0.4f, C2D_Color32(0, 0, 0, 255));
			float drawUsage = C3D_GetDrawingTime() * 6.0f;
			if (drawUsage > 100.0f) drawUsage = 100.0f;
			float drawLineLength = (drawUsage / 100.0f) * cpuMaxLength;
			C2D_DrawLine(cpuStartX, cpuStartY + 30, C2D_Color32(255, 255, 0, 255), cpuStartX + drawLineLength, cpuStartY + 30, C2D_Color32(255, 255, 0, 255), 5.0f, 0);
			C2D_TextBufClear(memBuf);
			char drawText[64];
			snprintf(memeText, sizeof(memeText), "GPU: %.2f%%", drawUsage);
			C2D_TextParse(&memtext[1], memBuf, memeText);
			C2D_TextOptimize(&memtext[1]);
			char wygText[64];
			snprintf(wygText, sizeof(wygText), "EXP: %lld", czas_wygasniecia);
			C2D_TextParse(&memtext[2], memBuf, wygText);
			C2D_TextOptimize(&memtext[2]);
			C2D_DrawText(&memtext[1], C2D_AlignLeft | C2D_WithColor, 20, 55, 1.0f, 0.4f, 0.4f, C2D_Color32(0, 0, 0, 255));
			C2D_DrawText(&g_totpText[0], C2D_AlignLeft | C2D_WithColor, 265, 10, 1.0f, 0.4f, 0.4f, C2D_Color32(0, 0, 0, 255));
			C2D_DrawText(&g_totpText[1], C2D_AlignLeft | C2D_WithColor, 265, 25, 1.0f, 0.4f, 0.4f, C2D_Color32(0, 0, 0, 255));
			C2D_DrawText(&g_totpText[2], C2D_AlignLeft | C2D_WithColor, 265, 40, 1.0f, 0.4f, 0.4f, C2D_Color32(0, 0, 0, 255));
			C2D_DrawText(&g_totpText[3], C2D_AlignLeft | C2D_WithColor, 265, 55, 1.0f, 0.4f, 0.4f, C2D_Color32(0, 0, 0, 255));
			C2D_DrawText(&g_totpText[4], C2D_AlignLeft | C2D_WithColor, 265, 70, 1.0f, 0.4f, 0.4f, C2D_Color32(0, 0, 0, 255));
			C2D_DrawText(&memtext[2], C2D_AlignLeft | C2D_WithColor, 265, 85, 1.0f, 0.4f, 0.4f, C2D_Color32(0, 0, 0, 255));
		}
		if (VOICEACT) {
			C2D_SceneBegin(top);
			if (!themeon) {
				drawShadowedText(&g_staticText[17], 380.0f, 2.0f, 0.5f, 1.0f, 1.0f, C2D_Color32(0x29, 0x6e, 0x44, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
			} else {
				drawShadowedText(&g_staticText[17], 380.0f, 2.0f, 0.5f, 1.0f, 1.0f, themeBaseColor, themeoutColor);

			}
		}

		C3D_FrameEnd(0);
	}
	json_t *opcjee = json_load_file("/3ds/opcje.json", 0, NULL);
	json_object_set_new(opcjee, "va", json_boolean(VOICEACT));
	json_dump_file(opcjee, "/3ds/opcje.json", JSON_COMPACT);
	json_decref(opcjee);
	close_logger();
    running = false;
	save_calczas();
    threadJoin(thread, UINT64_MAX);
    threadFree(thread);
    internet_thread_running = false;
    threadJoin(internet_thread, UINT64_MAX);
    threadFree(internet_thread);
	stop_request_thread();
	free(secrettotpglobal);
	C2D_SpriteSheetFree(background_top);
	C2D_SpriteSheetFree(background_down);
	C2D_SpriteSheetFree(logo);
	C2D_SpriteSheetFree(points);
	C2D_SpriteSheetFree(scan);
	C2D_SpriteSheetFree(settings);
	C2D_SpriteSheetFree(qrframe);
	C2D_TextBufDelete(g_staticBuf);
	C2D_TextBufDelete(kupon_text_Buf);
	C2D_FontFree(font[0]);
    ndspExit();
	cwavFree(bgm);
	cfguExit();
	freeCwavList();
	C2D_Fini();
	C3D_Fini();
	gfxExit();
	romfsExit();
	return 0;
}
