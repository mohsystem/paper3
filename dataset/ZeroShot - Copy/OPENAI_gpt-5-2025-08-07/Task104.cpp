#include <iostream>
#include <string>
#include <array>
#include <cstring>

static constexpr std::size_t BUFFER_SIZE = 16;

std::string handleInput(const std::string& input) {
    std::array<char, BUFFER_SIZE> buf{};
    std::size_t toCopy = input.size();
    if (toCopy > BUFFER_SIZE - 1) {
        toCopy = BUFFER_SIZE - 1;
    }
    if (toCopy > 0) {
        std::memcpy(buf.data(), input.data(), toCopy);
    }
    buf[toCopy] = '\0';
    return std::string(buf.data());
}

int main() {
    const std::string tests[5] = {
        "Hello",
        "1234567890ABCDEF",
        "",
        "SafeBuffer!",
        "This is a very long string that exceeds the buffer size!"
    };
    for (int i = 0; i < 5; ++i) {
        std::string res = handleInput(tests[i]);
        std::cout << "Test " << (i + 1) << ": " << res << std::endl;
    }
    return 0;
}