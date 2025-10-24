#include <iostream>
#include <string>

bool validate_pin(const std::string& s) {
    const size_t len = s.size();
    if (len != 4 && len != 6) return false;
    for (char ch : s) {
        if (ch < '0' || ch > '9') return false;
    }
    return true;
}

int main() {
    std::string tests[] = {
        "1234",
        "12345",
        "a234",
        "0000",
        "098765"
    };
    for (const auto& t : tests) {
        std::cout << t << " -> " << (validate_pin(t) ? "true" : "false") << "\n";
    }
    return 0;
}