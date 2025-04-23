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
#include <curl/curl.h> //curlkurwa
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


size_t totalsajz = 0;

bool mem_debug = false;
bool citra_machen = false;
//char *// global_response = NULL;
uint8_t* obrazek = NULL;
char *zabkazonefeed = NULL;
extern bool json_done;
bool themeon;
int currenttheme;
extern CWAVInfo cwavList[8]; // Adjust size accordingly
extern int cwavCount;
bool internet_available = false;
static Thread internet_thread;
static bool internet_thread_running = true;
char combinedText[128]; 
int selectioncodelol;
extern Button buttonsy[100];
int selectionthemelol;
extern bool przycskmachen;
bool touchoferta = true;
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
extern const char *userajd;
const char *nejmenmachen;
int amountzappsy;
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
//function for writing response to string ig 
// https://github.com/curl/curl/blob/master/docs/libcurl/opts/CURLOPT_WRITEFUNCTION.3#L83-L103
struct memory {
  char *response;
  size_t size;
};

// Enumeration for different scenes

// Structure to hold button properties
void createDirectory(const char* dirPath) {
    // Initialize FS_Path
    FS_Path fsPath = fsMakePath(PATH_ASCII, dirPath);

    // Open SDMC archive
    FS_Archive sdmcArchive;
    Result rc = FSUSER_OpenArchive(&sdmcArchive, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""));
    if (R_FAILED(rc)) {
        printf("Failed to open SDMC archive: 0x%08lX\n", rc);
        return;
    }

    // Create the directory
    rc = FSUSER_CreateDirectory(sdmcArchive, fsPath, 0);
    if (R_FAILED(rc)) {
        printf("Failed to create directory '%s': 0x%08lX\n", dirPath, rc);
    } else {
        printf("Directory '%s' created successfully.\n", dirPath);
    }

    // Close the archive
    FSUSER_CloseArchive(sdmcArchive);
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

// Simple sprite struct
typedef struct {
    C2D_Sprite spr;
    float dx, dy; // velocity
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
//setvbuf(fh, NULL, _IOFBF, 32768);
float easeInQuad(float t, float start, float end, float duration) {
    t /= duration;
    return start + (end - start) * (t * t);
}
float easeOutQuad(float t, float start, float end, float duration) {
    t /= duration;
    return start + (end - start) * (1 - (1 - t) * (1 - t));
}
// Function to create a simple QR code-like grid
// Function to generate QR Code data



// Pause until user presses a button
// void waitForInput(void) {
    // printf("Press any button to exit...\n");
    // while(aptMainLoop()) {
        // gspWaitForVBlank();
        // gfxSwapBuffers();
        // hidScanInput();
        // if(hidKeysDown()) break;
    // }
// }

// ---- END HELPER FUNCTIONS ----



// Initialize sprites
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
        char auth_header[1024];
        snprintf(auth_header, sizeof(auth_header), "Authorization: %s", id_token);
        headers = curl_slist_append(headers, auth_header);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

       
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_STDERR, fopen(LOG_FILE, "a"));
        //curl_easy_setopt(curl, CURLOPT_URL, url);
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
    // Initialize AC service
    if (R_FAILED(acInit())) {
        return false;
    }

    // Check connection status
    u32 status;
    bool connected = false;
    
    if (R_SUCCEEDED(ACU_GetStatus(&status))) {
        connected = (status == 3); // 3 means connected
    }

    // Clean up
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
void internet_check_thread(void* arg) {
    while (internet_thread_running) {
        check_internet(); 
        //printf("Internet Available: %s\n", internet_available ? "Yes" : "No");
		//log_to_file("schiz: %d\n", schizofrenia); wpierdala logi jak pojebane
		if (internet_available) {
			schizofrenia = false;
		} else {
			schizofrenia = true;
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


    curl_slist_free_all(headers);
    free(data);
    json_decref(json);
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
}
void process_json_response() {
    if (global_response.data) {
        log_to_file("Processing JSON response...");
		

        // Parse the stored JSON response
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

        // Free the response buffer after processing
        //free_global_response();
       // // global_response = NULL;
    } else {
        log_to_file("ERROR: No response data available to process.");
    }
}
extern char tileNames[100][256];  // Array to store names/titles
extern int tileCount;               // Track number of extracted names


void chuj() {
	log_to_file("chuj.."); //chuj 
}

void ping() {
	refresh_data("https://www.google.com/", "", NULL);
}

void kupony() {
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
		cwavPlay(sfx, 0, 1);  // Stereo sound (2 channels)
	} else {
		cwavPlay(sfx, 0, -1);  // Mono sound (-1 channel)
	}
	//freeC2DImage(&qr);
	log_to_file("Updating Ploy Zones...");
	updateploy(id_tokenk, refreshtoken);
	//int otpen = compute_magic_number(secrettotpglobal);
	//doBasicDemo(&qrImage, otpen, userajd);
}


void zappsy() {
	przycskmachen = false;
	json_done = false;
	timer = 0.0f;
    Scene = 12;
	startY = 0.0f;
	elapsed = 0.0f;
	endY = -400.0f; 
	
	if (sfx->numChannels == 2) {
		cwavPlay(sfx, 0, 1);  // Stereo sound (2 channels)
	} else {
		cwavPlay(sfx, 0, -1);  // Mono sound (-1 channel)
	}
	//freeC2DImage(&qr);
	updatezappsy(id_tokenk, refreshtoken);
	//int otpen = compute_magic_number(secrettotpglobal);
	//doBasicDemo(&qrImage, otpen, userajd);
	
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
		cwavPlay(sfx, 0, 1);  // Stereo sound (2 channels)
	} else {
		cwavPlay(sfx, 0, -1);  // Mono sound (-1 channel)
	}
	//freeC2DImage(&qr);
	//generate_qrcode();
	//int otpen = compute_magic_number(secrettotpglobal);
	//doBasicDemo(&qrImage, otpen, userajd);
}
void executeButtonFunction(int buttonIndex) {
    if (buttonIndex >= 0 && buttonIndex < 100 && buttonsy[buttonIndex].onClick != NULL) {
        buttonsy[buttonIndex].onClick(); // Call the function dynamically
    } else {
        log_to_file("Invalid button index or function not assigned!\n");
    }
}

