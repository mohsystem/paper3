// Chain-of-Through secure implementation for a simple Caesar cipher.
// 1) Problem understanding: Encrypt provided string using Caesar cipher.
// 2) Security requirements: Robust handling of inputs; avoid undefined behavior.
// 3) Secure coding generation: Normalize key; non-alpha unchanged.
// 4) Code review: Proper bounds and wrap-around.
// 5) Secure code output: Finalized and consistent with other languages.
#include <iostream>
#include <string>

static int normalizeKey(int key) {
    int k = key % 26;
    if (k < 0) k += 26;
    return k;
}

std::string encrypt(const std::string& text, int key) {
    int k = normalizeKey(key);
    std::string out;
    out.reserve(text.size());
    for (char c : text) {
        if (c >= 'A' && c <= 'Z') {
            int offset = c - 'A';
            char outc = static_cast<char>('A' + (offset + k) % 26);
            out.push_back(outc);
        } else if (c >= 'a' && c <= 'z') {
            int offset = c - 'a';
            char outc = static_cast<char>('a' + (offset + k) % 26);
            out.push_back(outc);
        } else {
            out.push_back(c);
        }
    }
    return out;
}

int main() {
    std::pair<std::string,int> tests[5] = {
        {"Hello, World!", 3},
        {"abcxyz ABCXYZ", 2},
        {"Attack at dawn! 123", 5},
        {"", 10},
        {"Zebra-123", -1}
    };
    for (const auto& t : tests) {
        std::cout << "Input: " << t.first << " | Key: " << t.second
                  << " | Encrypted: " << encrypt(t.first, t.second) << "\n";
    }
    return 0;
}