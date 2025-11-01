#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>

ssize_t writeImportantConfig(const char* path) {
    int fd = open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd < 0) {
        return -1;
    }
    const char* s = "important_config";
    size_t len = strlen(s);
    ssize_t w = write(fd, s, len);
    close(fd);
    return w;
}

int main() {
    printf("%zd\n", writeImportantConfig("important_config_c"));
    printf("%zd\n", writeImportantConfig("important_config_c_1"));
    printf("%zd\n", writeImportantConfig("important_config_c_2"));
    printf("%zd\n", writeImportantConfig("important_config_c_3"));
    printf("%zd\n", writeImportantConfig("important_config_c_4"));
    return 0;
}