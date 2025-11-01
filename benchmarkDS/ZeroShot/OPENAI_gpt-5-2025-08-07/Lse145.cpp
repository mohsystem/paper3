#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <cctype>
#include <ctime>

struct Record {
    std::string stock;
    int quantity;
    std::time_t ts;
};

static std::vector<Record> DB;

static inline std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
    return s.substr(start, end - start);
}

static bool is_valid_stock(const std::string& s) {
    if (s.empty() || s.size() > 15) return false;
    for (char c : s) {
        if (!(std::isdigit(static_cast<unsigned char>(c)) ||
              (c >= 'A' && c <= 'Z') ||
              c == '.' || c == '-')) {
            return false;
        }
    }
    return true;
}

// Normalize and validate stock name
std::string buy_function(const std::string& stockName) {
    std::string t = trim(stockName);
    if (t.empty()) {
        throw std::invalid_argument("Stock name must not be empty");
    }
    std::string normalized = t;
    std::transform(normalized.begin(), normalized.end(), normalized.begin(),
                   [](unsigned char c){ return static_cast<char>(std::toupper(c)); });
    if (!is_valid_stock(normalized)) {
        throw std::invalid_argument("Invalid stock name format");
    }
    return normalized;
}

// Insert into "database" after validation
Record buy_stock(const std::string& stockName, int quantity) {
    std::string normalized = buy_function(stockName);
    if (quantity <= 0 || quantity > 1000000) {
        throw std::invalid_argument("Quantity out of allowed range");
    }
    Record rec{normalized, quantity, std::time(nullptr)};
    DB.push_back(rec);
    return rec;
}

static void print_db() {
    std::cout << "DB size: " << DB.size() << "\n";
    for (const auto& r : DB) {
        std::cout << "Record{stock='" << r.stock << "', quantity=" << r.quantity << ", ts=" << r.ts << "}\n";
    }
}

int main() {
    // 5 test cases
    try {
        std::cout << "Test 1: " << buy_stock("AAPL", 10).stock << "\n";
    } catch (const std::exception& e) {
        std::cout << "Test 1 failed: " << e.what() << "\n";
    }

    try {
        std::cout << "Test 2: " << buy_stock("  msft ", 25).stock << "\n";
    } catch (const std::exception& e) {
        std::cout << "Test 2 failed: " << e.what() << "\n";
    }

    try {
        std::cout << "Test 3: " << buy_stock("AAPL; DROP TABLES", 5).stock << "\n";
    } catch (const std::exception& e) {
        std::cout << "Test 3 expected failure: " << e.what() << "\n";
    }

    try {
        std::cout << "Test 4: " << buy_stock("GOOG", -5).stock << "\n";
    } catch (const std::exception& e) {
        std::cout << "Test 4 expected failure: " << e.what() << "\n";
    }

    try {
        std::cout << "Test 5: " << buy_stock("BRK.B", 100).stock << "\n";
    } catch (const std::exception& e) {
        std::cout << "Test 5 failed: " << e.what() << "\n";
    }

    print_db();
    return 0;
}