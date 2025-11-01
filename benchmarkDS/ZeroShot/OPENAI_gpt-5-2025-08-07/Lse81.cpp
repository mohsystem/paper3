#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <stdexcept>

using Buffer = std::vector<uint8_t>;

// Allocates a buffer of 1024 bytes
Buffer allocateBuffer() {
    return Buffer(1024u, 0u);
}

// Reads up to 10 bytes from the provided std::istream into buf
// Returns number of bytes read
std::size_t readTenCharsToBuffer(std::istream& in, Buffer& buf) {
    if (!in.good() && !in.eof()) {
        throw std::runtime_error("Input stream is in a bad state");
    }
    if (buf.size() < 10) {
        throw std::invalid_argument("Buffer must be at least 10 bytes long");
    }

    std::size_t count = 0;
    while (count < 10) {
        int c = in.get();
        if (c == EOF) {
            break;
        }
        buf[count++] = static_cast<uint8_t>(c & 0xFF);
    }
    return count;
}

int main() {
    std::string tests[5] = {
        "HelloWorld!",     // >10
        "Short",           // <10
        "1234567890ABC",   // >10
        "",                // empty
        "ExactTen!!"       // exactly 10
    };

    for (int i = 0; i < 5; ++i) {
        Buffer buf = allocateBuffer();
        std::istringstream iss(tests[i]);
        std::size_t count = 0;
        try {
            count = readTenCharsToBuffer(iss, buf);
        } catch (const std::exception& e) {
            std::cout << "Test " << (i + 1) << " error: " << e.what() << "\n";
            continue;
        }
        std::string out(reinterpret_cast<const char*>(buf.data()), count);
        std::cout << "Test " << (i + 1) << ": readCount=" << count << ", data=\"" << out << "\"\n";
    }

    return 0;
}