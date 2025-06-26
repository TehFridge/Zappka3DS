#include "request.h"
#define MAX_QUEUE 10

static Request request_queue[MAX_QUEUE];
static int request_count = 0;
static char *sprite_memory = NULL;
static size_t sprite_memory_size = 0;
static LightLock request_lock;
bool youfuckedup = false;
static bool request_running = true;
static Thread request_thread;
static LightEvent request_event;
LightLock global_response_lock;
C2D_SpriteSheet kuponobraz;
C2D_Image kuponkurwa;
bool obrazekdone;
char *response;
bool schizofrenia;
long response_code;
bool requestdone;
bool need_to_load_image;
bool loadingshit;
ResponseBuffer global_response = {NULL, 0};
void log_memory_info(const char *context) {
    log_to_file("[MEMORY] %s - Global response buffer: %p (size: %zu)", 
                context, global_response.data, global_response.size);
    if (request_count > 0) {
        log_to_file("[MEMORY] Request queue first item: %p", &request_queue[0]);
    }
}
size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    if (!ptr || !userdata) {
        log_to_file("[write_callback] ERROR: NULL input");
        return 0;
    }

    size_t total_size = size * nmemb;
    ResponseBuffer *buf = (ResponseBuffer*)userdata;

    LightLock_Lock(&global_response_lock);

    char *new_data = realloc(buf->data, buf->size + total_size + 1);
    if (!new_data) {
        log_to_file("[write_callback] ERROR: realloc failed (want %zu more bytes)", total_size);
        LightLock_Unlock(&global_response_lock);
        return 0;
    }

    buf->data = new_data;
    memcpy(buf->data + buf->size, ptr, total_size);
    buf->size += total_size;
    buf->data[buf->size] = '\0';

    LightLock_Unlock(&global_response_lock);
    return total_size;
}


void safe_free_global_response() {
    LightLock_Lock(&global_response_lock);
    if (global_response.data) {
        free(global_response.data);
        global_response.data = NULL;
        global_response.size = 0;
    }
    LightLock_Unlock(&global_response_lock);
}


void free_global_response() {
    LightLock_Lock(&global_response_lock);
    if (global_response.data) {
        log_to_file("Freeing %zu bytes at %p", global_response.size, global_response.data);
        free(global_response.data);
        global_response.data = NULL;
        global_response.size = 0;
    }
    LightLock_Unlock(&global_response_lock);
}
void log_message(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void init_global_response() {
    LightLock_Init(&global_response_lock);
}

void log_request_to_file(const char *url, const char *data, struct curl_slist *headers, char *response) {
    FILE *log_file = fopen("request_log.txt", "a");
    if (log_file) {
        fprintf(log_file, "URL: %s\n", url);
        fprintf(log_file, "Data: %s\n", data ? data : "(None)");

        fprintf(log_file, "Headers:\n");
        struct curl_slist *header = headers;
        while (header) {
            fprintf(log_file, "  %s\n", header->data);
            header = header->next;
        }

        fprintf(log_file, "Response: %s\n", response ? response : "(None)");
        fprintf(log_file, "------------------------\n");

        fclose(log_file);
    } else {
        printf("Failed to open log file for writing.\n");
    }
}
void load_image(void) {
    if (!global_response.data || global_response.size == 0) {
        log_to_file("load_image: No data to load image from.");
        return;
    }

    char *img_data = malloc(global_response.size);
    if (!img_data) {
        log_to_file("load_image: malloc failed.");
        return;
    }

    memcpy(img_data, global_response.data, global_response.size);

    if (sprite_memory) {
        free(sprite_memory);
        sprite_memory = NULL;
        sprite_memory_size = 0;
    }

    kuponobraz = C2D_SpriteSheetLoadFromMem(img_data, global_response.size);
    if (!kuponobraz) {
        log_to_file("load_image: failed to load sprite sheet.");
        free(img_data);
        return;
    }

    sprite_memory = img_data;
    sprite_memory_size = global_response.size;
    kuponkurwa = C2D_SpriteSheetGetImage(kuponobraz, 0);
    obrazekdone = true;
}


void print_headers(struct curl_slist *headers) {
    struct curl_slist *current = headers;
    while (current) {
        log_to_file("%s\n", current->data);
        current = current->next;
    }
}
bool czasfuckup = false;

bool refresh_data(const char *url, const char *data, struct curl_slist *headers) {
    bool request_failed = false;
    youfuckedup = false;
    czasfuckup = false;
    requestdone = false;
    loadingshit = true;

    if (!url) {
        log_to_file("[refresh_data] ERROR: URL is NULL.");
    }
	log_to_file("[refresh_data] Connecting to: %s", url);
    safe_free_global_response();

    CURL *curl = curl_easy_init();
    if (!curl) {
        log_to_file("[refresh_data] ERROR: curl_easy_init() failed.");
    }

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(curl, CURLOPT_CAINFO, "romfs:/cacert.pem");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    if (data && strcmp(url, "https://zabka-snrs.zabka.pl/v4/server/time") != 0) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    }

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &global_response);

    CURLcode res = curl_easy_perform(curl);

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    if (response_code == 401) youfuckedup = true;
    if (response_code == 0) czasfuckup = true;

    if (res != CURLE_OK) {
        log_to_file("[refresh_data] ERROR: %s", curl_easy_strerror(res));
        request_failed = true;
    } else {
        log_to_file("[refresh_data] Success. Code: %ld", response_code);
    }

    curl_easy_cleanup(curl);
    loadingshit = false;
    requestdone = true;

    if (strstr(url, "szprink") && global_response.data && global_response.size > 0) {
        need_to_load_image = true;
    }
	log_to_file("[refresh_data] Request Done");
	return request_failed;
}

