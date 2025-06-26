#ifndef REQUEST_H
#define REQUEST_H
#include <citro2d.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <3ds.h>
#include <stdarg.h>
#include "logs.h"
#include <curl/curl.h>
extern C2D_SpriteSheet kuponobraz;
extern C2D_Image kuponkurwa;
extern bool obrazekdone;
extern char *response;
extern bool schizofrenia;
extern long response_code;
extern bool requestdone;
extern bool loadingshit;
extern LightLock global_response_lock;
extern bool czasfuckup;
typedef struct {
    char url[256];         
    char *data;             
    struct curl_slist *headers; 
    void **response;        
    size_t *response_size; 
    LightEvent *event;     
    bool is_binary;         
    bool owns_data;         
} Request;
typedef struct {
    char *data;
    size_t size;
} ResponseBuffer;

extern ResponseBuffer global_response;

size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata);
void free_global_response();
void init_global_response();
void log_message(const char *format, ...);
void safe_free_global_response();
void log_request_to_file(const char *url, const char *data, struct curl_slist *headers, char *response);
void load_image();
bool refresh_data(const char *url, const char *data, struct curl_slist *headers);
void request_worker(void* arg);

void queue_request(const char *url, const char *data, struct curl_slist *headers, void **response, size_t *response_size, LightEvent *event, bool is_binary);
void start_request_thread();
void stop_request_thread();

#endif