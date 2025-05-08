#ifndef ZAPPKA_QR_TOTP_H
#define ZAPPKA_QR_TOTP_H

#include "qrcodegen.h"
#include <mbedtls/md.h>
#include <citro2d.h>
#include "zappka_totp_qr.h"
#include <jansson.h>
typedef struct {
    C3D_Tex *buff;
    C3D_Tex *backbuff;
} QRTex;
extern C2D_TextBuf totpBuf;
extern C2D_Text g_totpText[5];
extern QRTex qr;
extern C2D_Image qrImage;
void qr_swap_buf(QRTex *qr);
void hmac_sha1(const unsigned char *key, size_t key_len, const unsigned char *message, size_t message_len, unsigned char *output);
uint32_t ctotp(const uint8_t *arr, size_t index);
void save_calczas();
int compute_magic_number(const char *secretHex);
void swizzleTexture(uint8_t* dst, const uint8_t* src, int width, int height);
void freeC2DImage(QRTex *qr);
void createQrImage(const uint8_t qrcode[], QRTex *qr, C2D_Image *img);
void printQr(const uint8_t qrcode[]);
bool doBasicDemo(C2D_Image* qrImage, int totp, const char* usajd);

#endif
