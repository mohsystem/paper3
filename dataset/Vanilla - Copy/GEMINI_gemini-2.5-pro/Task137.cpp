#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <iomanip>

// A struct to represent a product
struct Product {
    int id;
    std::string name;
    double price;
    int quantity;
};

// Initializes and returns the product table
std::map<std::string, Product> initializeProductTable() {
    std::map<std::string, Product> table;
    table["Laptop"] = {101, "Laptop", 1200.50, 10};
    table["Mouse"] = {102, "Mouse", 25.00, 150};
    table["Keyboard"] = {103, "Keyboard", 75.99, 80};
    table["Monitor"] = {104, "Monitor", 300.00, 45};
    table["USB Hub"] = {105, "USB Hub", 15.50, 200};
    return table;
}

/**
 * Inquires the product table for a product's details by its name.
 * @param productName The name of the product to search for.
 * @return A string with the product details or a "not found" message.
 */
std::string getProductDetails(const std::string& productName) {
    static const std::map<std::string, Product> productTable = initializeProductTable();
    
    auto it = productTable.find(productName);
    
    if (it != productTable.end()) {
        const Product& product = it->second;
        std::stringstream ss;
        ss << "ID: " << product.id 
           << ", Name: " << product.name 
           << ", Price: " << std::fixed << std::setprecision(2) << product.price 
           << ", Quantity: " << product.quantity;
        return ss.str();
    } else {
        return "Product not found.";
    }
}

int main() {
    // 5 test cases
    std::string testCases[] = {"Laptop", "Monitor", "Mouse", "Webcam", "USB Hub"};

    std::cout << "--- C++ Product Inquiry System ---" << std::endl;
    for (const auto& testCase : testCases) {
        std::cout << "Querying for: " << testCase << std::endl;
        std::string details = getProductDetails(testCase);
        std::cout << "Result: " << details << std::endl << std::endl;
    }

    return 0;
}