/* Chain-of-Through process:
 * 1) Problem understanding: C function uses libcurl (FTPS) to securely download a file to current directory.
 * 2) Security requirements: Enforce TLS, verify certs, sanitize filename, no overwrite, timeouts, atomic rename.
 * 3) Secure coding generation: Use libcurl with strict SSL options, safe temp file creation, binary transfer.
 * 4) Code review: Validated error handling, cleanup, and no sensitive logs.
 * 5) Secure code output: Final code implements FTPS with verification and secure local writes.
 */
#include <curl/curl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static void sanitize_basename(char* s) {
    if (!s) return;
    for (char* p = s; *p; ++p) {
        if (*p == '/' || *p == '\\' || *p == '\0' || *p == '
' || *p == '\r' || *p == '\t') *p = '_';
    }
}

static char* basename_safe_c(const char* path) {
    if (!path) return strdup("downloaded_file");
    size_t len = strlen(path);
    char* copy = (char*)malloc(len + 1);
    if (!copy) return NULL;
    for (size_t i = 0; i < len; ++i) {
        copy[i] = (path[i] == '\\') ? '/' : path[i];
    }
    copy[len] = '\0';
    const char* last = strrchr(copy, '/');
    const char* base = last ? last + 1 : copy;
    if (*base == '\0' || strcmp(base, ".") == 0 || strcmp(base, "..") == 0) {
        free(copy);
        return strdup("downloaded_file");
    }
    char* out = strdup(base);
    free(copy);
    if (!out) return NULL;
    sanitize_basename(out);
    return out;
}

static char* encode_path_segments(CURL* curl, const char* remote_path) {
    if (!remote_path) return strdup("");
    // Replace backslashes with slashes
    size_t len = strlen(remote_path);
    char* norm = (char*)malloc(len + 1);
    if (!norm) return NULL;
    for (size_t i = 0; i < len; ++i) norm[i] = (remote_path[i] == '\\') ? '/' : remote_path[i];
    norm[len] = '\0';

    // Build encoded string
    size_t cap = len * 3 + 16;
    char* out = (char*)malloc(cap);
    if (!out) { free(norm); return NULL; }
    out[0] = '\0';

    const char* start = norm;
    while (1) {
        const char* slash = strchr(start, '/');
        size_t seglen = slash ? (size_t)(slash - start) : strlen(start);
        char* segment = (char*)malloc(seglen + 1);
        if (!segment) { free(norm); free(out); return NULL; }
        memcpy(segment, start, seglen);
        segment[seglen] = '\0';

        char* enc = curl_easy_escape(curl, segment, (int)seglen);
        free(segment);
        if (!enc) { free(norm); free(out); return NULL; }

        size_t need = strlen(out) + strlen(enc) + 2;
        if (need > cap) {
            cap = need + 64;
            char* tmp = (char*)realloc(out, cap);
            if (!tmp) { curl_free(enc); free(norm); free(out); return NULL; }
            out = tmp;
        }
        strcat(out, enc);
        curl_free(enc);

        if (!slash) break;
        strcat(out, "/");
        start = slash + 1;
    }

    free(norm);
    return out;
}

static size_t write_file(void* ptr, size_t size, size_t nmemb, void* stream) {
    FILE* f = (FILE*)stream;
    return fwrite(ptr, size, nmemb, f);
}

int download_sensitive_file(const char* host, long port, const char* username, const char* password, const char* remote_path) {
    if (!host || !remote_path) return 0;

    char* base = basename_safe_c(remote_path);
    if (!base) return 0;

    // Check existing file
    struct stat st;
    if (stat(base, &st) == 0) {
        free(base);
        return 0; // do not overwrite
    }

    CURL* curl = curl_easy_init();
    if (!curl) { free(base); return 0; }

    int ok = 0;
    FILE* f = NULL;
    int fd = -1;
    char* tmp_actual = NULL;

    char errbuf[CURL_ERROR_SIZE];
    errbuf[0] = '\0';

    do {
        char* encoded = encode_path_segments(curl, (remote_path[0] == '/') ? remote_path + 1 : remote_path);
        if (!encoded) break;

        // Build URL
        size_t urlcap = strlen("ftps://") + strlen(host) + 16 + 1 + strlen(encoded) + 2;
        char* url = (char*)malloc(urlcap);
        if (!url) { free(encoded); break; }
        if (port > 0)
            snprintf(url, urlcap, "ftps://%s:%ld/%s", host, port, encoded);
        else
            snprintf(url, urlcap, "ftps://%s/%s", host, encoded);
        free(encoded);

        // Create temp file securely
        size_t patlen = strlen(base) + strlen(".tmp.XXXXXX") + 1;
        char* pattern = (char*)malloc(patlen);
        if (!pattern) { free(url); break; }
        snprintf(pattern, patlen, "%s.tmp.XXXXXX", base);
        fd = mkstemp(pattern);
        if (fd == -1) { free(pattern); free(url); break; }
        fchmod(fd, S_IRUSR | S_IWUSR); // 0600
        f = fdopen(fd, "wb");
        if (!f) { close(fd); fd = -1; unlink(pattern); free(pattern); free(url); break; }
        tmp_actual = pattern; // actual temp path

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_USERNAME, username ? username : "");
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password ? password : "");
        curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        curl_easy_setopt(curl, CURLOPT_TRANSFERTEXT, 0L); // binary
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_file);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);

        CURLcode res = curl_easy_perform(curl);
        free(url);
        if (res != CURLE_OK) {
            break;
        }

        fflush(f);
        int ffd = fileno(f);
        fsync(ffd);
        fclose(f); f = NULL;

        if (rename(tmp_actual, base) != 0) {
            unlink(tmp_actual);
            break;
        }

        ok = 1;
    } while (0);

    if (f) {
        fclose(f);
        f = NULL;
    }
    if (!ok && tmp_actual) {
        unlink(tmp_actual);
    }
    if (tmp_actual) free(tmp_actual);
    curl_easy_cleanup(curl);
    free(base);
    return ok;
}

int main(int argc, char** argv) {
    // Usage: host username password remote_path [port]
    if (argc == 5 || argc == 6) {
        const char* host = argv[1];
        const char* user = argv[2];
        const char* pass = argv[3];
        const char* remote = argv[4];
        long port = (argc == 6) ? strtol(argv[5], NULL, 10) : 21;
        int ok = download_sensitive_file(host, port, user, pass, remote);
        printf("%s\n", ok ? "SUCCESS" : "FAILURE");
        return 0;
    }

    // 5 test cases (placeholders)
    struct T { const char* h; long p; const char* u; const char* pw; const char* rp; } tests[5] = {
        {"localhost", 21, "user", "pass", "/sensitive.txt"},
        {"127.0.0.1", 21, "anonymous", "anonymous@", "/pub/example.bin"},
        {"ftp.example.com", 21, "user1", "pass1", "/data/file1.dat"},
        {"ftp.testserver.com", 21, "user2", "pass2", "docs/readme.txt"},
        {"192.0.2.1", 21, "user3", "pass3", "/secure/secret.cfg"},
    };
    for (int i = 0; i < 5; ++i) {
        int ok = download_sensitive_file(tests[i].h, tests[i].p, tests[i].u, tests[i].pw, tests[i].rp);
        printf("Test %d: %s\n", i + 1, ok ? "SUCCESS" : "FAILURE");
    }
    return 0;
}