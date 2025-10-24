#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <curl/curl.h>

struct Memory {
    char *data;
    size_t size;
};

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t total = size * nmemb;
    struct Memory *mem = (struct Memory *)userp;
    char *ptr = (char*)realloc(mem->data, mem->size + total + 1);
    if (!ptr) return 0;
    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, total);
    mem->size += total;
    mem->data[mem->size] = '\0';
    return total;
}

static void trim_inplace(char *s) {
    if (!s) return;
    size_t len = strlen(s);
    size_t start = 0;
    while (start < len && isspace((unsigned char)s[start])) start++;
    size_t end = len;
    while (end > start && isspace((unsigned char)s[end - 1])) end--;
    if (start > 0) memmove(s, s + start, end - start);
    s[end - start] = '\0';
}

char* fetch_title(const char* url) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        char *empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    struct Memory chunk = {0};
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Task76-C/1.0 (+https://example.com)");
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, ""); // enable auto decompression
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK || chunk.data == NULL) {
        if (chunk.data) free(chunk.data);
        char *empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    // Create lowercase copy for case-insensitive search
    size_t n = chunk.size;
    char *lower = (char*)malloc(n + 1);
    if (!lower) {
        free(chunk.data);
        char *empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    for (size_t i = 0; i < n; ++i) lower[i] = (char)tolower((unsigned char)chunk.data[i]);
    lower[n] = '\0';

    char *start = strstr(lower, "<title");
    if (!start) {
        free(lower);
        char *empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        free(chunk.data);
        return empty;
    }
    char *gt = strchr(start, '>');
    if (!gt) {
        free(lower);
        char *empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        free(chunk.data);
        return empty;
    }
    char *end = strstr(gt, "</title");
    if (!end) {
        free(lower);
        char *empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        free(chunk.data);
        return empty;
    }

    // Calculate indices relative to original data
    size_t idx_start = (size_t)(gt - lower) + 1;
    size_t idx_end = (size_t)(end - lower);
    if (idx_end < idx_start) idx_end = idx_start;

    size_t len = idx_end - idx_start;
    char *title = (char*)malloc(len + 1);
    if (!title) {
        free(lower);
        free(chunk.data);
        char *empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    memcpy(title, chunk.data + idx_start, len);
    title[len] = '\0';

    trim_inplace(title);

    free(lower);
    free(chunk.data);
    return title;
}

int main(int argc, char** argv) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    const char* defaults[5] = {
        "https://www.example.com/",
        "https://www.wikipedia.org/",
        "https://www.iana.org/domains/reserved",
        "https://www.github.com/",
        "https://www.stackoverflow.com/"
    };

    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            char* t = fetch_title(argv[i]);
            printf("%s => %s\n", argv[i], t ? t : "");
            if (t) free(t);
        }
    } else {
        for (int i = 0; i < 5; ++i) {
            char* t = fetch_title(defaults[i]);
            printf("%s => %s\n", defaults[i], t ? t : "");
            if (t) free(t);
        }
    }

    curl_global_cleanup();
    return 0;
}