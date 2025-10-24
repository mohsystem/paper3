#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct {
    char* path;
    unsigned long long checksum;
} Item;

typedef struct {
    Item* items;
    int count;
    int index;
    pthread_mutex_t mtx;
} WorkQueue;

typedef struct {
    WorkQueue* q;
} WorkerArg;

static int is_regular_file(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return S_ISREG(st.st_mode);
}

static void* worker_thread(void* arg) {
    WorkerArg* wa = (WorkerArg*)arg;
    for (;;) {
        int myIndex = -1;
        pthread_mutex_lock(&wa->q->mtx);
        if (wa->q->index < wa->q->count) {
            myIndex = wa->q->index;
            wa->q->index++;
        }
        pthread_mutex_unlock(&wa->q->mtx);
        if (myIndex == -1) break;

        Item* it = &wa->q->items[myIndex];
        unsigned long long sum = 0;
        FILE* f = fopen(it->path, "rb");
        if (!f) {
            it->checksum = (unsigned long long)(-1);
            continue;
        }
        unsigned char buf[8192];
        size_t r;
        while ((r = fread(buf, 1, sizeof(buf), f)) > 0) {
            for (size_t i = 0; i < r; ++i) sum += buf[i];
        }
        fclose(f);
        it->checksum = sum;
    }
    return NULL;
}

// Returns the number of files processed
int process_directory(const char* dir_path, int thread_count) {
    DIR* dir = opendir(dir_path);
    if (!dir) {
        fprintf(stderr, "Not a directory or cannot open: %s\n", dir_path);
        return -1;
    }

    // Collect files
    Item* items = NULL;
    int cap = 16, count = 0;
    items = (Item*)malloc(sizeof(Item) * cap);
    struct dirent* ent;
    while ((ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
        char* full = NULL;
        size_t len = strlen(dir_path) + 1 + strlen(ent->d_name) + 1;
        full = (char*)malloc(len);
        snprintf(full, len, "%s/%s", dir_path, ent->d_name);
        if (!is_regular_file(full)) {
            free(full);
            continue;
        }
        if (count == cap) {
            cap *= 2;
            items = (Item*)realloc(items, sizeof(Item) * cap);
        }
        items[count].path = full;
        items[count].checksum = 0;
        count++;
    }
    closedir(dir);

    WorkQueue q;
    q.items = items;
    q.count = count;
    q.index = 0;
    pthread_mutex_init(&q.mtx, NULL);

    int n = thread_count > 0 ? thread_count : 1;
    pthread_t* th = (pthread_t*)malloc(sizeof(pthread_t) * (size_t)n);
    WorkerArg wa;
    wa.q = &q;
    for (int i = 0; i < n; ++i) {
        pthread_create(&th[i], NULL, worker_thread, &wa);
    }
    for (int i = 0; i < n; ++i) {
        pthread_join(th[i], NULL);
    }

    // Print results
    for (int i = 0; i < count; ++i) {
        // Extract basename for display
        const char* base = strrchr(items[i].path, '/');
        base = base ? base + 1 : items[i].path;
        printf("  %s -> checksum=%llu\n", base, (unsigned long long)items[i].checksum);
    }

    for (int i = 0; i < count; ++i) free(items[i].path);
    free(items);
    free(th);
    pthread_mutex_destroy(&q.mtx);

    return count;
}

static char* create_test_dir(const char* prefix) {
    char* tmpl = (char*)malloc(strlen("/tmp/") + strlen(prefix) + 16);
    sprintf(tmpl, "/tmp/%sXXXXXX", prefix);
    char* path = mkdtemp(tmpl);
    if (!path) {
        free(tmpl);
        return NULL;
    }
    return tmpl; // mkdtemp modifies tmpl in-place, return it
}

static void create_random_file(const char* dir, const char* name, int size_kb) {
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", dir, name);
    FILE* f = fopen(path, "wb");
    if (!f) return;
    unsigned char buf[8192];
    int remaining = size_kb * 1024;
    while (remaining > 0) {
        int chunk = remaining > (int)sizeof(buf) ? (int)sizeof(buf) : remaining;
        for (int i = 0; i < chunk; ++i) buf[i] = (unsigned char)(rand() & 0xFF);
        fwrite(buf, 1, (size_t)chunk, f);
        remaining -= chunk;
    }
    fclose(f);
}

static void run_test_case(const char* name, int files, int threads) {
    char prefix[64];
    snprintf(prefix, sizeof(prefix), "%s_", name);
    char* dir = create_test_dir(prefix);
    if (!dir) {
        fprintf(stderr, "Failed to create temp dir\n");
        return;
    }
    printf("Test %s - dir: %s threads: %d\n", name, dir, threads);
    for (int i = 0; i < files; ++i) {
        char fname[64];
        snprintf(fname, sizeof(fname), "file_%d.dat", i);
        int size_kb = 32 + (rand() % 97);
        create_random_file(dir, fname, size_kb);
    }
    int processed = process_directory(dir, threads);
    printf("\n");
    (void)processed;
    // Note: temp dirs are not removed for inspection; could add cleanup if desired.
    free(dir);
}

int main(void) {
    srand((unsigned)time(NULL));
    run_test_case("case1", 5, 1);
    run_test_case("case2", 6, 2);
    run_test_case("case3", 7, 3);
    run_test_case("case4", 8, 4);
    run_test_case("case5", 9, 5);
    return 0;
}