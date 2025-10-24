#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <stdexcept>

std::string encrypt(const std::string& message, const std::string& key) {
    if (key.empty()) throw std::invalid_argument("Key must not be empty");
    std::ostringstream ss;
    ss << std::uppercase << std::hex << std::setfill('0');
    for (size_t i = 0; i < message.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(message[i]) ^ static_cast<unsigned char>(key[i % key.size()]);
        ss << std::setw(2) << static_cast<int>(c);
    }
    return ss.str();
}

int main() {
    std::cout << encrypt("HELLO WORLD", "KEY") << std::endl;
    std::cout << encrypt("Attack at dawn!", "secret") << std::endl;
    std::cout << encrypt("OpenAI", "GPT") << std::endl;
    std::cout << encrypt("The quick brown fox jumps over the lazy dog.", "cipher") << std::endl;
    std::cout << encrypt("Lorem ipsum dolor sit amet", "xyz") << std::endl;
    return 0;
}