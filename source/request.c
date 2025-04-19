#include "request.h"
#define MAX_QUEUE 10

static Request request_queue[MAX_QUEUE];
static int request_count = 0;
static LightLock request_lock;
static bool request_running = true;  // Renamed variable
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
ResponseBuffer global_response = {NULL, 0}; // Global response buffer
void log_memory_info(const char *context) {
    log_to_file("[MEMORY] %s - Global response buffer: %p (size: %zu)", 
                context, global_response.data, global_response.size);
    if (request_count > 0) {
        log_to_file("[MEMORY] Request queue first item: %p", &request_queue[0]);
    }
}
size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    log_to_file("[MEMORY] write_callback - received data at %p, size %zu", ptr, size * nmemb);
    
    size_t total_size = size * nmemb;
    ResponseBuffer *buf = (ResponseBuffer*)userdata;
    
    log_to_file("[MEMORY] Current buffer: %p, size: %zu", buf->data, buf->size);

    LightLock_Lock(&global_response_lock);
    
    char *new_data = realloc(buf->data, buf->size + total_size + 1);
	if (!new_data) {
		log_to_file("[write_callback] ERROR: realloc failed! Wanted %zu more bytes", total_size);
		LightLock_Unlock(&global_response_lock);
		return 0;
	}

    
    buf->data = new_data;
    memcpy(buf->data + buf->size, ptr, total_size);
    buf->size += total_size;
    buf->data[buf->size] = '\0';
    
    LightLock_Unlock(&global_response_lock);
    log_to_file("[MEMORY] New buffer: %p, size: %zu", buf->data, buf->size);
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
void load_image() {
    if (kuponobraz) {
        C2D_SpriteSheetFree(kuponobraz);
        kuponobraz = NULL;
    }
	LightLock_Lock(&global_response_lock);
	char *img_data = malloc(global_response.size);
	if (img_data) {
		memcpy(img_data, global_response.data, global_response.size);
		kuponobraz = C2D_SpriteSheetLoadFromMem(img_data, global_response.size);
		free(img_data);  // You can safely free it after if the library copies internally
	}
	LightLock_Unlock(&global_response_lock);
	if (kuponobraz == NULL) {
		log_to_file("Error loading sprite sheet from memory!\n");
	} else {
		log_to_file("Sprite sheet loaded successfully!\n");
	}
	kuponkurwa = C2D_SpriteSheetGetImage(kuponobraz, 0);
	obrazekdone = true;
	// if (!&kuponkurwa) {
		// log_to_file("[ERROR] Failed to decode image data\n");
	// } else {
		// log_to_file("[DEBUG] Decoded image at %p\n", &kuponkurwa);
	// }
	//safe_free_global_response();
}
void print_headers(struct curl_slist *headers) {
    struct curl_slist *current = headers;
    while (current) {
        log_to_file("%s\n", current->data);
        current = current->next;
    }
}
extern void refresh_data(const char *url, const char *data, struct curl_slist *headers) {
    log_memory_info("Entering refresh_data");
    
    requestdone = false;
    loadingshit = true;
    
    if (global_response.data) {
        log_to_file("[refresh_data] Freeing previous response buffer at %p...", global_response.data);
        free_global_response();
    }

    CURL *curl;
    CURLcode res;

    log_to_file("[refresh_data] Initializing CURL...");
    curl = curl_easy_init();
    log_to_file("[MEMORY] CURL handle: %p", curl);
    if (curl) {
        log_to_file("[refresh_data] Starting request to URL: %s", url);
        log_to_file("[refresh_data] Request Data: %s", data);

        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		//curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
		curl_easy_setopt(curl, CURLOPT_CAINFO, "romfs:/cacert.pem");
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
        //curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4); 
        //curl_easy_setopt(curl, CURLOPT_DNS_CACHE_TIMEOUT, 300);
        //curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15L); 
        //curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L); 
		//curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
		//curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
		//curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
		//curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		//curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		log_to_file("[refresh_data] Setting Verbose Output...", data);
        //curl_easy_setopt(curl, CURLOPT_STDERR, fopen(LOG_FILE, "a"));
		//log_to_file("[refresh_data] Open Logs...", data);
        curl_easy_setopt(curl, CURLOPT_URL, url);
		log_to_file("[refresh_data] Set Url... %s", url);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		log_to_file("[refresh_data] Set Headers... ");
		print_headers(headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
		log_to_file("[refresh_data] Set Data... %s", data);
		//ResponseBuffer response = {NULL, 0};
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
		log_to_file("[refresh_data] Set CallbackFunc...", data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &global_response); 
		log_to_file("[refresh_data] Set Global Response...", data);
		log_to_file("[refresh_data] sze: %d", global_response.size);
		char* cainfo = NULL;
		curl_easy_getinfo(curl, CURLINFO_CAINFO, &cainfo);
		if (cainfo) {
			log_to_file("Default CA certificate path: %s\n", cainfo);
		} else {
			log_to_file("No default CA certificate path set.\n");
		}
        log_memory_info("Before curl_easy_perform");
        res = curl_easy_perform(curl);
        log_memory_info("After curl_easy_perform");

		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

		if (res != CURLE_OK) {
			log_to_file("[refresh_data] ERROR: curl_easy_perform() failed: %s", curl_easy_strerror(res));
		} else if (response_code == 0) {
			log_to_file("[refresh_data] WARNING: No response code received! Possible network issue.");
			schizofrenia = true;
		}


        if (res != CURLE_OK) {
            log_to_file("[refresh_data] ERROR: curl_easy_perform() failed: %s", curl_easy_strerror(res));
            log_to_file("[refresh_data] Request unsuccessful! Response Code: %ld", response_code);
        } else {
            log_to_file("[refresh_data] Request successful! Response Code: %ld", response_code);
			if (strstr(url, "szprink")) {
				log_to_file("[refresh_data] Response image");
				// FILE *file = fopen("output.t3x", "wb"); // Otwórz plik do zapisu binarnego
			    // fwrite(&obrazek, sizeof(uint8_t), totalsajz, file); // Zapisz bajty do pliku
				// fclose(file); // Zamknij plik
			} else {
				log_to_file("[refresh_data] Response: %s", global_response.data ? global_response.data : "NULL");
			}
        }
        //log_request_to_file(url, data, headers, global_response.data);
		curl_easy_cleanup(curl); // DO THIS
		curl = NULL;

		loadingshit = false;
		requestdone = true;
		if (strstr(url, "szprink")) {
			need_to_load_image = true;
		}

    } else {
        log_to_file("[refresh_data] ERROR: Failed to initialize CURL.");
    }
	log_memory_info("Exiting refresh_data");
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

        refresh_data(req.url, req.data, req.headers);
		if (req.response && req.response_size) {
			LightLock_Lock(&global_response_lock);
			*req.response = global_response.data;
			*req.response_size = global_response.size;
			LightLock_Unlock(&global_response_lock);
		}


        if (req.owns_data && req.data) {
            free(req.data);
        }
        if (req.headers) {
            curl_slist_free_all(req.headers);
        }
        if (req.event) {
            LightEvent_Signal(req.event);
        }
		if (requestdone && need_to_load_image) {
			load_image(); 
			need_to_load_image = false;
		}		
    }
}

void queue_request(const char *url, const char *data, struct curl_slist *headers,
                   void **response, size_t *response_size, LightEvent *event, bool is_binary) {
    log_memory_info("Entering queue_request");

    if (!url || !response || !response_size) {
        log_message("[queue_request] ERROR: Invalid arguments\n");
        return;
    }

    LightLock_Lock(&request_lock);

    log_to_file("[MEMORY] Request queue address: %p, count: %d", request_queue, request_count);

    if (request_count >= MAX_QUEUE) {
        log_message("[queue_request] ERROR: Queue full\n");
        LightLock_Unlock(&request_lock);
        return;
    }

    Request *req = &request_queue[request_count];
    memset(req, 0, sizeof(Request)); 


    strncpy(req->url, url, sizeof(req->url) - 1);
    req->url[sizeof(req->url) - 1] = '\0';

    if (data) {
        if (is_binary) {
            req->data = strdup(data);
            if (!req->data) {
                LightLock_Unlock(&request_lock);
                return;
            }
            memcpy(req->data, data, *response_size);
        } else {
            req->data = strdup(data);
            if (!req->data) {
                LightLock_Unlock(&request_lock);
                return;
            }
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
	log_memory_info("Exiting queue_request");
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
