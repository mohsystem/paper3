#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <time.h>

typedef struct {
    char *path;
    char *hash;
} Pair;

typedef struct {
    Pair *items;
    size_t count;
} Result;

typedef struct {
    char **paths;
    size_t count;
    size_t next_index;
    pthread_mutex_t idx_mtx;
    Pair *out_items;
} Work;

// FNV-1a 64-bit
static uint64_t fnv1a_64_stream(FILE *f) {
    const uint64_t FNV_OFFSET = 1469598103934665603ULL;
    const uint64_t FNV_PRIME = 1099511628211ULL;
    uint64_t hash = FNV_OFFSET;
    const size_t BUF = 64 * 1024;
    unsigned char *buf = (unsigned char*)malloc(BUF);
    if (!buf) return 0;
    size_t n;
    while ((n = fread(buf, 1, BUF, f)) > 0) {
        for (size_t i = 0; i < n; ++i) {
            hash ^= buf[i];
            hash *= FNV_PRIME;
        }
    }
    free(buf);
    return hash;
}

static char *hex64(uint64_t v) {
    char *s = (char*)malloc(17);
    if (!s) return NULL;
    snprintf(s, 17, "%016llx", (unsigned long long)v);
    return s;
}

static void *worker(void *arg) {
    Work *w = (Work*)arg;
    for (;;) {
        size_t idx;
        pthread_mutex_lock(&w->idx_mtx);
        idx = w->next_index;
        if (idx >= w->count) {
            pthread_mutex_unlock(&w->idx_mtx);
            break;
        }
        w->next_index++;
        pthread_mutex_unlock(&w->idx_mtx);

        const char *path = w->paths[idx];
        char *hash = NULL;
        FILE *f = fopen(path, "rb");
        if (!f) {
            hash = strdup("ERROR_open");
        } else {
            uint64_t h = fnv1a_64_stream(f);
            if (ferror(f)) {
                hash = strdup("ERROR_read");
            } else {
                hash = hex64(h);
            }
            fclose(f);
        }
        w->out_items[idx].path = strdup(path);
        w->out_items[idx].hash = hash ? hash : strdup("ERROR_unknown");
    }
    return NULL;
}

// Returns a Result containing array of Pair {path, hash}; caller must free.
Result process_directory(const char *dirPath, int threads) {
    Result res;
    res.items = NULL;
    res.count = 0;

    if (!dirPath) {
        return res;
    }

    char dirAbs[PATH_MAX];
    if (!realpath(dirPath, dirAbs)) {
        return res;
    }

    struct stat stDir;
    if (lstat(dirAbs, &stDir) != 0 || !S_ISDIR(stDir.st_mode) || S_ISLNK(stDir.st_mode)) {
        return res;
    }

    // Collect files
    DIR *d = opendir(dirAbs);
    if (!d) {
        return res;
    }
    size_t cap = 16;
    size_t cnt = 0;
    char **paths = (char**)malloc(sizeof(char*) * cap);
    if (!paths) {
        closedir(d);
        return res;
    }

    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
        char pbuf[PATH_MAX];
        snprintf(pbuf, sizeof(pbuf), "%s/%s", dirAbs, ent->d_name);
        struct stat st;
        if (lstat(pbuf, &st) != 0) continue;
        if (S_ISLNK(st.st_mode)) continue;
        if (S_ISREG(st.st_mode)) {
            if (cnt == cap) {
                cap *= 2;
                char **np = (char**)realloc(paths, sizeof(char*) * cap);
                if (!np) break;
                paths = np;
            }
            paths[cnt++] = strdup(pbuf);
        }
    }
    closedir(d);

    if (cnt == 0) {
        free(paths);
        return res;
    }

    res.items = (Pair*)calloc(cnt, sizeof(Pair));
    if (!res.items) {
        for (size_t i = 0; i < cnt; ++i) free(paths[i]);
        free(paths);
        res.count = 0;
        return res;
    }
    res.count = cnt;

    int hw = (int)sysconf(_SC_NPROCESSORS_ONLN);
    if (hw <= 0) hw = 4;
    int max_threads = threads;
    if (max_threads < 1) max_threads = 1;
    if (max_threads > hw * 4) max_threads = hw * 4;

    Work work;
    work.paths = paths;
    work.count = cnt;
    work.next_index = 0;
    work.out_items = res.items;
    pthread_mutex_init(&work.idx_mtx, NULL);

    pthread_t *tids = (pthread_t*)malloc(sizeof(pthread_t) * (size_t)max_threads);
    if (!tids) {
        pthread_mutex_destroy(&work.idx_mtx);
        for (size_t i = 0; i < cnt; ++i) free(paths[i]);
        free(paths);
        free(res.items);
        res.items = NULL;
        res.count = 0;
        return res;
    }

    for (int i = 0; i < max_threads; ++i) {
        pthread_create(&tids[i], NULL, worker, &work);
    }
    for (int i = 0; i < max_threads; ++i) {
        pthread_join(tids[i], NULL);
    }

    pthread_mutex_destroy(&work.idx_mtx);
    free(tids);
    for (size_t i = 0; i < cnt; ++i) free(paths[i]);
    free(paths);

    return res;
}