void request_worker(void* arg) {
    while (request_running) {
        LightLock_Lock(&request_lock);
        if (request_count == 0) {
            LightLock_Unlock(&request_lock);
            LightEvent_Wait(&request_event);
            continue;
        }

        Request req = request_queue[0];
        memmove(request_queue, request_queue + 1, (request_count - 1) * sizeof(Request));
        request_count--;
        LightLock_Unlock(&request_lock);

        if (!req.url) {
            log_to_file("[request_worker] ERROR: request has no URL");
            continue;
        }

        refresh_data(req.url, req.data, req.headers);

        if (req.response && req.response_size && global_response.data) {
            LightLock_Lock(&global_response_lock);
            *req.response = global_response.data;
            *req.response_size = global_response.size;
            LightLock_Unlock(&global_response_lock);
        }

        if (req.owns_data && req.data) free(req.data);
        if (req.headers) curl_slist_free_all(req.headers);
        if (req.event) LightEvent_Signal(req.event);

        if (requestdone && need_to_load_image) {
            load_image();
            need_to_load_image = false;
        }
    }
}


void queue_request(const char *url, const char *data, struct curl_slist *headers,
                   void **response, size_t *response_size, LightEvent *event, bool is_binary) {
    if (!url || !response || !response_size) {
        log_to_file("[queue_request] ERROR: Missing critical args");
        return;
    }

    LightLock_Lock(&request_lock);

    if (request_count >= MAX_QUEUE) {
        log_to_file("[queue_request] ERROR: Request queue full");
        LightLock_Unlock(&request_lock);
        return;
    }

    Request *req = &request_queue[request_count];
    memset(req, 0, sizeof(Request));

    strncpy(req->url, url, sizeof(req->url) - 1);
    req->url[sizeof(req->url) - 1] = '\0';

    if (data) {
        req->data = strdup(data);
        if (!req->data) {
            log_to_file("[queue_request] ERROR: strdup(data) failed");
            LightLock_Unlock(&request_lock);
            return;
        }
        req->owns_data = true;
    }

    req->headers = headers;
    req->response = response;
    req->response_size = response_size;
    req->event = event;
    req->is_binary = is_binary;

    request_count++;
    LightEvent_Signal(&request_event);
    LightLock_Unlock(&request_lock);
}



void start_request_thread() {
    LightLock_Init(&request_lock);
    LightEvent_Init(&request_event, RESET_ONESHOT); 
    request_thread = threadCreate(request_worker, NULL, 32 * 1024, 0x30, -2, false);
}


void stop_request_thread() {
    LightLock_Lock(&request_lock);
    request_running = false;
    LightEvent_Signal(&request_event);  
    LightLock_Unlock(&request_lock);

    threadJoin(request_thread, UINT64_MAX);
    threadFree(request_thread);
}
