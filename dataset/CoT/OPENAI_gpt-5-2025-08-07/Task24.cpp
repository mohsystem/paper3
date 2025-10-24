#include <iostream>
#include <string>
#include <vector>

class Task24 {
public:
    static bool isValidPIN(const std::string& pin) {
        size_t len = pin.size();
        if (len != 4 && len != 6) return false;
        for (char ch : pin) {
            if (ch < '0' || ch > '9') return false; // ASCII digit check only
        }
        return true;
    }
};

int main() {
    std::vector<std::string> tests = {"1234", "12345", "a234", "098765", ""};
    for (const auto& t : tests) {
        std::cout << t << " -> " << (Task24::isValidPIN(t) ? "true" : "false") << std::endl;
    }
    return 0;
}