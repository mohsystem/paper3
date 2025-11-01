// Chain-of-Through secure implementation in C++
// 1) Problem understanding: create "important_config" with secure permissions.
// 2) Security requirements: exclusive creation, restrictive permissions (0600), validate filename.
// 3) Secure coding generation: use open with O_CREAT|O_EXCL and controlled umask; Windows best-effort.
// 4) Code review: validation prevents traversal, proper error handling, minimal privileges.
// 5) Secure code output: final mitigations applied.

#include <iostream>
#include <string>
#include <cstdio>

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

static bool isValidBaseName(const std::string& name) {
    if (name.empty()) return false;
    if (name == "." || name == "..") return false;
    if (name.find('/') != std::string::npos) return false;
    if (name.find('\\') != std::string::npos) return false;
#ifdef _WIN32
    if (name.find(':') != std::string::npos) return false; // avoid drive spec
#endif
    return true;
}

bool createSecureConfig(const std::string& filename) {
    if (!isValidBaseName(filename)) {
        return false;
    }

#ifdef _WIN32
    int fd = _open(filename.c_str(), _O_CREAT | _O_EXCL | _O_WRONLY | _O_BINARY, _S_IREAD | _S_IWRITE);
    if (fd == -1) {
        return false;
    }
    // Best-effort permission restriction
    _chmod(filename.c_str(), _S_IREAD | _S_IWRITE);
    _close(fd);
    return true;
#else
    mode_t old_umask = umask(0); // ensure mode isn't masked
    int fd = open(filename.c_str(), O_CREAT | O_EXCL | O_WRONLY, static_cast<mode_t>(0600));
    int saved_errno = 0;
    if (fd == -1) {
        saved_errno = errno;
    }
    umask(old_umask);
    if (fd == -1) {
        (void)saved_errno;
        return false;
    }
    // No need to write content; just close
    if (close(fd) != 0) {
        return false;
    }
    return true;
#endif
}

static void cleanup(const std::string& filename) {
    std::remove(filename.c_str());
}

int main() {
    const std::string fname = "important_config";
    for (int i = 1; i <= 5; ++i) {
        cleanup(fname);
        bool ok = createSecureConfig(fname);
        std::cout << "Test " << i << " createSecureConfig(\"" << fname << "\") -> " << (ok ? "true" : "false") << std::endl;
    }
    return 0;
}