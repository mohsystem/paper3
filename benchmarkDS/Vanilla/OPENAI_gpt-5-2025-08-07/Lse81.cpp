#include <iostream>
#include <vector>
#include <sstream>
#include <string>

class Lse81 {
public:
    static std::vector<char> allocateBuffer() {
        return std::vector<char>(1024);
    }

    static std::size_t read10CharsIntoBuf(std::vector<char>& buf) {
        std::cin.read(buf.data(), 10);
        return static_cast<std::size_t>(std::cin.gcount());
    }
};

int main() {
    std::string tests[5] = {
        "abcdefghij",
        "1234567890",
        "short",
        "tenchars!!",
        "with newline\nmore"
    };

    for (int i = 0; i < 5; ++i) {
        std::istringstream ss(tests[i]);
        std::streambuf* old = std::cin.rdbuf(ss.rdbuf());

        std::vector<char> buf = Lse81::allocateBuffer();
        std::size_t n = Lse81::read10CharsIntoBuf(buf);

        std::cout << "Test" << (i + 1) << ": ";
        std::cout.write(buf.data(), static_cast<std::streamsize>(n));
        std::cout << std::endl;

        std::cin.rdbuf(old);
    }
    return 0;
}