#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdarg>
#include <memory>

/**
 * Note: The prompt describes a pattern where a single formatted string is executed as a command.
 * This is a classic command injection vulnerability. A secure implementation, as provided below,
 * avoids this by never executing a command through a shell (like with system()). Instead, the command ("ls")
 * and its arguments are passed as a distinct list of strings to the execvp function.
 * This ensures that arguments, even if they contain shell metacharacters like ';' or '|',
 * are treated as literal strings and not as separate commands to be executed.
 */

/**
 * Creates a formatted string for use as a command argument.
 * This function corresponds to the "print function" described in the prompt,
 * but is used securely to format individual arguments, not the entire command string.
 *
 * @param format A C-style format string.
 * @param ...    Variable arguments for the format string.
 * @return A formatted std::string.
 */
std::string formatArgument(const char* format, ...) {
    va_list args1;
    va_start(args1, format);
    va_list args2;
    va_copy(args2, args1);

    int size = vsnprintf(nullptr, 0, format, args1);
    va_end(args1);
    
    if (size < 0) {
        return "";
    }
    
    std::vector<char> buffer(size + 1);
    vsnprintf(buffer.data(), buffer.size(), format, args2);
    va_end(args2);
    
    return std::string(buffer.data());
}


/**
 * Executes the 'ls' command with the given arguments in a secure way.
 *
 * @param args A vector of string arguments for the 'ls' command.
 * @return The combined standard output and standard error of the command.
 */
std::string executeLs(const std::vector<std::string>& args) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        return "Error: pipe() failed.";
    }

    pid_t pid = fork();
    if (pid == -1) {
        return "Error: fork() failed.";
    }

    if (pid == 0) { // Child process
        close(pipefd[0]); // Close read end
        dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to pipe
        dup2(pipefd[1], STDERR_FILENO); // Redirect stderr to pipe
        close(pipefd[1]); // Close original write end

        std::vector<const char*> argv;
        argv.push_back("ls");
        for (const auto& arg : args) {
            argv.push_back(arg.c_str());
        }
        argv.push_back(nullptr);

        execvp("ls", const_cast<char* const*>(argv.data()));
        // If execvp returns, it must have failed.
        perror("execvp");
        _exit(127);
    } else { // Parent process
        close(pipefd[1]); // Close write end

        std::string output;
        char buffer[256];
        ssize_t bytes_read;
        while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
            output.append(buffer, bytes_read);
        }
        close(pipefd[0]);

        int status;
        waitpid(pid, &status, 0);
        
        output += "---\nExit Code: " + std::to_string(WEXITSTATUS(status)) + "\n";
        return output;
    }
}

int main() {
    std::cout << "--- Test Case 1: ls -l ---" << std::endl;
    std::cout << executeLs({"-l"}) << std::endl;

    std::cout << "--- Test Case 2: ls -a / ---" << std::endl;
    std::cout << executeLs({"-a", "/"}) << std::endl;

    std::cout << "--- Test Case 3: ls /nonexistent_dir ---" << std::endl;
    std::cout << executeLs({"/nonexistent_dir"}) << std::endl;
    
    // Using the formatArgument function to build an argument
    std::string formatted_arg = formatArgument("test_file_%d.txt", 123);
    std::cout << "--- Test Case 4: ls " << formatted_arg << " ---" << std::endl;
    std::cout << executeLs({formatted_arg}) << std::endl;

    // Security Test Case: Attempting command injection.
    // The secure implementation will pass "; id" as a single argument to 'ls',
    // which will result in an error from 'ls', rather than executing the 'id' command.
    std::cout << "--- Test Case 5 (Security): ls '; id' ---" << std::endl;
    std::cout << executeLs({"; id"}) << std::endl;

    return 0;
}