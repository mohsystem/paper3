#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <algorithm>
#include <cctype>
#include <stdexcept>

struct Product {
    int id;
    std::string name;
    double price;
    std::string description;
};

static bool isSafeChar(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c==' ' || c=='_' || c=='-' || c=='.' || c=='\'';
}

static std::string normalizeQuery(const std::string& q) {
    if (q.empty()) throw std::invalid_argument("Query cannot be empty.");
    if (q.size() > 100) throw std::invalid_argument("Query too long.");
    for (char c : q) {
        if (!isSafeChar(c)) throw std::invalid_argument("Query contains disallowed characters.");
    }
    std::string trimmed = q;
    // trim
    while (!trimmed.empty() && std::isspace(static_cast<unsigned char>(trimmed.front()))) trimmed.erase(trimmed.begin());
    while (!trimmed.empty() && std::isspace(static_cast<unsigned char>(trimmed.back()))) trimmed.pop_back();
    if (trimmed.empty()) throw std::invalid_argument("Query cannot be empty.");
    std::string lower = trimmed;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c){ return std::tolower(c); });
    return lower;
}

static int normalizeId(int id) {
    if (id < 0 || id > 1000000000) throw std::invalid_argument("ID out of allowed range.");
    return id;
}

// Function: query product details by ID
std::optional<Product> getProductDetailsById(const std::unordered_map<int, Product>& productTable, int id) {
    int safeId = normalizeId(id);
    auto it = productTable.find(safeId);
    if (it == productTable.end()) return std::nullopt;
    return it->second;
}

// Function: query product details by name (case-insensitive substring)
std::vector<Product> getProductDetailsByName(const std::unordered_map<int, Product>& productTable, const std::string& nameQuery) {
    std::string q = normalizeQuery(nameQuery);
    std::vector<Product> results;
    for (const auto& kv : productTable) {
        std::string nameLower = kv.second.name;
        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), [](unsigned char c){ return std::tolower(c); });
        if (nameLower.find(q) != std::string::npos) {
            results.push_back(kv.second);
        }
    }
    return results;
}

static std::unordered_map<int, Product> buildSampleTable() {
    std::unordered_map<int, Product> t;
    t.emplace(1, Product{1, "Apple iPhone 14", 799.00, "Latest Apple smartphone"});
    t.emplace(2, Product{2, "Samsung Galaxy S23", 749.99, "Flagship Android phone"});
    t.emplace(3, Product{3, "Google Pixel 8", 699.00, "Google's premium device"});
    t.emplace(4, Product{4, "Apple AirPods Pro", 249.00, "Noise-cancelling earbuds"});
    t.emplace(5, Product{5, "Dell XPS 13", 1199.00, "Ultrabook laptop"});
    return t;
}

static void printProduct(const Product* p) {
    if (!p) {
        std::cout << "null\n";
        return;
    }
    std::cout << "Product{id=" << p->id << ", name='" << p->name << "', price=" << p->price
              << ", description='" << p->description << "'}\n";
}

int main() {
    auto table = buildSampleTable();

    // Test case 1: by ID existing
    std::cout << "Test1 (ID=3): ";
    auto p1 = getProductDetailsById(table, 3);
    if (p1) printProduct(&*p1); else printProduct(nullptr);

    // Test case 2: by ID not existing
    std::cout << "Test2 (ID=999): ";
    auto p2 = getProductDetailsById(table, 999);
    if (p2) printProduct(&*p2); else printProduct(nullptr);

    // Test case 3: by name "apple"
    std::cout << "Test3 (name='apple'):\n";
    for (const auto& p : getProductDetailsByName(table, "apple")) {
        printProduct(&p);
    }

    // Test case 4: by name "XPS"
    std::cout << "Test4 (name='XPS'):\n";
    for (const auto& p : getProductDetailsByName(table, "XPS")) {
        printProduct(&p);
    }

    // Test case 5: invalid query input
    std::cout << "Test5 (invalid name):\n";
    try {
        auto bad = getProductDetailsByName(table, "bad; DROP TABLE product;--");
        (void)bad;
        std::cout << "Unexpected success\n";
    } catch (const std::exception& ex) {
        std::cout << "  Caught expected error: " << ex.what() << "\n";
    }

    return 0;
}