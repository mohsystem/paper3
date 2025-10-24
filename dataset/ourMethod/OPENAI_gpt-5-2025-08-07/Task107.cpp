#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <cctype>

static bool isValidName(const std::string& name) {
    if (name.empty() || name.size() > 64) return false;
    for (char c : name) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-' || c == '.' || c == '/')) {
            return false;
        }
    }
    return true;
}

class SafeResource {
public:
    SafeResource(std::string name, int capacity)
        : name_(name.empty() ? "resource" : std::move(name)),
          capacity_(capacity < 0 ? 0 : (capacity > 1000000 ? 1000000 : capacity)) {}
    std::string toString() const {
        return "SafeResource(name=" + name_ + ",capacity=" + std::to_string(capacity_) + ")";
    }
private:
    std::string name_;
    int capacity_;
};

using Registry = std::unordered_map<std::string, std::string>;

static std::vector<std::string> jndi_lookup(
    const std::unordered_map<std::string, std::string>& initial_bindings,
    const std::vector<std::string>& names
) {
    // Build registry with validated keys
    Registry reg;
    for (const auto& kv : initial_bindings) {
        if (isValidName(kv.first)) {
            reg[kv.first] = kv.second;
        }
    }
    std::vector<std::string> results;
    results.reserve(names.size());
    for (const auto& n : names) {
        if (!isValidName(n)) {
            results.emplace_back("ERROR: invalid-name");
            continue;
        }
        auto it = reg.find(n);
        if (it == reg.end()) {
            results.emplace_back("NOT_FOUND");
        } else {
            const std::string& v = it->second;
            if (v.size() > 256) {
                results.emplace_back(v.substr(0, 256));
            } else {
                results.emplace_back(v);
            }
        }
    }
    return results;
}

static void printCase(const std::string& title, const std::vector<std::string>& res) {
    std::cout << title << ": [";
    for (size_t i = 0; i < res.size(); ++i) {
        std::cout << res[i];
        if (i + 1 < res.size()) std::cout << ", ";
    }
    std::cout << "]\n";
}

int main() {
    // Prepare initial registry with stringified values for simplicity
    std::unordered_map<std::string, std::string> initial;
    initial["config/db/url"] = "jdbc:postgresql://localhost:5432/app";
    initial["config/maxConnections"] = "20";
    SafeResource cache("CacheService", 128);
    initial["service/cache"] = cache.toString();
    initial["feature/enabled"] = "true";
    initial["metrics.endpoint"] = "http://localhost:8080/metrics";

    // Test 1
    auto r1 = jndi_lookup(initial, {"config/db/url", "config/maxConnections", "service/cache"});
    printCase("Test1", r1);

    // Test 2
    auto r2 = jndi_lookup(initial, {"config/missing", "feature/enabled", "service/unknown"});
    printCase("Test2", r2);

    // Test 3
    auto r3 = jndi_lookup(initial, {"bad name", std::string(65, 'a'), "valid.name-OK/1"});
    printCase("Test3", r3);

    // Test 4
    auto initial2 = initial;
    initial2["config/maxConnections"] = "50";
    auto r4 = jndi_lookup(initial2, {"config/maxConnections", "metrics.endpoint"});
    printCase("Test4", r4);

    // Test 5
    auto r5 = jndi_lookup(initial, {"metrics.endpoint", "valid.name-OK/1"});
    printCase("Test5", r5);

    return 0;
}