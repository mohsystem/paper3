#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <iomanip>

struct Product {
    std::string name;
    double price;
    std::string description;
};

// Use a function to initialize the map to avoid static initialization order issues.
const std::map<std::string, Product>& getProductTable() {
    static const std::map<std::string, Product> productTable = {
        {"PROD101", {"Laptop", 999.99, "High-performance laptop"}},
        {"PROD102", {"Mouse", 25.50, "Ergonomic wireless mouse"}},
        {"PROD103", {"Keyboard", 75.00, "Mechanical gaming keyboard"}},
        {"PROD104", {"Monitor", 300.00, "27-inch 4K UHD monitor"}}
    };
    return productTable;
}

// Helper function to check if a string is empty or just whitespace
bool isStringBlank(const std::string& s) {
    return s.find_first_not_of(" \t\n\v\f\r") == std::string::npos;
}

/**
 * @brief Inquires the product table for product details in a secure manner.
 * This function is secure because:
 * 1. Input is validated to reject blank strings.
 * 2. It uses a std::map for lookups, which is not vulnerable to injection attacks.
 * 3. `map::find` safely handles keys that are not present.
 * 4. The `const std::string&` parameter prevents unnecessary copies and modification.
 * 
 * @param productId The ID of the product to look up.
 * @return A string with product details or an error message.
 */
std::string getProductDetails(const std::string& productId) {
    // 1. Input Validation: Check for empty or whitespace-only strings.
    if (isStringBlank(productId)) {
        return "Error: Product ID cannot be empty.";
    }

    const auto& productTable = getProductTable();

    // 2. Data Lookup: Safely look up the product in the map using find().
    auto it = productTable.find(productId);

    // 3. Result Handling: Format the output or return a 'not found' message.
    if (it != productTable.end()) {
        const Product& product = it->second;
        std::ostringstream oss;
        oss << "Name: " << product.name << ", Price: " << std::fixed 
            << std::setprecision(2) << product.price 
            << ", Description: " << product.description;
        return oss.str();
    } else {
        return "Product not found.";
    }
}


int main() {
    std::cout << "--- Product Inquiry System ---" << std::endl;

    // Test Case 1: Valid product ID
    std::string test1 = "PROD101";
    std::cout << "Querying for ID: '" << test1 << "'" << std::endl;
    std::cout << "Result: " << getProductDetails(test1) << std::endl << std::endl;
    
    // Test Case 2: Another valid product ID
    std::string test2 = "PROD103";
    std::cout << "Querying for ID: '" << test2 << "'" << std::endl;
    std::cout << "Result: " << getProductDetails(test2) << std::endl << std::endl;

    // Test Case 3: Non-existent product ID
    std::string test3 = "PROD999";
    std::cout << "Querying for ID: '" << test3 << "'" << std::endl;
    std::cout << "Result: " << getProductDetails(test3) << std::endl << std::endl;

    // Test Case 4: Whitespace string input
    std::string test4 = "   ";
    std::cout << "Querying for ID: '" << test4 << "'" << std::endl;
    std::cout << "Result: " << getProductDetails(test4) << std::endl << std::endl;

    // Test Case 5: Empty string input (C++ strings cannot be null)
    std::string test5 = "";
    std::cout << "Querying for ID: '" << test5 << "'" << std::endl;
    std::cout << "Result: " << getProductDetails(test5) << std::endl << std::endl;
    
    return 0;
}