static bool running = true;

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
		cwavPlay(sfx, 0, 1);  // Stereo sound (2 channels)
	} else {
		cwavPlay(sfx, 0, -1);  // Mono sound (-1 channel)
	}
	//freeC2DImage(&qr);
	generate_qrcode();
	//int otpen = compute_magic_number(secrettotpglobal);
	//doBasicDemo(&qrImage, otpen, userajd);
	
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
		cwavPlay(sfx, 0, 1);  // Stereo sound (2 channels)
	} else {
		cwavPlay(sfx, 0, -1);  // Mono sound (-1 channel)
	}
	CWAV* bgm = cwavList[1].cwav;
	CWAV* menu = cwavList[3].cwav;  // Get the CWAV object for the current sound
	cwavStop(menu, 0, 1);  // Stereo sound (2 channels)
	if (bgm->numChannels == 2) {
		cwavPlay(bgm, 0, 1);  // Stereo sound (2 channels)
	} else {
		cwavPlay(bgm, 0, -1);  // Mono sound (-1 channel)
		
	}
}
// Background thread function
void qr_thread(void* arg) {
    while (running) {
        generate_qrcode(); // Generate QR code
        
        // Sleep for 30 seconds (30 * 1,000,000,000 nanoseconds)
        svcSleepThread(2 * 1000000000LL);
    }
}
int main(int argc, char* argv[]) {
    // Init libs
	cwavEnvMode_t mode = CWAV_ENV_DSP;
	cwavUseEnvironment(mode);
    romfsInit();
	cfguInit(); 
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();
    ndspInit();
	json_t *jsonfl;
    //consoleInit(GFX_BOTTOM, NULL);
    Result ret;
    //
	init_logger();
    SOC_buffer = (u32*)memalign(SOC_ALIGN, SOC_BUFFERSIZE);
	if(SOC_buffer == NULL) {
		printf("memalign: failed to allocate\n");
	}

	// Now intialise soc:u service
	if ((ret = socInit(SOC_buffer, SOC_BUFFERSIZE)) != 0) {
    	printf("socInit: 0x%08X\n", (unsigned int)ret);
	}
	//QRTex qr = { NULL, NULL };
    //from now on socket is initialized and we happy use curl
	//char zappka_name[];
	C2D_TextBuf memBuf = C2D_TextBufNew(128);  // Allocate once
	C2D_Text memtext;
	start_request_thread();
	if (access("/3ds/data.json", F_OK) == 0) {
		isLogged = true;
		
	    jsonfl = json_load_file("/3ds/data.json", 0, NULL);
		json_t *tempajd = json_object_get(jsonfl, "user_id");
		json_t *nwmsecret = json_object_get(jsonfl, "hex_secret");
		json_t *nejmen = json_object_get(jsonfl, "name");
		json_t *refren = json_object_get(jsonfl, "refresh");
		json_t *ajdentokenen = json_object_get(jsonfl, "token");
		nejmenmachen = json_string_value(nejmen);
		refreshtoken = json_string_value(refren);
		id_tokenk = json_string_value(ajdentokenen);
		userajd = json_string_value(tempajd);
		secrettotpglobal = json_string_value(nwmsecret);

		snprintf(combinedText, sizeof(combinedText), "Witaj %s!", nejmenmachen);
		int otpen = compute_magic_number(secrettotpglobal);
		doBasicDemo(&qrImage, otpen, userajd);
		if (!citra_machen){
			if (is_network_connected()) {
				sprawdzajtokenasa(id_tokenk, refreshtoken);
				//updatezappsy(id_tokenk, refreshtoken);
			} else {
			}
		} else {
			sprawdzajtokenasa(id_tokenk, refreshtoken);
		}
	} else {
		isLogged = false;
	}
    char* url;
    char* body;
	
	Thread thread = threadCreate(qr_thread, NULL, 32 * 1024, 0x30, -2, false);
	if (!citra_machen){
		start_internet_thread(); 
	}
	
    // Create screens
    C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    C3D_RenderTarget* bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

	//printf(response);
 
	static SwkbdState swkbd;
	static char mybuf[60];
	static char mybuf2[60];
	static SwkbdStatusData swkbdStatus;
	static SwkbdLearningData swkbdLearning;
	SwkbdButton button = SWKBD_BUTTON_NONE;
	bool didit = false;
	bool swkbdTriggered = false; // Add this flag variable outside of your loop.
	//int Scene;
    Scene = 1;
	bool sceneStarted = false; 
	bool rei = false; //daj to kurwa jako funkcje plz
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
	C2D_TextFontParse(&themeText[0], font[0], themeBuf, themes[0]);
    C2D_TextOptimize(&themeText[0]);
    if (!themeon) {
		background_top = C2D_SpriteSheetLoad("romfs:/gfx/bg.t3x");
		background_down = C2D_SpriteSheetLoad("romfs:/gfx/bottombg.t3x");
		logo = C2D_SpriteSheetLoad("romfs:/gfx/logo.t3x");
		// dawaj button'y
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
    // Initialize sprites
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
	//C2D_TextFontParse(&g_staticText[9], font[0], g_staticBuf, "chuj");
	//C2D_TextOptimize(&g_staticText[9]); 
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
	CWAV* bgm = cwavList[1].cwav;
	CWAV* loginbgm = cwavList[2].cwav;  // Get the CWAV object for the current sound
	CWAV* menu = cwavList[3].cwav;  // Get the CWAV object for the current sound

	if (bgm->numChannels == 2) {
		cwavPlay(bgm, 0, 1);  // Stereo sound (2 channels)
	} else {
		cwavPlay(bgm, 0, -1);  // Mono sound (-1 channel)
		
	}
    // Main loop
	int textOffset = 0;
    int lastTouchY = -1;
	// Place these at the top of your file or in a suitable global/state struct
	float scrollVelocity = 0.0f;
	float friction = 0.9f;
	int prevTouchX = -1;
	bool isDragging = false;
	int dragStartX = -1;
	int dragStartY = -1;
	const int SCROLL_THRESHOLD = 10;

    while(aptMainLoop()) {
		// printf("scena:         %i\n", Scene);
		// printf("tajmer2:         %f\n", timer2);
        //gspWaitForVBlank();
       // gfxSwapBuffers();
        hidScanInput();
        // Respond to user input
        u32 kDown = hidKeysDown();
		u32 kHeld = hidKeysHeld();
        if (kDown & KEY_START) {
           // printf("\n** Quitting... **\n");
            break; // Exit on START button
        }
        touchPosition touch;
        hidTouchRead(&touch);
        // Check for touch input
		sfx = cwavList[0].cwav;  // Assign the CWAV object
		if (przycskmachen) {
			if (kHeld & KEY_TOUCH) {
				if (dragStartX == -1 || dragStartY == -1) {
					// First touch
					dragStartX = touch.px;
					dragStartY = touch.py;
					isDragging = false;
					scrollVelocity = 0.0f; // stop momentum on new touch
				} else {
					int dx = abs(touch.px - dragStartX);
					int dy = abs(touch.py - dragStartY);
					if (dx > SCROLL_THRESHOLD || dy > SCROLL_THRESHOLD) {
						isDragging = true;

						// 🧼 Cancel all pending button presses if drag starts
						for (int i = 0; i < 100; i++) {
							buttonsy[i].isPressed = false;
						}
					}
				}

				if (isDragging) {
					// Perform scrolling
					if (lastTouchY >= 0) {
						int delta = touch.px - lastTouchY;
						textOffset -= delta;
						if (textOffset < 0) textOffset = 0;

						scrollVelocity = (float)delta; // update velocity
					}
					lastTouchY = touch.px;
					prevTouchX = touch.px;
				} else {
					// Not dragging — handle buttons
					for (int i = 0; i < 100; i++) {
						if (buttonsy[i].scene != Scene && buttonsy[i].scene2 != Scene) continue;
						if (isButtonPressed(&buttonsy[i], touch, Scene)) {
							buttonsy[i].isPressed = true;
							break; // Only one button
						}
					}
				}
			} else {
				// Finger lifted
				if (!isDragging) {
					// Only run button release if it was a tap
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
				// drag end — let momentum take over
				lastTouchY = -7;
				dragStartX = -1;
				dragStartY = -1;
				isDragging = false;
				prevTouchX = -1;
			}
		}


		// Apply momentum scrolling when not touching
		if (!(kHeld & KEY_TOUCH) && fabs(scrollVelocity) > 0.1f) {
			textOffset -= scrollVelocity;
			if (textOffset < 0) textOffset = 0;

			scrollVelocity *= friction;

			// Stop if very slow
			if (fabs(scrollVelocity) < 0.1f) {
				scrollVelocity = 0.0f;
			}
		}


        if (kDown & KEY_A) {
            if (Scene == 1 & timer > 40) {
				// Access CWAV object directly from the array
				
				cwavStop(bgm, 0, 1);
				if (sfx->numChannels == 2) {
					cwavPlay(sfx, 0, 1);  // Stereo sound (2 channels)
				} else {
					cwavPlay(sfx, 0, -1);  // Mono sound (-1 channel)
				}
                startY = 0.0f;
				timer = 0.0f;
                endY = -400.0f;   
                elapsed = 0;
				if (!isLogged) {
					Scene = 2;
				} else {
					Scene = 5;
					//cwavPlay(menu, 0, 1);
				}
            } else if (Scene == 2 & timer > 119) {
				// Access CWAV object directly from the array
				
				cwavStop(bgm, 0, 1);
                startY = 0.0f;
				timer = 0.0f;
                endY = -400.0f;   
                elapsed = 0;
                Scene = 3;
				if (sfx->numChannels == 2) {
					cwavPlay(sfx, 0, 1);  // Stereo sound (2 channels)
				} else {
					cwavPlay(sfx, 0, -1);  // Mono sound (-1 channel)
				}
				
				cwavPlay(loginbgm, 0, 1);  // Stereo sound (2 channels)
            }
        }
        if (kDown & KEY_B) {
			if (!schizofrenia) {
				if (Scene == 10 & timer > 40) {
					// Access CWAV object directly from the array
					przycskmachen = true;
					
					if (sfx->numChannels == 2) {
						cwavPlay(sfx, 0, 1);  // Stereo sound (2 channels)
					} else {
						cwavPlay(sfx, 0, -1);  // Mono sound (-1 channel)
					}
					startY = 0.0f;
					timer = 0.0f;
					endY = -400.0f;   
					elapsed = 0;
					Scene = 11;
				}
			}
            if (Scene == 12 & timer > 40) {
				// Access CWAV object directly from the array
				przycskmachen = true;
				
				if (sfx->numChannels == 2) {
					cwavPlay(sfx, 0, 1);  // Stereo sound (2 channels)
				} else {
					cwavPlay(sfx, 0, -1);  // Mono sound (-1 channel)
				}
                startY = 0.0f;
				timer = 0.0f;
                endY = -400.0f;   
                elapsed = 0;
				Scene = 13;
            } 
            if (Scene == 14 & timer > 40) {
				// Access CWAV object directly from the array
				przycskmachen = true;
				
				if (sfx->numChannels == 2) {
					cwavPlay(sfx, 0, 1);  // Stereo sound (2 channels)
				} else {
					cwavPlay(sfx, 0, -1);  // Mono sound (-1 channel)
				}
                startY = 0.0f;
				timer = 0.0f;
                endY = -400.0f;   
                elapsed = 0;
				Scene = 15;
            } 
            if (Scene == 16 & timer > 40) {
				// Access CWAV object directly from the array
				przycskmachen = true;
				
				if (sfx->numChannels == 2) {
					cwavPlay(sfx, 0, 1);  // Stereo sound (2 channels)
				} else {
					cwavPlay(sfx, 0, -1);  // Mono sound (-1 channel)
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
				//log_to_file("max: %d\n", maxThemes);
				//log_to_file("curr: %d\n", selectionthemelol);
				if (selectionthemelol == 0){
					selectionthemelol = maxThemes;
				} else {
					selectionthemelol--;

				}
			}
		}
		if (kDown & KEY_DRIGHT) {
			if (Scene == 16 & timer > 40) {
				//log_to_file("max: %d\n", maxThemes);
				//log_to_file("curr: %d\n", selectionthemelol);
				if (selectionthemelol == maxThemes){
					selectionthemelol = 0;
				} else {
					selectionthemelol++;
				}
			}
		}

        
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        
		
        // Render the scene
        if (Scene == 1) {
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
			//drawShadowedText(&g_staticText[1], 300.0f, 190.0f, 0.5f, 1.5f, 1.5f, C2D_Color32(0x78, 0xc1, 0x91, timer3), C2D_Color32(0xff, 0xff, 0xff, timer3));
			if (timer > 80) {
				if (!themeon){
					drawShadowedText(&g_staticText[2], 200.0f, currentY2 + 90.0f, 0.5f, 1.0f, 1.0f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				} else {
					drawShadowedText(&g_staticText[2], 200.0f, currentY2 + 90.0f, 0.5f, 1.0f, 1.0f, themeBaseColor, themeoutColor);		
				}
			}
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
			//drawShadowedText(&g_staticText[1], 300.0f, 190.0f, 0.5f, 1.5f, 1.5f, C2D_Color32(0x78, 0xc1, 0x91, timer3), C2D_Color32(0xff, 0xff, 0xff, timer3));
			if (timer > 80) {
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
			//drawShadowedText(&g_staticText[1], 300.0f, 190.0f, 0.5f, 1.5f, 1.5f, C2D_Color32(0x78, 0xc1, 0x91, timer3), C2D_Color32(0xff, 0xff, 0xff, timer3));
			if (timer > 80) {
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
            C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(bottom);
            
            C2D_DrawImageAt(bgdown, x, y, 0.0f, NULL, 1.0f, 1.0f);
			for (int i = 0; i < 100; i++) {
				drawButton(&buttonsy[i], Scene);
			}			
			
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));
			// if (timer < 80) {
				// C2D_DrawImageAt(buttonmed, currentY2/2.5f - 30, 70.0f, 0.0f, NULL, 0.5f, 0.5f);
				// drawShadowedText(&g_staticText[3], currentY2/2.5f + 70, 90.0f, 0.5f, 2.0f, 2.0f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0x00, 0x00, 0x00, 0xff));
				// C2D_DrawImageAt(buttonmed, -currentY2/2.5f + 175, 70.0f, 0.0f, NULL, 0.5f, 0.5f);
				// drawShadowedText(&g_staticText[4], -currentY2/2.5f + 240, 90.0f, 0.5f, 2.0f, 2.0f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0x00, 0x00, 0x00, 0xff));
			// }
			
		} else if (Scene == 5) {	
			if (!sceneStarted) {
				// This block runs only once when the scene starts
				cwavStop(loginbgm, 0, 1);
				cwavPlay(menu, 0, 1);
				sceneStarted = true; // Set the flag to true to prevent this block from running again
				isLogged = true;
				//generate_qrcode();
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
				if (!schizofrenia) {
					generatingQR = false;
				} else {
					Scene = 10;
					generatingQR = true;
				}
			} else if (timer < 209) {
				// Access CWAV object directly from the array
				timer += 1.0f;
			} else if (timer > 208) {
				// Access CWAV object directly from the array
				if (schizofrenia) {
					timer = 0.0f;
					generatingQR = true;
					startY = 0.0f;
					elapsed = 0.0f;
					endY = -400.0f; 
					Scene = 10;
				}
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
			//drawShadowedText(&g_staticText[1], 300.0f, 190.0f, 0.5f, 1.5f, 1.5f, C2D_Color32(0x78, 0xc1, 0x91, timer3), C2D_Color32(0xff, 0xff, 0xff, timer3));
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
				Scene = 5;
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
			//drawShadowedText(&g_staticText[1], 300.0f, 190.0f, 0.5f, 1.5f, 1.5f, C2D_Color32(0x78, 0xc1, 0x91, timer3), C2D_Color32(0xff, 0xff, 0xff, timer3));
			if (timer > 80) {
				Scene = 7;
			} else {
				if (!themeon){
					drawShadowedText(&g_staticText[6], 200.0f, currentY2 + 90.0f, 0.5f, 1.0f, 1.0f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				} else {
					drawShadowedText(&g_staticText[6], 200.0f, currentY2 + 90.0f, 0.5f, 1.0f, 1.0f, themeBaseColor, themeoutColor);		
				}
			}
            C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(bottom);
            
            C2D_DrawImageAt(bgdown, x, y, 0.0f, NULL, 1.0f, 1.0f);
			for (int i = 0; i < 100; i++) {
				drawButton(&buttonsy[i], Scene);
			}			
			
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));
			// if (timer < 80) {
				// C2D_DrawImageAt(buttonmed, currentY2/2.5f - 30, 70.0f, 0.0f, NULL, 0.5f, 0.5f);
				// drawShadowedText(&g_staticText[3], currentY2/2.5f + 70, 90.0f, 0.5f, 2.0f, 2.0f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0x00, 0x00, 0x00, 0xff));
				// C2D_DrawImageAt(buttonmed, -currentY2/2.5f + 175, 70.0f, 0.0f, NULL, 0.5f, 0.5f);
				// drawShadowedText(&g_staticText[4], -currentY2/2.5f + 240, 90.0f, 0.5f, 2.0f, 2.0f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0x00, 0x00, 0x00, 0xff));
			// }
			
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
			//drawShadowedText(&g_staticText[1], 300.0f, 190.0f, 0.5f, 1.5f, 1.5f, C2D_Color32(0x78, 0xc1, 0x91, timer3), C2D_Color32(0xff, 0xff, 0xff, timer3));
            C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(bottom);
            
            C2D_DrawImageAt(bgdown, x, y, 0.0f, NULL, 1.0f, 1.0f);
			for (int i = 0; i < 100; i++) {
				drawButton(&buttonsy[i], Scene);
			}			
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));
			// if (timer < 80) {
				// C2D_DrawImageAt(buttonmed, currentY2/2.5f - 30, 70.0f, 0.0f, NULL, 0.5f, 0.5f);
				// drawShadowedText(&g_staticText[3], currentY2/2.5f + 70, 90.0f, 0.5f, 2.0f, 2.0f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0x00, 0x00, 0x00, 0xff));
				// C2D_DrawImageAt(buttonmed, -currentY2/2.5f + 175, 70.0f, 0.0f, NULL, 0.5f, 0.5f);
				// drawShadowedText(&g_staticText[4], -currentY2/2.5f + 240, 90.0f, 0.5f, 2.0f, 2.0f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0x00, 0x00, 0x00, 0xff));
			// }
			
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
			//drawShadowedText(&g_staticText[1], 300.0f, 190.0f, 0.5f, 1.5f, 1.5f, C2D_Color32(0x78, 0xc1, 0x91, timer3), C2D_Color32(0xff, 0xff, 0xff, timer3));
            C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(bottom);
            
            C2D_DrawImageAt(bgdown, x, y, 0.0f, NULL, 1.0f, 1.0f);
			for (int i = 0; i < 100; i++) {
				drawButton(&buttonsy[i], Scene);
			}			
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));
			// if (timer < 80) {
				// C2D_DrawImageAt(buttonmed, currentY2/2.5f - 30, 70.0f, 0.0f, NULL, 0.5f, 0.5f);
				// drawShadowedText(&g_staticText[3], currentY2/2.5f + 70, 90.0f, 0.5f, 2.0f, 2.0f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0x00, 0x00, 0x00, 0xff));
				// C2D_DrawImageAt(buttonmed, -currentY2/2.5f + 175, 70.0f, 0.0f, NULL, 0.5f, 0.5f);
				// drawShadowedText(&g_staticText[4], -currentY2/2.5f + 240, 90.0f, 0.5f, 2.0f, 2.0f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0x00, 0x00, 0x00, 0xff));
			// }
			
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
				// Access CWAV object directly from the array
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
						// buttonsy[0].x = currentY;
						// buttonsy[1].x = currentY; 
						// buttonsy[2].x = -currentY + 140; 
						// buttonsy[3].x = -currentY + 140; 
					}
                    elapsed += deltaTime;
                }
            }
                
            C2D_DrawImageAt(bgtop, x, y, 0.0f, NULL, 1.0f, 1.0f);
			if (timer < 120) {
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
				if (schizofrenia) {
					if (!themeon) {
						drawShadowedText_noncentered(&g_staticText[11], 0.0f, -currentY + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x29, 0x6e, 0x44, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
					} else {
						drawShadowedText_noncentered(&g_staticText[11], 0.0f, -currentY + 215.0f, 0.5f, 0.8f, 0.8f, C2D_Color32(0x74, 0x1d, 0x4a, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
					}
				}
			}
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0xff, 0xff, 0xff, timer2));	
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));
			//drawShadowedText(&g_staticText[1], 300.0f, 190.0f, 0.5f, 1.5f, 1.5f, C2D_Color32(0x78, 0xc1, 0x91, timer3), C2D_Color32(0xff, 0xff, 0xff, timer3));
            C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(bottom);
            
            C2D_DrawImageAt(bgdown, x, y, 0.0f, NULL, 1.0f, 1.0f);
			if (!schizofrenia) {
				C2D_DrawImageAt(gobackplz, currentY + 5.0f, 210.0f, 0.0f, NULL, 1.0f, 1.0f);
			}
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
				// Access CWAV object directly from the array
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
						// buttonsy[0].x = currentY;
						// buttonsy[1].x = currentY; 
						// buttonsy[2].x = -currentY + 140; 
						// buttonsy[3].x = -currentY + 140; 
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
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0xff, 0xff, 0xff, timer2));	
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));
			//drawShadowedText(&g_staticText[1], 300.0f, 190.0f, 0.5f, 1.5f, 1.5f, C2D_Color32(0x78, 0xc1, 0x91, timer3), C2D_Color32(0xff, 0xff, 0xff, timer3));
            C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
            C2D_SceneBegin(bottom);
            
            C2D_DrawImageAt(bgdown, x, y, 0.0f, NULL, 1.0f, 1.0f);
			for (int i = 0; i < 100; i++) {
				drawButton(&buttonsy[i], Scene);
			}
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0xff, 0xff, 0xff, timer2));
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));
			if (!schizofrenia) {
				C2D_DrawImageAt(gobackplz, currentY + 5.0f, 210.0f, 0.0f, NULL, 1.0f, 1.0f);
			}
			
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
				// Access CWAV object directly from the array
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
						// buttonsy[0].x = currentY;
						// buttonsy[1].x = currentY; 
						// buttonsy[2].x = -currentY + 140; 
						// buttonsy[3].x = -currentY + 140; 
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
				//C2D_DrawImageAt(qr_framen_machen, 0.0f, currentY, 0.0f, NULL, 1.0f, 1.0f);
				//C2D_DrawImageAt(qrImage, 112.0f, currentY + 27, 0.0f, NULL, 1.0f, 1.0f);
				
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
			//drawShadowedText(&g_staticText[1], 300.0f, 190.0f, 0.5f, 1.5f, 1.5f, C2D_Color32(0x78, 0xc1, 0x91, timer3), C2D_Color32(0xff, 0xff, 0xff, timer3));
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
					//drawShadowedText(&g_staticText[1], 160.0f, -currentY + 90, 0.5f, 1.6f, 1.6f, C2D_Color32(0xff, 0xff, 0xff, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
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
				// Access CWAV object directly from the array
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
						// buttonsy[0].x = currentY;
						// buttonsy[1].x = currentY; 
						// buttonsy[2].x = -currentY + 140; 
						// buttonsy[3].x = -currentY + 140; 
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
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0xff, 0xff, 0xff, timer2));	
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));
			//drawShadowedText(&g_staticText[1], 300.0f, 190.0f, 0.5f, 1.5f, 1.5f, C2D_Color32(0x78, 0xc1, 0x91, timer3), C2D_Color32(0xff, 0xff, 0xff, timer3));
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
				// Access CWAV object directly from the array
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
						buttonsy[4].y = -currentY + 140; 
						// buttonsy[0].x = currentY;
						// buttonsy[1].x = currentY; 
						// buttonsy[2].x = -currentY + 140; 
						// buttonsy[3].x = -currentY + 140; 
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
				//C2D_DrawImageAt(qr_framen_machen, 0.0f, currentY, 0.0f, NULL, 1.0f, 1.0f);
				//C2D_DrawImageAt(qrImage, 112.0f, currentY + 27, 0.0f, NULL, 1.0f, 1.0f);
				
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


			//drawShadowedText(&g_staticText[1], 300.0f, 190.0f, 0.5f, 1.5f, 1.5f, C2D_Color32(0x78, 0xc1, 0x91, timer3), C2D_Color32(0xff, 0xff, 0xff, timer3));
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
				// C2D_DrawImageAt(zappsybaren, 0.0f, -currentY, 0.0f, NULL, 1.0f, 1.0f);
				// if (json_done) {
					// drawShadowedText(&g_staticText[9], 160.0f, -currentY + 80, 0.5f, 2.3f, 2.3f, C2D_Color32(0xff, 0xff, 0xff, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
					// //drawShadowedText(&g_staticText[1], 160.0f, -currentY + 90, 0.5f, 1.6f, 1.6f, C2D_Color32(0xff, 0xff, 0xff, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				// } else {
					// drawShadowedText(&g_staticText[1], 160.0f, -currentY + 93, 0.5f, 1.6f, 1.6f, C2D_Color32(0xff, 0xff, 0xff, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
					// if (global_response && strstr(global_response, "points") != NULL) {
						// process_json_response();
						// rebuild_buffer();
						// json_done = true;
					// }
				// }
				if (json_done) {
					if (!ofertanow) {
						for (int i = 0; i < tileCount + 1; i++) {
							// Calculate the button's position
							float buttonX = 5.0f + (i * 142.0f) - textOffset;
							float buttonY = currentY + 25.0f;

							// Draw the button image
							//C2D_DrawImageAt(couponbutton, buttonX, buttonY, 0.0f, NULL, 1.0f, 1.0f);
							//log_to_file("Button %zu", i + 3);
							buttonsy[i + 5].x = buttonX;
							buttonsy[i + 5].y = buttonY; 

							// Position the text so it's centered inside the button
							float textX = buttonX + 65.0f;
							float textY = buttonY + 140.0f;
							// Draw the shadowed text inside the button
							if (!themeon){
								drawShadowedText(&g_kuponText[i], textX, textY, 0.5f, 0.85f, 0.85f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
							} else {
								drawShadowedText(&g_kuponText[i], textX, textY, 0.5f, 0.85f, 0.85f, themeBaseColor, themeoutColor);
							}
						}
					} else {
						float textX = 160.0f;
						float textY = currentY + 25.0f + 30.0f;
						// Draw the shadowed text inside the button
						if (!themeon){
							drawShadowedTextWrapped(&g_kuponText[0], textX, textY, 0.5f, 0.6f, 0.6f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
							drawShadowedTextWrapped(&g_kuponText[1], textX, textY - 50.0f, 0.5f, 1.5f, 1.5f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
						} else {
							drawShadowedTextWrapped(&g_kuponText[0], textX, textY, 0.5f, 0.6f, 0.6f, themeBaseColor, themeoutColor);
							drawShadowedTextWrapped(&g_kuponText[1], textX, textY - 50.0f, 0.5f, 1.5f, 1.5f, themeBaseColor, themeoutColor);
						}
						if (!canredeem) {
							 C2D_DrawImageAt(za_malo, 40.0f, -currentY + 140, 0.0f, NULL, 1.0f, 1.0f);
						}
						if (aktywacja_done) {
							if (!themeon){
								drawShadowedTextWrapped(&g_staticText[13], textX, textY + 100.0f, 0.5f, 1.5f, 1.5f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
							} else {
								drawShadowedTextWrapped(&g_staticText[13], textX, textY + 100.0f, 0.5f, 1.5f, 1.5f, themeBaseColor, themeoutColor);
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
						//zabkazonefeed = global_response;
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
				// Access CWAV object directly from the array
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
						buttonsy[4].y = -currentY + 140; 
						
					}
					if (timer > 120) {
						currentY = easeOutQuad(elapsed, startY, endY, duration);
						// buttonsy[0].x = currentY;
						// buttonsy[1].x = currentY; 
						// buttonsy[2].x = -currentY + 140; 
						// buttonsy[3].x = -currentY + 140; 
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


			//drawShadowedText(&g_staticText[1], 300.0f, 190.0f, 0.5f, 1.5f, 1.5f, C2D_Color32(0x78, 0xc1, 0x91, timer3), C2D_Color32(0xff, 0xff, 0xff, timer3));
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
				// C2D_DrawImageAt(zappsybaren, 0.0f, -currentY, 0.0f, NULL, 1.0f, 1.0f);
				// if (json_done) {
					// drawShadowedText(&g_staticText[9], 160.0f, -currentY + 80, 0.5f, 2.3f, 2.3f, C2D_Color32(0xff, 0xff, 0xff, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
					// //drawShadowedText(&g_staticText[1], 160.0f, -currentY + 90, 0.5f, 1.6f, 1.6f, C2D_Color32(0xff, 0xff, 0xff, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
				// } else {
					// drawShadowedText(&g_staticText[1], 160.0f, -currentY + 93, 0.5f, 1.6f, 1.6f, C2D_Color32(0xff, 0xff, 0xff, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
					// if (global_response && strstr(global_response, "points") != NULL) {
						// process_json_response();
						// rebuild_buffer();
						// json_done = true;
					// }
				// }
				if (json_done) {
					if (!ofertanow) {
						for (int i = 0; i < tileCount + 1; i++) {
							// Calculate the button's position
							float buttonX = 5.0f + (i * 142.0f) - textOffset;
							float buttonY = currentY + 25.0f;

							// Draw the button image
							//C2D_DrawImageAt(couponbutton, buttonX, buttonY, 0.0f, NULL, 1.0f, 1.0f);
							//log_to_file("Button %zu", i + 3);
							buttonsy[i + 5].x = buttonX;
							buttonsy[i + 5].y = buttonY; 

							// Position the text so it's centered inside the button
							float textX = buttonX + 65.0f;
							float textY = buttonY + 140.0f;
							// Draw the shadowed text inside the button
							if (!themeon){
								drawShadowedText(&g_kuponText[i], textX, textY, 0.5f, 0.85f, 0.85f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
							} else {
								drawShadowedText(&g_kuponText[i], textX, textY, 0.5f, 0.85f, 0.85f, themeBaseColor, themeoutColor);
							}
						}
					} else {
						float textX = 160.0f;
						float textY = currentY + 25.0f + 30.0f;
						if (!themeon){
							drawShadowedTextWrapped(&g_kuponText[0], textX, textY, 0.5f, 0.6f, 0.6f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
							drawShadowedTextWrapped(&g_kuponText[1], textX, textY - 50.0f, 0.5f, 1.5f, 1.5f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
						} else {
							drawShadowedTextWrapped(&g_kuponText[0], textX, textY, 0.5f, 0.6f, 0.6f, themeBaseColor, themeoutColor);
							drawShadowedTextWrapped(&g_kuponText[1], textX, textY - 50.0f, 0.5f, 1.5f, 1.5f, themeBaseColor, themeoutColor);
						}
						if (!canredeem) {
							 C2D_DrawImageAt(za_malo, 40.0f, -currentY + 140, 0.0f, NULL, 1.0f, 1.0f);
						}
						if (aktywacja_done) {
							if (!themeon){
								drawShadowedTextWrapped(&g_staticText[13], textX, textY + 100.0f, 0.5f, 1.5f, 1.5f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
							} else {
								drawShadowedTextWrapped(&g_staticText[13], textX, textY + 100.0f, 0.5f, 1.5f, 1.5f, themeBaseColor, themeoutColor);
							}
						}
					}

				} else {
					drawShadowedText(&g_staticText[1], 160.0f, -currentY + 93, 0.5f, 1.6f, 1.6f, C2D_Color32(0x78, 0xc1, 0x91, 0xff), C2D_Color32(0xff, 0xff, 0xff, 0xff));
					if (!loadingshit) {
						//zabkazonefeed = global_response;
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
				// Access CWAV object directly from the array
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
						// buttonsy[1].x = currentY; 
						// buttonsy[2].x = -currentY + 140; 
						// buttonsy[3].x = -currentY + 140; 
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
				//C2D_DrawImageAt(qr_framen_machen, 0.0f, currentY, 0.0f, NULL, 1.0f, 1.0f);
				//C2D_DrawImageAt(qrImage, 112.0f, currentY + 27, 0.0f, NULL, 1.0f, 1.0f);
				
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
			//drawShadowedText(&g_staticText[1], 300.0f, 190.0f, 0.5f, 1.5f, 1.5f, C2D_Color32(0x78, 0xc1, 0x91, timer3), C2D_Color32(0xff, 0xff, 0xff, timer3));
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
				// Access CWAV object directly from the array
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
						// buttonsy[1].x = currentY; 
						// buttonsy[2].x = -currentY + 140; 
						// buttonsy[3].x = -currentY + 140; 
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
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0xff, 0xff, 0xff, timer2));	
			C2D_DrawRectSolid(0.0f,0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT, C2D_Color32(0x00, 0x00, 0x00, timer3));
			//drawShadowedText(&g_staticText[1], 300.0f, 190.0f, 0.5f, 1.5f, 1.5f, C2D_Color32(0x78, 0xc1, 0x91, timer3), C2D_Color32(0xff, 0xff, 0xff, timer3));
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


		
		C3D_FrameEnd(0);
	} 
	close_logger();
    running = false; // Stop the thread
    threadJoin(thread, UINT64_MAX); // Wait for thread to finish
    threadFree(thread); // Clean up thread resources
    internet_thread_running = false;
    threadJoin(internet_thread, UINT64_MAX);
    threadFree(internet_thread);
	stop_request_thread();
	// Free resources
	free(secrettotpglobal);
	C2D_SpriteSheetFree(background_top);
	C2D_SpriteSheetFree(background_down);
	C2D_SpriteSheetFree(logo);
	//C2D_SpriteSheetFree(buttons);
	C2D_SpriteSheetFree(points);
	C2D_SpriteSheetFree(scan);
	C2D_SpriteSheetFree(settings);
	C2D_SpriteSheetFree(qrframe);
	
    // Cleanup audio things and de-init platform features
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
