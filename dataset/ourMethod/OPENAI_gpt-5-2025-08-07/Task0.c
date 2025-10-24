// Concurrent file processing system in C with secure file handling.
// Builds on POSIX systems: gcc -Wall -Wextra -O2 -pthread task.c -o task
// This program scans a directory, enqueues regular files, and processes them concurrently.
// Each file is processed by only one thread at a time using per-file lock files.

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <inttypes.h>

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif

#ifndef NAME_MAX
#define NAME_MAX 255
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

// Policy limits
#define READ_CHUNK (64 * 1024)
#define MAX_FILE_SIZE (10LL * 1024LL * 1024LL * 1024LL) // 10 GiB, adjust as needed
#define MAX_NAME_LEN 255

// Work queue for file names (single producer, multiple consumers)
typedef struct {
    pthread_mutex_t mtx;
    pthread_cond_t  cv_not_empty;
    pthread_cond_t  cv_not_full;
    char          **items;
    size_t          capacity;
    size_t          head;
    size_t          tail;
    size_t          count;
    bool            closed; // no more items to be pushed
} WorkQueue;

static int wq_init(WorkQueue *q, size_t initial_capacity) {
    if (!q || initial_capacity == 0) return -1;
    q->items = (char**)calloc(initial_capacity, sizeof(char*));
    if (!q->items) return -1;
    if (pthread_mutex_init(&q->mtx, NULL) != 0) { free(q->items); q->items = NULL; return -1; }
    if (pthread_cond_init(&q->cv_not_empty, NULL) != 0) { pthread_mutex_destroy(&q->mtx); free(q->items); q->items = NULL; return -1; }
    if (pthread_cond_init(&q->cv_not_full, NULL) != 0) { pthread_cond_destroy(&q->cv_not_empty); pthread_mutex_destroy(&q->mtx); free(q->items); q->items = NULL; return -1; }
    q->capacity = initial_capacity;
    q->head = q->tail = q->count = 0;
    q->closed = false;
    return 0;
}

static void wq_close(WorkQueue *q) {
    if (!q) return;
    pthread_mutex_lock(&q->mtx);
    q->closed = true;
    pthread_cond_broadcast(&q->cv_not_empty);
    pthread_mutex_unlock(&q->mtx);
}

static void wq_destroy(WorkQueue *q) {
    if (!q) return;
    pthread_mutex_lock(&q->mtx);
    // Free any remaining items
    for (size_t i = 0; i < q->count; i++) {
        size_t idx = (q->head + i) % q->capacity;
        free(q->items[idx]);
        q->items[idx] = NULL;
    }
    pthread_mutex_unlock(&q->mtx);
    pthread_cond_destroy(&q->cv_not_full);
    pthread_cond_destroy(&q->cv_not_empty);
    pthread_mutex_destroy(&q->mtx);
    free(q->items);
    q->items = NULL;
    q->capacity = q->head = q->tail = q->count = 0;
    q->closed = true;
}

// Internal: grow ring buffer; must be called with mutex held
static int wq_grow_unlocked(WorkQueue *q) {
    size_t newcap = q->capacity * 2;
    if (newcap < q->capacity) return -1; // overflow
    char **newitems = (char**)calloc(newcap, sizeof(char*));
    if (!newitems) return -1;
    // Copy in order
    for (size_t i = 0; i < q->count; i++) {
        size_t idx = (q->head + i) % q->capacity;
        newitems[i] = q->items[idx];
    }
    free(q->items);
    q->items = newitems;
    q->capacity = newcap;
    q->head = 0;
    q->tail = q->count;
    return 0;
}

// Push a copy of name into queue. Returns 0 on success, -1 on error, 1 if queue closed.
static int wq_push(WorkQueue *q, const char *name) {
    if (!q || !name) return -1;
    int rc = 0;
    pthread_mutex_lock(&q->mtx);
    if (q->closed) {
        rc = 1;
        goto out;
    }
    if (q->count == q->capacity) {
        if (wq_grow_unlocked(q) != 0) {
            rc = -1;
            goto out;
        }
    }
    size_t len = strnlen(name, MAX_NAME_LEN + 1);
    if (len == 0 || len > MAX_NAME_LEN) {
        rc = -1;
        goto out;
    }
    char *copy = (char*)malloc(len + 1);
    if (!copy) { rc = -1; goto out; }
    memcpy(copy, name, len);
    copy[len] = '\0';
    q->items[q->tail] = copy;
    q->tail = (q->tail + 1) % q->capacity;
    q->count++;
    pthread_cond_signal(&q->cv_not_empty);
out:
    pthread_mutex_unlock(&q->mtx);
    return rc;
}

