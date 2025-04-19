#ifndef ZAPPKA_API_PROCESSING_H
#define ZAPPKA_API_PROCESSING_H
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <3ds.h>
#include <stddef.h>
#include <stdarg.h>
#include <jansson.h>
#include "cwav_shit.h"
#include "request.h"
#include "buttons.h"
#include "zappka_totp_qr.h"
#define LOG_FILE "curl_log.txt"
extern bool offermachen;
extern bool categoryornah;
extern char *ploy_iden;
extern char ploy_iden_2[25];
extern bool activated;
extern bool aktywacja_done;
typedef struct {
    void* data;
    size_t size;
} ResponseMachen;
void removeTrailingNewline(char* str);
void wrapText(const char* input, int maxWidth, char* output);
void updateprom();
void update_promki(const char* mejntoken, const char* refrenentokenenkurwen);
void loadmore_promki(const char* mejntoken, const char* refrenentokenenkurwen, const char* aftermachen, const char* afterajd);
void activate_coupon(const char* mejntoken, const char* refrenentokenenkurwen);
void process_ids(int selectedbutton);
void process_kupony();
void process_category();
void aktywujkurwe();
void process_ofertamachen();
void process_ployoffers();
void updatezappsy(const char* mejntoken, const char* refrenentokenenkurwen);
void updateploy(const char* mejntoken, const char* refrenentokenenkurwen);
void sprawdzajtokenasa(const char* mejntoken, const char* refrenentokenenkurwen); 
void login_flow(const char *phone_number, const char *verification_code);
#endif // ZAPPKA_API_PROCESSING_H