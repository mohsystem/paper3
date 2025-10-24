#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cctype>
#include <sstream>
#include <limits>

struct Product {
    int id;
    std::string name;
    double price;
    int stock;
};

class ProductDB {
public:
    explicit ProductDB(const std::vector<Product>& products) {
        for (const auto& p : products) {
            byId[p.id] = p;
            byName[p.name] = p;
        }
    }

    const Product* findById(int id) const {
        auto it = byId.find(id);
        if (it == byId.end()) return nullptr;
        return &it->second;
    }

    const Product* findByName(const std::string& name) const {
        auto it = byName.find(name);
        if (it == byName.end()) return nullptr;
        return &it->second;
    }

private:
    std::unordered_map<int, Product> byId;
    std::unordered_map<std::string, Product> byName;
};

static std::string escapeJson(const std::string& s) {
    std::ostringstream oss;
    for (unsigned char c : s) {
        switch (c) {
            case '\\': oss << "\\\\"; break;
            case '"':  oss << "\\\""; break;
            case '\b': oss << "\\b";  break;
            case '\f': oss << "\\f";  break;
            case '\n': oss << "\\n";  break;
            case '\r': oss << "\\r";  break;
            case '\t': oss << "\\t";  break;
            default:
                if (c < 0x20) {
                    oss << "\\u";
                    static const char* hex = "0123456789abcdef";
                    oss << '0' << '0' << hex[(c >> 4) & 0xF] << hex[c & 0xF];
                } else {
                    oss << c;
                }
        }
    }
    return oss.str();
}

static std::string errorJson(const std::string& msg) {
    return std::string("{\"ok\":false,\"error\":\"") + escapeJson(msg) + "\"}";
}

static std::string okJson(const Product& p) {
    std::ostringstream oss;
    oss.setf(std::ios::fixed);
    oss.precision(2);
    oss << "{\"ok\":true,\"product\":{"
        << "\"id\":" << p.id << ","
        << "\"name\":\"" << escapeJson(p.name) << "\","
        << "\"price\":" << p.price << ","
        << "\"stock\":" << p.stock
        << "}}";
    return oss.str();
}

static bool isDigits(const std::string& s) {
    if (s.empty() || s.size() > 9) return false;
    for (char ch : s) if (!std::isdigit(static_cast<unsigned char>(ch))) return false;
    return true;
}

static bool isValidName(const std::string& s) {
    if (s.empty() || s.size() > 50) return false;
    for (char ch : s) {
        if (!(std::isalnum(static_cast<unsigned char>(ch)) || ch == ' ' || ch == '-' || ch == '_')) {
            return false;
        }
    }
    return true;
}

std::string processQuery(const std::string& query) {
    if (query.empty() || query.size() > 100) {
        return errorJson("Query length must be between 1 and 100");
    }

    ProductDB db({
        {1001, "Widget", 9.99, 100},
        {1002, "Gadget", 12.49, 50},
        {1003, "Thingamajig", 7.95, 0},
        {2001, "Doodad", 5.00, 25},
        {3001, "Sprocket", 15.75, 10}
    });

    if (query.rfind("id=", 0) == 0) {
        std::string idStr = query.substr(3);
        if (!isDigits(idStr)) {
            return errorJson("Invalid id format. Expected 1-9 digits.");
        }
        long long idLL = 0;
        try {
            idLL = std::stoll(idStr);
        } catch (...) {
            return errorJson("Invalid id value");
        }
        if (idLL <= 0 || idLL > 1000000000LL) {
            return errorJson("Id out of allowed range");
        }
        int id = static_cast<int>(idLL);
        const Product* p = db.findById(id);
        if (!p) return errorJson("Product not found");
        return okJson(*p);
    } else if (query.rfind("name=", 0) == 0) {
        std::string name = query.substr(5);
        if (!isValidName(name)) {
            return errorJson("Invalid name. Allowed: A-Z a-z 0-9 space - _ (1-50 chars)");
        }
        const Product* p = db.findByName(name);
        if (!p) return errorJson("Product not found");
        return okJson(*p);
    } else {
        return errorJson("Invalid query. Use id=<digits> or name=<allowed_name>");
    }
}

int main() {
    std::vector<std::string> tests = {
        "id=1002",
        "name=Widget",
        "id=9999",
        "name=Invalid*Name!",
        "foo=bar"
    };
    for (const auto& q : tests) {
        std::cout << "Query: " << q << "\n";
        std::cout << "Result: " << processQuery(q) << "\n";
        std::cout << "---\n";
    }
    return 0;
}