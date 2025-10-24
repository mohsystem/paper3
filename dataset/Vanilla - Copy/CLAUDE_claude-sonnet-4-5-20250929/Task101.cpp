
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <string>

bool createExecutableScript(const std::string& filename) {
    // Create the file
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error creating file" << std::endl;
        return false;
    }
    
    file << "#!/bin/bash\\n";
    file << "echo 'Hello from script.sh'\\n";
    file.close();
    
    // Make it executable (chmod +x)
    if (chmod(filename.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0) {
        std::cerr << "Error setting permissions" << std::endl;
        return false;
    }
    
    return true;
}

int main() {
    // Test case 1: Create script.sh
    std::cout << "Test 1: " << createExecutableScript("script.sh") << std::endl;
    
    // Test case 2: Create test1.sh
    std::cout << "Test 2: " << createExecutableScript("test1.sh") << std::endl;
    
    // Test case 3: Create test2.sh
    std::cout << "Test 3: " << createExecutableScript("test2.sh") << std::endl;
    
    // Test case 4: Create test3.sh
    std::cout << "Test 4: " << createExecutableScript("test3.sh") << std::endl;
    
    // Test case 5: Create test4.sh
    std::cout << "Test 5: " << createExecutableScript("test4.sh") << std::endl;
    
    return 0;
}
