/* Chain-of-Through secure implementation in C
   1) Problem understanding: create "important_config" with secure permissions.
   2) Security requirements: exclusive creation, restrictive permissions (0600), validate filename.
   3) Secure coding generation: use open with O_CREAT|O_EXCL and controlled umask; Windows best-effort.
   4) Code review: validation prevents traversal, proper error handling, minimal privileges.
   5) Secure code output: final mitigations applied.
*/

#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
  #include <io.h>
  #include <fcntl.h>
  #include <sys/stat.h>
  #define PATH_SEP1 '\\'
  #define PATH_SEP2 '/'
#else
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <fcntl.h>
  #include <unistd.h>
  #define PATH_SEP1 '/'
  #define PATH_SEP2 '/'
#endif

static int isValidBaseName(const char* name) {
    if (name == NULL || name[0] == '\0') return 0;
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) return 0;
    for (const char* p = name; *p; ++p) {
        if (*p == PATH_SEP1 || *p == PATH_SEP2) return 0;
#ifdef _WIN32
        if (*p == ':') return 0; /* avoid drive spec */
#endif
    }
    return 1;
}

int create_secure_config(const char* filename) {
    if (!isValidBaseName(filename)) {
        return 0;
    }

#ifdef _WIN32
    int fd = _open(filename, _O_CREAT | _O_EXCL | _O_WRONLY | _O_BINARY, _S_IREAD | _S_IWRITE);
    if (fd == -1) {
        return 0;
    }
    _chmod(filename, _S_IREAD | _S_IWRITE);
    _close(fd);
    return 1;
#else
    mode_t old_umask = umask(0);
    int fd = open(filename, O_CREAT | O_EXCL | O_WRONLY, (mode_t)0600);
    int saved_errno = errno;
    umask(old_umask);
    if (fd == -1) {
        (void)saved_errno;
        return 0;
    }
    if (close(fd) != 0) {
        return 0;
    }
    return 1;
#endif
}

static void cleanup(const char* filename) {
#ifdef _WIN32
    _unlink(filename);
#else
    unlink(filename);
#endif
}

int main(void) {
    const char* fname = "important_config";
    for (int i = 1; i <= 5; ++i) {
        cleanup(fname);
        int ok = create_secure_config(fname);
        printf("Test %d create_secure_config(\"%s\") -> %s\n", i, fname, ok ? "true" : "false");
    }
    return 0;
}