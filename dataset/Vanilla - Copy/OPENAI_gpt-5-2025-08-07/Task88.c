#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <process.h>
#define GETPID _getpid
#else
#include <unistd.h>
#define GETPID getpid
#endif

static char* utf8_encode_cp(int cp, char* out) {
    if (cp <= 0x7F) {
        out[0] = (char)cp;
        out[1] = '\0';
    } else if (cp <= 0x7FF) {
        out[0] = (char)(0xC0 | ((cp >> 6) & 0x1F));
        out[1] = (char)(0x80 | (cp & 0x3F));
        out[2] = '\0';
    } else if (cp <= 0xFFFF) {
        out[0] = (char)(0xE0 | ((cp >> 12) & 0x0F));
        out[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
        out[2] = (char)(0x80 | (cp & 0x3F));
        out[3] = '\0';
    } else {
        out[0] = (char)(0xF0 | ((cp >> 18) & 0x07));
        out[1] = (char)(0x80 | ((cp >> 12) & 0x3F));
        out[2] = (char)(0x80 | ((cp >> 6) & 0x3F));
        out[3] = (char)(0x80 | (cp & 0x3F));
        out[4] = '\0';
    }
    return out;
}

static const char* get_temp_dir() {
#ifdef _WIN32
    const char* env = getenv("TEMP");
    if (env && *env) return env;
    return "C:\\Windows\\Temp";
#else
    const char* env = getenv("TMPDIR");
    if (env && *env) return env;
    return "/tmp";
#endif
}

char* create_unicode_temp_file(int length) {
    int pool[] = {
        'A','B','C','x','y','z','0','1','2','3',
        0x00E9, /* é */
        0x03B1, /* α */
        0x0416, /* Ж */
        0x4E2D, /* 中 */
        0x6F22, /* 漢 */
        0x3042, /* あ */
        0x20AC, /* € */
        0x1F642, /* 🙂 */
        0x1F9E1  /* 🧡 */
    };
    size_t pool_size = sizeof(pool)/sizeof(pool[0]);

    srand((unsigned int)(time(NULL) ^ GETPID()));

    int* cps = (int*)malloc(sizeof(int) * (size_t)length);
    if (!cps) return NULL;
    for (int i = 0; i < length; ++i) {
        cps[i] = pool[rand() % (int)pool_size];
    }

    size_t utf8_cap = (size_t)length * 4 + 1;
    char* utf8_str = (char*)malloc(utf8_cap);
    if (!utf8_str) { free(cps); return NULL; }
    utf8_str[0] = '\0';

    char enc[5];
    for (int i = 0; i < length; ++i) {
        utf8_encode_cp(cps[i], enc);
        strncat(utf8_str, enc, utf8_cap - strlen(utf8_str) - 1);
    }

    size_t notation_cap = (size_t)length * 10 + 1;
    char* notation = (char*)malloc(notation_cap);
    if (!notation) { free(cps); free(utf8_str); return NULL; }
    notation[0] = '\0';
    for (int i = 0; i < length; ++i) {
        int cp = cps[i];
        int width = (cp <= 0xFFFF) ? 4 : 6;
        char buf[16];
        snprintf(buf, sizeof(buf), "U+%0*X", width, cp);
        strncat(notation, buf, notation_cap - strlen(notation) - 1);
        if (i + 1 < length) strncat(notation, " ", notation_cap - strlen(notation) - 1);
    }

    const char* dir = get_temp_dir();
#ifdef _WIN32
    const char sep = '\\';
#else
    const char sep = '/';
#endif

    char namebuf[128];
    snprintf(namebuf, sizeof(namebuf), "task88_%ld_%d.txt", (long)time(NULL), rand());

    size_t path_len = strlen(dir) + 1 + strlen(namebuf) + 1;
    char* path = (char*)malloc(path_len);
    if (!path) { free(cps); free(utf8_str); free(notation); return NULL; }
    snprintf(path, path_len, "%s%c%s", dir, sep, namebuf);

    FILE* f = fopen(path, "wb");
    if (!f) {
        free(cps); free(utf8_str); free(notation); free(path);
        return NULL;
    }
    fwrite(utf8_str, 1, strlen(utf8_str), f);
    fwrite("\n", 1, 1, f);
    fwrite(notation, 1, strlen(notation), f);
    fclose(f);

    free(cps);
    free(utf8_str);
    free(notation);
    return path;
}

int main() {
    char* p1 = create_unicode_temp_file(8);
    char* p2 = create_unicode_temp_file(10);
    char* p3 = create_unicode_temp_file(12);
    char* p4 = create_unicode_temp_file(14);
    char* p5 = create_unicode_temp_file(16);

    if (p1) { printf("%s\n", p1); free(p1); }
    if (p2) { printf("%s\n", p2); free(p2); }
    if (p3) { printf("%s\n", p3); free(p3); }
    if (p4) { printf("%s\n", p4); free(p4); }
    if (p5) { printf("%s\n", p5); free(p5); }
    return 0;
}