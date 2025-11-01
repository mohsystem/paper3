
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
        if (file.is_open()) {
            file << content;
            file.close();
            
            // Set restrictive permissions (owner read/write only - 0600)
#ifdef _WIN32
            chmod(filename.c_str(), _S_IREAD | _S_IWRITE);
#else
            chmod(filename.c_str(), S_IRUSR | S_IWUSR);
#endif
        } else {
            std::cerr << "Error: Could not open file " << filename << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

int main() {
    // Test case 1: Default secret.txt
    createSecretFile("secret.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    // Test case 2: Different filename
    createSecretFile("secret2.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    // Test case 3: Different content
    createSecretFile("secret3.txt", "U2FsdGVkX1/TestData123==");
    
    // Test case 4: Another test file
    createSecretFile("secret4.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    // Test case 5: Fifth test file
    createSecretFile("secret5.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    std::cout << "Secret files created with restrictive permissions" << std::endl;
    
    return 0;
}
