#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

bool writeImportantConfig(const char* path, const char* content) {
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) return false;

    size_t len = strlen(content);
    size_t written = 0;
    bool ok = true;

    while (written < len) {
        ssize_t n = write(fd, content + written, len - written);
        if (n <= 0) { ok = false; break; }
        written += (size_t)n;
    }

    if (ok) {
        if (fchmod(fd, 0644) == -1) ok = false;
    }

    close(fd);
    return ok;
}

int main() {
    for (int i = 1; i <= 5; ++i) {
        bool res = writeImportantConfig("important_config", "important_config");
        printf("Test %d: %s\n", i, res ? "true" : "false");
    }
    return 0;
}