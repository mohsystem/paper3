#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

static int binary_search(const int* arr, size_t len, int target) {
    if (arr == NULL && len > 0) return -1;
    size_t left = 0;
    size_t right = len == 0 ? 0 : len - 1;
    int found = -1;
    if (len == 0) return -1;

    while (left <= right) {
        size_t mid = left + ((right - left) >> 1);
        int val = arr[mid];
        if (val == target) {
            found = (int)mid;
            break;
        } else if (val < target) {
            left = mid + 1;
            if (left == 0) break; // overflow guard
        } else {
            if (mid == 0) break;
            right = mid - 1;
        }
    }
    return found;
}

static void sanitize_label(const char* in, char* out, size_t outsz) {
    if (!out || outsz == 0) return;
    size_t i = 0;
    for (; in && in[i] != '\0' && i + 1 < outsz && i < 128; ++i) {
        char c = in[i];
        if ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == ' ' || c == '-' || c == '_') {
            out[i] = c;
        } else {
            out[i] = '_';
        }
    }
    out[i] = '\0';
}

static int robust_write(int fd, const char* buf, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t w = write(fd, buf + total, len - total);
        if (w < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        total += (size_t)w;
    }
    return 0;
}

static long long monotonic_time_ns(void) {
#if defined(CLOCK_MONOTONIC)
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        return (long long)ts.tv_sec * 1000000000LL + (long long)ts.tv_nsec;
    }
#endif
    struct timespec ts;
#if defined(TIME_UTC)
    if (timespec_get(&ts, TIME_UTC) == TIME_UTC) {
        return (long long)ts.tv_sec * 1000000000LL + (long long)ts.tv_nsec;
    }
#endif
    return 0;
}

static char* write_execution_time_to_temp_file(long long nanos, const char* label) {
    char tmpl[] = "/tmp/task87_XXXXXX";
    int fd = mkstemp(tmpl);
    if (fd < 0) {
        return NULL;
    }

    fchmod(fd, 0600);

    char safe_label[129];
    sanitize_label(label ? label : "", safe_label, sizeof(safe_label));

    char buf[256];
    int n = snprintf(buf, sizeof(buf), "label=%s\nexecution_time_ns=%lld\n", safe_label, nanos);
    if (n < 0 || (size_t)n >= sizeof(buf)) {
        close(fd);
        return NULL;
    }

    if (ftruncate(fd, 0) != 0) {
        close(fd);
        return NULL;
    }

    if (robust_write(fd, buf, (size_t)n) != 0) {
        close(fd);
        return NULL;
    }

    fsync(fd);
    close(fd);

    char* path = (char*)malloc(strlen(tmpl) + 1);
    if (!path) {
        return NULL;
    }
    strcpy(path, tmpl);
    return path;
}

int main(void) {
    int arr1[] = {1, 3, 5, 7, 9};
    int arr2[] = {2, 4, 6, 8, 10};
    int arr3[] = {0};
    int arr4[] = {-10, -5, 0, 5, 10, 15};
    int arr5[] = {1,2,3,4,5,6,7,8,9,10};

    int targets[] = {7, 1, 0, 15, 11};

    long long start = monotonic_time_ns();

    int r1 = binary_search(arr1, sizeof(arr1)/sizeof(arr1[0]), targets[0]);
    int r2 = binary_search(arr2, sizeof(arr2)/sizeof(arr2[0]), targets[1]);
    int r3 = binary_search(arr3, sizeof(arr3)/sizeof(arr3[0]), targets[2]);
    int r4 = binary_search(arr4, sizeof(arr4)/sizeof(arr4[0]), targets[3]);
    int r5 = binary_search(arr5, sizeof(arr5)/sizeof(arr5[0]), targets[4]);

    long long end = monotonic_time_ns();
    long long total = (end >= start) ? (end - start) : 0;

    char* path = write_execution_time_to_temp_file(total, "total_run");

    printf("Binary search results (indices): [%d, %d, %d, %d, %d]\n", r1, r2, r3, r4, r5);
    if (path) {
        printf("Execution time written to: %s\n", path);
        free(path);
    } else {
        fprintf(stderr, "Failed to write execution time.\n");
    }

    return 0;
}