#include "zappka_totp_qr.h"
#include "logs.h"
#define JAVA_INT_MAX 0x7FFFFFFF
#define BLOCK_SIZE 64  // SHA-1 uses a 64-byte block
#define OUTPUT_SIZE 20 // SHA-1 produces a 20-byte hash
#define BORDER 1 // Define the border size around the QR code
QRTex qr; 
C2D_Image qrImage;
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

    // Initialize HMAC context
    mbedtls_md_init(&ctx);
    info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA1);
    if (info == NULL) {
        printf("SHA1 not supported\n");
        return;
    }

    // Setup HMAC
    mbedtls_md_setup(&ctx, info, 1);
    mbedtls_md_hmac_starts(&ctx, key, key_len);
    mbedtls_md_hmac_update(&ctx, message, message_len);
    mbedtls_md_hmac_finish(&ctx, output);

    // Cleanup
    mbedtls_md_free(&ctx);
}

// Function to convert 4 bytes to an integer
uint32_t ctotp(const uint8_t *arr, size_t index) {
    uint32_t result = 0;
    for (size_t i = index; i < index + 4; i++) {
        result = (result << 8) | (arr[i] & 0xFF);
    }
    return result;
}

// Function to compute the magic number
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

    time_t t = time(NULL) - 3600;
    int ts = (int)(t / 30);
    
   // FILE* fptr = fopen("filename.txt", "a");
    // if (fptr == NULL) {
        // perror("Failed to open file");
        // free(secret);
        // return -1;
    // }
	//log_to_file("Secret bytes: ");
	for (int i = 0; i < secretLen; i++) {
		//log_to_file("%02X ", secret[i]);  // Print in uppercase hex format
	}
	//log_to_file("\n");

    //log_to_file("Timestamp: %lld, TS: %d\n", t, ts);

    unsigned char msg[8];
    for (int i = 0; i < 8; i++) {
        msg[i] = (ts >> (8 * (7 - i))) & 0xFF;
    }

    //log_to_file("Big-endian timestamp: ");
    for (int i = 0; i < 8; i++) {
        //log_to_file("%02X ", msg[i]);
    }
    //log_to_file("\n");

    unsigned char outputBytes[20];
    hmac_sha1(secret, secretLen, msg, sizeof(msg), outputBytes);

    int offset = outputBytes[19] & 0xF;
	int extractedValue = ctotp(outputBytes, offset);
	//log_to_file("Extracted Value (offset %d): %d\n", offset, extractedValue);
	int extractedValue2 = (ctotp(outputBytes, offset) & JAVA_INT_MAX);
	//log_to_file("Extracted Value and javant(offset %d): %d\n", offset, extractedValue2);
    int magicNumber = (ctotp(outputBytes, offset) & JAVA_INT_MAX) % 1000000;
	//log_to_file("totp: %d\n", magicNumber);
    //log_to_file("HMAC: ");
    //for (int i = 0; i < 20; i++) fprintf(fptr, "%02X ", outputBytes[i]);
    //log_to_file("\nOffset: %d\n", offset);

    free(secret);
    //fclose(fptr);
    return (int)(magicNumber);
}

//thx to piepie for the swizzle code :3
void swizzleTexture(uint8_t* dst, const uint8_t* src, int width, int height) {
    int rowblocks = (width >> 3);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Compute the swizzled index
            u32 swizzledIndex = ((((y >> 3) * rowblocks + (x >> 3)) << 6) + 
                                ((x & 1) | ((y & 1) << 1) | ((x & 2) << 1) | 
                                ((y & 2) << 2) | ((x & 4) << 2) | ((y & 4) << 3)));

            // Copy pixel data
            dst[swizzledIndex * 4]     = src[(y * width + x) * 4];     // R
            dst[swizzledIndex * 4 + 1] = src[(y * width + x) * 4 + 1]; // G
            dst[swizzledIndex * 4 + 2] = src[(y * width + x) * 4 + 2]; // B
            dst[swizzledIndex * 4 + 3] = src[(y * width + x) * 4 + 3]; // A
        }
    }
}
void freeC2DImage(QRTex *qr) {
    if (qr) {
        if (qr->buff) {
            C3D_TexDelete(qr->buff);  // Delete the front buffer texture
            free(qr->buff);           // Free the C3D_Tex structure
        }
        if (qr->backbuff) {
            C3D_TexDelete(qr->backbuff);  // Delete the back buffer texture
            free(qr->backbuff);           // Free the C3D_Tex structure
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

    // Only (re)allocate if size changes
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

                    buffer[index] = 0xFF;       // Alpha
                    buffer[index + 1] = color;  // Green
                    buffer[index + 2] = color;  // Blue
                    buffer[index + 3] = color;  // Red
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
    
    // Open the file for writing
    FILE* fptr = fopen("filename.txt", "w");
    if (fptr == NULL) {
        perror("Failed to open file");
        return;
    }

    // Loop through each module of the QR code and print to the file
    for (int y = -border; y < size + border; y++) {
        for (int x = -border; x < size + border; x++) {
            fprintf(fptr, (qrcodegen_getModule(qrcode, x, y) ? "##" : "  "));
        }
        fprintf(fptr, "\n");
    }

    // Close the file
    fclose(fptr);
}
bool doBasicDemo(C2D_Image* qrImage, int totp, const char* usajd) {
    char text[256]; // Buffer for the formatted string
    snprintf(text, sizeof(text), "https://srln.pl/view/dashboard?ploy=%s&loyal=%06d", usajd, totp);
    FILE* fptr = fopen("filename.txt", "a");
    fprintf(fptr, "https://srln.pl/view/dashboard?ploy=%s&loyal=%06d", usajd, totp);
    fclose(fptr);
    //enum qrcodegen_Ecc errCorLvl = qrcodegen_Ecc_HIGH;  // Error correction level

    // Make and print the QR Code symbol
    uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
    uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
    bool ok = qrcodegen_encodeText(text, tempBuffer, qrcode, qrcodegen_Ecc_HIGH, qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_0, true);
    if (ok) {
		//printQr(qrcode);
        createQrImage(qrcode, &qr, qrImage);
    }
    return ok;
}

