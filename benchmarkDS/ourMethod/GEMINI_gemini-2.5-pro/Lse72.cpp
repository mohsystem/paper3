#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <system_error>

// For secure file creation, we use POSIX functions.
// On Windows, similar functionality can be achieved with CreateFileW.
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

namespace fs = std::filesystem;

/**
 * @brief Securely uploads a file to a designated 'uploads' directory.
 * The 'uploads' directory is created in the current working directory.
 * This function prevents path traversal attacks and race conditions (TOCTOU).
 *
 * @param filename The name of the file to be saved. This will be sanitized.
 * @param content The string content of the file.
 * @return true if the upload was successful, false otherwise.
 */
bool upload_file(const std::string& filename, const std::string& content) {
    if (filename.empty()) {
        std::cerr << "Error: Filename is empty." << std::endl;
        return false;
    }

    try {
        // Define the base directory for uploads in the current working directory.
        fs::path uploads_dir = fs::current_path() / "uploads";

        // Create the directory if it doesn't exist.
        std::error_code ec;
        if (!fs::exists(uploads_dir)) {
            if (!fs::create_directory(uploads_dir, ec)) {
                std::cerr << "Error: Could not create uploads directory: " << ec.message() << std::endl;
                return false;
            }
        } else if (!fs::is_directory(uploads_dir)) {
            std::cerr << "Error: 'uploads' exists but is not a directory." << std::endl;
            return false;
        }

        // Sanitize the filename to prevent path traversal.
        // .filename() securely extracts the filename part.
        fs::path sanitized_filename = fs::path(filename).filename();
        if (sanitized_filename.empty() || sanitized_filename.string() == "." || sanitized_filename.string() == "..") {
            std::cerr << "Error: Filename is invalid after sanitization." << std::endl;
            return false;
        }

        // Construct the destination path.
        fs::path dest_path = uploads_dir / sanitized_filename;

        // Security Check: Use weakly_canonical to resolve paths without requiring them
        // to exist. Ensure the destination's parent is the uploads directory.
        if (fs::weakly_canonical(dest_path).parent_path() != fs::weakly_canonical(uploads_dir)) {
            std::cerr << "Error: Path traversal attempt detected." << std::endl;
            return false;
        }

        // Use POSIX open() for secure, atomic file creation to prevent TOCTOU.
        // O_CREAT | O_EXCL: Fails if the file already exists.
        // O_WRONLY: Write-only.
        // S_IRUSR | S_IWUSR (0600): Permissions for owner read/write.
        int fd = open(dest_path.c_str(), O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
        if (fd == -1) {
            perror("Error opening file");
            return false;
        }

        // Write content to the file descriptor.
        ssize_t bytes_written = write(fd, content.c_str(), content.length());
        if (bytes_written < 0 || (size_t)bytes_written != content.length()) {
            perror("Error writing to file");
            close(fd);
            // Attempt to remove partially written file
            unlink(dest_path.c_str());
            return false;
        }

        // Close the file descriptor.
        if (close(fd) == -1) {
            perror("Error closing file");
            return false;
        }
        
        std::cout << "Successfully uploaded file to: " << fs::absolute(dest_path) << std::endl;
        return true;

    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        return false;
    }
}

int main() {
    std::cout << "--- Running Test Cases ---" << std::endl;

    // Test Case 1: Valid upload
    std::cout << "\n--- Test Case 1: Valid upload ---" << std::endl;
    upload_file("test1.txt", "This is a valid test file.");

    // Test Case 2: Path traversal attempt
    std::cout << "\n--- Test Case 2: Path traversal attempt ---" << std::endl;
    upload_file("../test2.txt", "This should fail.");
    upload_file("subdir/../../test2_b.txt", "This should also fail.");

    // Test Case 3: Empty filename
    std::cout << "\n--- Test Case 3: Empty filename ---" << std::endl;
    upload_file("", "Empty filename.");

    // Test Case 4: Re-uploading the same file (should fail)
    std::cout << "\n--- Test Case 4: Re-uploading same file ---" << std::endl;
    upload_file("test1.txt", "This should fail because it already exists.");

    // Test Case 5: Filename with path components (should be sanitized)
    std::cout << "\n--- Test Case 5: Filename with path components ---" << std::endl;
    upload_file("/etc/passwd", "This content should not be written to /etc/passwd");
    upload_file("C:\\Users\\test\\myfile.txt", "This should be saved as myfile.txt in uploads");

    std::cout << "\n--- Test Cases Finished ---" << std::endl;
    return 0;
}