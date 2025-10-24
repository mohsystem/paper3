#include <iostream>
#include <string>
#include <unordered_set>
#include <random>

static const std::string ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

std::string generateSessionId(std::size_t length) {
    if (length == 0) throw std::invalid_argument("length must be > 0");
    static std::unordered_set<std::string> issued;
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    std::uniform_int_distribution<std::size_t> dist(0, ALPHABET.size() - 1);

    for (;;) {
        std::string id;
        id.reserve(length);
        for (std::size_t i = 0; i < length; ++i) {
            id.push_back(ALPHABET[dist(gen)]);
        }
        if (issued.insert(id).second) {
            return id;
        }
    }
}

int main() {
    std::size_t lengths[5] = {16, 24, 32, 40, 48};
    for (auto len : lengths) {
        std::cout << generateSessionId(len) << std::endl;
    }
    return 0;
}