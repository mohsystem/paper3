#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

// Platform-specific includes for process creation
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

/**
 * Executes an OS command securely using user-provided data.
 * This function avoids command injection by executing the command directly
 * without invoking a shell. On POSIX systems, it uses fork() and execvp().
 * On Windows, it uses CreateProcess(). The user input is always treated as a
 * single, literal argument.
 *
 * @param userInput The data provided by the user, to be used as an argument.
 */
void secureExecuteCommand(const std::string& userInput) {
    std::cout << "--- Executing command with input: \"" << userInput << "\" ---" << std::endl;

    if (userInput.empty()) {
        std::cerr << "Error: Input cannot be empty." << std::endl;
        std::cout << "--------------------------------------------------\n" << std::endl;
        return;
    }

#ifdef _WIN32
    // Windows implementation using CreateProcess
    std::string command = "ping -n 1 " + userInput;

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // CreateProcess is safer than system() as it doesn't invoke cmd.exe by default.
    // We pass the entire command line, and the OS parses it.
    if (!CreateProcessA(NULL,   // No module name (use command line)
        (LPSTR)command.c_str(), // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi)            // Pointer to PROCESS_INFORMATION structure
    ) {
        std::cerr << "CreateProcess failed (" << GetLastError() << ")." << std::endl;
        std::cout << "--------------------------------------------------\n" << std::endl;
        return;
    }

    // Wait until child process exits.
    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    std::cout << "Exit Code: " << exitCode << std::endl;

    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

#else
    // POSIX implementation using fork and execvp
    pid_t pid = fork();

    if (pid == -1) {
        // Fork failed
        perror("fork failed");
        return;
    } else if (pid == 0) {
        // Child process
        // Arguments must be in a null-terminated array of char pointers
        const char* command = "ping";
        const char* arg1 = "-c";
        const char* arg2 = "1";
        // The user input is the final argument
        const char* arg3 = userInput.c_str();

        char* argv[] = { (char*)command, (char*)arg1, (char*)arg2, (char*)arg3, NULL };

        // execvp replaces the current process image with a new one.
        // It does NOT invoke a shell, so meta-characters in userInput are not interpreted.
        execvp(argv[0], argv);

        // If execvp returns, it must have failed
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        int status;
        // Wait for the child process to finish
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            std::cout << "Exit Code: " << WEXITSTATUS(status) << std::endl;
        }
    }
#endif
    std::cout << "--------------------------------------------------\n" << std::endl;
}

int main() {
    // Test Case 1: Benign, valid input
    secureExecuteCommand("8.8.8.8");

    // Test Case 2: Another benign, valid input
    secureExecuteCommand("localhost");

    // Test Case 3: Malicious input attempt (Unix style)
    // This will fail to ping a host named "127.0.0.1; ls" but will NOT execute "ls"
    secureExecuteCommand("127.0.0.1; ls");

    // Test Case 4: Malicious input attempt (Windows style)
    // This will fail to ping a host named "127.0.0.1 && dir" but will NOT execute "dir"
    secureExecuteCommand("127.0.0.1 && dir");

    // Test Case 5: Input with spaces that should be treated as a single argument
    // This will likely fail, but demonstrates it's not splitting the argument.
    secureExecuteCommand("not a valid hostname");

    return 0;
}