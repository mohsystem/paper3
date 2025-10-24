#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <limits>

/*
 Steps applied:
 1) Problem understanding
 2) Security requirements
 3) Secure coding generation
 4) Code review
 5) Secure code output
*/

struct Product {
    int id;
    std::string name;
    double price;
    int stock;
};

class ProductRepository {
public:
    ProductRepository(const std::vector<Product>& products) {
        for (const auto& p : products) {
            byId[p.id] = p;
            byName[normalizeName(p.name)] = p;
        }
    }

    const Product* findByIdSecure(int id) const {
        if (id < 0 || id > 1000000000) return nullptr;
        auto it = byId.find(id);
        if (it == byId.end()) return nullptr;
        return &it->second;
    }

    const Product* findByNameSecure(const std::string& name) const {
        std::string norm = normalizeName(name);
        if (norm.empty()) return nullptr;
        auto it = byName.find(norm);
        if (it == byName.end()) return nullptr;
        return &it->second;
    }

private:
    std::unordered_map<int, Product> byId;
    std::unordered_map<std::string, Product> byName;

    static bool isAllowedChar(char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || c==' ' || c=='_' || c=='-';
    }

    static std::string normalizeName(const std::string& input) {
        // Trim
        size_t start = 0, end = input.size();
        while (start < end && std::isspace(static_cast<unsigned char>(input[start]))) start++;
        while (end > start && std::isspace(static_cast<unsigned char>(input[end-1]))) end--;
        if (start >= end) return "";
        std::string trimmed = input.substr(start, end - start);

        // Validate allowed chars
        for (char c : trimmed) {
            if (!isAllowedChar(c)) return "";
        }

        // Collapse spaces
        std::string collapsed;
        collapsed.reserve(trimmed.size());
        bool prevSpace = false;
        for (char c : trimmed) {
            if (std::isspace(static_cast<unsigned char>(c))) {
                if (!prevSpace) {
                    collapsed.push_back(' ');
                    prevSpace = true;
                }
            } else {
                collapsed.push_back(c);
                prevSpace = false;
            }
        }

        // Lowercase
        std::string lower;
        lower.reserve(collapsed.size());
        for (char c : collapsed) {
            lower.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        }
        return lower;
    }
};

class Task137Service {
public:
    explicit Task137Service(const ProductRepository& repo) : repo(repo) {}

    std::string processUserInput(const std::string& input) const {
        if (input.size() > 1024) return error("Input too long");
        std::string trimmed = trim(input);
        if (trimmed.empty()) return error("Invalid input");

        // Parse key:value
        size_t colon = trimmed.find(':');
        if (colon == std::string::npos) {
            return error("Unsupported query. Use 'id:<digits>' or 'name:<text>'");
        }
        std::string key = toLower(trim(trimmed.substr(0, colon)));
        std::string value = trim(trimmed.substr(colon + 1));

        if (key == "id") {
            if (!isAllDigits(value) || value.size() > 9) return error("Invalid ID");
            long long idll = 0;
            try {
                idll = std::stoll(value);
            } catch (...) { return error("Invalid ID"); }
            if (idll < 0 || idll > 1000000000LL) return error("Invalid ID");
            const Product* p = repo.findByIdSecure(static_cast<int>(idll));
            if (!p) return notFound();
            return toJson(*p);
        } else if (key == "name") {
            const Product* p = repo.findByNameSecure(value);
            if (!p) return notFound();
            return toJson(*p);
        } else {
            return error("Unsupported query. Use 'id:<digits>' or 'name:<text>'");
        }
    }

private:
    const ProductRepository& repo;

    static std::string trim(const std::string& s) {
        size_t a = 0, b = s.size();
        while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) a++;
        while (b > a && std::isspace(static_cast<unsigned char>(s[b-1]))) b--;
        return s.substr(a, b - a);
    }

    static std::string toLower(const std::string& s) {
        std::string out; out.reserve(s.size());
        for (char c : s) out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        return out;
    }

    static bool isAllDigits(const std::string& s) {
        if (s.empty()) return false;
        for (char c : s) if (!std::isdigit(static_cast<unsigned char>(c))) return false;
        return true;
    }

    static std::string escapeJson(const std::string& s) {
        std::ostringstream oss;
        for (char c : s) {
            switch (c) {
                case '\"': oss << "\\\""; break;
                case '\\': oss << "\\\\"; break;
                case '\b': oss << "\\b"; break;
                case '\f': oss << "\\f"; break;
                case '\n': oss << "\\n"; break;
                case '\r': oss << "\\r"; break;
                case '\t': oss << "\\t"; break;
                default:
                    if (static_cast<unsigned char>(c) < 0x20) {
                        char buf[7];
                        std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                        oss << buf;
                    } else {
                        oss << c;
                    }
            }
        }
        return oss.str();
    }

    static std::string formatPrice(double price) {
        std::ostringstream oss;
        oss.setf(std::ios::fixed);
        oss.precision(2);
        oss << price;
        return oss.str();
    }

    static std::string toJson(const Product& p) {
        std::ostringstream oss;
        oss << "{\"id\":" << p.id
            << ",\"name\":\"" << escapeJson(p.name) << "\""
            << ",\"price\":" << formatPrice(p.price)
            << ",\"stock\":" << p.stock
            << "}";
        return oss.str();
    }

    static std::string error(const std::string& msg) {
        return std::string("{\"error\":\"") + escapeJson(msg) + "\"}";
    }

    static std::string notFound() {
        return std::string("{\"message\":\"Product not found\"}");
    }
};

static ProductRepository buildRepository() {
    std::vector<Product> products = {
        {1001, "Wireless Mouse", 25.99, 42},
        {1002, "Mechanical Keyboard", 79.49, 12},
        {1003, "USB-C Cable", 9.99, 150},
        {1004, "27-inch Monitor", 199.99, 8},
        {1005, "Webcam Pro", 49.95, 5}
    };
    return ProductRepository(products);
}

int main() {
    ProductRepository repo = buildRepository();
    Task137Service service(repo);

    std::vector<std::string> tests = {
        "id:1001",
        "name:Wireless Mouse",
        "name:   usb-c   cable ",
        "id:9999",
        "name:DROP TABLE products;--"
    };

    for (const auto& t : tests) {
        std::cout << "Query: " << t << "\n";
        std::cout << "Result: " << service.processUserInput(t) << "\n";
    }
    return 0;
}