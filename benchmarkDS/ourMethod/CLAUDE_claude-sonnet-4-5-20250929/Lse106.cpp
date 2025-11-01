
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <random>
#include <sstream>
#include <iomanip>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#define O_CLOEXEC 0
#endif

// Secure method to create file with secret info
std::string createSecretFile(const std::string& secretInfo) {
    // Use secure random for unique filename
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    
    std::stringstream ss;
    ss << "secret_" << std::hex << std::setfill('0') 
       << std::setw(16) << dis(gen) << ".txt";
    std::string filename = ss.str();
    
    // Get temp directory path
    const char* tmpdir = getenv("TMPDIR");
    if (!tmpdir) tmpdir = "/tmp";
    std::string filepath = std::string(tmpdir) + "/" + filename;
    
    // Open file with restrictive permissions (O_CREAT | O_EXCL for atomic creation)
    // O_NOFOLLOW prevents following symlinks
    int fd = open(filepath.c_str(), 
                  O_CREAT | O_EXCL | O_WRONLY | O_CLOEXEC | O_NOFOLLOW,
                  S_IRUSR | S_IWUSR); // 0600 permissions
    
    if (fd < 0) {
        throw std::runtime_error("Failed to create file");
    }
    
    // Write secret data
    ssize_t written = write(fd, secretInfo.c_str(), secretInfo.length());
    if (written < 0 || static_cast<size_t>(written) != secretInfo.length()) {
        close(fd);
        throw std::runtime_error("Failed to write secret data");
    }
    
    // Ensure data is written to disk
    if (fsync(fd) < 0) {
        close(fd);
        throw std::runtime_error("Failed to sync file");
    }
    
    close(fd);
    
    // Change permissions to read-only for owner (0400)
    if (chmod(filepath.c_str(), S_IRUSR) < 0) {
        throw std::runtime_error("Failed to set read-only permissions");
    }
    
    return filepath;
}

int main() {
    // Test cases
    std::string testSecrets[] = {
        "SuperSecret123!",
        "ClassifiedInformation",
        "TopSecretData",
        "ConfidentialInfo",
        "RestrictedAccess"
    };
    
    for (const auto& secret : testSecrets) {
        try {
            std::string filename = createSecretFile(secret);
            std::cout << filename << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error creating secret file: " << e.what() << std::endl;
        }
    }
    
    return 0;
}
