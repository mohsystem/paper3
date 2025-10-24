#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

int binary_search(const int* arr, size_t n, int target) {
    if (arr == NULL || n == 0) return -1;
    size_t low = 0;
    size_t high = n - 1;
    while (low <= high) {
        size_t mid = low + ((high - low) >> 1);
        int val = arr[mid];
        if (val == target) return (int)mid;
        if (val < target) {
            low = mid + 1;
        } else {
            if (mid == 0) break; // prevent size_t underflow
            high = mid - 1;
        }
    }
    return -1;
}

static char* dup_cstr(const char* s) {
    size_t len = strlen(s);
    char* copy = (char*)malloc(len + 1);
    if (!copy) return NULL;
    memcpy(copy, s, len + 1);
    return copy;
}

char* measure_and_write_time(const int* arr, size_t n, int target) {
    struct timespec start, end;
#if defined(CLOCK_MONOTONIC)
    clock_gettime(CLOCK_MONOTONIC, &start);
#else
    clock_gettime(CLOCK_REALTIME, &start);
#endif
    int idx = binary_search(arr, n, target);
#if defined(CLOCK_MONOTONIC)
    clock_gettime(CLOCK_MONOTONIC, &end);
#else
    clock_gettime(CLOCK_REALTIME, &end);
#endif

    long long elapsed = (long long)(end.tv_sec - start.tv_sec) * 1000000000LL +
                        (long long)(end.tv_nsec - start.tv_nsec);

    char tmpl[] = "/tmp/task87_time_XXXXXX";
    int fd = mkstemp(tmpl);
    if (fd == -1) {
        return NULL;
    }

    char buf[128];
    int len = snprintf(buf, sizeof(buf), "elapsed_nanos=%lld, index=%d\n", elapsed, idx);
    if (len > 0) {
        (void)write(fd, buf, (size_t)len);
    }
    close(fd);

    return dup_cstr(tmpl); // caller should free
}

int main(void) {
    int t0[] = {1, 3, 5, 7, 9}; int target0 = 7;
    int t1[] = {2, 4, 6, 8, 10}; int target1 = 5;
    int* t2 = NULL; size_t n2 = 0; int target2 = 1;
    int t3[] = {-10, -5, 0, 5, 10}; int target3 = -10;

    size_t n0 = sizeof(t0)/sizeof(t0[0]);
    size_t n1 = sizeof(t1)/sizeof(t1[0]);
    size_t n3 = sizeof(t3)/sizeof(t3[0]);

    size_t n4 = 100000;
    int* t4 = (int*)malloc(n4 * sizeof(int));
    if (!t4) return 1;
    for (size_t i = 0; i < n4; ++i) t4[i] = (int)i;
    int target4 = 12345;

    int idx0 = binary_search(t0, n0, target0);
    char* p0 = measure_and_write_time(t0, n0, target0);
    printf("Test 1: target=%d, index=%d, timeFile=%s\n", target0, idx0, p0 ? p0 : "(error)");
    free(p0);

    int idx1 = binary_search(t1, n1, target1);
    char* p1 = measure_and_write_time(t1, n1, target1);
    printf("Test 2: target=%d, index=%d, timeFile=%s\n", target1, idx1, p1 ? p1 : "(error)");
    free(p1);

    int idx2 = binary_search(t2, n2, target2);
    char* p2 = measure_and_write_time(t2, n2, target2);
    printf("Test 3: target=%d, index=%d, timeFile=%s\n", target2, idx2, p2 ? p2 : "(error)");
    free(p2);

    int idx3 = binary_search(t3, n3, target3);
    char* p3 = measure_and_write_time(t3, n3, target3);
    printf("Test 4: target=%d, index=%d, timeFile=%s\n", target3, idx3, p3 ? p3 : "(error)");
    free(p3);

    int idx4 = binary_search(t4, n4, target4);
    char* p4 = measure_and_write_time(t4, n4, target4);
    printf("Test 5: target=%d, index=%d, timeFile=%s\n", target4, idx4, p4 ? p4 : "(error)");
    free(p4);

    free(t4);
    return 0;
}