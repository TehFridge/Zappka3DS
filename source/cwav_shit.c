#include "cwav_shit.h"

const char* fileList[] = {
    "romfs:/sfx_1.bcwav", //0
	"romfs:/bgm.bcwav", //1
	"romfs:/login.bcwav", //2
	"romfs:/day.bcwav", //3
	"romfs:/splash.bcwav", //4
	"romfs:/night.bcwav", //5
	"romfs:/brak_konta.bcwav", //6
	"romfs:/glodny.bcwav", //7
	"romfs:/lody.bcwav", //8
	"romfs:/menu_kuponow.bcwav", //9
	"romfs:/napoje.bcwav", //10
	"romfs:/piwo.bcwav", //11
	"romfs:/qr.bcwav", //12
	"romfs:/slodycze.bcwav", //13
	"romfs:/sms.bcwav", //14
	"romfs:/snacki.bcwav", //15
	"romfs:/telefon.bcwav", //16
	"romfs:/title_screen.bcwav", //17
	"romfs:/zabka_cafe.bcwav", //18
};

const u8 maxSPlayList[] = {
    2, 2, 2, 14, 1, 14, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

const char* bit_str[] = {
    "0000", "0001", "0010", "0011",
    "0100", "0101", "0110", "0111",
    "1000", "1001", "1010", "1011",
    "1100", "1101", "1110", "1111"
};

void print_u32_binary(u32 val) {
    for (u32 i = 0; i < 4; i++) {
        u32 toprint = val >> ((3 - i) * 8);
        printf("%s%s", bit_str[(toprint >> 4) & 0xF], bit_str[toprint & 0x0F]);
    }
}


CWAVInfo cwavList[99]; 
int cwavCount = 0;
CWAV* sfx = NULL;  

void populateCwavList() {
    for (u32 i = 0; i < sizeof(fileList) / sizeof(char*); i++) {
        CWAV* cwav = (CWAV*)malloc(sizeof(CWAV));

        FILE* file = fopen(fileList[i], "rb");
        if (!file) {
            cwavFree(cwav);
            free(cwav);
            continue;
        }

        fseek(file, 0, SEEK_END);
        u32 fileSize = ftell(file);
        void* buffer = linearAlloc(fileSize);
        if (!buffer)
            svcBreak(USERBREAK_PANIC);

        fseek(file, 0, SEEK_SET); 
        fread(buffer, 1, fileSize, file);
        fclose(file);

        cwavLoad(cwav, buffer, maxSPlayList[i]);
        cwav->dataBuffer = buffer;

        if (cwav->loadStatus == CWAV_SUCCESS) {
            cwavList[cwavCount].filename = fileList[i];
            cwavList[cwavCount].cwav = cwav;
            cwavCount++;
        } else {
            cwavFree(cwav);
            linearFree(cwav->dataBuffer);
            free(cwav);
        }
    }
}

void freeCwavList() {
    for (int i = 0; i < cwavCount; i++) {
        cwavFree(cwavList[i].cwav);
        linearFree(cwavList[i].cwav->dataBuffer);
        free(cwavList[i].cwav);
    }
}