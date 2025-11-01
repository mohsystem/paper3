#include <iostream>
#include <string>
#include <sstream>
#include <cstring>

#if defined(_WIN32)
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#else
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#endif

static std::string mode_to_symbolic(unsigned int mode) {
    std::string s;
    s.reserve(10);
    s.push_back('-');
    s.push_back((mode & 0400) ? 'r' : '-');
    s.push_back((mode & 0200) ? 'w' : '-');
    s.push_back((mode & 0100) ? 'x' : '-');
    s.push_back((mode & 0040) ? 'r' : '-');
    s.push_back((mode & 0020) ? 'w' : '-');
    s.push_back((mode & 0010) ? 'x' : '-');
    s.push_back((mode & 0004) ? 'r' : '-');
    s.push_back((mode & 0002) ? 'w' : '-');
    s.push_back((mode & 0001) ? 'x' : '-');
    return s;
}

std::string create_and_restrict(const std::string& path, const std::string& data) {
#if defined(_WIN32)
    // Windows best-effort: create/truncate and set _S_IREAD | _S_IWRITE for owner (approximation)
    struct _stat stbuf;
    if (_stat(path.c_str(), &stbuf) == 0) {
        // Refuse to operate on reparse points would require extra WinAPI; skip here
    }
    int fd = _open(path.c_str(), _O_WRONLY | _O_CREAT | _O_TRUNC | _O_BINARY, _S_IREAD | _S_IWRITE);
    if (fd < 0) {
        std::ostringstream oss; oss << "ERROR: cannot open: " << strerror(errno);
        std::cout << oss.str() << std::endl;
        return oss.str();
    }
    int wrote = _write(fd, data.c_str(), (unsigned int)data.size());
    (void)wrote;
    _close(fd);
    // _chmod to read/write; no true POSIX perms on Windows
    _chmod(path.c_str(), _S_IREAD | _S_IWRITE);

    // Build a simplified string: assume owner rw, others none
    std::string perm = "-rw-------";
    std::cout << perm << std::endl;
    return perm;
#else
    struct stat lst;
    if (lstat(path.c_str(), &lst) == 0 && S_ISLNK(lst.st_mode)) {
        std::string msg = "ERROR: Refusing to operate on symlink";
        std::cout << msg << std::endl;
        return msg;
    }

    int flags = O_WRONLY | O_CREAT | O_TRUNC;
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif
#ifdef O_CLOEXEC
    flags |= O_CLOEXEC;
#endif

    int fd = open(path.c_str(), flags, 0600);
    if (fd < 0) {
        std::ostringstream oss; oss << "ERROR: open failed: " << strerror(errno);
        std::cout << oss.str() << std::endl;
        return oss.str();
    }

    ssize_t total = 0;
    const char* buf = data.c_str();
    ssize_t nleft = (ssize_t)data.size();
    while (nleft > 0) {
        ssize_t n = write(fd, buf + total, (size_t)nleft);
        if (n < 0) {
            std::ostringstream oss; oss << "ERROR: write failed: " << strerror(errno);
            std::cout << oss.str() << std::endl;
            close(fd);
            return oss.str();
        }
        total += n;
        nleft -= n;
    }
    fsync(fd);
    close(fd);

    chmod(path.c_str(), 0600);

    struct stat st;
    if (lstat(path.c_str(), &st) != 0) {
        std::ostringstream oss; oss << "ERROR: stat failed: " << strerror(errno);
        std::cout << oss.str() << std::endl;
        return oss.str();
    }
    std::string perm = mode_to_symbolic(st.st_mode);
    std::cout << perm << std::endl;
    return perm;
#endif
}

int main() {
    std::string secret = "U2FsdGVkX1/R+WzJcxgvX/Iw==";
    create_and_restrict("secret.txt", secret);
    create_and_restrict("secret1.txt", secret);
    create_and_restrict("secret2.txt", secret);
    create_and_restrict("secret3.txt", secret);
    create_and_restrict("secret4.txt", secret);
    return 0;
}