// Pop an item. Returns 1 and sets *out_name on success, 0 if empty and closed, -1 on error.
// Caller owns the returned string and must free it.
static int wq_pop(WorkQueue *q, char **out_name) {
    if (!q || !out_name) return -1;
    pthread_mutex_lock(&q->mtx);
    for (;;) {
        if (q->count > 0) {
            char *name = q->items[q->head];
            q->items[q->head] = NULL;
            q->head = (q->head + 1) % q->capacity;
            q->count--;
            *out_name = name;
            pthread_mutex_unlock(&q->mtx);
            return 1;
        }
        if (q->closed) {
            pthread_mutex_unlock(&q->mtx);
            return 0;
        }
        pthread_cond_wait(&q->cv_not_empty, &q->mtx);
    }
}

// Results aggregator
typedef struct {
    char    *name;
    int      status;    // 0=success, else errno-like
    uint64_t checksum;  // valid if status==0
    off_t    size;      // valid if status==0
} FileResult;

typedef struct {
    pthread_mutex_t mtx;
    FileResult     *items;
    size_t          count;
    size_t          capacity;
} ResultAggregator;

static int ra_init(ResultAggregator *ra, size_t initial_capacity) {
    if (!ra || initial_capacity == 0) return -1;
    ra->items = (FileResult*)calloc(initial_capacity, sizeof(FileResult));
    if (!ra->items) return -1;
    ra->count = 0;
    ra->capacity = initial_capacity;
    if (pthread_mutex_init(&ra->mtx, NULL) != 0) { free(ra->items); ra->items = NULL; return -1; }
    return 0;
}

static void ra_destroy(ResultAggregator *ra) {
    if (!ra) return;
    pthread_mutex_lock(&ra->mtx);
    for (size_t i = 0; i < ra->count; i++) {
        free(ra->items[i].name);
        ra->items[i].name = NULL;
    }
    free(ra->items);
    ra->items = NULL;
    ra->count = ra->capacity = 0;
    pthread_mutex_unlock(&ra->mtx);
    pthread_mutex_destroy(&ra->mtx);
}

static int ra_add(ResultAggregator *ra, const char *name, int status, uint64_t checksum, off_t size) {
    if (!ra || !name) return -1;
    int rc = 0;
    pthread_mutex_lock(&ra->mtx);
    if (ra->count == ra->capacity) {
        size_t newcap = ra->capacity * 2;
        if (newcap < ra->capacity) { rc = -1; goto out; }
        FileResult *newitems = (FileResult*)realloc(ra->items, newcap * sizeof(FileResult));
        if (!newitems) { rc = -1; goto out; }
        // Zero new space
        memset(newitems + ra->capacity, 0, (newcap - ra->capacity) * sizeof(FileResult));
        ra->items = newitems;
        ra->capacity = newcap;
    }
    size_t len = strnlen(name, MAX_NAME_LEN + 1);
    if (len == 0 || len > MAX_NAME_LEN) { rc = -1; goto out; }
    char *copy = (char*)malloc(len + 1);
    if (!copy) { rc = -1; goto out; }
    memcpy(copy, name, len);
    copy[len] = '\0';
    ra->items[ra->count].name = copy;
    ra->items[ra->count].status = status;
    ra->items[ra->count].checksum = checksum;
    ra->items[ra->count].size = size;
    ra->count++;
out:
    pthread_mutex_unlock(&ra->mtx);
    return rc;
}

static void ra_print(const ResultAggregator *ra) {
    if (!ra) return;
    // It's okay to read without lock after join in this program's flow
    for (size_t i = 0; i < ra->count; i++) {
        const FileResult *r = &ra->items[i];
        if (r->status == 0) {
            printf("Processed: %s size=%jd checksum=%" PRIu64 "\n", r->name, (intmax_t)r->size, r->checksum);
        } else {
            printf("Skipped/Failed: %s error=%d\n", r->name, r->status);
        }
    }
}

