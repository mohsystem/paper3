#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <unordered_set>

struct Purchase {
    unsigned long id;
    std::string stock;
    int quantity;
};

class Database {
public:
    Database() : nextId_(1), maxRecords_(1000000) {
        purchases_.reserve(16);
    }

    unsigned long insert(const std::string& stock, int qty) {
        if (purchases_.size() >= maxRecords_) {
            throw std::runtime_error("Capacity reached");
        }
        Purchase p{ nextId_, stock, qty };
        purchases_.push_back(p);
        return nextId_++;
    }

    std::vector<Purchase> all() const {
        return purchases_;
    }

private:
    std::vector<Purchase> purchases_;
    unsigned long nextId_;
    size_t maxRecords_;
};

// Allowed tickers
static const std::unordered_set<std::string> ALLOWED_TICKERS{
    "AAPL","GOOG","MSFT","TSLA","AMZN"
};

static bool isValidTickerFormat(const std::string& s) {
    if (s.empty() || s.size() > 15) return false;
    if (!(s[0] >= 'A' && s[0] <= 'Z')) return false;
    for (size_t i = 1; i < s.size(); ++i) {
        char c = s[i];
        bool ok = (c >= 'A' && c <= 'Z') ||
                  (c >= '0' && c <= '9') ||
                  c == '.' || c == '-';
        if (!ok) return false;
    }
    return true;
}

static std::string trim(const std::string& in) {
    size_t start = 0;
    while (start < in.size() && std::isspace(static_cast<unsigned char>(in[start]))) start++;
    if (start == in.size()) return "";
    size_t end = in.size() - 1;
    while (end > start && std::isspace(static_cast<unsigned char>(in[end]))) end--;
    return in.substr(start, end - start + 1);
}

static bool isValidQuantity(int q) {
    return q >= 1 && q <= 1000000;
}

// buy_function: verify stock availability
bool buy_function(const std::string& stockName) {
    return ALLOWED_TICKERS.find(stockName) != ALLOWED_TICKERS.end();
}

// buy_stock: validate, call buy_function, insert
std::string buy_stock(Database& db, const std::string& stockInput, int quantity) {
    if (!isValidQuantity(quantity)) {
        return "ERROR: Invalid quantity";
    }
    std::string trimmed = trim(stockInput);
    if (trimmed.empty()) {
        return "ERROR: Invalid stock name";
    }
    std::string upper = trimmed;
    std::transform(upper.begin(), upper.end(), upper.begin(),
                   [](unsigned char c){ return static_cast<char>(std::toupper(c)); });
    if (!isValidTickerFormat(upper)) {
        return "ERROR: Invalid stock name";
    }
    if (!buy_function(upper)) {
        return "ERROR: Stock not available";
    }
    try {
        unsigned long id = db.insert(upper, quantity);
        return "SUCCESS: id=" + std::to_string(id) + " stock=" + upper + " qty=" + std::to_string(quantity);
    } catch (const std::runtime_error&) {
        return "ERROR: Database capacity reached";
    } catch (...) {
        return "ERROR: Internal error";
    }
}

int main() {
    Database db;
    struct Case { std::string s; int q; };
    std::vector<Case> tests = {
        {"AAPL", 10},
        {" msft ", 5},
        {"AAPL;DELETE", 1},
        {"GOOG", 0},
        {"IBM", 1}
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        std::cout << "Test " << (i+1) << ": " << buy_stock(db, tests[i].s, tests[i].q) << "\n";
    }
    for (const auto& p : db.all()) {
        std::cout << "Record: id=" << p.id << " stock=" << p.stock << " qty=" << p.quantity << "\n";
    }
    return 0;
}