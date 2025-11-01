#include <iostream>
#include <string>
#include <iomanip>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

// Function to format and print file permissions from a mode_t
void print_permissions_from_mode(mode_t mode) {
    char perms[11];
    strcpy(perms, "----------");
    if (S_ISDIR(mode)) perms[0] = 'd';
    if (S_ISLNK(mode)) perms[0] = 'l';

    if (mode & S_IRUSR) perms[1] = 'r';
    if (mode & S_IWUSR) perms[2] = 'w';
    if (mode & S_IXUSR) perms[3] = 'x';

    if (mode & S_IRGRP) perms[4] = 'r';
    if (mode & S_IWGRP) perms[5] = 'w';
    if (mode & S_IXGRP) perms[6] = 'x';

    if (mode & S_IROTH) perms[7] = 'r';
    if (mode & S_IWOTH) perms[8] = 'w';
    if (mode & S_IXOTH) perms[9] = 'x';
    
    std::cout << perms << " (octal: " << std::oct << std::setw(4) << std::setfill('0') << (mode & 0777) << std::dec << ")";
}

bool create_and_secure_file(const std::string& file_name, const std::string& content) {
    // Restrictive permissions: S_IRUSR | S_IWUSR is 0600
    mode_t permissions = S_IRUSR | S_IWUSR;
    
    // Atomically create a new file with restrictive permissions using low-level I/O.
    // O_EXCL fails if the file already exists, preventing race conditions.
    int fd = open(file_name.c_str(), O_WRONLY | O_CREAT | O_EXCL, permissions);
    
    if (fd == -1) {
        std::cerr << "Error: Failed to create file " << file_name << ": " << strerror(errno) << std::endl;
        return false;
    }
    
    ssize_t bytes_written = write(fd, content.c_str(), content.length());
    if (bytes_written == -1 || static_cast<size_t>(bytes_written) != content.length()) {
        std::cerr << "Error: Failed to write to file " << file_name << ": " 
                  << (bytes_written == -1 ? strerror(errno) : "Incomplete write") << std::endl;
        close(fd);
        return false;
    }
    std::cout << "Successfully created and wrote to " << file_name << std::endl;
    
    // Use fstat on the file descriptor to securely get file status without a TOCTOU race.
    struct stat file_stat;
    if (fstat(fd, &file_stat) == -1) {
        std::cerr << "Warning: Could not fstat file descriptor: " << strerror(errno) << std::endl;
    } else {
        std::cout << "File permissions for " << file_name << ": ";
        print_permissions_from_mode(file_stat.st_mode);
        std::cout << std::endl;
    }

    if (close(fd) == -1) {
        std::cerr << "Error: Failed to close file " << file_name << ": " << strerror(errno) << std::endl;
        return false;
    }

    return true;
}

int main() {
    const std::string file_name = "secret.txt";
    const std::string content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";

    for (int i = 1; i <= 5; ++i) {
        std::cout << "--- Test Case " << i << " ---" << std::endl;
        if (create_and_secure_file(file_name, content)) {
            std::cout << "Test case " << i << " successful." << std::endl;
        } else {
            std::cerr << "Test case " << i << " failed." << std::endl;
        }

        // Cleanup for the next test case
        if (remove(file_name.c_str()) != 0 && errno != ENOENT) {
             std::cerr << "Failed to clean up file " << file_name << ": " << strerror(errno) << std::endl;
        }
        std::cout << std::endl;
    }
    
    return 0;
}