// Utility: check filename is a simple base name (no slashes, not "." or "..", reasonable length)
static bool is_valid_basename(const char *name) {
    if (!name) return false;
    size_t len = strnlen(name, MAX_NAME_LEN + 1);
    if (len == 0 || len > MAX_NAME_LEN) return false;
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) return false;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)name[i];
        if (c == '/') return false;
        if (c == '\0') break;
    }
    // Optionally skip hidden files and lock files
    if (name[0] == '.') return false;
    const char *dot = strrchr(name, '.');
    if (dot && (strcmp(dot, ".lock") == 0 || strcmp(dot, ".out") == 0 || strcmp(dot, ".tmp") == 0)) {
        return false;
    }
    return true;
}

// Securely open a directory and validate it's a directory
static int safe_open_dir(const char *basepath) {
    if (!basepath) return -1;
    int dirfd = open(basepath, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dirfd < 0) return -1;
    struct stat st;
    if (fstat(dirfd, &st) != 0 || !S_ISDIR(st.st_mode)) {
        close(dirfd);
        return -1;
    }
    return dirfd;
}

// Compute 64-bit FNV-1a hash for chunk (simple checksum)
static uint64_t fnv1a_update(uint64_t hash, const uint8_t *data, size_t len) {
    const uint64_t FNV_prime = 1099511628211ULL;
    for (size_t i = 0; i < len; i++) {
        hash ^= (uint64_t)data[i];
        hash *= FNV_prime;
    }
    return hash;
}

// Process an already opened and validated file descriptor by reading in chunks.
// Returns 0 on success with checksum_out and size_out set; -1 on error.
static int process_file_fd(int fd, uint64_t *checksum_out, off_t *size_out) {
    if (!checksum_out || !size_out) return -1;
    uint8_t *buf = (uint8_t*)malloc(READ_CHUNK);
    if (!buf) return -1;
    uint64_t h = 1469598103934665603ULL; // FNV offset basis
    off_t total = 0;
    for (;;) {
        ssize_t n = read(fd, buf, READ_CHUNK);
        if (n < 0) {
            if (errno == EINTR) continue;
            free(buf);
            return -1;
        }
        if (n == 0) break;
        if ((uint64_t)total + (uint64_t)n < (uint64_t)total) { // overflow check
            free(buf);
            errno = EOVERFLOW;
            return -1;
        }
        total += n;
        h = fnv1a_update(h, buf, (size_t)n);
    }
    free(buf);
    *checksum_out = h;
    *size_out = total;
    return 0;
}

// Try to acquire a per-file lock by creating a lock file alongside the target.
// Returns 1 on lock acquired (lockfd_out valid), 0 if already locked by others, -1 on error.
static int try_lock_file(int dirfd, const char *basename, int *lockfd_out) {
    if (!basename || !lockfd_out) return -1;
    char lockname[NAME_MAX + 6];
    int r = snprintf(lockname, sizeof(lockname), "%s.lock", basename);
    if (r <= 0 || (size_t)r >= sizeof(lockname)) { errno = ENAMETOOLONG; return -1; }
    int fd = openat(dirfd, lockname, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);
    if (fd < 0) {
        if (errno == EEXIST) return 0;
        return -1;
    }
    *lockfd_out = fd;
    return 1;
}

static void unlock_file(int dirfd, const char *basename, int lockfd) {
    if (!basename) return;
    char lockname[NAME_MAX + 6];
    int r = snprintf(lockname, sizeof(lockname), "%s.lock", basename);
    if (r <= 0 || (size_t)r >= sizeof(lockname)) { close(lockfd); return; }
    close(lockfd);
    (void)unlinkat(dirfd, lockname, 0);
}

// Worker arguments
typedef struct {
    int dirfd;
    WorkQueue *queue;
    ResultAggregator *results;
} WorkerArgs;

