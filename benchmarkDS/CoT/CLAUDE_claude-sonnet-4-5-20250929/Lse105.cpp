
#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#define chmod _chmod
#endif

void createSecretFile(const std::string& filename, const std::string& content) {
    try {
        // Write content to file
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not create file " << filename << std::endl;
            return;
        }
        
        file << content;
        file.close();
        
        // Set restrictive permissions (owner read/write only: 0600)
        #ifdef _WIN32
        if (chmod(filename.c_str(), _S_IREAD | _S_IWRITE) != 0) {
        #else
        if (chmod(filename.c_str(), S_IRUSR | S_IWUSR) != 0) {
        #endif
            std::cerr << "Error: Could not set file permissions" << std::endl;
            return;
        }
        
        std::cout << "File '" << filename << "' created successfully with restrictive permissions." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

int main() {
    // Test case 1: Standard execution
    createSecretFile("secret.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    // Test case 2: Different filename
    createSecretFile("secret2.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    // Test case 3: Empty content
    createSecretFile("secret3.txt", "");
    
    // Test case 4: Longer content
    createSecretFile("secret4.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    // Test case 5: Special characters in content
    createSecretFile("secret5.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==!@#$%");
    
    return 0;
}
