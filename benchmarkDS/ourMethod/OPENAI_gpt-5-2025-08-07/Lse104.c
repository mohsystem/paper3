#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <limits.h>

#define MAX_PATH_LEN 4096
static const char *CONTENT = "important_config";

static int split_components(const char *rel, char comps[][256], size_t *count) {
    *count = 0;
    size_t len = strlen(rel);
    if (len == 0) return -1;
    if (rel[0] == '/') return -1;
    size_t i = 0;
    while (i <= len) {
        char seg[256];
        size_t s = 0;
        while (i < len && rel[i] != '/') {
            if (s + 1 >= sizeof(seg)) return -1;
            seg[s++] = rel[i++];
        }
        if (i == len || rel[i] == '/') {
            // delimiter or end
        }
        seg[s] = '\0';
        if (s == 0 || strcmp(seg, ".") == 0 || strcmp(seg, "..") == 0) return -1;
        strncpy(comps[*count], seg, 255);
        comps[*count][255] = '\0';
        (*count)++;
        if (i == len) break;
        // skip '/'
        i++;
    }
    return *count > 0 ? 0 : -1;
}

static int safe_write_important_config(const char *base_dir, const char *rel_path) {
    if (!base_dir || !rel_path) return 0;
    if (strlen(base_dir) == 0 || strlen(rel_path) == 0) return 0;
    if (strlen(rel_path) > MAX_PATH_LEN) return 0;
    if (strchr(base_dir, '\0') == NULL || strchr(rel_path, '\0') == NULL) { /* always true */ }

    // Split components
    char comps[128][256];
    size_t comp_count = 0;
    if (split_components(rel_path, comps, &comp_count) != 0) return 0;

    int basefd = open(base_dir, O_RDONLY | O_CLOEXEC);
    if (basefd < 0) return 0;

    struct stat st;
    if (fstat(basefd, &st) != 0 || !S_ISDIR(st.st_mode)) {
        close(basefd);
        return 0;
    }

    int dirfd = basefd;
    for (size_t i = 0; i + 1 < comp_count; ++i) {
        int nfd = openat(dirfd, comps[i], O_RDONLY | O_CLOEXEC | O_NOFOLLOW | O_DIRECTORY);
        if (nfd < 0) {
            if (dirfd != basefd) close(dirfd);
            close(basefd);
            return 0;
        }
        struct stat st2;
        if (fstat(nfd, &st2) != 0 || !S_ISDIR(st2.st_mode)) {
            close(nfd);
            if (dirfd != basefd) close(dirfd);
            close(basefd);
            return 0;
        }
        if (dirfd != basefd) close(dirfd);
        dirfd = nfd;
    }

    const char *finalName = comps[comp_count - 1];
    if (strlen(finalName) == 0 || strlen(finalName) > 255) {
        if (dirfd != basefd) close(dirfd);
        close(basefd);
        return 0;
    }

    // Check if destination is a symlink (best-effort)
    int dstfd = openat(dirfd, finalName, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (dstfd >= 0) {
        // opened a non-symlink; close
        close(dstfd);
    } else {
        if (errno == ELOOP) {
            if (dirfd != basefd) close(dirfd);
            close(basefd);
            return 0;
        }
    }

    // Create unique temp name
    unsigned char rnd[8];
    int ur = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
    if (ur >= 0) {
        ssize_t got = read(ur, rnd, sizeof(rnd));
        close(ur);
        if (got != (ssize_t)sizeof(rnd)) {
            for (size_t i = 0; i < sizeof(rnd); ++i) rnd[i] = (unsigned char)((getpid() + i * 17 + time(NULL)) & 0xFF);
        }
    } else {
        for (size_t i = 0; i < sizeof(rnd); ++i) rnd[i] = (unsigned char)((getpid() + i * 17 + time(NULL)) & 0xFF);
    }
    char tmpname[128];
    snprintf(tmpname, sizeof(tmpname), ".tmp-%02x%02x%02x%02x%02x%02x%02x%02x-%d",
             rnd[0], rnd[1], rnd[2], rnd[3], rnd[4], rnd[5], rnd[6], rnd[7], getpid());

    int tfd = openat(dirfd, tmpname, O_RDWR | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);
    if (tfd < 0) {
        if (dirfd != basefd) close(dirfd);
        close(basefd);
        return 0;
    }

    int ok = 0;
    do {
        struct stat stt;
        if (fstat(tfd, &stt) != 0 || !S_ISREG(stt.st_mode)) break;

        size_t len = strlen(CONTENT);
        const char *p = CONTENT;
        while (len > 0) {
            ssize_t w = write(tfd, p, len);
            if (w < 0) { ok = 0; break; }
            p += (size_t)w;
            len -= (size_t)w;
        }
        if (len != 0) break;

        if (fsync(tfd) != 0) break;
        if (close(tfd) != 0) { tfd = -1; break; }
        tfd = -1;

        if (renameat(dirfd, tmpname, dirfd, finalName) != 0) break;
        if (fsync(dirfd) != 0) break;

        ok = 1;
    } while (0);

    if (!ok) {
        if (tfd >= 0) close(tfd);
        unlinkat(dirfd, tmpname, 0);
    }

    if (dirfd != basefd) close(dirfd);
    close(basefd);
    return ok;
}

int main(void) {
    char tmpl[] = "/tmp/safe-base-XXXXXX";
    char *base = mkdtemp(tmpl);
    if (!base) {
        printf("Setup failed\n");
        return 0;
    }
    char subdir[PATH_MAX];
    snprintf(subdir, sizeof(subdir), "%s/%s", base, "subdir");
    mkdir(subdir, 0700);

    const char *tests[5] = {
        "config.txt",            // OK
        "subdir/settings.conf",  // OK
        "../escape.txt",         // FAIL
        "/abs/path.txt",         // FAIL
        "nested/../final.conf"   // FAIL
    };

    for (int i = 0; i < 5; ++i) {
        int res = safe_write_important_config(base, tests[i]);
        printf("%s -> %s\n", tests[i], res ? "OK" : "FAIL");
    }
    return 0;
}