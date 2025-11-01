#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif
#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif

bool create_secure_file(const char* path, mode_t mode) {
    int flags = O_CREAT | O_EXCL | O_WRONLY | O_NOFOLLOW | O_CLOEXEC;
    int fd = open(path, flags, mode);
    if (fd < 0) {
        return false;
    }
    bool ok = true;
    if (fchmod(fd, mode) != 0) ok = false;
    if (close(fd) != 0) ok = false;
    return ok;
}

int main() {
    const char* tests[5] = {
        "important_config",
        "important_config_1",
        "important_config_2",
        "important_config",
        "important_config_3"
    };
    for (int i = 0; i < 5; ++i) {
        bool ok = create_secure_file(tests[i], 0600);
        printf("%s:%s\n", tests[i], ok ? "true" : "false");
    }
    return 0;
}