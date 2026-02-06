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
bool loadCwavIndex(u32 index);
void unloadCwavIndex(u32 index);
void initCwavSystem(void);

void freeAllCwavs(void);
void playCwav(u32 index,bool stereo);
void stopCwav(u32 index);
#endif