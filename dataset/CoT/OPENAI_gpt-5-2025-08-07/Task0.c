#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>
#include <limits.h>
#include <unistd.h>
#include <time.h>

// Result structure to return from process_directory
typedef struct {
    char **files;   // array of file names (base name)
    char **hashes;  // corresponding FNV-1a 64-bit hex strings
    int count;
} Result;

static int is_regular_file(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return S_ISREG(st.st_mode);
}

// FNV-1a 64-bit streaming hash; returns hex string (malloc'd, 17 chars + null)
static char* fnv1a64_hex(const char *path) {
    const uint64_t FNV_OFFSET_BASIS = 0xcbf29ce484222325ULL;
    const uint64_t FNV_PRIME = 0x100000001b3ULL;
    uint64_t h = FNV_OFFSET_BASIS;

    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    unsigned char *buf = (unsigned char*)malloc(64 * 1024);
    if (!buf) { fclose(f); return NULL; }

    size_t n;
    while ((n = fread(buf, 1, 64 * 1024, f)) > 0) {
        for (size_t i = 0; i < n; ++i) {
            h ^= buf[i];
            h *= FNV_PRIME;
        }
    }
    free(buf);
    fclose(f);

    char *hex = (char*)malloc(17);
    if (!hex) return NULL;
    // 16 hex digits, zero-padded
    snprintf(hex, 17, "%016llx", (unsigned long long)(h & 0xFFFFFFFFFFFFFFFFULL));
    return hex;
}

typedef struct {
    char **paths;       // full paths
    char **names;       // base names
    char **hashes;      // results
    int count;
    int next_index;
    pthread_mutex_t idx_mtx;
} Context;

static void* worker(void *arg) {
    Context *ctx = (Context*)arg;
    while (1) {
        int i;
        pthread_mutex_lock(&ctx->idx_mtx);
        i = ctx->next_index;
        if (i >= ctx->count) {
            pthread_mutex_unlock(&ctx->idx_mtx);
            break;
        }
        ctx->next_index++;
        pthread_mutex_unlock(&ctx->idx_mtx);

        char *hex = fnv1a64_hex(ctx->paths[i]);
        if (!hex) {
            // On error, set empty hash
            hex = (char*)malloc(2);
            if (hex) strcpy(hex, "");
        }
        ctx->hashes[i] = hex;
    }
    return NULL;
}

