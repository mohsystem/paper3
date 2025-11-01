#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if defined(_WIN32)
#include <direct.h>
#include <io.h>
#define PATH_SEP '\\'
#define MKDIR(p) _mkdir(p)
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define PATH_SEP '/'
#define MKDIR(p) mkdir(p, 0700)
#endif

static int is_allowed_char(char c) {
    return (isalnum((unsigned char)c) || c == '.' || c == '_' || c == '-');
}

static const char* windows_reserved[] = {
    "CON","PRN","AUX","NUL",
    "COM1","COM2","COM3","COM4","COM5","COM6","COM7","COM8","COM9",
    "LPT1","LPT2","LPT3","LPT4","LPT5","LPT6","LPT7","LPT8","LPT9",
    NULL
};

static int is_windows_reserved_name(const char* name) {
    char upper[512];
    size_t n = strlen(name);
    if (n >= sizeof(upper)) n = sizeof(upper) - 1;
    for (size_t i = 0; i < n; i++) upper[i] = (char)toupper((unsigned char)name[i]);
    upper[n] = '\0';
    for (int i = 0; windows_reserved[i] != NULL; i++) {
        if (strcmp(upper, windows_reserved[i]) == 0) return 1;
    }
    return 0;
}

static void split_ext(const char* fname, char* nameOut, size_t nameCap, char* extOut, size_t extCap) {
    const char* lastDot = strrchr(fname, '.');
    if (lastDot && lastDot != fname && *(lastDot + 1) != '\0') {
        size_t nameLen = (size_t)(lastDot - fname);
        if (nameLen >= nameCap) nameLen = nameCap - 1;
        memcpy(nameOut, fname, nameLen);
        nameOut[nameLen] = '\0';
        snprintf(extOut, extCap, "%s", lastDot);
    } else {
        snprintf(nameOut, nameCap, "%s", fname);
        extOut[0] = '\0';
    }
}

static void basename_sanitize(const char* input, char* output, size_t cap) {
    const char* base = input;
    const char* p = input;
    while (*p) {
        if (*p == '/' || *p == '\\') base = p + 1;
        p++;
    }
    if (*base == '\0') {
        snprintf(output, cap, "upload.bin");
        return;
    }
    size_t j = 0;
    for (size_t i = 0; base[i] && j + 1 < cap; i++) {
        char c = base[i];
        output[j++] = is_allowed_char(c) ? c : '_';
    }
    output[j] = '\0';
    if (output[0] == '.') output[0] = '_';
    if (output[0] == '\0') snprintf(output, cap, "upload.bin");
}

static void limit_length(char* name, const char* ext, size_t maxTotal) {
    size_t nameLen = strlen(name);
    size_t extLen = strlen(ext);
    if (nameLen + extLen > maxTotal) {
        if (maxTotal > extLen) {
            name[maxTotal - extLen] = '\0';
        } else {
            name[0] = 'u';
            name[1] = '\0';
        }
    }
}

char* upload_file(const unsigned char* data, size_t len, const char* filename) {
    if (!data) return NULL;

    char cleaned[512];
    basename_sanitize(filename ? filename : "", cleaned, sizeof(cleaned));

    char name[512], ext[64];
    split_ext(cleaned, name, sizeof(name), ext, sizeof(ext));

    if (is_windows_reserved_name(name)) {
        char tmp[512];
        snprintf(tmp, sizeof(tmp), "_%s", name);
        snprintf(name, sizeof(name), "%s", tmp);
    }
    if (ext[0] == '\0') snprintf(ext, sizeof(ext), "%s", ".bin");

    limit_length(name, ext, 255);

    // Create uploads directory in current working directory
    const char* uploadsDir = "uploads";
    MKDIR(uploadsDir); // ignore errors if exists

    // Build candidate path and ensure uniqueness
    int counter = 0;
    char candidate[1024];
    FILE* f = NULL;
    do {
        if (counter == 0)
            snprintf(candidate, sizeof(candidate), "%s%c%s%s", uploadsDir, PATH_SEP, name, ext);
        else
            snprintf(candidate, sizeof(candidate), "%s%c%s-%d%s", uploadsDir, PATH_SEP, name, counter, ext);

        f = fopen(candidate, "rb");
        if (f) {
            fclose(f);
            f = NULL;
            counter++;
        } else {
            break;
        }
    } while (counter < 1000000);

    f = fopen(candidate, "wb");
    if (!f) return NULL;
    size_t written = fwrite(data, 1, len, f);
    fclose(f);
    if (written != len) {
        remove(candidate);
        return NULL;
    }

    // Return absolute-ish path? We'll return relative path for portability
    char* out = (char*)malloc(strlen(candidate) + 1);
    if (!out) return NULL;
    strcpy(out, candidate);
    return out;
}

int main(void) {
    const unsigned char d1[] = "Hello, world!";
    const unsigned char d2[] = "Another content";
    const unsigned char d3[] = "Malicious path attempt";
    const unsigned char d4[] = "Absolute path attempt";
    const unsigned char d5[] = "Special chars & spaces";

    char* r1 = upload_file(d1, sizeof(d1)-1, "greeting.txt");
    char* r2 = upload_file(d2, sizeof(d2)-1, "../evil.txt");
    char* r3 = upload_file(d3, sizeof(d3)-1, "/etc/passwd");
    char* r4 = upload_file(d4, sizeof(d4)-1, "COM1");
    char* r5 = upload_file(d5, sizeof(d5)-1, "my file (final)!.pdf");

    if (r1) { printf("%s\n", r1); free(r1); } else { printf("\n"); }
    if (r2) { printf("%s\n", r2); free(r2); } else { printf("\n"); }
    if (r3) { printf("%s\n", r3); free(r3); } else { printf("\n"); }
    if (r4) { printf("%s\n", r4); free(r4); } else { printf("\n"); }
    if (r5) { printf("%s\n", r5); free(r5); } else { printf("\n"); }

    return 0;
}