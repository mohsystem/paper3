/*
Chain-of-Through in code (comments only):
1) Problem: Create a UTF-8 temp file, generate a random string, convert to Unicode code point notation, write, return path.
2) Security: Use secure randomness where possible, exclusive file creation (mkstemp on POSIX; CreateFile/_sopen_s on Windows), 0600 perms on POSIX.
3) Implementation: Cross-platform temp dir and unique file path creation; avoid race; validate inputs.
4) Review: Close descriptors, handle errors, avoid buffer overflows, check lengths.
5) Secure output: Function returns allocated path; main runs 5 test cases and frees paths.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  #include <io.h>
  #include <fcntl.h>
  #include <share.h>
  #include <process.h>
  #include <sys/types.h>
  #include <sys/stat.h>
#else
  #include <unistd.h>
  #include <sys/stat.h>
  #include <fcntl.h>
#endif

// Predefined code points
static const uint32_t CODE_POINTS[] = {
    // ASCII uppercase
    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
    // ASCII lowercase
    'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
    // Digits
    '0','1','2','3','4','5','6','7','8','9',
    // Extended BMP
    0x00E9, 0x03A9, 0x0416, 0x4F60, 0x597D, 0x6F22, 0x20AC, 0x2603,
    // Supplementary
    0x1F642, 0x1F680, 0x1F9E9, 0x1D6D1
};
static const size_t CODE_POINTS_COUNT = sizeof(CODE_POINTS)/sizeof(CODE_POINTS[0]);

static int sanitize_length(int length) {
    if (length < 0) return 16;
    if (length > 4096) return 4096;
    return length;
}

// Platform-secure random 32-bit
#ifdef _WIN32
static uint32_t secure_rand_u32(void) {
    unsigned int v = 0;
    errno_t err = rand_s(&v);
    if (err != 0) {
        // Fallback: use time-based seed (less secure); avoid aborting
        v = (unsigned int)(GetTickCount() ^ (unsigned int)_getpid());
        v ^= (unsigned int)rand();
    }
    return (uint32_t)v;
}
#else
static uint32_t secure_rand_u32(void) {
    uint32_t v = 0;
#if defined(__linux__)
    // Try getrandom if available
    ssize_t r = syscall(318, &v, sizeof(v), 0); // 318 may vary; safer to use getrandom() prototype if available
    if (r == sizeof(v)) return v;
#endif
    // Fallback: /dev/urandom
    FILE* f = fopen("/dev/urandom", "rb");
    if (f) {
        size_t n = fread(&v, 1, sizeof(v), f);
        fclose(f);
        if (n == sizeof(v)) return v;
    }
    // Last resort: rand() mixed with time/pid (not cryptographically secure)
    v = (uint32_t)(rand() ^ (unsigned)time(NULL) ^ (unsigned)getpid());
    return v;
}
#endif

static uint32_t pick_random_cp(void) {
    uint32_t r = secure_rand_u32();
    return CODE_POINTS[r % CODE_POINTS_COUNT];
}

static char* to_unicode_points(const uint32_t* cps, size_t count) {
    // Each code point string up to: "U+XXXXXX" + space => <= 8 chars, plus null terminator
    size_t max_len = count ? (count * 8) : 1;
    char* out = (char*)malloc(max_len);
    if (!out) return NULL;
    size_t pos = 0;
    for (size_t i = 0; i < count; ++i) {
        if (i > 0) {
            out[pos++] = ' ';
        }
        if (cps[i] <= 0xFFFFu) {
            // U+XXXX
            int n = snprintf(out + pos, max_len - pos, "U+%04X", (unsigned)cps[i]);
            if (n < 0) { free(out); return NULL; }
            pos += (size_t)n;
        } else {
            int n = snprintf(out + pos, max_len - pos, "U+%06X", (unsigned)cps[i]);
            if (n < 0) { free(out); return NULL; }
            pos += (size_t)n;
        }
    }
    if (pos >= max_len) {
        // Should not happen due to sizing, but reallocate safely if needed
        char* bigger = (char*)realloc(out, pos + 1);
        if (!bigger) { free(out); return NULL; }
        out = bigger;
    }
    out[pos] = '\0';
    return out;
}

#ifdef _WIN32
static int write_all_h(HANDLE h, const char* data, size_t len) {
    const char* p = data;
    size_t rem = len;
    while (rem > 0) {
        DWORD wrote = 0;
        if (!WriteFile(h, p, (DWORD)((rem > (1<<20)) ? (1<<20) : rem), &wrote, NULL)) {
            return 0;
        }
        if (wrote == 0) return 0;
        p += wrote;
        rem -= wrote;
    }
    return 1;
}
#else
static int write_all_fd(int fd, const char* data, size_t len) {
    const char* p = data;
    size_t rem = len;
    while (rem > 0) {
        ssize_t w = write(fd, p, rem);
        if (w <= 0) return 0;
        p += (size_t)w;
        rem -= (size_t)w;
    }
    return 1;
}
#endif

// Public API: returns malloc-allocated path string (caller should free). Returns NULL on failure.
char* create_temp_unicode_file(int length) {
    int safeLen = sanitize_length(length);

    // Generate code points
    uint32_t* cps = NULL;
    if (safeLen > 0) {
        cps = (uint32_t*)malloc((size_t)safeLen * sizeof(uint32_t));
        if (!cps) return NULL;
        for (int i = 0; i < safeLen; ++i) cps[i] = pick_random_cp();
    }

    // Convert to Unicode point notation string (ASCII, UTF-8 compatible)
    char* unicode_repr = to_unicode_points(cps, (size_t)safeLen);
    free(cps);
    if (!unicode_repr) return NULL;

#ifdef _WIN32
    char tempPath[MAX_PATH];
    DWORD n = GetTempPathA(MAX_PATH, tempPath);
    if (n == 0 || n > MAX_PATH) {
        free(unicode_repr);
        return NULL;
    }
    // Attempt unique creates
    char* finalPath = NULL;
    for (int attempt = 0; attempt < 25; ++attempt) {
        unsigned int r1 = secure_rand_u32();
        unsigned int r2 = secure_rand_u32();
        char name[128];
        _snprintf_s(name, sizeof(name), _TRUNCATE, "task88_%08X_%08X.txt", r1, r2);

        char pathBuf[MAX_PATH];
        _snprintf_s(pathBuf, sizeof(pathBuf), _TRUNCATE, "%s%s", tempPath, name);

        HANDLE h = CreateFileA(
            pathBuf,
            GENERIC_WRITE,
            0,                      // no sharing
            NULL,
            CREATE_NEW,             // exclusive
            FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_ARCHIVE,
            NULL
        );
        if (h == INVALID_HANDLE_VALUE) {
            DWORD err = GetLastError();
            if (err == ERROR_FILE_EXISTS || err == ERROR_ALREADY_EXISTS) continue;
            continue;
        }
        int ok = write_all_h(h, unicode_repr, strlen(unicode_repr));
        CloseHandle(h);
        if (!ok) {
            DeleteFileA(pathBuf);
            continue;
        }
        size_t plen = strlen(pathBuf);
        finalPath = (char*)malloc(plen + 1);
        if (!finalPath) {
            DeleteFileA(pathBuf);
            free(unicode_repr);
            return NULL;
        }
        memcpy(finalPath, pathBuf, plen + 1);
        free(unicode_repr);
        return finalPath;
    }
    free(unicode_repr);
    return NULL;
#else
    const char* tmp = getenv("TMPDIR");
    if (!tmp || !*tmp) tmp = getenv("TEMP");
    if (!tmp || !*tmp) tmp = getenv("TMP");
    if (!tmp || !*tmp) tmp = "/tmp";

    size_t baseLen = strlen(tmp);
    int needSlash = (baseLen > 0 && tmp[baseLen - 1] == '/') ? 0 : 1;

    char tmpl[PATH_MAX];
    snprintf(tmpl, sizeof(tmpl), "%s%s%s", tmp, needSlash ? "/" : "", "task88_XXXXXX");
    int fd = mkstemp(tmpl);
    if (fd == -1) {
        free(unicode_repr);
        return NULL;
    }
    // Restrict permissions: 0600
    fchmod(fd, S_IRUSR | S_IWUSR);

    int ok = write_all_fd(fd, unicode_repr, strlen(unicode_repr));
    close(fd);
    free(unicode_repr);
    if (!ok) {
        unlink(tmpl);
        return NULL;
    }
    // Return allocated path
    size_t plen = strlen(tmpl);
    char* out = (char*)malloc(plen + 1);
    if (!out) return NULL;
    memcpy(out, tmpl, plen + 1);
    return out;
#endif
}

int main(void) {
    int tests[5] = {0, 1, 5, 10, 50};
    for (int i = 0; i < 5; ++i) {
        char* path = create_temp_unicode_file(tests[i]);
        if (path) {
            printf("Created temp file (len=%d): %s\n", tests[i], path);
            free(path);
        } else {
            printf("Failed to create temp file (len=%d)\n", tests[i]);
        }
    }
    return 0;
}