
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>

#ifdef _WIN32
    #include <io.h>
    #include <sys/stat.h>
#else
    #include <sys/stat.h>
    #include <unistd.h>
#endif

bool createExecutableScript(const std::string& fileName) {
    try {
        // Create and write to the file
        std::ofstream file(fileName, std::ios::out | std::ios::trunc);
        
        if (!file.is_open()) {
            std::cerr << "Error: Could not create file " << fileName << std::endl;
            return false;
        }
        
        // Write shell script header
        file << "#!/bin/bash\\n";
        file << "# This is an executable script\\n";
        
        // Close the file properly
        file.close();
        
        if (file.fail()) {
            std::cerr << "Error: Failed to write to file " << fileName << std::endl;
            return false;
        }
        
        // Make it executable
        #ifdef _WIN32
            // Windows - limited permission model
            if (_chmod(fileName.c_str(), _S_IREAD | _S_IWRITE | _S_IEXEC) != 0) {
                std::cerr << "Error: Could not set permissions on Windows" << std::endl;
                return false;
            }
        #else
            // Unix/Linux/Mac - use chmod with 0755 permissions
            if (chmod(fileName.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0) {
                std::cerr << "Error: Could not set executable permissions" << std::endl;
                return false;
            }
        #endif
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return false;
    }
}

int main() {
    std::cout << "Test Case 1: Create script.sh" << std::endl;
    bool result1 = createExecutableScript("script.sh");
    std::cout << "Result: " << (result1 ? "Success" : "Failed") << "\\n" << std::endl;
    
    std::cout << "Test Case 2: Create test_script.sh" << std::endl;
    bool result2 = createExecutableScript("test_script.sh");
    std::cout << "Result: " << (result2 ? "Success" : "Failed") << "\\n" << std::endl;
    
    std::cout << "Test Case 3: Create another_script.sh" << std::endl;
    bool result3 = createExecutableScript("another_script.sh");
    std::cout << "Result: " << (result3 ? "Success" : "Failed") << "\\n" << std::endl;
    
    std::cout << "Test Case 4: Create deploy.sh" << std::endl;
    bool result4 = createExecutableScript("deploy.sh");
    std::cout << "Result: " << (result4 ? "Success" : "Failed") << "\\n" << std::endl;
    
    std::cout << "Test Case 5: Create backup.sh" << std::endl;
    bool result5 = createExecutableScript("backup.sh");
    std::cout << "Result: " << (result5 ? "Success" : "Failed") << "\\n" << std::endl;
    
    return 0;
}
