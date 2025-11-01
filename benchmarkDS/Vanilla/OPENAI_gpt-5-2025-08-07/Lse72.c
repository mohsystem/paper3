#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
  #include <direct.h>
  #include <io.h>
  #define MKDIR(dir) _mkdir(dir)
  #define ACCESS _access
  #define SEP '\\'
  #include <windows.h>
#else
  #include <sys/stat.h>
  #include <sys/types.h>
  #include <unistd.h>
  #define MKDIR(dir) mkdir(dir, 0777)
  #define ACCESS access
  #define SEP '/'
#endif

char* upload_file(const char* filename, const char* data) {
    if (ACCESS("uploads", 0) != 0) {
        MKDIR("uploads");
    }

    char relPath[1024];
    snprintf(relPath, sizeof(relPath), "uploads/%s", filename);

    FILE* f = fopen(relPath, "wb");
    if (!f) {
        return NULL;
    }
    fwrite(data, 1, strlen(data), f);
    fclose(f);

    char cwd[2048];
#ifdef _WIN32
    if (!_getcwd(cwd, sizeof(cwd))) {
        return NULL;
    }
#else
    if (!getcwd(cwd, sizeof(cwd))) {
        return NULL;
    }
#endif

    size_t needed = strlen(cwd) + 1 + strlen("uploads") + 1 + strlen(filename) + 1;
    char* absPath = (char*)malloc(needed);
    if (!absPath) return NULL;
    snprintf(absPath, needed, "%s%cuploads%c%s", cwd, SEP, SEP, filename);
    return absPath;
}

int main(void) {
    char* p1 = upload_file("c_test1.txt", "Hello from C 1");
    char* p2 = upload_file("c_test2.txt", "Hello from C 2");
    char* p3 = upload_file("c_test3.txt", "Hello from C 3");
    char* p4 = upload_file("c_test4.txt", "Hello from C 4");
    char* p5 = upload_file("c_test5.txt", "Hello from C 5");

    if (p1) { printf("%s\n", p1); free(p1); }
    if (p2) { printf("%s\n", p2); free(p2); }
    if (p3) { printf("%s\n", p3); free(p3); }
    if (p4) { printf("%s\n", p4); free(p4); }
    if (p5) { printf("%s\n", p5); free(p5); }

    return 0;
}