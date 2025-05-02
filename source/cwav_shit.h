#ifndef CWAV_SHIT_H
#define CWAV_SHIT_H
#include <cwav.h>
#include <ncsnd.h>
#include <curl/curl.h>
#include <malloc.h>


void print_u32_binary(u32 val);

typedef struct {
    char* filename;
    CWAV* cwav;
} CWAVInfo;

extern CWAV* sfx;
void populateCwavList();

void freeCwavList();

#endif