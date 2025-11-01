#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string>
#include <cstring>
#include <iostream>

ssize_t writeImportantConfig(const std::string& path) {
    int fd = open(path.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd < 0) {
        return -1;
    }
    const char* s = "important_config";
    size_t len = std::strlen(s);
    ssize_t w = write(fd, s, len);
    close(fd);
    return w;
}

int main() {
    std::cout << writeImportantConfig("important_config_cpp") << std::endl;
    std::cout << writeImportantConfig("important_config_cpp_1") << std::endl;
    std::cout << writeImportantConfig("important_config_cpp_2") << std::endl;
    std::cout << writeImportantConfig("important_config_cpp_3") << std::endl;
    std::cout << writeImportantConfig("important_config_cpp_4") << std::endl;
    return 0;
}