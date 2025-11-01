#include <stdio.h>

#ifdef _WIN32
#include <io.h>
#include <sys/stat.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

int create_restrictive_file(const char* path, const char* content) {
    FILE* f = fopen(path, "w");
    if (!f) return 0;
    if (fputs(content, f) == EOF) {
        fclose(f);
        return 0;
    }
    fclose(f);

#ifdef _WIN32
    if (_chmod(path, _S_IREAD | _S_IWRITE) != 0) {
        // Best-effort on Windows.
    }
#else
    if (chmod(path, 0600) != 0) {
        return 0;
    }
#endif
    return 1;
}

int main(void) {
    const char* content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";
    create_restrictive_file("secret.txt", content);
    create_restrictive_file("secret1.txt", content);
    create_restrictive_file("secret2.txt", content);
    create_restrictive_file("secret3.txt", content);
    create_restrictive_file("secret4.txt", content);
    return 0;
}