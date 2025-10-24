
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cstring>

class Task131 {
public:
    // Simple deserialization for basic types from string
    template<typename T>
    static T deserialize(const std::string& data) {
        std::istringstream iss(data);
        T value;
        iss >> value;
        return value;
    }
    
    // Deserialize string
    static std::string deserializeString(const std::string& data) {
        return data;
    }
    
    // Deserialize from file
    static std::string deserializeFromFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file");
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
    
    // Deserialize vector of integers from comma-separated string
    static std::vector<int> deserializeIntVector(const std::string& data) {
        std::vector<int> result;
        std::stringstream ss(data);
        std::string item;
        while (std::getline(ss, item, ',')) {
            result.push_back(std::stoi(item));
        }
        return result;
    }
    
    // Serialize for testing
    template<typename T>
    static std::string serialize(const T& value) {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }
    
    static std::string serializeIntVector(const std::vector<int>& vec) {
        std::ostringstream oss;
        for (size_t i = 0; i < vec.size(); ++i) {
            oss << vec[i];
            if (i < vec.size() - 1) oss << ",";
        }
        return oss.str();
    }
};

int main() {
    // Test Case 1: Deserialize integer
    std::cout << "Test Case 1: Integer" << std::endl;
    int num = 42;
    std::string serialized1 = Task131::serialize(num);
    int deserialized1 = Task131::deserialize<int>(serialized1);
    std::cout << "Original: " << num << std::endl;
    std::cout << "Deserialized: " << deserialized1 << std::endl;
    std::cout << std::endl;
    
    // Test Case 2: Deserialize string
    std::cout << "Test Case 2: String" << std::endl;
    std::string str = "Hello, World!";
    std::string serialized2 = str;
    std::string deserialized2 = Task131::deserializeString(serialized2);
    std::cout << "Original: " << str << std::endl;
    std::cout << "Deserialized: " << deserialized2 << std::endl;
    std::cout << std::endl;
    
    // Test Case 3: Deserialize double
    std::cout << "Test Case 3: Double" << std::endl;
    double dbl = 3.14159;
    std::string serialized3 = Task131::serialize(dbl);
    double deserialized3 = Task131::deserialize<double>(serialized3);
    std::cout << "Original: " << dbl << std::endl;
    std::cout << "Deserialized: " << deserialized3 << std::endl;
    std::cout << std::endl;
    
    // Test Case 4: Deserialize vector of integers
    std::cout << "Test Case 4: Vector<int>" << std::endl;
    std::vector<int> vec = {1, 2, 3, 4, 5};
    std::string serialized4 = Task131::serializeIntVector(vec);
    std::vector<int> deserialized4 = Task131::deserializeIntVector(serialized4);
    std::cout << "Original: ";
    for (int v : vec) std::cout << v << " ";
    std::cout << std::endl << "Deserialized: ";
    for (int v : deserialized4) std::cout << v << " ";
    std::cout << std::endl << std::endl;
    
    // Test Case 5: Deserialize long
    std::cout << "Test Case 5: Long" << std::endl;
    long lng = 1234567890L;
    std::string serialized5 = Task131::serialize(lng);
    long deserialized5 = Task131::deserialize<long>(serialized5);
    std::cout << "Original: " << lng << std::endl;
    std::cout << "Deserialized: " << deserialized5 << std::endl;
    
    return 0;
}
