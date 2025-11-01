#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib> // For std::system

// The main class as requested by the prompt.
class Lse114 {
public:
    /**
     * Executes the 'ls' command by building a command string and using std::system.
     *
     * SECURITY WARNING: This function demonstrates the pattern of building a command
     * string and passing it to a shell via `std::system`. This pattern is ONLY
     * secure because the arguments provided in the main function are hardcoded,
     * trusted constants. If any part of the command string were derived from
     * external input (e.g., user input), it would be critically vulnerable to
     * command injection. The input would need to be rigorously sanitized/escaped
     * before being included in the command string. For greater security, especially
     * when dealing with external input, it is better to use functions like `execvp`
     * that do not invoke a shell.
     *
     * @param args A vector of string arguments for the 'ls' command.
     */
    static void executeLs(const std::vector<std::string>& args) {
        std::stringstream command_stream;
        command_stream << "ls";

        for (const auto& arg : args) {
            // A simple quoting mechanism to handle spaces, but not fully secure
            // against all malicious inputs. A real implementation would need
            // more robust shell escaping.
            command_stream << " '" << arg << "'";
        }
        
        std::string command = command_stream.str();
        
        std::cout << "Executing: " << command << std::endl;
        std::cout << "------------------------------------" << std::endl;

        int return_code = std::system(command.c_str());

        std::cout << "------------------------------------" << std::endl;
        std::cout << "Exited with code: " << return_code << std::endl;
        std::cout << std::endl;
    }
};

int main() {
    std::cout << "--- Test Case 1: ls -l . ---" << std::endl;
    Lse114::executeLs({"-l", "."});

    std::cout << "--- Test Case 2: ls -a / ---" << std::endl;
    Lse114::executeLs({"-a", "/"});

    std::cout << "--- Test Case 3: ls (no arguments) ---" << std::endl;
    Lse114::executeLs({});

    std::cout << "--- Test Case 4: ls /tmp ---" << std::endl;
    Lse114::executeLs({"/tmp"});

    std::cout << "--- Test Case 5: ls /non_existent_directory_xyz ---" << std::endl;
    Lse114::executeLs({"/non_existent_directory_xyz"});

    return 0;
}