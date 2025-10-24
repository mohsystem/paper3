#include <iostream>
#include <string>
#include <cstdio>
#include <array>
#include <sys/wait.h>

/**
 * Executes a bash shell command and returns its output.
 *
 * @param cmd The shell command to execute.
 * @return A string containing the standard output, standard error, and exit code.
 */
std::string executeCommand(const std::string& cmd) {
    // 1. Validation: Check for empty command
    if (cmd.empty()) {
        return "Validation Error: Command cannot be empty.";
    }

    // 2. Redirect stderr to stdout to capture both streams in one pipe
    std::string cmd_with_redirect = cmd + " 2>&1";
    std::string result = "";
    std::array<char, 256> buffer;
    
    // 3. Execute command using popen
    FILE* pipe = popen(cmd_with_redirect.c_str(), "r");
    if (!pipe) {
        return "Execution Error: popen() failed!";
    }

    // 4. Read the output from the pipe
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }

    // 5. Close the pipe and get the command's exit status
    int status = pclose(pipe);
    
    // Remove the last newline character if it exists
    if (!result.empty() && result[result.length() - 1] == '\n') {
        result.pop_back();
    }
    
    // 6. Append the exit code to the result
    if (WIFEXITED(status)) {
        result += "\nExited with code: " + std::to_string(WEXITSTATUS(status));
    } else {
        result += "\nProcess did not terminate normally.";
    }
    
    return result;
}

int main() {
    std::string testCommands[] = {
        "echo 'Hello from Bash!'",            // Test Case 1: Simple echo
        "ls -l",                              // Test Case 2: List files
        "ls non_existent_directory",          // Test Case 3: Command with an error
        "echo 'one\\ntwo\\nthree' | grep 'two'", // Test Case 4: Command with a pipe
        "invalidcommand_xyz_123"              // Test Case 5: Invalid command
    };

    for (int i = 0; i < 5; ++i) {
        std::cout << "--- Executing Test Case " << (i + 1) << ": `" << testCommands[i] << "` ---" << std::endl;
        std::string result = executeCommand(testCommands[i]);
        std::cout << "Result:\n" << result << std::endl;
        std::cout << "--- End Test Case " << (i + 1) << " ---\n" << std::endl;
    }

    return 0;
}