#include "zappka_totp_qr.h"
#include "logs.h"
#define JAVA_INT_MAX 0x7FFFFFFF
#define BLOCK_SIZE 64
#define OUTPUT_SIZE 20
#define BORDER 1
QRTex qr; 
C2D_Image qrImage;
const char *usernan;
static time_t onl = 0;
static time_t last_loc = 0;
static bool czas_cached = false;
extern time_t czas_wygasniecia;
extern const char *id_tokenk;
extern char *refreshtoken;
void qr_swap_buf(QRTex *qr) {
    C3D_Tex *tmp;
    tmp = qr->buff;
    qr->buff = qr->backbuff;
    qr->backbuff = tmp;
}

void hmac_sha1(const unsigned char *key, size_t key_len, 
               const unsigned char *message, size_t message_len, 
               unsigned char *output) {
    mbedtls_md_context_t ctx;
    const mbedtls_md_info_t *info;
    mbedtls_md_init(&ctx);
    info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA1);
    if (info == NULL) {
        printf("SHA1 not supported\n");
        return;
    }
    mbedtls_md_setup(&ctx, info, 1);
    mbedtls_md_hmac_starts(&ctx, key, key_len);
    mbedtls_md_hmac_update(&ctx, message, message_len);
    mbedtls_md_hmac_finish(&ctx, output);
    mbedtls_md_free(&ctx);
}
uint32_t ctotp(const uint8_t *arr, size_t index) {
    uint32_t result = 0;
    for (size_t i = index; i < index + 4; i++) {
        result = (result << 8) | (arr[i] & 0xFF);
    }
    return result;
}
time_t calibrate_czas() {
	if (!czas_cached) {
		json_t *czasfl = json_load_file("/3ds/czas.json", 0, NULL);
		if (czasfl) {
			onl = json_integer_value(json_object_get(czasfl, "onlineczas"));
			last_loc = json_integer_value(json_object_get(czasfl, "localczas"));
			json_decref(czasfl);
			czas_cached = true;
		} else {
			// Don't cache, so we try again next call
			onl = 0;
			last_loc = time(NULL);
			// czas_cached = false;
		}
	}

	time_t now = time(NULL);
	time_t offset = now - last_loc;
	time_t calibrated = onl + offset;

	// UI updates
	char stext[64], lstext[64], lsotext[64];
	snprintf(stext, sizeof(stext), "SV_TS: %lld", onl);
	C2D_TextParse(&g_totpText[2], totpBuf, stext);
	C2D_TextOptimize(&g_totpText[2]);

	snprintf(lstext, sizeof(lstext), "L_TS: %lld", last_loc);
	C2D_TextParse(&g_totpText[3], totpBuf, lstext);
	C2D_TextOptimize(&g_totpText[3]);

	snprintf(lsotext, sizeof(lsotext), "TS_OFFSET: %lld", calibrated);
	C2D_TextParse(&g_totpText[4], totpBuf, lsotext);
	C2D_TextOptimize(&g_totpText[4]);

	return calibrated;
}


void save_calczas() {
	time_t now = time(NULL);
	time_t new_onl = onl + (now - last_loc);

	json_t *czasfl = json_object(); // Don't reload, just write
	json_object_set_new(czasfl, "onlineczas", json_integer(new_onl));
	json_object_set_new(czasfl, "localczas", json_integer(now));
	json_dump_file(czasfl, "/3ds/czas.json", JSON_COMPACT);
	json_decref(czasfl);

	// Update cached values
	onl = new_onl;
	last_loc = now;
	czas_cached = true;
}

