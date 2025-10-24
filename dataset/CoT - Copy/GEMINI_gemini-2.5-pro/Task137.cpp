#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <iomanip>
#include <vector>

// Struct to represent a product.
struct Product {
    std::string name;
    double price;
};

// Function to initialize and return the product table.
// Using a function ensures the map is initialized before its first use.
const std::map<int, Product>& getProductTable() {
    static const std::map<int, Product> productTable = {
        {101, {"Laptop", 1200.50}},
        {102, {"Mouse", 25.00}},
        {103, {"Keyboard", 75.75}},
        {104, {"Monitor", 300.00}},
        {105, {"Webcam", 50.25}}
    };
    return productTable;
}

/**
 * @brief Inquires the product table for a product's details based on its ID.
 *
 * @param productId The ID of the product to look up.
 * @return A string containing the product details or a "not found" message.
 */
std::string getProductDetails(int productId) {
    const auto& productTable = getProductTable();
    
    // Use .find() to safely search for the key. It returns an iterator.
    auto it = productTable.find(productId);

    // If the iterator is not at the end, the key was found.
    if (it != productTable.end()) {
        const Product& product = it->second;
        // Use ostringstream for safe and efficient string building.
        std::ostringstream oss;
        oss << "Product ID: " << productId 
            << ", Name: " << product.name 
            << ", Price: $" << std::fixed << std::setprecision(2) << product.price;
        return oss.str();
    } else {
        return "Product with ID " + std::to_string(productId) + " not found.";
    }
}

int main() {
    std::cout << "--- Product Inquiry System ---" << std::endl;

    // Test cases
    std::vector<int> testCases = {101, 104, 105, 999, 102};

    for (size_t i = 0; i < testCases.size(); ++i) {
        std::cout << "Test Case " << (i + 1) << ": Searching for product ID " << testCases[i] << std::endl;
        std::string result = getProductDetails(testCases[i]);
        std::cout << "Result: " << result << std::endl << std::endl;
    }

    return 0;
}