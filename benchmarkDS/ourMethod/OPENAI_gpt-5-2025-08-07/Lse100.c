#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int create_important_config(const char *base_dir) {
    if (base_dir == NULL || base_dir[0] == '\0') {
        return -1;
    }

    int dir_flags = O_RDONLY;
#ifdef O_DIRECTORY
    dir_flags |= O_DIRECTORY;
#endif
#ifdef O_CLOEXEC
    dir_flags |= O_CLOEXEC;
#endif
#ifdef O_NOFOLLOW
    dir_flags |= O_NOFOLLOW;
#endif

    int dirfd = open(base_dir, dir_flags);
    if (dirfd < 0) {
        return -2;
    }

    struct stat st;
    if (fstat(dirfd, &st) != 0) {
        int saved = errno;
        close(dirfd);
        errno = saved;
        return -3;
    }
    if (!S_ISDIR(st.st_mode)) {
        close(dirfd);
        return -4;
    }

    const char *filename = "important_config";
    int flags = O_WRONLY | O_CREAT | O_EXCL;
#ifdef O_CLOEXEC
    flags |= O_CLOEXEC;
#endif
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif

    int fd = openat(dirfd, filename, flags, 0600);
    if (fd < 0) {
        int e = errno;
        close(dirfd);
        if (e == EEXIST) {
            return 1;
        }
        if (e == ELOOP) {
            return -5;
        }
        return -6;
    }

    struct stat fst;
    if (fstat(fd, &fst) != 0) {
        int saved = errno;
        close(fd);
        close(dirfd);
        errno = saved;
        return -7;
    }
    if (!S_ISREG(fst.st_mode)) {
        close(fd);
        close(dirfd);
        return -8;
    }

    // Persist metadata to disk (best-effort)
    (void)fsync(fd);
    close(fd);
    (void)fsync(dirfd);
    close(dirfd);

    return 0;
}

static void print_result(int r) {
    printf("Result: %d\n", r);
}

static char *make_temp_dir(const char *prefix) {
    size_t len = strlen(prefix) + 7 + 1;
    char *tmpl = (char *)malloc(len);
    if (!tmpl) return NULL;
    snprintf(tmpl, len, "%sXXXXXX", prefix);
    if (!mkdtemp(tmpl)) {
        free(tmpl);
        return NULL;
    }
    return tmpl; // Caller must free
}

int main(void) {
    char *base = make_temp_dir("/tmp/secure_base_c_");
    if (!base) {
        printf("Setup error\n");
        return 0;
    }

    // Test 1: create in base
    print_result(create_important_config(base));
    // Test 2: create again (exists)
    print_result(create_important_config(base));

    // Test 3: nestedA
    char nestedA[1024];
    snprintf(nestedA, sizeof(nestedA), "%s/%s", base, "nestedA");
    mkdir(nestedA, 0700);
    print_result(create_important_config(nestedA));

    // Test 4: nestedB
    char nestedB[1024];
    snprintf(nestedB, sizeof(nestedB), "%s/%s", base, "nestedB");
    mkdir(nestedB, 0700);
    print_result(create_important_config(nestedB));

    // Test 5: symlink to nestedA (best-effort)
#if defined(__unix__) || defined(__APPLE__)
    char linkPath[1024];
    snprintf(linkPath, sizeof(linkPath), "%s/%s", base, "link_to_nestedA");
    unlink(linkPath);
    if (symlink(nestedA, linkPath) == 0) {
        print_result(create_important_config(linkPath));
    } else {
        print_result(create_important_config(base));
    }
#else
    print_result(create_important_config(base));
#endif

    free(base);
    return 0;
}