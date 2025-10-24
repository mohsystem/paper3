
#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>

bool createExecutableScript(const std::string& filename) {
    try {
        // Create the file
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error creating file" << std::endl;
            return false;
        }
        
        // Write shell script header
        file << "#!/bin/bash\\n";
        file.close();
        
        // Make it executable (Unix/Linux/Mac)
        #ifdef _WIN32
            // Windows - files are executable by default
            return true;
        #else
            // Unix-based systems
            if (chmod(filename.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0) {
                std::cerr << "Error setting permissions" << std::endl;
                return false;
            }
        #endif
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}

int main() {
    // Test case 1: Create script.sh
    std::cout << "Test 1: Creating script.sh" << std::endl;
    bool result1 = createExecutableScript("script.sh");
    std::cout << "Result: " << (result1 ? "Success" : "Failed") << std::endl;
    
    // Test case 2: Create test1.sh
    std::cout << "\\nTest 2: Creating test1.sh" << std::endl;
    bool result2 = createExecutableScript("test1.sh");
    std::cout << "Result: " << (result2 ? "Success" : "Failed") << std::endl;
    
    // Test case 3: Create test2.sh
    std::cout << "\\nTest 3: Creating test2.sh" << std::endl;
    bool result3 = createExecutableScript("test2.sh");
    std::cout << "Result: " << (result3 ? "Success" : "Failed") << std::endl;
    
    // Test case 4: Overwrite existing file
    std::cout << "\\nTest 4: Creating script.sh again (overwrites)" << std::endl;
    bool result4 = createExecutableScript("script.sh");
    std::cout << "Result: " << (result4 ? "Success" : "Failed") << std::endl;
    
    // Test case 5: Create another script
    std::cout << "\\nTest 5: Creating deploy.sh" << std::endl;
    bool result5 = createExecutableScript("deploy.sh");
    std::cout << "Result: " << (result5 ? "Success" : "Failed") << std::endl;
    
    // Cleanup test files
    remove("script.sh");
    remove("test1.sh");
    remove("test2.sh");
    remove("deploy.sh");
    
    return 0;
}