static int validate_open_file(int dirfd, const char *basename, int *out_fd, off_t *size_out) {
    if (!basename || !out_fd || !size_out) return -1;
    int fd = openat(dirfd, basename, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd < 0) return -1;
    struct stat st;
    if (fstat(fd, &st) != 0) { close(fd); return -1; }
    if (!S_ISREG(st.st_mode)) { close(fd); errno = EISDIR; return -1; }
    if (st.st_size < 0 || (uint64_t)st.st_size > (uint64_t)MAX_FILE_SIZE) { close(fd); errno = EFBIG; return -1; }
    *out_fd = fd;
    *size_out = st.st_size;
    return 0;
}

static void* worker_main(void *arg) {
    WorkerArgs *wa = (WorkerArgs*)arg;
    if (!wa || !wa->queue || !wa->results) return NULL;
    for (;;) {
        char *name = NULL;
        int pr = wq_pop(wa->queue, &name);
        if (pr <= 0) break; // closed and empty or error
        int lockfd = -1;
        int lrc = try_lock_file(wa->dirfd, name, &lockfd);
        if (lrc <= 0) {
            // Either locked by someone else or error. Record and continue.
            int err = (lrc < 0) ? errno : EBUSY;
            (void)ra_add(wa->results, name, err, 0, 0);
            free(name);
            continue;
        }
        int fd = -1;
        off_t size = 0;
        int vrc = validate_open_file(wa->dirfd, name, &fd, &size);
        if (vrc != 0) {
            int err = errno;
            (void)ra_add(wa->results, name, err, 0, 0);
            unlock_file(wa->dirfd, name, lockfd);
            free(name);
            continue;
        }
        uint64_t checksum = 0;
        int prc = process_file_fd(fd, &checksum, &size);
        int err = (prc == 0) ? 0 : errno;
        close(fd);
        (void)ra_add(wa->results, name, err, checksum, size);
        unlock_file(wa->dirfd, name, lockfd);
        free(name);
    }
    return NULL;
}

// Scan directory and push valid regular file candidates (names only).
// Actual validation is done after opening in workers.
static int scan_directory_into_queue(int dirfd, WorkQueue *q) {
    if (dirfd < 0 || !q) return -1;
    int dupfd = fcntl(dirfd, F_DUPFD_CLOEXEC, 3);
    if (dupfd < 0) return -1;
    DIR *dp = fdopendir(dupfd);
    if (!dp) { close(dupfd); return -1; }
    int rc = 0;
    for (;;) {
        errno = 0;
        struct dirent *de = readdir(dp);
        if (!de) {
            if (errno != 0) rc = -1;
            break;
        }
        if (!is_valid_basename(de->d_name)) continue;
        // Push name; queue makes a copy
        if (wq_push(q, de->d_name) != 0) { rc = -1; break; }
    }
    closedir(dp); // closes dupfd
    wq_close(q);
    return rc;
}

// Create file with content written repeatedly without loading entire content into memory.
static int create_file_with_pattern(int dirfd, const char *name, const unsigned char *pattern, size_t pattern_len, size_t total_bytes) {
    if (!name || !pattern || pattern_len == 0) return -1;
    int fd = openat(dirfd, name, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);
    if (fd < 0) return -1;
    size_t written = 0;
    while (written < total_bytes) {
        size_t to_write = pattern_len;
        if (to_write > total_bytes - written) to_write = total_bytes - written;
        ssize_t n = write(fd, pattern, to_write);
        if (n < 0) {
            if (errno == EINTR) continue;
            close(fd);
            (void)unlinkat(dirfd, name, 0);
            return -1;
        }
        if (n == 0) break;
        written += (size_t)n;
    }
    // Optionally fsync for durability (not strictly required for the test)
    // fsync(fd);
    close(fd);
    return 0;
}

// Create a temporary directory securely using mkdtemp.
static int make_temp_dir(char *buf, size_t buflen) {
    if (!buf || buflen < 32) return -1;
    const char *tmp = getenv("TMPDIR");
    if (!tmp || tmp[0] == '\0') tmp = "/tmp";
    int r = snprintf(buf, buflen, "%s/cproc.%ld.XXXXXX", tmp, (long)getpid());
    if (r <= 0 || (size_t)r >= buflen) return -1;
    char *path = mkdtemp(buf);
    if (!path) return -1;
    return 0;
}

