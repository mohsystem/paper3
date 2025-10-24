#include <iostream>
#include <string>
#include <random>

static const std::string ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

std::string generateToken(const std::string& userId, int length) {
    int n = std::max(16, length);
    std::string token;
    token.reserve(n);
    std::random_device rd;
    std::uniform_int_distribution<int> dist(0, static_cast<int>(ALPHABET.size() - 1));
    for (int i = 0; i < n; ++i) {
        token.push_back(ALPHABET[dist(rd)]);
    }
    return token;
}

int main() {
    std::cout << "user1: " << generateToken("user1", 32) << std::endl;
    std::cout << "user2: " << generateToken("user2", 24) << std::endl;
    std::cout << "user3: " << generateToken("user3", 48) << std::endl;
    std::cout << "user4: " << generateToken("user4", 64) << std::endl;
    std::cout << "user5: " << generateToken("user5", 32) << std::endl;
    return 0;
}