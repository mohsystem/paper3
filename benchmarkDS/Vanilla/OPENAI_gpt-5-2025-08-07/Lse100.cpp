#include <iostream>
#include <string>

#ifdef _WIN32
  #include <io.h>
  #include <fcntl.h>
  #include <sys/stat.h>
#else
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <fcntl.h>
  #include <unistd.h>
#endif

bool ensure_config_file(const std::string& path, int mode = 0600) {
#ifdef _WIN32
    int fd = _open(path.c_str(), _O_WRONLY | _O_CREAT, _S_IREAD | _S_IWRITE);
    if (fd == -1) return false;
    _close(fd);
    _chmod(path.c_str(), _S_IREAD | _S_IWRITE);
    return true;
#else
    int fd = open(path.c_str(), O_WRONLY | O_CREAT, static_cast<mode_t>(mode));
    if (fd == -1) return false;
    close(fd);
    chmod(path.c_str(), static_cast<mode_t>(mode));
    return true;
#endif
}

int main() {
    for (int i = 1; i <= 5; ++i) {
        bool ok = ensure_config_file("important_config");
        std::cout << "Test " << i << ": " << (ok ? "1" : "0") << std::endl;
    }
    return 0;
}