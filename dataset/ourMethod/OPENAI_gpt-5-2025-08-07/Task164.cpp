#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include <stdexcept>

static std::string to_lower_copy(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (unsigned char ch : s) {
        out.push_back(static_cast<char>(std::tolower(ch)));
    }
    return out;
}

static std::string extract_last_name(const std::string& s) {
    // trim
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) start++;
    if (start == s.size()) throw std::invalid_argument("Invalid name: empty after trim");
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) end--;
    if (end - start < 3) throw std::invalid_argument("Invalid name: too short");
    // find last space
    size_t pos = s.rfind(' ', end - 1);
    if (pos == std::string::npos || pos <= start || pos >= end - 1) {
        throw std::invalid_argument("Invalid name format (must contain first and last name): " + s);
    }
    return s.substr(pos + 1, end - (pos + 1));
}

std::vector<std::string> lastNameLenSort(const std::vector<std::string>& names) {
    struct Item {
        std::string original;
        std::string lastLower;
        size_t lastLen;
        size_t index;
    };
    std::vector<Item> items;
    items.reserve(names.size());
    for (size_t i = 0; i < names.size(); ++i) {
        const auto& n = names[i];
        std::string last = extract_last_name(n);
        items.push_back(Item{n, to_lower_copy(last), last.size(), i});
    }
    std::stable_sort(items.begin(), items.end(), [](const Item& a, const Item& b) {
        if (a.lastLen != b.lastLen) return a.lastLen < b.lastLen;
        if (a.lastLower != b.lastLower) return a.lastLower < b.lastLower;
        return a.index < b.index;
    });
    std::vector<std::string> out;
    out.reserve(items.size());
    for (const auto& it : items) out.push_back(it.original);
    return out;
}

static void print_vec(const std::vector<std::string>& v) {
    std::cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        std::cout << v[i];
        if (i + 1 < v.size()) std::cout << ", ";
    }
    std::cout << "]\n";
}

int main() {
    std::vector<std::vector<std::string>> tests = {
        {
            "Jennifer Figueroa",
            "Heather Mcgee",
            "Amanda Schwartz",
            "Nicole Yoder",
            "Melissa Hoffman"
        },
        {
            "Jim Bo",
            "Ana Li",
            "Zoe Wu",
            "Tom Ng",
            "Foo Bar"
        },
        {
            "Mary Jane Watson",
            "Jean Claude Van Damme",
            "Peter Parker",
            "Tony Stark",
            "Bruce Wayne"
        },
        {
            "  Alice   Smith  ",
            "bob a",
            "CARL B",
            "dave  aa",
            "Eve   Z"
        },
        {
            "Alice Hope",
            "Bob Hope",
            "Carl Hope",
            "Ann Hope",
            "Zed Hope"
        }
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        try {
            auto res = lastNameLenSort(tests[i]);
            std::cout << "Test " << (i + 1) << ": ";
            print_vec(res);
        } catch (const std::exception& ex) {
            std::cout << "Error: " << ex.what() << "\n";
        }
    }
    return 0;
}