// Orchestrates the concurrent processing
static int run_processor(const char *basedir, unsigned int thread_count, ResultAggregator *out_results) {
    if (!basedir || thread_count == 0 || !out_results) return -1;
    int dirfd = safe_open_dir(basedir);
    if (dirfd < 0) return -1;

    WorkQueue queue;
    if (wq_init(&queue, 32) != 0) { close(dirfd); return -1; }

    ResultAggregator results;
    if (ra_init(&results, 32) != 0) { wq_destroy(&queue); close(dirfd); return -1; }

    pthread_t *threads = (pthread_t*)calloc(thread_count, sizeof(pthread_t));
    WorkerArgs *args = (WorkerArgs*)calloc(thread_count, sizeof(WorkerArgs));
    if (!threads || !args) {
        free(threads); free(args);
        ra_destroy(&results);
        wq_destroy(&queue);
        close(dirfd);
        return -1;
    }

    for (unsigned int i = 0; i < thread_count; i++) {
        args[i].dirfd = dirfd;
        args[i].queue = &queue;
        args[i].results = &results;
        if (pthread_create(&threads[i], NULL, worker_main, &args[i]) != 0) {
            // If thread creation fails, reduce thread_count and continue
            thread_count = i;
            break;
        }
    }

    int scan_rc = scan_directory_into_queue(dirfd, &queue);
    (void)scan_rc; // scanning errors will stop early; workers will finish as queue closes

    for (unsigned int i = 0; i < thread_count; i++) {
        (void)pthread_join(threads[i], NULL);
    }

    // Move results out
    *out_results = results; // transfer ownership of internal buffers
    // Clean up
    free(threads);
    free(args);
    wq_destroy(&queue);
    close(dirfd);
    return 0;
}

// Test harness: create 5 files and process them with multiple threads
int main(void) {
    char tmpdir[PATH_MAX];
    if (make_temp_dir(tmpdir, sizeof(tmpdir)) != 0) {
        fprintf(stderr, "Failed to create temp dir\n");
        return 1;
    }
    int dirfd = safe_open_dir(tmpdir);
    if (dirfd < 0) {
        fprintf(stderr, "Failed to open temp dir\n");
        return 1;
    }

    // 5 test files
    struct {
        const char *name;
        const char *pattern;
        size_t      plen;
        size_t      total;
    } tests[5] = {
        { "a.txt",  "Hello, world!\n", 14, 14 * 100 },
        { "b.bin",  "\x00\x01\x02\x03\x04\x05\x06\x07", 8, 8 * 25000 },
        { "c.data", "ABCDE", 5, 5 * 100003 },
        { "d.log",  "logline\n", 8, 8 * 50000 },
        { "e.big",  "X", 1, 1024 * 1024 }, // 1 MiB
    };

    for (size_t i = 0; i < 5; i++) {
        if (create_file_with_pattern(dirfd, tests[i].name, (const unsigned char*)tests[i].pattern, tests[i].plen, tests[i].total) != 0) {
            fprintf(stderr, "Failed to create test file %s: %s\n", tests[i].name, strerror(errno));
            close(dirfd);
            return 1;
        }
    }
    close(dirfd);

    ResultAggregator results;
    if (run_processor(tmpdir, 4, &results) != 0) {
        fprintf(stderr, "Processing failed\n");
        return 1;
    }

    // Print results
    ra_print(&results);

    // Cleanup: best-effort remove temp dir and files
    int cfd = safe_open_dir(tmpdir);
    if (cfd >= 0) {
        int dupfd = fcntl(cfd, F_DUPFD_CLOEXEC, 3);
        if (dupfd >= 0) {
            DIR *dp = fdopendir(dupfd);
            if (dp) {
                struct dirent *de;
                while ((de = readdir(dp)) != NULL) {
                    if (!is_valid_basename(de->d_name)) continue;
                    (void)unlinkat(cfd, de->d_name, 0);
                }
                closedir(dp);
            } else {
                close(dupfd);
            }
        }
        close(cfd);
    }
    (void)rmdir(tmpdir);

    ra_destroy(&results);
    return 0;
}