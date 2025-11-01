#include <fstream>
#include <string>

#ifdef _WIN32
#include <io.h>
#include <sys/stat.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

bool createRestrictiveFile(const std::string& path, const std::string& content) {
    try {
        std::ofstream ofs(path, std::ios::out | std::ios::trunc | std::ios::binary);
        if (!ofs) return false;
        ofs << content;
        ofs.close();
    } catch (...) {
        return false;
    }

#ifdef _WIN32
    if (_chmod(path.c_str(), _S_IREAD | _S_IWRITE) != 0) {
        // On Windows, this sets read/write for owner; finer POSIX-like perms are not available.
    }
#else
    if (chmod(path.c_str(), 0600) != 0) {
        return false;
    }
#endif
    return true;
}

int main() {
    std::string content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";
    createRestrictiveFile("secret.txt", content);
    createRestrictiveFile("secret1.txt", content);
    createRestrictiveFile("secret2.txt", content);
    createRestrictiveFile("secret3.txt", content);
    createRestrictiveFile("secret4.txt", content);
    return 0;
}