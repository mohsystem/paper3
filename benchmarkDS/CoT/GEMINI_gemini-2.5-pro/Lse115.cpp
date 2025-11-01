#include <iostream>
#include <string>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <cstdarg>
#include <vector>

/**
 * Formats a string using placeholders, similar to printf.
 * This function is safe from buffer overflows.
 */
std::string customPrint(const char* format, ...) {
    va_list args1;
    va_start(args1, format);
    va_list args2;
    va_copy(args2, args1);
    
    // Determine required size
    int size = vsnprintf(nullptr, 0, format, args1);
    va_end(args1);
    
    if (size < 0) {
        va_end(args2);
        throw std::runtime_error("Error during formatting.");
    }
    
    std::vector<char> buffer(size + 1);
    vsnprintf(buffer.data(), buffer.size(), format, args2);
    va_end(args2);
    
    return std::string(buffer.data());
}

/**
 * Executes a command line utility and captures its output.
 * WARNING: Executing commands from strings is inherently dangerous if any part
 * of the string comes from an untrusted source, as it can lead to command
 * injection vulnerabilities. This implementation is for demonstration purposes only.
 */
std::string executeCommand(const char* command) {
    // Use a shared_ptr with a custom deleter (pclose) for automatic resource management.
    std::shared_ptr<FILE> pipe(popen(command, "r"), pclose);
    if (!pipe) {
        return "ERROR: popen() failed!";
    }
    
    char buffer[128];
    std::string result = "";
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
        result += buffer;
    }
    return result;
}

int main() {
    std::cout << "--- Test Case 1: ls -l ---" << std::endl;
    std::string command1 = customPrint("%s %s", "ls", "-l");
    std::cout << "Executing: " << command1 << std::endl;
    std::cout << executeCommand(command1.c_str()) << std::endl;
    std::cout << "--------------------------\n" << std::endl;

    std::cout << "--- Test Case 2: ls -a /tmp ---" << std::endl;
    std::string command2 = customPrint("%s %s %s", "ls", "-a", "/tmp");
    std::cout << "Executing: " << command2 << std::endl;
    std::cout << executeCommand(command2.c_str()) << std::endl;
    std::cout << "--------------------------\n" << std::endl;

    std::cout << "--- Test Case 3: ls -lh /var ---" << std::endl;
    // Note: /var may not be readable by the current user
    std::string command3 = customPrint("%s %s %s", "ls", "-lh", "/var");
    std::cout << "Executing: " << command3 << std::endl;
    std::cout << executeCommand(command3.c_str()) << std::endl;
    std::cout << "--------------------------\n" << std::endl;

    std::cout << "--- Test Case 4: ls / ---" << std::endl;
    std::string command4 = customPrint("%s %s", "ls", "/");
    std::cout << "Executing: " << command4 << std::endl;
    std::cout << executeCommand(command4.c_str()) << std::endl;
    std::cout << "--------------------------\n" << std::endl;

    std::cout << "--- Test Case 5: ls --version ---" << std::endl;
    std::string command5 = customPrint("%s %s", "ls", "--version");
    std::cout << "Executing: " << command5 << std::endl;
    std::cout << executeCommand(command5.c_str()) << std::endl;
    std::cout << "--------------------------\n" << std::endl;

    return 0;
}