// Helpers for tests
static int make_temp_dir(char *out, size_t outsz, const char *prefix) {
    char tmpl[PATH_MAX];
    snprintf(tmpl, sizeof(tmpl), "/tmp/%sXXXXXX", prefix);
    char *p = mkdtemp(tmpl);
    if (!p) return -1;
    strncpy(out, p, outsz - 1);
    out[outsz - 1] = '\0';
    return 0;
}

static int write_file(const char *dir, const char *name, const char *data) {
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/%s", dir, name);
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    fwrite(data, 1, strlen(data), f);
    fclose(f);
    return 0;
}

static void populate(const char *dir, int count, const char *seed) {
    for (int i = 0; i < count; ++i) {
        char name[64];
        snprintf(name, sizeof(name), "file_%02d.dat", i);
        char buf[256];
        int rep = (i % 17) + 1;
        char xs[64];
        memset(xs, 'X', (size_t)rep);
        xs[rep] = '\0';
        snprintf(buf, sizeof(buf), "seed:%s; index:%d; data:%s", seed, i, xs);
        write_file(dir, name, buf);
    }
}

static void free_result(Result *r) {
    if (!r || !r->items) return;
    for (size_t i = 0; i < r->count; ++i) {
        free(r->items[i].path);
        free(r->items[i].hash);
    }
    free(r->items);
    r->items = NULL;
    r->count = 0;
}

int main(void) {
    srand((unsigned)time(NULL));

    // 1) Empty directory
    char d1[PATH_MAX];
    if (make_temp_dir(d1, sizeof(d1), "cfp1_") != 0) return 1;
    Result r1 = process_directory(d1, 4);
    printf("Test1 files=%zu\n", r1.count);
    free_result(&r1);

    // 2) Small set, 2 threads
    char d2[PATH_MAX];
    make_temp_dir(d2, sizeof(d2), "cfp2_");
    populate(d2, 5, "A");
    Result r2 = process_directory(d2, 2);
    printf("Test2 files=%zu\n", r2.count);
    free_result(&r2);

    // 3) Larger set, 4 threads
    char d3[PATH_MAX];
    make_temp_dir(d3, sizeof(d3), "cfp3_");
    populate(d3, 20, "B");
    Result r3 = process_directory(d3, 4);
    printf("Test3 files=%zu\n", r3.count);
    free_result(&r3);

    // 4) Larger set, 8 threads
    char d4[PATH_MAX];
    make_temp_dir(d4, sizeof(d4), "cfp4_");
    populate(d4, 50, "C");
    Result r4 = process_directory(d4, 8);
    printf("Test4 files=%zu\n", r4.count);
    free_result(&r4);

    // 5) Single-threaded run
    char d5[PATH_MAX];
    make_temp_dir(d5, sizeof(d5), "cfp5_");
    populate(d5, 10, "D");
    Result r5 = process_directory(d5, 1);
    printf("Test5 files=%zu\n", r5.count);
    free_result(&r5);

    return 0;
}