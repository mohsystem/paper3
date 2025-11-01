#include <iostream>
#include <string>
#include <cstdio>
#include <cstdarg>
#include <vector>
#include <memory>
#include <array>
#include <stdexcept>

// Forward declaration
class Lse115;

class Lse115 {
public:
    /**
     * Creates a command string from a format string and a variable number of arguments.
     * This is a variadic function similar to printf.
     * @param format The format string.
     * @param ... The arguments to be formatted into the string.
     * @return The formatted command string.
     */
    static std::string print(const char* format, ...) {
        va_list args1;
        va_start(args1, format);
        va_list args2;
        va_copy(args2, args1);

        // Determine the required buffer size by a dry run
        int size = vsnprintf(nullptr, 0, format, args1);
        va_end(args1);

        if (size < 0) {
            throw std::runtime_error("Error during formatting: vsnprintf failed.");
        }

        // Allocate buffer and format the string for real
        std::vector<char> buffer(static_cast<size_t>(size) + 1);
        vsnprintf(buffer.data(), buffer.size(), format, args2);
        va_end(args2);

        return std::string(buffer.data());
    }

    /**
     * Executes the given command line utility string.
     * It prints the standard output and standard error of the command.
     * Note: This uses popen, which is POSIX-specific.
     * @param command The command string to execute.
     */
    static void executeLs(const std::string& command) {
        std::cout << "Executing command: " << command << std::endl;
        std::array<char, 128> buffer;
        std::string result;
        
        // Redirect stderr to stdout to capture both streams
        std::string full_command = command + " 2>&1";
        
        // Use a unique_ptr for RAII-style management of the FILE pointer
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(full_command.c_str(), "r"), pclose);
        
        if (!pipe) {
            throw std::runtime_error("popen() failed!");
        }
        
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        
        // The unique_ptr's destructor will call pclose automatically when it goes out of scope.
        // To get the exit code, we would need to call pclose manually or use a custom deleter.
        // For simplicity, this example just prints the output.
        // A more robust solution would be needed to accurately report the exit code.
        std::cout << result;
        std::cout << "----------------------------------------" << std::endl;
    }
};

int main() {
    // Test Case 1: ls -l
    std::string cmd1 = Lse115::print("%s %s", "ls", "-l");
    Lse115::executeLs(cmd1);

    // Test Case 2: ls -a
    std::string cmd2 = Lse115::print("%s %s", "ls", "-a");
    Lse115::executeLs(cmd2);

    // Test Case 3: ls -lh
    std::string cmd3 = Lse115::print("%s %s", "ls", "-lh");
    Lse115::executeLs(cmd3);

    // Test Case 4: ls /tmp
    std::string cmd4 = Lse115::print("%s %s", "ls", "/tmp");
    Lse115::executeLs(cmd4);

    // Test Case 5: ls -l /
    std::string cmd5 = Lse115::print("%s %s %s", "ls", "-l", "/");
    Lse115::executeLs(cmd5);

    return 0;
}