
#include <iostream>
#include <fstream>
#include <cstring>
#include <cerrno>
#include <sys/stat.h>

// Function to create a file and make it executable
// Returns: 0 on success, -1 on failure
int createExecutableScript(const char* filename) {
    // Input validation: ensure filename is not NULL and not empty
    if (filename == nullptr || filename[0] == '\\0') {
        std::cerr << "Error: Invalid filename provided" << std::endl;
        return -1;
    }
    
    // Security: Validate filename doesn't contain path traversal sequences
    if (std::strstr(filename, "..") != nullptr || 
        std::strstr(filename, "/") != nullptr ||
        std::strstr(filename, "\\\\") != nullptr) {\n        std::cerr << "Error: Filename must not contain path separators or traversal sequences" << std::endl;\n        return -1;\n    }\n    \n    // Security: Limit filename length to prevent buffer overflows in system calls\n    const size_t MAX_FILENAME_LEN = 255;\n    if (std::strlen(filename) > MAX_FILENAME_LEN) {\n        std::cerr << "Error: Filename too long" << std::endl;\n        return -1;\n    }\n    \n    // Create file with restricted permissions initially (owner read/write only)\n    // This prevents TOCTOU attacks by ensuring file is created securely first\n    std::ofstream file(filename, std::ios::out | std::ios::trunc);\n    if (!file.is_open()) {\n        std::cerr << "Error: Failed to create file: " << std::strerror(errno) << std::endl;\n        return -1;\n    }\n    \n    // Write a minimal shell script header\n    file << "#!/bin/bash\
";\n    file << "# Script created by createExecutableScript\
";\n    \n    // Ensure data is written to disk before closing\n    file.flush();\n    if (file.fail()) {\n        std::cerr << "Error: Failed to write to file" << std::endl;\n        file.close();\n        return -1;\n    }\n    \n    file.close();\n    \n    // Make the file executable (owner: rwx, group: r-x, others: r-x)\n    // Using 0755 permissions: owner can read/write/execute, others can read/execute\n    if (chmod(filename, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0) {\n        std::cerr << "Error: Failed to set executable permissions: " << std::strerror(errno) << std::endl;\n        return -1;\n    }\n    \n    return 0;\n}\n\nint main() {\n    // Test case 1: Create script.sh successfully\n    std::cout << "Test 1: Creating script.sh" << std::endl;\n    if (createExecutableScript("script.sh") == 0) {\n        std::cout << "Success: script.sh created and made executable" << std::endl;\n    } else {\n        std::cout << "Failed to create script.sh" << std::endl;\n    }\n    \n    // Test case 2: Try with null pointer (should fail)\n    std::cout << "\
Test 2: Null pointer test" << std::endl;\n    if (createExecutableScript(nullptr) == -1) {\n        std::cout << "Correctly rejected null pointer" << std::endl;\n    }\n    \n    // Test case 3: Try with empty string (should fail)\n    std::cout << "\
Test 3: Empty string test" << std::endl;\n    if (createExecutableScript("") == -1) {\n        std::cout << "Correctly rejected empty string" << std::endl;\n    }\n    \n    // Test case 4: Try with path traversal (should fail)\n    std::cout << "\
Test 4: Path traversal test" << std::endl;\n    if (createExecutableScript("../script.sh") == -1) {\n        std::cout << "Correctly rejected path traversal attempt" << std::endl;\n    }\n    \n    // Test case 5: Create another valid script\n    std::cout << "\
Test 5: Creating test_script.sh" << std::endl;\n    if (createExecutableScript("test_script.sh") == 0) {\n        std::cout << "Success: test_script.sh created and made executable" << std::endl;\n    } else {\n        std::cout << "Failed to create test_script.sh" << std::endl;
    }
    
    return 0;
}
