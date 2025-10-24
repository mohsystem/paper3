#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

struct Memory {
    char *data;
    size_t size;
};

static size_t write_to_memory(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t total = size * nmemb;
    struct Memory *mem = (struct Memory *)userp;

    char *ptr = (char *)realloc(mem->data, mem->size + total + 1);
    if (ptr == NULL) {
        return 0; // out of memory
    }
    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, total);
    mem->size += total;
    mem->data[mem->size] = '\0';
    return total;
}

char* fetch_url(const char* url) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        char *err = (char*)malloc(64);
        if (err) strcpy(err, "ERROR: Failed to initialize CURL");
        return err;
    }

    struct Memory chunk;
    chunk.data = (char *)malloc(1);
    chunk.size = 0;
    if (!chunk.data) {
        curl_easy_cleanup(curl);
        char *err = (char*)malloc(64);
        if (err) strcpy(err, "ERROR: Out of memory");
        return err;
    }

    char errbuf[CURL_ERROR_SIZE];
    errbuf[0] = '\0';

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Task160/1.0");
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_memory);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    char *result = NULL;

    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        // Prepare "HTTP <code>\n<body>"
        size_t header_len = 16; // enough for "HTTP " + code + "\n"
        char codebuf[32];
        snprintf(codebuf, sizeof(codebuf), "HTTP %ld\n", http_code);
        size_t code_len = strlen(codebuf);
        result = (char*)malloc(code_len + chunk.size + 1);
        if (result) {
            memcpy(result, codebuf, code_len);
            if (chunk.size > 0) memcpy(result + code_len, chunk.data, chunk.size);
            result[code_len + chunk.size] = '\0';
        }
    } else {
        const char *msg = errbuf[0] ? errbuf : curl_easy_strerror(res);
        size_t len = strlen(msg) + strlen("ERROR: ") + 1;
        result = (char*)malloc(len);
        if (result) {
            snprintf(result, len, "ERROR: %s", msg);
        }
    }

    curl_easy_cleanup(curl);
    free(chunk.data);
    return result;
}

int main(void) {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    const char* urls[5] = {
        "https://www.example.com/",
        "https://httpbin.org/get",
        "https://jsonplaceholder.typicode.com/todos/1",
        "http://httpstat.us/200",
        "http://httpstat.us/404"
    };

    for (int i = 0; i < 5; ++i) {
        char *res = fetch_url(urls[i]);
        printf("=== Test %d: %s ===\n", i + 1, urls[i]);
        if (res) {
            size_t n = strlen(res);
            if (n > 500) {
                fwrite(res, 1, 500, stdout);
                printf("...\n\n");
            } else {
                printf("%s\n\n", res);
            }
            free(res);
        } else {
            printf("ERROR: Failed to fetch\n\n");
        }
    }

    curl_global_cleanup();
    return 0;
}