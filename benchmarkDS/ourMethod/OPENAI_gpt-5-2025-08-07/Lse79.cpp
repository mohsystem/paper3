#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <limits>

static constexpr std::size_t BUF_SIZE = 1024;
static constexpr std::size_t READ_100 = 100;

static std::size_t readAtMost(std::istream& in, char* dest, std::size_t destSize, std::size_t maxToRead) {
    if (dest == nullptr || destSize == 0) return 0;
    std::size_t limit = maxToRead;
    if (limit > destSize) limit = destSize;

    std::size_t total = 0;
    while (total < limit && in.good()) {
        std::size_t toRead = limit - total;
        // cap to streamsize to avoid overflow
        if (toRead > static_cast<std::size_t>((std::numeric_limits<std::streamsize>::max)())) {
            toRead = static_cast<std::size_t>((std::numeric_limits<std::streamsize>::max)());
        }
        in.read(dest + static_cast<std::streamsize>(total), static_cast<std::streamsize>(toRead));
        std::streamsize got = in.gcount();
        if (got <= 0) break;
        total += static_cast<std::size_t>(got);
    }
    return total;
}

std::pair<std::string, std::string> allocateAndCopy100Twice(std::istream& in) {
    std::vector<char> buf(BUF_SIZE, 0);
    std::vector<char> buf1(BUF_SIZE, 0);

    std::size_t n1 = readAtMost(in, buf.data(), buf.size(), READ_100);
    std::size_t n2 = readAtMost(in, buf1.data(), buf1.size(), READ_100);

    std::string s1(buf.data(), buf.data() + n1);
    std::string s2(buf1.data(), buf1.data() + n2);
    return {s1, s2};
}

std::string readUpTo1024(std::istream& in) {
    std::vector<char> buf(BUF_SIZE, 0);
    std::size_t n = readAtMost(in, buf.data(), buf.size(), BUF_SIZE);
    return std::string(buf.data(), buf.data() + n);
}

int main() {
    // Test case 1: Less than 100 bytes
    {
        std::string t1 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
        std::istringstream in1(t1);
        auto res1 = allocateAndCopy100Twice(in1);
        std::cout << "T1 buf: " << res1.first << "\n";
        std::cout << "T1 buf1: " << res1.second << "\n";
    }

    // Test case 2: More than 100 bytes (150 'x')
    {
        std::string t2(150, 'x');
        std::istringstream in2(t2);
        auto res2 = allocateAndCopy100Twice(in2);
        std::cout << "T2 buf len: " << res2.first.size() << "\n";
        std::cout << "T2 buf1 len: " << res2.second.size() << "\n";
    }

    // Test case 3: Empty input
    {
        std::string t3;
        std::istringstream in3(t3);
        auto res3 = allocateAndCopy100Twice(in3);
        std::cout << "T3 buf: '" << res3.first << "'\n";
        std::cout << "T3 buf1: '" << res3.second << "'\n";
    }

    // Test case 4: readUpTo1024 with 300 bytes
    {
        std::string t4(300, 'y');
        std::istringstream in4(t4);
        std::string s4 = readUpTo1024(in4);
        std::cout << "T4 len: " << s4.size() << "\n";
    }

    // Test case 5: Unicode-like (UTF-8) content (will be treated as bytes in std::string)
    {
        std::string t5 = "Hello \xE4\xBD\xA0\xE5\xA5\xBD\xE4\xB8\x96\xE7\x95\x8C \xF0\x9F\x99\x82\xF0\x9F\x9A\x80";
        t5 = t5 + t5 + t5 + t5 + t5; // repeat
        std::istringstream in5(t5);
        auto res5 = allocateAndCopy100Twice(in5);
        std::cout << "T5 buf: " << res5.first << "\n";
        std::cout << "T5 buf1: " << res5.second << "\n";
    }

    return 0;
}