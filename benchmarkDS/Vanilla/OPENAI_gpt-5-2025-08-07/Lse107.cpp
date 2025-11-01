#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>

std::string perm_string_from_mode(mode_t mode) {
    std::string s;
    s.reserve(9);
    s.push_back((mode & S_IRUSR) ? 'r' : '-');
    s.push_back((mode & S_IWUSR) ? 'w' : '-');
    s.push_back((mode & S_IXUSR) ? 'x' : '-');
    s.push_back((mode & S_IRGRP) ? 'r' : '-');
    s.push_back((mode & S_IWGRP) ? 'w' : '-');
    s.push_back((mode & S_IXGRP) ? 'x' : '-');
    s.push_back((mode & S_IROTH) ? 'r' : '-');
    s.push_back((mode & S_IWOTH) ? 'w' : '-');
    s.push_back((mode & S_IXOTH) ? 'x' : '-');
    return s;
}

std::string secure_file(const std::string& filename, const std::string& content, int mode) {
    std::ofstream ofs(filename, std::ios::out | std::ios::trunc);
    ofs << content;
    ofs.close();

    chmod(filename.c_str(), mode);

    struct stat st{};
    if (stat(filename.c_str(), &st) != 0) {
        return "000 ---------";
    }
    int perm = st.st_mode & 0777;
    char buf[5];
    std::snprintf(buf, sizeof(buf), "%03o", perm);
    return std::string(buf) + " " + perm_string_from_mode(st.st_mode);
}

int main() {
    std::string filename = "secret.txt";
    std::string content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";
    int mode = 0600;

    std::cout << secure_file(filename, content, mode) << std::endl;
    std::cout << secure_file(filename, content, mode) << std::endl;
    std::cout << secure_file(filename, content, mode) << std::endl;
    std::cout << secure_file(filename, content, mode) << std::endl;
    std::cout << secure_file(filename, content, mode) << std::endl;

    return 0;
}