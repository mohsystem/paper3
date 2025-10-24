#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstdio> // For remove()

// The following headers are for POSIX systems (Linux, macOS, etc.)
#include <unistd.h>
#include <sys/wait.h>

/**
 * @brief Securely displays a file's content using a system command.
 * Note: On Windows, the command "cat" might need to be changed to "type".
 * This implementation is POSIX-specific due to fork() and execlp().
 * @param filename The name of the file to display.
 */
void displayFileContent(const std::string& filename) {
    std::cout << "--- Attempting to display: '" << filename << "' ---" << std::endl;

    // Using fork() and one of the exec() family of functions is the secure way to
    // run external commands on POSIX systems. The filename is passed as a distinct
    // argument to the new process and is not interpreted by a shell, which prevents
    // command injection.
    pid_t pid = fork();

    if (pid == -1) {
        // Fork failed
        perror("fork failed");
        return;
    } else if (pid == 0) {
        // This is the child process.
        // execlp searches for the command in PATH. The arguments are passed separately:
        // command, arg0 (conventionally the command name), arg1 (the filename),
        // and a NULL terminator.
        execlp("cat", "cat", filename.c_str(), (char*)NULL);

        // If execlp returns, it means an error occurred.
        perror("execlp failed");
        exit(EXIT_FAILURE);
    } else {
        // This is the parent process.
        int status;
        waitpid(pid, &status, 0); // Wait for the child process to complete.
        if (WIFEXITED(status)) {
            std::cout << "-> Command finished with exit code: " << WEXITSTATUS(status) << std::endl;
        } else {
            std::cout << "-> Command terminated abnormally" << std::endl;
        }
    }
    std::cout << "--- End of attempt ---" << std::endl << std::endl;
}

int main() {
    // 1. Setup: Create dummy files for testing
    const std::string file1 = "test_file_1.txt";
    const std::string file2 = "file with spaces.txt";
    try {
        std::ofstream ofs1(file1);
        ofs1 << "Hello from test file 1.";
        ofs1.close();

        std::ofstream ofs2(file2);
        ofs2 << "This file has spaces in its name.";
        ofs2.close();
        std::cout << "Created test files." << std::endl << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to create test files: " << e.what() << std::endl;
        return 1;
    }

    // 2. Test cases
    const std::vector<std::string> testCases = {
        file1,                                // 1. Valid file
        file2,                                // 2. Valid file with spaces
        "non_existent_file.txt",              // 3. Non-existent file
        "test_file_1.txt; ls -l",             // 4. Command injection attempt (benign)
        "| whoami"                            // 5. Command injection attempt (malicious)
    };

    for (const auto& testCase : testCases) {
        displayFileContent(testCase);
    }

    // 3. Cleanup: Delete the dummy files
    if (remove(file1.c_str()) != 0 || remove(file2.c_str()) != 0) {
        perror("Error cleaning up files");
    } else {
        std::cout << "Cleaned up test files." << std::endl;
    }
    
    return 0;
}