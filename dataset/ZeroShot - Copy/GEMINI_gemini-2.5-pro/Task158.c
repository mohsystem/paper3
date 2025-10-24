#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
void secureExecuteCommand(const char* userInput) {
    printf("--- Executing command with input: \"%s\" ---\n", userInput);

    if (userInput == NULL || userInput[0] == '\0') {
        fprintf(stderr, "Error: Input cannot be null or empty.\n");
        printf("--------------------------------------------------\n\n");
        return;
    }

#ifdef _WIN32
    // Windows implementation using CreateProcess
    char command[256];
    // Securely construct the command line string
    snprintf(command, sizeof(command), "ping -n 1 %s", userInput);

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessA(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        fprintf(stderr, "CreateProcess failed (%lu).\n", GetLastError());
        printf("--------------------------------------------------\n\n");
        return;
    }

    // Wait until child process exits.
    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    printf("Exit Code: %lu\n", exitCode);

    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

#else
    // POSIX implementation using fork and execvp
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork failed");
        return;
    } else if (pid == 0) {
        // Child process
        char* argv[] = { "ping", "-c", "1", (char*)userInput, NULL };

        // execvp replaces the current process image with a new one.
        // It does NOT invoke a shell, so meta-characters in userInput are not interpreted.
        execvp(argv[0], argv);

        // If execvp returns, it must have failed.
        perror("execvp failed");
        exit(127); // Exit with a common code for "command not found"
    } else {
        // Parent process
        int status;
        // Wait for the child process to finish.
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("Exit Code: %d\n", WEXITSTATUS(status));
        }
    }
#endif
    printf("--------------------------------------------------\n\n");
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