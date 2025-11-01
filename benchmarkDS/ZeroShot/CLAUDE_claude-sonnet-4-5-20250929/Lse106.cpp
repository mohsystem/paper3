
#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#define chmod _chmod
#endif

std::string createSecretFile(const std::string& secretInfo, const std::string& filename) {
    try {
        // Write the secret info to file
        std::ofstream file(filename);
        if (!file.is_open()) {
            return "Error: Could not open file";
        }
        file << secretInfo;
        file.close();
        
        // Change permissions to read-only for owner
        #ifdef _WIN32
        chmod(filename.c_str(), _S_IREAD);
        #else
        chmod(filename.c_str(), S_IRUSR);
        #endif
        
        return filename;
    } catch (const std::exception& e) {
        return std::string("Error: ") + e.what();
    }
}

int main() {
    // Test case 1
    std::string result1 = createSecretFile("Super secret password: admin123", "secret1.txt");
    std::cout << result1 << std::endl;
    
    // Test case 2
    std::string result2 = createSecretFile("API Key: abc123xyz789", "secret2.txt");
    std::cout << result2 << std::endl;
    
    // Test case 3
    std::string result3 = createSecretFile("Database credentials: user=root, pass=secret", "secret3.txt");
    std::cout << result3 << std::endl;
    
    // Test case 4
    std::string result4 = createSecretFile("Private key: RSA-2048-BEGIN", "secret4.txt");
    std::cout << result4 << std::endl;
    
    // Test case 5
    std::string result5 = createSecretFile("Social Security Number: 123-45-6789", "secret5.txt");
    std::cout << result5 << std::endl;
    
    return 0;
}
