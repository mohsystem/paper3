#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <stdexcept>

static bool is_valid_digits(const std::string& s) {
    if (s.empty()) return false;
    for (char c : s) {
        if (!std::isdigit(static_cast<unsigned char>(c))) return false;
    }
    return true;
}

static std::string strip_leading_zeros(const std::string& s) {
    size_t i = 0;
    while (i < s.size() && s[i] == '0') ++i;
    if (i == s.size()) return "0";
    return s.substr(i);
}

static std::string multiply_big_by_digit(const std::string& a, int d) {
    if (d < 0 || d > 9) throw std::invalid_argument("digit out of range");
    if (d == 0 || a == "0") return "0";
    if (d == 1) return a;
    std::string res;
    res.resize(a.size() + 1);
    int carry = 0;
    int ri = (int)res.size() - 1;
    for (int i = (int)a.size() - 1; i >= 0; --i) {
        int digit = a[i] - '0';
        int prod = digit * d + carry;
        res[ri--] = char('0' + (prod % 10));
        carry = prod / 10;
    }
    if (carry > 0) {
        res[ri] = char('0' + carry);
    } else {
        ri++;
    }
    return res.substr((size_t)ri);
}

static std::string product_of_digits(const std::string& s) {
    std::string prod = "1";
    for (char c : s) {
        int d = c - '0';
        if (d == 0) return "0";
        prod = multiply_big_by_digit(prod, d);
    }
    return prod;
}

int persistence(const std::string& num) {
    if (!is_valid_digits(num)) throw std::invalid_argument("Input must contain only digits");
    std::string s = strip_leading_zeros(num);
    if (s.size() == 1) return 0;
    int count = 0;
    while (s.size() > 1) {
        s = product_of_digits(s);
        count++;
    }
    return count;
}

int main() {
    std::vector<std::string> tests = {"39", "999", "4", "25", "77"};
    for (const auto& t : tests) {
        try {
            std::cout << "persistence(" << t << ") = " << persistence(t) << "\n";
        } catch (const std::exception& e) {
            std::cout << "Error for input " << t << ": " << e.what() << "\n";
        }
    }
    return 0;
}