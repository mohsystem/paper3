
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <vector>

class Lse48 {
public:
    static std::vector<char> img(const std::map<std::string, std::string>& requestArgs) {
        std::vector<char> data;
        
        auto it = requestArgs.find("file");
        if (it == requestArgs.end() || it->second.empty()) {
            return data;
        }
        
        std::string file = it->second;
        std::ifstream inputFile(file, std::ios::binary);
        
        if (!inputFile.is_open()) {
            std::cerr << "Error: Could not open file " << file << std::endl;
            return data;
        }
        
        inputFile.seekg(0, std::ios::end);
        std::streamsize size = inputFile.tellg();
        inputFile.seekg(0, std::ios::beg);
        
        data.resize(size);
        inputFile.read(data.data(), size);
        inputFile.close();
        
        return data;
    }
};

int main() {
    // Test case 1: Valid image file
    std::map<std::string, std::string> test1 = {{"file", "test1.jpg"}};
    std::vector<char> result1 = Lse48::img(test1);
    std::cout << "Test 1 - File: test1.jpg, Result: " << (result1.size() > 0 ? "Success (" + std::to_string(result1.size()) + " bytes)" : "Failed") << std::endl;
    
    // Test case 2: Valid text file
    std::map<std::string, std::string> test2 = {{"file", "test2.txt"}};
    std::vector<char> result2 = Lse48::img(test2);
    std::cout << "Test 2 - File: test2.txt, Result: " << (result2.size() > 0 ? "Success (" + std::to_string(result2.size()) + " bytes)" : "Failed") << std::endl;
    
    // Test case 3: Empty file parameter
    std::map<std::string, std::string> test3 = {{"file", ""}};
    std::vector<char> result3 = Lse48::img(test3);
    std::cout << "Test 3 - Empty file, Result: " << (result3.size() > 0 ? "Success" : "Failed (Expected)") << std::endl;
    
    // Test case 4: Null file parameter
    std::map<std::string, std::string> test4;
    std::vector<char> result4 = Lse48::img(test4);
    std::cout << "Test 4 - Null file, Result: " << (result4.size() > 0 ? "Success" : "Failed (Expected)") << std::endl;
    
    // Test case 5: Non-existent file
    std::map<std::string, std::string> test5 = {{"file", "nonexistent.jpg"}};
    std::vector<char> result5 = Lse48::img(test5);
    std::cout << "Test 5 - Non-existent file, Result: " << (result5.size() > 0 ? "Success" : "Failed (Expected)") << std::endl;
    
    return 0;
}
