#include <iostream>
#include <vector>
#include <string>

// Allocates a buffer of exactly 1024 bytes
std::vector<char> allocate_buffer(std::size_t requested_size) {
    const std::size_t FIXED_SIZE = 1024;
    return std::vector<char>(FIXED_SIZE, '\0');
}

// Reads up to 10 bytes from the provided input stream into buf[0..n-1]
// Returns the number of bytes read (0..10). On invalid buffer, returns 0.
std::size_t read_ten_into(std::istream& in, std::vector<char>& buf) {
    if (!in.good() || buf.size() < 10) {
        return 0;
    }
    in.read(buf.data(), 10);
    std::size_t n = static_cast<std::size_t>(in.gcount());
    if (buf.size() > n) {
        buf[n] = '\0'; // Null-terminate for convenience when printing
    }
    return n;
}

int main() {
    for (int i = 1; i <= 5; ++i) {
        std::cout << "Test " << i << ": Please type at least 10 characters and press Enter:\n";
        std::cout.flush();
        auto buffer = allocate_buffer(1024);
        std::size_t n = read_ten_into(std::cin, buffer);
        std::cout << "Read " << n << " byte(s): ";
        std::cout.write(buffer.data(), static_cast<std::streamsize>(n));
        std::cout << "\n";
    }
    return 0;
}