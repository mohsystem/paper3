#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

ssize_t write_important_config(const std::string& path) {
    const char* data = "important_config";
    int fd = open(path.c_str(), O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        return -1;
    }
    ssize_t written = write(fd, data, std::strlen(data));
    close(fd);
    return written;
}

int main() {
    std::vector<std::string> tests = {
        "cpp_test1.txt",
        "cpp_test2.txt",
        "cpp_test3.txt",
        "cpp_test4.txt",
        "cpp_test5.txt"
    };
    for (const auto& t : tests) {
        ssize_t n = write_important_config(t);
        std::cout << "Wrote " << n << " bytes to " << t << std::endl;
    }
    return 0;
}