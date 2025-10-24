
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <iomanip>
#include <algorithm>

using namespace std;

class Product {
public:
    string id;
    string name;
    double price;
    string category;
    int stock;
    
    Product() {}
    
    Product(string id, string name, double price, string category, int stock)
        : id(id), name(name), price(price), category(category), stock(stock) {}
    
    string toString() const {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "ID: %s, Name: %s, Price: $%.2f, Category: %s, Stock: %d",
                id.c_str(), name.c_str(), price, category.c_str(), stock);
        return string(buffer);
    }
};

map<string, Product> productTable;

void initializeProductTable() {
    productTable["P001"] = Product("P001", "Laptop", 999.99, "Electronics", 50);
    productTable["P002"] = Product("P002", "Mouse", 29.99, "Electronics", 200);
    productTable["P003"] = Product("P003", "Keyboard", 79.99, "Electronics", 150);
    productTable["P004"] = Product("P004", "Monitor", 299.99, "Electronics", 75);
    productTable["P005"] = Product("P005", "Desk Chair", 199.99, "Furniture", 30);
}

Product* queryProduct(const string& productId) {
    auto it = productTable.find(productId);
    if (it != productTable.end()) {
        return &(it->second);
    }
    return nullptr;
}

vector<Product> queryProductsByCategory(const string& category) {
    vector<Product> results;
    for (auto& pair : productTable) {
        string catLower = pair.second.category;
        string searchLower = category;
        transform(catLower.begin(), catLower.end(), catLower.begin(), ::tolower);
        transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
        
        if (catLower == searchLower) {
            results.push_back(pair.second);
        }
    }
    return results;
}

vector<Product> queryProductsByPriceRange(double minPrice, double maxPrice) {
    vector<Product> results;
    for (auto& pair : productTable) {
        if (pair.second.price >= minPrice && pair.second.price <= maxPrice) {
            results.push_back(pair.second);
        }
    }
    return results;
}

int main() {
    initializeProductTable();
    
    cout << "Test Case 1: Query by Product ID" << endl;
    Product* product1 = queryProduct("P001");
    if (product1) {
        cout << product1->toString() << endl;
    } else {
        cout << "Product not found" << endl;
    }
    cout << endl;
    
    cout << "Test Case 2: Query by Product ID (Non-existent)" << endl;
    Product* product2 = queryProduct("P999");
    if (product2) {
        cout << product2->toString() << endl;
    } else {
        cout << "Product not found" << endl;
    }
    cout << endl;
    
    cout << "Test Case 3: Query by Category" << endl;
    vector<Product> electronics = queryProductsByCategory("Electronics");
    cout << "Electronics products: " << electronics.size() << endl;
    for (const auto& p : electronics) {
        cout << p.toString() << endl;
    }
    cout << endl;
    
    cout << "Test Case 4: Query by Price Range (50-100)" << endl;
    vector<Product> midRange = queryProductsByPriceRange(50.0, 100.0);
    cout << "Products in range: " << midRange.size() << endl;
    for (const auto& p : midRange) {
        cout << p.toString() << endl;
    }
    cout << endl;
    
    cout << "Test Case 5: Query by Category (Furniture)" << endl;
    vector<Product> furniture = queryProductsByCategory("Furniture");
    cout << "Furniture products: " << furniture.size() << endl;
    for (const auto& p : furniture) {
        cout << p.toString() << endl;
    }
    
    return 0;
}