// List regular files in dir (non-recursive); returns arrays of full paths and base names
static int list_regular_files(const char *dir_path, char ***out_paths, char ***out_names) {
    DIR *dir = opendir(dir_path);
    if (!dir) return 0;
    struct dirent *ent;
    size_t cap = 16;
    size_t count = 0;
    char **paths = (char**)malloc(cap * sizeof(char*));
    char **names = (char**)malloc(cap * sizeof(char*));
    if (!paths || !names) { closedir(dir); free(paths); free(names); return 0; }

    while ((ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
        char full[PATH_MAX];
        snprintf(full, sizeof(full), "%s/%s", dir_path, ent->d_name);
        if (!is_regular_file(full)) continue;

        if (count == cap) {
            cap *= 2;
            char **np = (char**)realloc(paths, cap * sizeof(char*));
            char **nn = (char**)realloc(names, cap * sizeof(char*));
            if (!np || !nn) { free(np); free(nn); break; }
            paths = np; names = nn;
        }
        paths[count] = strdup(full);
        names[count] = strdup(ent->d_name);
        if (!paths[count] || !names[count]) {
            free(paths[count]); free(names[count]);
            break;
        }
        count++;
    }
    closedir(dir);
    *out_paths = paths;
    *out_names = names;
    return (int)count;
}

// Core concurrent processing: returns Result with file names and hashes
Result process_directory(const char *dir_path, int thread_count) {
    Result res = {0};
    if (!dir_path || thread_count <= 0) return res;

    char **paths = NULL;
    char **names = NULL;
    int count = list_regular_files(dir_path, &paths, &names);
    if (count <= 0) {
        res.files = NULL; res.hashes = NULL; res.count = 0;
        return res;
    }
    char **hashes = (char**)calloc((size_t)count, sizeof(char*));
    if (!hashes) {
        for (int i = 0; i < count; ++i) { free(paths[i]); free(names[i]); }
        free(paths); free(names);
        return res;
    }

    Context ctx;
    ctx.paths = paths;
    ctx.names = names;
    ctx.hashes = hashes;
    ctx.count = count;
    ctx.next_index = 0;
    pthread_mutex_init(&ctx.idx_mtx, NULL);

    int workers = thread_count < count ? thread_count : (count > 0 ? count : 1);
    pthread_t *ths = (pthread_t*)malloc((size_t)workers * sizeof(pthread_t));
    if (!ths) {
        pthread_mutex_destroy(&ctx.idx_mtx);
        for (int i = 0; i < count; ++i) { free(paths[i]); free(names[i]); }
        free(paths); free(names); free(hashes);
        return res;
    }

    for (int i = 0; i < workers; ++i) {
        pthread_create(&ths[i], NULL, worker, &ctx);
    }
    for (int i = 0; i < workers; ++i) {
        pthread_join(ths[i], NULL);
    }
    pthread_mutex_destroy(&ctx.idx_mtx);
    free(ths);

    // Prepare result
    res.files = names; // base names
    res.hashes = hashes;
    res.count = count;

    // Free paths (full) as they are not returned
    for (int i = 0; i < count; ++i) free(paths[i]);
    free(paths);

    return res;
}

void free_result(Result *r) {
    if (!r) return;
    if (r->files) {
        for (int i = 0; i < r->count; ++i) free(r->files[i]);
        free(r->files);
    }
    if (r->hashes) {
        for (int i = 0; i < r->count; ++i) free(r->hashes[i]);
        free(r->hashes);
    }
    r->files = NULL; r->hashes = NULL; r->count = 0;
}

// Helpers for tests
static int write_file(const char *path, size_t size_bytes, unsigned char seed) {
    // Ensure directory exists
    char dir[PATH_MAX];
    strncpy(dir, path, sizeof(dir));
    dir[sizeof(dir)-1] = '\0';
    char *slash = strrchr(dir, '/');
    if (slash) {
        *slash = '\0';
        mkdir(dir, 0700);
    }
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    unsigned char *buf = (unsigned char*)malloc(8192);
    if (!buf) { fclose(f); return -1; }
    memset(buf, seed, 8192);
    size_t remaining = size_bytes;
    unsigned char s = seed;
    while (remaining > 0) {
        size_t to_write = remaining < 8192 ? remaining : 8192;
        fwrite(buf, 1, to_write, f);
        remaining -= to_write;
        s = (unsigned char)(s + 1);
        if (to_write == 8192) memset(buf, s, 8192);
    }
    free(buf);
    fclose(f);
    return 0;
}

static void print_results(const char *title, const Result *r) {
    printf("=== %s ===\n", title);
    if (!r || r->count == 0) {
        printf("\n");
        return;
    }
    // Sort by file name for stable output
    int n = r->count;
    int *idx = (int*)malloc((size_t)n * sizeof(int));
    for (int i = 0; i < n; ++i) idx[i] = i;
    // Simple insertion sort
    for (int i = 1; i < n; ++i) {
        int j = i;
        int key = idx[i];
        while (j > 0 && strcmp(r->files[idx[j-1]], r->files[key]) > 0) {
            idx[j] = idx[j-1];
            j--;
        }
        idx[j] = key;
    }
    for (int i = 0; i < n; ++i) {
        int k = idx[i];
        printf("%s -> %s\n", r->files[k], r->hashes[k] ? r->hashes[k] : "");
    }
    free(idx);
    printf("\n");
}

static void make_dir(char *out, size_t outsz, const char *base) {
    snprintf(out, outsz, "%s/task0-%ld", "/tmp", (long)time(NULL));
    mkdir(out, 0700);
    // Ensure uniqueness per test
    char uniq[64];
    snprintf(uniq, sizeof(uniq), "%s-%ld", base, random());
    strncat(out, "/", outsz - strlen(out) - 1);
    strncat(out, uniq, outsz - strlen(out) - 1);
    mkdir(out, 0700);
}

int main(void) {
    // Seed randomness
    srandom((unsigned int)time(NULL) ^ (unsigned int)getpid());

    // Test 1: 3 small files, 1 thread
    char dir1[PATH_MAX]; make_dir(dir1, sizeof(dir1), "tc1");
    char p1[PATH_MAX]; snprintf(p1, sizeof(p1), "%s/a.txt", dir1); write_file(p1, 1024, 1);
    char p2[PATH_MAX]; snprintf(p2, sizeof(p2), "%s/b.txt", dir1); write_file(p2, 2048, 2);
    char p3[PATH_MAX]; snprintf(p3, sizeof(p3), "%s/c.txt", dir1); write_file(p3, 4096, 3);
    Result r1 = process_directory(dir1, 1);
    print_results("Test 1 (3 files, 1 thread)", &r1);
    free_result(&r1);

    // Test 2: 10 varied files, 2 threads
    char dir2[PATH_MAX]; make_dir(dir2, sizeof(dir2), "tc2");
    for (int i = 0; i < 10; ++i) {
        char path[PATH_MAX];
        snprintf(path, sizeof(path), "%s/file%d.bin", dir2, i);
        write_file(path, (size_t)((i + 1) * 1500), (unsigned char)(10 + i));
    }
    Result r2 = process_directory(dir2, 2);
    print_results("Test 2 (10 files, 2 threads)", &r2);
    free_result(&r2);

    // Test 3: 5 files incl empty, 4 threads
    char dir3[PATH_MAX]; make_dir(dir3, sizeof(dir3), "tc3");
    char pe[PATH_MAX]; snprintf(pe, sizeof(pe), "%s/empty.dat", dir3); write_file(pe, 0, 0);
    for (int i = 0; i < 4; ++i) {
        char path[PATH_MAX];
        snprintf(path, sizeof(path), "%s/d%d.dat", dir3, i);
        write_file(path, (size_t)((i + 1) * 8192), (unsigned char)(20 + i));
    }
    Result r3 = process_directory(dir3, 4);
    print_results("Test 3 (5 files incl empty, 4 threads)", &r3);
    free_result(&r3);

    // Test 4: empty directory, 3 threads
    char dir4[PATH_MAX]; make_dir(dir4, sizeof(dir4), "tc4");
    Result r4 = process_directory(dir4, 3);
    print_results("Test 4 (0 files, 3 threads)", &r4);
    free_result(&r4);

    // Test 5: 20 larger files, 8 threads
    char dir5[PATH_MAX]; make_dir(dir5, sizeof(dir5), "tc5");
    for (int i = 0; i < 20; ++i) {
        char path[PATH_MAX];
        snprintf(path, sizeof(path), "%s/large_%02d.bin", dir5, i);
        size_t size = 200 * 1024 + (size_t)(random() % (50 * 1024 + 1));
        write_file(path, size, (unsigned char)(30 + i));
    }
    Result r5 = process_directory(dir5, 8);
    print_results("Test 5 (20 larger files, 8 threads)", &r5);
    free_result(&r5);

    return 0;
}