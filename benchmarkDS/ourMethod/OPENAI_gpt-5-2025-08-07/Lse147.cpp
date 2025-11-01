#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <sstream>
#include <iomanip>

struct Order {
    std::string stockName;
    int quantity;
    std::time_t createdAt;
};

class InMemoryDatabase {
public:
    bool insertOrder(const std::string& stockName, int quantity) {
        Order o{stockName, quantity, std::time(nullptr)};
        orders_.push_back(o);
        return true;
    }

    std::vector<Order> getAll() const {
        return orders_;
    }

private:
    std::vector<Order> orders_;
};

static const int MAX_STOCK_NAME_LEN = 64;
static const int MAX_QUANTITY = 1000000;

std::string sanitizeStockName(const std::string& input) {
    // Trim
    size_t start = 0, end = input.size();
    while (start < end && std::isspace(static_cast<unsigned char>(input[start]))) start++;
    while (end > start && std::isspace(static_cast<unsigned char>(input[end - 1]))) end--;
    std::string s = input.substr(start, end - start);
    if (s.size() > static_cast<size_t>(MAX_STOCK_NAME_LEN)) {
        s.resize(MAX_STOCK_NAME_LEN);
    }
    return s;
}

bool isValidStockName(const std::string& s) {
    if (s.empty() || s.size() > static_cast<size_t>(MAX_STOCK_NAME_LEN)) return false;
    for (unsigned char c : s) {
        if (!(std::isalnum(c) || c == ' ' || c == '_' || c == '-' || c == '.')) {
            return false;
        }
    }
    return true;
}

bool isValidQuantity(int q) {
    return q >= 1 && q <= MAX_QUANTITY;
}

std::string formatTime(std::time_t t) {
    std::tm tmBuf{};
#if defined(_WIN32)
    localtime_s(&tmBuf, &t);
#else
    localtime_r(&t, &tmBuf);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tmBuf, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string stock_view(InMemoryDatabase& db) {
    std::ostringstream oss;
    oss << "Stock Orders:\n";
    auto all = db.getAll();
    int i = 1;
    for (const auto& o : all) {
        oss << i++ << ". " << o.stockName << " - " << o.quantity << " @ " << formatTime(o.createdAt) << "\n";
    }
    return oss.str();
}

std::string render_form() {
    std::ostringstream html;
    html << "<!doctype html>\n<html><head><meta charset=\"utf-8\"><title>Buy Order</title></head>\n"
         << "<body>\n<h1>Buy Order</h1>\n<form method=\"POST\" action=\"/buy_order\">\n"
         << "  <label>Stock Name: <input type=\"text\" name=\"stock\" maxlength=\"64\" required></label><br>\n"
         << "  <label>Quantity: <input type=\"number\" name=\"qty\" min=\"1\" max=\"1000000\" required></label><br>\n"
         << "  <button type=\"submit\">Submit</button>\n"
         << "</form>\n</body></html>\n";
    return html.str();
}

std::string buy_order(InMemoryDatabase& db, const std::string& stockNameInput, int quantityInput) {
    std::string stockName = sanitizeStockName(stockNameInput);
    if (!isValidStockName(stockName)) {
        return "Error: invalid stock name.";
    }
    if (!isValidQuantity(quantityInput)) {
        return "Error: invalid quantity.";
    }
    if (!db.insertOrder(stockName, quantityInput)) {
        return "Error: failed to insert order.";
    }
    std::ostringstream oss;
    oss << "OK: Order inserted. Redirecting to /stock_view ...\n";
    oss << stock_view(db);
    return oss.str();
}

int main() {
    InMemoryDatabase db;
    std::cout << "== Rendered Form (Demo) ==\n" << render_form() << "\n";

    // 5 test cases
    std::cout << "Test 1: " << buy_order(db, "AAPL", 10) << "\n";
    std::cout << "Test 2: " << buy_order(db, "GOOG", 25) << "\n";
    std::cout << "Test 3: " << buy_order(db, "MSFT", 0) << "\n";
    std::cout << "Test 4: " << buy_order(db, "DROP TABLE;--", 5) << "\n";
    std::cout << "Test 5: " << buy_order(db, "BRK.B Class A", 100) << "\n";
    return 0;
}