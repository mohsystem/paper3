#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif
#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif
#ifndef O_DIRECTORY
#define O_DIRECTORY 0
#endif

// Writes the string "important_config" into a file named "important_config" inside base_dir.
// Returns 0 on success, -1 on failure.
int write_important_config(const char* base_dir) {
    if (base_dir == NULL || base_dir[0] == '\0') {
        return -1;
    }

    int dfd = open(base_dir, O_RDONLY | O_CLOEXEC | O_NOFOLLOW | O_DIRECTORY);
    if (dfd < 0) {
        // Fallback without O_DIRECTORY; validate after open
        dfd = open(base_dir, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
        if (dfd < 0) {
            return -1;
        }
        struct stat dst;
        if (fstat(dfd, &dst) != 0 || !S_ISDIR(dst.st_mode)) {
            close(dfd);
            return -1;
        }
    }

    int fd = openat(dfd, "important_config", O_RDWR | O_CREAT | O_CLOEXEC | O_NOFOLLOW, 0644);
    if (fd < 0) {
        close(dfd);
        return -1;
    }

    struct stat st;
    if (fstat(fd, &st) != 0 || !S_ISREG(st.st_mode)) {
        close(fd);
        close(dfd);
        return -1;
    }

    if (ftruncate(fd, 0) != 0) {
        close(fd);
        close(dfd);
        return -1;
    }

    const char* content = "important_config";
    size_t len = strlen(content);
    size_t off = 0;
    while (off < len) {
        ssize_t n = write(fd, content + off, len - off);
        if (n <= 0) {
            close(fd);
            close(dfd);
            return -1;
        }
        off += (size_t)n;
    }

    // Best-effort fsync to flush content
    (void)fsync(fd);

    close(fd);
    close(dfd);
    return 0;
}

int main(void) {
    // 5 test cases
    char tmpl1[] = "/tmp/c_icase1_XXXXXX";
    char* dir1 = mkdtemp(tmpl1);
    printf("Test1: %s\n", (dir1 && write_important_config(dir1) == 0) ? "true" : "false");

    // Test2: write again to same dir
    printf("Test2: %s\n", (dir1 && write_important_config(dir1) == 0) ? "true" : "false");

    // Test3: base_dir is a file (should fail)
    char tfile[] = "/tmp/c_icase3_XXXXXX";
    int tfd = mkstemp(tfile);
    if (tfd >= 0) close(tfd);
    printf("Test3: %s\n", write_important_config(tfile) == 0 ? "true" : "false");

    // Test4: non-existent directory (should fail)
    char nonexist[512];
    snprintf(nonexist, sizeof(nonexist), "%s/%s", dir1 ? dir1 : "/tmp", "nonexistent_subdir_do_not_create");
    printf("Test4: %s\n", write_important_config(nonexist) == 0 ? "true" : "false");

    // Test5: valid subdir
    char tmpl5[] = "/tmp/c_icase5_XXXXXX";
    char* dir5 = mkdtemp(tmpl5);
    char sub[512];
    snprintf(sub, sizeof(sub), "%s/conf", dir5 ? dir5 : "/tmp");
    mkdir(sub, 0700);
    printf("Test5: %s\n", write_important_config(sub) == 0 ? "true" : "false");

    return 0;
}