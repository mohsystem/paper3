#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>

ssize_t write_important_config(const char* path) {
    const char* data = "important_config";
    int fd = open(path, O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        return -1;
    }
    ssize_t written = write(fd, data, strlen(data));
    close(fd);
    return written;
}

int main(void) {
    const char* tests[5] = {
        "c_test1.txt",
        "c_test2.txt",
        "c_test3.txt",
        "c_test4.txt",
        "c_test5.txt"
    };
    for (int i = 0; i < 5; ++i) {
        ssize_t n = write_important_config(tests[i]);
        printf("Wrote %zd bytes to %s\n", n, tests[i]);
    }
    return 0;
}