int compute_magic_number(const char *secretHex) {
    int secretLen = strlen(secretHex) / 2;
    unsigned char *secret = malloc(secretLen);
    if (!secret) {
        perror("no sory totp se dziś ne policzysz, pamięć jebła");
        return -1;
    }
    for (int i = 0; i < secretLen; i++) {
        sscanf(secretHex + 2 * i, "%2hhx", &secret[i]);
    }

    time_t t = calibrate_czas();
	if (t > czas_wygasniecia) {
		sprawdzajtokenasa(id_tokenk, refreshtoken);
	}
	//save_calczas();
    int ts = (int)(t / 30);
	for (int i = 0; i < secretLen; i++) {
	}

    unsigned char msg[8];
    for (int i = 0; i < 8; i++) {
        msg[i] = (ts >> (8 * (7 - i))) & 0xFF;
    }
    for (int i = 0; i < 8; i++) {
    }

    unsigned char outputBytes[20];
    hmac_sha1(secret, secretLen, msg, sizeof(msg), outputBytes);

    int offset = outputBytes[19] & 0xF;
	int extractedValue = ctotp(outputBytes, offset);
	int extractedValue2 = (ctotp(outputBytes, offset) & JAVA_INT_MAX);
    int magicNumber = (ctotp(outputBytes, offset) & JAVA_INT_MAX) % 1000000;
	C2D_TextBufClear(totpBuf);
	char totptext[64];
	snprintf(totptext, sizeof(totptext), "TOTP: %06d", magicNumber);
	C2D_TextParse(&g_totpText[0], totpBuf, totptext);
	C2D_TextOptimize(&g_totpText[0]);
	char tstext[64];
	snprintf(tstext, sizeof(tstext), "TS: %lld", t);
	C2D_TextParse(&g_totpText[1], totpBuf, tstext);
	C2D_TextOptimize(&g_totpText[1]);
    free(secret);
    return (int)(magicNumber);
}
void swizzleTexture(uint8_t* dst, const uint8_t* src, int width, int height) {
    int rowblocks = (width >> 3);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            u32 swizzledIndex = ((((y >> 3) * rowblocks + (x >> 3)) << 6) + 
                                ((x & 1) | ((y & 1) << 1) | ((x & 2) << 1) | 
                                ((y & 2) << 2) | ((x & 4) << 2) | ((y & 4) << 3)));
            dst[swizzledIndex * 4]     = src[(y * width + x) * 4];
            dst[swizzledIndex * 4 + 1] = src[(y * width + x) * 4 + 1];
            dst[swizzledIndex * 4 + 2] = src[(y * width + x) * 4 + 2];
            dst[swizzledIndex * 4 + 3] = src[(y * width + x) * 4 + 3];
        }
    }
}
void freeC2DImage(QRTex *qr) {
    if (qr) {
        if (qr->buff) {
            C3D_TexDelete(qr->buff);
            free(qr->buff);
        }
        if (qr->backbuff) {
            C3D_TexDelete(qr->backbuff);
            free(qr->backbuff);
        }
    }
}
void createQrImage(const uint8_t qrcode[], QRTex *qr, C2D_Image *img) {
    static C3D_Tex* tex = NULL;
    static Tex3DS_SubTexture* subtex = NULL;
    static uint8_t* persistentSwizzledBuffer = NULL;
    static uint8_t* buffer = NULL;
    static int currentTexSize = 0;

    int size = qrcodegen_getSize(qrcode);
    int imgSize = (size + 2 * BORDER) * 4;

    int texSize = 1;
    while (texSize < imgSize) texSize *= 2;

    size_t totalBytes = texSize * texSize * 4;
    log_to_file("Using %zu bytes for QR texture", totalBytes);
    if (texSize != currentTexSize) {
        if (buffer) linearFree(buffer);
        buffer = (uint8_t*)linearMemAlign(totalBytes, 0x80);

        if (!buffer) {
            log_to_file("Failed to alloc main buffer (%zu bytes)", totalBytes);
            return;
        }

        if (persistentSwizzledBuffer) linearFree(persistentSwizzledBuffer);
        persistentSwizzledBuffer = (uint8_t*)linearMemAlign(totalBytes, 0x80);

        if (!persistentSwizzledBuffer) {
            log_to_file("Persistent swizzle buffer allocation failed");
            linearFree(buffer);
            buffer = NULL;
            return;
        }

        if (tex) {
            C3D_TexDelete(tex);
            free(tex);
        }
        tex = (C3D_Tex*)malloc(sizeof(C3D_Tex));
        if (!tex || !C3D_TexInit(tex, texSize, texSize, GPU_RGBA8)) {
            log_to_file("Failed to init texture");
            if (tex) free(tex);
            tex = NULL;
            linearFree(buffer);
            linearFree(persistentSwizzledBuffer);
            buffer = NULL;
            persistentSwizzledBuffer = NULL;
            return;
        }

        currentTexSize = texSize;
    }

    memset(buffer, 0xFF, texSize * texSize * 4);

    for (int y = 0; y < size + 2 * BORDER; y++) {
        for (int x = 0; x < size + 2 * BORDER; x++) {
            int px = x * 4;
            int py = y * 4;
            uint8_t color = (x >= BORDER && x < size + BORDER && y >= BORDER && y < size + BORDER && qrcodegen_getModule(qrcode, x - BORDER, y - BORDER)) ? 0x00 : 0xFF;

            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    int newX = (y * 4 + i);
                    int newY = (texSize - px - j - 1);
                    int index = (newY * texSize + newX) * 4;

                    buffer[index] = 0xFF;
                    buffer[index + 1] = color;
                    buffer[index + 2] = color;
                    buffer[index + 3] = color;
                }
            }
        }
    }

    swizzleTexture(persistentSwizzledBuffer, buffer, texSize, texSize);
    C3D_TexUpload(tex, persistentSwizzledBuffer);

    qr->backbuff = tex;
    qr_swap_buf(qr);

    if (!subtex) {
        subtex = (Tex3DS_SubTexture*)malloc(sizeof(Tex3DS_SubTexture));
        if (!subtex) {
            log_to_file("subtex alloc failed");
            return;
        }
    }

    subtex->width = (float)(imgSize);
    subtex->height = (float)(imgSize);
    float offset = 0.131f;
    subtex->left = (float)(texSize - imgSize) / (float)(texSize * 2) - offset;
    subtex->right = (float)(texSize + imgSize) / (float)(texSize * 2) - offset;
    subtex->top = 0.0f;
    subtex->bottom = (float)(imgSize) / (float)(texSize);

    img->tex = qr->buff;
    img->subtex = subtex;
}




