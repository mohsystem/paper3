#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
  #include <windows.h>
  #include <io.h>
  #include <fcntl.h>
  #include <share.h>
  #include <sys/stat.h>
  #define CLOSEFD _close
  #define WRITEFD _write
#else
  #include <unistd.h>
  #include <fcntl.h>
  #include <sys/stat.h>
  #include <time.h>
  #define CLOSEFD close
  #define WRITEFD write
#endif

int binary_search(const int* arr, size_t n, int target) {
    if (arr == NULL) return -1;
    size_t left = 0;
    size_t right = (n == 0) ? 0 : n - 1;
    if (n == 0) return -1;
    while (left <= right) {
        size_t mid = left + ((right - left) >> 1);
        int val = arr[mid];
        if (val == target) return (int)mid;
        if (val < target) left = mid + 1;
        else {
            if (mid == 0) break;
            right = mid - 1;
        }
    }
    return -1;
}

char* write_execution_time_to_temp(long long nanos) {
#ifdef _WIN32
    char tmpPath[MAX_PATH];
    DWORD len = GetTempPathA(MAX_PATH, tmpPath);
    if (len == 0 || len > MAX_PATH) {
        return NULL;
    }

    const char* baseTemplate = "task87_bsearch_XXXXXX";
    char templateBuf[64];
    strncpy(templateBuf, baseTemplate, sizeof(templateBuf));
    templateBuf[sizeof(templateBuf) - 1] = '\0';

    // Attempt multiple times to avoid race conditions
    char fullPath[MAX_PATH];
    int attempts = 10;
    for (int i = 0; i < attempts; ++i) {
        // Reset template
        strncpy(templateBuf, baseTemplate, sizeof(templateBuf));
        templateBuf[sizeof(templateBuf) - 1] = '\0';

        if (_mktemp_s(templateBuf, sizeof(templateBuf)) != 0) continue;

        snprintf(fullPath, sizeof(fullPath), "%s%s", tmpPath, templateBuf);

        int fd;
        if (_sopen_s(&fd, fullPath, _O_WRONLY | _O_CREAT | _O_EXCL | _O_BINARY, _SH_DENYRW, _S_IREAD | _S_IWRITE) == 0) {
            char buf[64];
            int writtenLen = snprintf(buf, sizeof(buf), "%lld\n", nanos);
            if (writtenLen < 0) {
                CLOSEFD(fd);
                return NULL;
            }
            if (WRITEFD(fd, buf, (unsigned int)writtenLen) < 0) {
                CLOSEFD(fd);
                return NULL;
            }
            CLOSEFD(fd);
            char* result = (char*)malloc(strlen(fullPath) + 1);
            if (!result) return NULL;
            strcpy(result, fullPath);
            return result;
        }
    }
    return NULL;
#else
    const char* tmpdir = getenv("TMPDIR");
    if (!tmpdir || tmpdir[0] == '\0') tmpdir = "/tmp";
    char templatePath[512];
    snprintf(templatePath, sizeof(templatePath), "%s/%s", tmpdir, "task87_bsearch_XXXXXX");
    int fd = mkstemp(templatePath);
    if (fd == -1) return NULL;
    fchmod(fd, S_IRUSR | S_IWUSR);

    char buf[64];
    int len = snprintf(buf, sizeof(buf), "%lld\n", nanos);
    if (len < 0) {
        CLOSEFD(fd);
        return NULL;
    }
    if (WRITEFD(fd, buf, (size_t)len) < 0) {
        CLOSEFD(fd);
        return NULL;
    }
    CLOSEFD(fd);
    char* result = (char*)malloc(strlen(templatePath) + 1);
    if (!result) return NULL;
    strcpy(result, templatePath);
    return result;
#endif
}

static long long now_ns() {
#ifdef _WIN32
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    long double ns = ((long double)counter.QuadPart * 1000000000.0L) / (long double)freq.QuadPart;
    return (long long)ns;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000000000LL + (long long)ts.tv_nsec;
#endif
}

int main(void) {
    long long start = now_ns();

    int a1[] = {1,3,5,7,9};
    printf("Test 1: %d\n", binary_search(a1, sizeof(a1)/sizeof(a1[0]), 7)); // expected 3

    int a2[] = {};
    printf("Test 2: %d\n", binary_search(a2, 0, 1)); // expected -1

    int a3[] = {-10,-3,0,0,5,8};
    printf("Test 3: %d\n", binary_search(a3, sizeof(a3)/sizeof(a3[0]), 0)); // expected 2 or 3

    int a4[] = {1};
    printf("Test 4: %d\n", binary_search(a4, sizeof(a4)/sizeof(a4[0]), 1)); // expected 0

    int a5[] = {2,4,6,8,10,12,14,16};
    printf("Test 5: %d\n", binary_search(a5, sizeof(a5)/sizeof(a5[0]), 15)); // expected -1

    long long end = now_ns();
    long long elapsed = end - start;

    char* path = write_execution_time_to_temp(elapsed);
    if (path) {
        printf("Execution time (ns) written to: %s\n", path);
        free(path);
    } else {
        fprintf(stderr, "Failed to write execution time\n");
    }

    return 0;
}