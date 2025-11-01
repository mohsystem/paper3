#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int ensure_config_file(const char* path, int mode) {
#ifdef _WIN32
    int fd = _open(path, _O_WRONLY | _O_CREAT, _S_IREAD | _S_IWRITE);
    if (fd == -1) return 0;
    _close(fd);
    _chmod(path, _S_IREAD | _S_IWRITE);
    return 1;
#else
    int fd = open(path, O_WRONLY | O_CREAT, (mode_t)mode);
    if (fd == -1) return 0;
    close(fd);
    chmod(path, (mode_t)mode);
    return 1;
#endif
}

int main(void) {
    for (int i = 1; i <= 5; ++i) {
        int ok = ensure_config_file("important_config", 0600);
        printf("Test %d: %d\n", i, ok ? 1 : 0);
    }
    return 0;
}