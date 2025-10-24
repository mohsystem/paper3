#include <iostream>
#include <unordered_map>
#include <map>
#include <vector>
#include <regex>
#include <stdexcept>
#include <string>

// Step 1: Problem understanding
// Secure in-memory JNDI-like registry for string resources with bind/rebind/lookup.
// Step 2: Security - no network, restricted names and value sizes.

class SafeContext {
private:
    std::unordered_map<std::string, std::string> store;
    const std::regex nameRe{R"(^(?!/)(?!.*//)[A-Za-z0-9_.\-/]{1,256}(?<!/)$)"};
    static constexpr size_t MAX_VALUE_LENGTH = 8192;

    bool validName(const std::string& name) const {
        return std::regex_match(name, nameRe);
    }
    void validateValue(const std::string& v) const {
        if (v.size() > MAX_VALUE_LENGTH) throw std::runtime_error("Value too large");
    }

public:
    void bind(const std::string& name, const std::string& value) {
        if (!validName(name)) throw std::runtime_error("Invalid name: " + name);
        validateValue(value);
        if (store.find(name) != store.end()) throw std::runtime_error("Already bound: " + name);
        store[name] = value;
    }
    void rebind(const std::string& name, const std::string& value) {
        if (!validName(name)) throw std::runtime_error("Invalid name: " + name);
        validateValue(value);
        store[name] = value;
    }
    std::string lookup(const std::string& name) const {
        if (!validName(name)) throw std::runtime_error("Invalid name: " + name);
        auto it = store.find(name);
        if (it == store.end()) throw std::runtime_error("Name not found: " + name);
        return it->second;
    }
};

std::map<std::string, std::string> performLookups(const std::map<std::string, std::string>& initialBindings,
                                                  const std::vector<std::string>& lookupNames) {
    SafeContext ctx;
    for (const auto& kv : initialBindings) {
        // Use rebind to simplify duplicates
        if (!std::regex_match(kv.first, std::regex(R"(^(?!/)(?!.*//)[A-Za-z0-9_.\-/]{1,256}(?<!/)$)"))) {
            throw std::runtime_error("Invalid name: " + kv.first);
        }
        if (kv.second.size() > 8192) throw std::runtime_error("Value too large");
        ctx.rebind(kv.first, kv.second);
    }
    std::map<std::string, std::string> results;
    for (const auto& name : lookupNames) {
        if (!std::regex_match(name, std::regex(R"(^(?!/)(?!.*//)[A-Za-z0-9_.\-/]{1,256}(?<!/)$)"))) {
            results[name] = "<INVALID_NAME>";
            continue;
        }
        try {
            results[name] = ctx.lookup(name);
        } catch (...) {
            results[name] = "<NOT_FOUND>";
        }
    }
    return results;
}

int main() {
    // Test 1
    std::map<std::string, std::string> bind1{{"config/db/url", "jdbc:h2:mem:test1"}, {"feature/flagA", "true"}};
    std::vector<std::string> look1{"config/db/url", "feature/flagA", "missing/name"};
    auto res1 = performLookups(bind1, look1);
    std::cout << "Test1:";
    for (auto& kv : res1) std::cout << " " << kv.first << "=" << kv.second;
    std::cout << "\n";

    // Test 2
    std::map<std::string, std::string> bind2{{"service/endpoint", "https://api.v2.example.com"}};
    std::vector<std::string> look2{"service/endpoint"};
    auto res2 = performLookups(bind2, look2);
    std::cout << "Test2:";
    for (auto& kv : res2) std::cout << " " << kv.first << "=" << kv.second;
    std::cout << "\n";

    // Test 3
    std::map<std::string, std::string> bind3{{"app/env/prod/log/level", "INFO"}, {"app/env/prod/threads", "16"}};
    std::vector<std::string> look3{"app/env/prod/log/level", "app/env/prod/threads"};
    auto res3 = performLookups(bind3, look3);
    std::cout << "Test3:";
    for (auto& kv : res3) std::cout << " " << kv.first << "=" << kv.second;
    std::cout << "\n";

    // Test 4
    try {
        std::map<std::string, std::string> bind4{{"/bad/leading/slash", "x"}};
        std::vector<std::string> look4{"/bad/leading/slash", "also//bad"};
        auto res4 = performLookups(bind4, look4);
        std::cout << "Test4:";
        for (auto& kv : res4) std::cout << " " << kv.first << "=" << kv.second;
        std::cout << "\n";
    } catch (const std::exception& e) {
        std::cout << "Test4: Caught expected exception: " << e.what() << "\n";
    }

    // Test 5
    std::string big;
    big.reserve(10000);
    for (int i = 0; i < 1000; ++i) big += "0123456789";
    std::map<std::string, std::string> bind5{{"blob/data", big}};
    std::vector<std::string> look5{"blob/data", "not/present"};
    auto res5 = performLookups(bind5, look5);
    std::cout << "Test5: blob/data size=" << res5["blob/data"].size() << " not/present=" << res5["not/present"] << "\n";

    return 0;
}