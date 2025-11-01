
#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#define chmod _chmod
#endif

std::string createSecretFile(const std::string& secretInfo, const std::string& filename) {
    // Write the secret info to file
    std::ofstream file(filename);
    if (file.is_open()) {
        file << secretInfo;
        file.close();
        
        // Change permissions to read-only for owner
#ifdef _WIN32
        chmod(filename.c_str(), _S_IREAD);
#else
        chmod(filename.c_str(), S_IRUSR);
#endif
        
        return filename;
    }
    return "";
}

int main() {
    // Test case 1
    std::string result1 = createSecretFile("Super secret password: 12345", "secret1.txt");
    std::cout << result1 << std::endl;
    
    // Test case 2
    std::string result2 = createSecretFile("Top secret data: classified", "secret2.txt");
    std::cout << result2 << std::endl;
    
    // Test case 3
    std::string result3 = createSecretFile("Hidden treasure location: X marks the spot", "secret3.txt");
    std::cout << result3 << std::endl;
    
    // Test case 4
    std::string result4 = createSecretFile("API Key: abc123xyz789", "secret4.txt");
    std::cout << result4 << std::endl;
    
    // Test case 5
    std::string result5 = createSecretFile("Secret formula: H2O + C6H12O6", "secret5.txt");
    std::cout << result5 << std::endl;
    
    return 0;
}
