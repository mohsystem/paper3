
#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>

void createSecretFile(const std::string& filename, const std::string& content) {
    // Write content to file
    std::ofstream file(filename);
    if (file.is_open()) {
        file << content;
        file.close();
        
        // Set restrictive permissions (owner read/write only - 0600)
        #ifdef _WIN32
            _chmod(filename.c_str(), _S_IREAD | _S_IWRITE);
        #else
            chmod(filename.c_str(), S_IRUSR | S_IWUSR);
        #endif
    } else {
        std::cerr << "Unable to open file: " << filename << std::endl;
    }
}

int main() {
    // Test case 1
    createSecretFile("secret.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    // Test case 2
    createSecretFile("secret2.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    // Test case 3
    createSecretFile("secret3.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    // Test case 4
    createSecretFile("secret4.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    // Test case 5
    createSecretFile("secret5.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    std::cout << "Files created with restrictive permissions" << std::endl;
    
    return 0;
}
