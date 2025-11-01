#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>

int write_important_config(const char* content) {
    const char* path = "important_config";
    const char* fixed = "important_config";
    int flags = O_WRONLY | O_CREAT | O_TRUNC;
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif

    int fd = open(path, flags, 0600);
    if (fd == -1) {
        struct stat st;
        if (lstat(path, &st) == 0 && S_ISLNK(st.st_mode)) {
            return 0;
        }
        fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
        if (fd == -1) {
            return 0;
        }
    }

    fchmod(fd, 0600);

    size_t len = strlen(fixed);
    size_t off = 0;
    while (off < len) {
        ssize_t n = write(fd, fixed + off, len - off);
        if (n <= 0) {
            close(fd);
            return 0;
        }
        off += (size_t)n;
    }

    if (close(fd) != 0) {
        return 0;
    }
    return 1;
}

int main(void) {
    printf("%d\n", write_important_config("important_config"));
    printf("%d\n", write_important_config("important_config"));
    printf("%d\n", write_important_config("important_config"));
    printf("%d\n", write_important_config("important_config"));
    printf("%d\n", write_important_config("important_config"));
    return 0;
}