void printQr(const uint8_t qrcode[]) {
    int size = qrcodegen_getSize(qrcode);
    int border = 4;
    FILE* fptr = fopen("filename.txt", "w");
    if (fptr == NULL) {
        perror("Failed to open file");
        return;
    }
    for (int y = -border; y < size + border; y++) {
        for (int x = -border; x < size + border; x++) {
            fprintf(fptr, (qrcodegen_getModule(qrcode, x, y) ? "##" : "  "));
        }
        fprintf(fptr, "\n");
    }
    fclose(fptr);
}
bool doBasicDemo(C2D_Image* qrImage, int totp, const char* usajd) {
    char text[256];
	if (strcmp(usernan, "no") == 0) {
		snprintf(text, sizeof(text), "https://srln.pl/view/dashboard?ploy=%s&loyal=%06d", usajd, totp);
	} else {
		snprintf(text, sizeof(text), "https://srln.pl/view/dashboard?ploy=%s&pay=%06d", usajd, totp);
	}
    FILE* fptr = fopen("filename.txt", "a");
    fprintf(fptr, "https://srln.pl/view/dashboard?ploy=%s&loyal=%06d", usajd, totp);
    fclose(fptr);
    uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
    uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
    bool ok = qrcodegen_encodeText(text, tempBuffer, qrcode, qrcodegen_Ecc_HIGH, qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_0, true);
    if (ok) {
        createQrImage(qrcode, &qr, qrImage);
    }
    return ok;
}

