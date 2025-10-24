#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#define MKDIR(path) mkdir(path, 0777)
#endif

#define UPLOAD_DIR "uploads"

static void sanitize_filename(const char* name, char* out, size_t out_size) {
    if (!name || !*name) {
        strncpy(out, "file", out_size - 1);
        out[out_size - 1] = '\0';
        return;
    }
    size_t j = 0;
    for (size_t i = 0; name[i] != '\0' && j + 1 < out_size; ++i) {
        unsigned char c = (unsigned char)name[i];
        if ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '.' || c == '_' || c == '-') {
            out[j++] = (char)c;
        } else {
            out[j++] = '_';
        }
        if (j >= 200) break;
    }
    if (j == 0) {
        strncpy(out, "file", out_size - 1);
        out[out_size - 1] = '\0';
        return;
    }
    out[j] = '\0';
}

char* upload_file(const char* filename, const unsigned char* data, size_t len) {
    MKDIR(UPLOAD_DIR);

    char safe[256];
    sanitize_filename(filename, safe, sizeof(safe));

    static unsigned long counter = 0;
    counter++;
    long long ms = 0;
    {
        struct timespec ts;
#if defined(_WIN32)
        // Fallback to time for Windows without clock_gettime
        ms = (long long)time(NULL) * 1000;
#else
        clock_gettime(CLOCK_REALTIME, &ts);
        ms = (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#endif
    }

    char unique[64];
    snprintf(unique, sizeof(unique), "%lld_%lu", ms, counter);

    char save_name[512];
    snprintf(save_name, sizeof(save_name), "%s_%s", unique, safe);

    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", UPLOAD_DIR, save_name);

    FILE* fp = fopen(path, "wb");
    if (!fp) {
        const char* prefix = "ERROR: cannot open file for writing: ";
        size_t out_len = strlen(prefix) + strlen(path) + 1;
        char* out = (char*)malloc(out_len);
        if (!out) return NULL;
        snprintf(out, out_len, "%s%s", prefix, path);
        return out;
    }
    if (len > 0 && data) {
        size_t written = fwrite(data, 1, len, fp);
        if (written != len) {
            fclose(fp);
            const char* prefix = "ERROR: write failed for: ";
            size_t out_len = strlen(prefix) + strlen(path) + 1;
            char* out = (char*)malloc(out_len);
            if (!out) return NULL;
            snprintf(out, out_len, "%s%s", prefix, path);
            return out;
        }
    }
    fclose(fp);

    const char* fmt = "OK: stored as %s (name=%s, size=%zu bytes)";
    size_t out_len = snprintf(NULL, 0, fmt, path, filename ? filename : "file", len) + 1;
    char* out = (char*)malloc(out_len);
    if (!out) return NULL;
    snprintf(out, out_len, fmt, path, filename ? filename : "file", len);
    return out;
}

int main(void) {
    // 5 test cases
    char* r1 = upload_file("hello.txt", (const unsigned char*)"Hello, world!", 13);
    if (r1) { puts(r1); free(r1); }

    unsigned char img_mock[256];
    for (size_t i = 0; i < sizeof(img_mock); ++i) img_mock[i] = (unsigned char)(rand() % 256);
    char* r2 = upload_file("image.png", img_mock, sizeof(img_mock));
    if (r2) { puts(r2); free(r2); }

    const char* pdf_content = "This is a mock PDF content.";
    char* r3 = upload_file("../report.pdf", (const unsigned char*)pdf_content, strlen(pdf_content));
    if (r3) { puts(r3); free(r3); }

    unsigned char data_bin[1024];
    for (size_t i = 0; i < sizeof(data_bin); ++i) data_bin[i] = (unsigned char)(rand() % 256);
    char* r4 = upload_file("data.bin", data_bin, sizeof(data_bin));
    if (r4) { puts(r4); free(r4); }

    const char* long_name = "resume_with_a_very_long_name_that_should_be_sanitized_and_truncated_if_needed_because_it_might_be_too_long_for_some_file_systems.docx";
    const char* resume_content = "Resume content bytes";
    char* r5 = upload_file(long_name, (const unsigned char*)resume_content, strlen(resume_content));
    if (r5) { puts(r5); free(r5); }

    return 0;
}