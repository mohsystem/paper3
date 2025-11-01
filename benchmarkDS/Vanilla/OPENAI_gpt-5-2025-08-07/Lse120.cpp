#include <iostream>
#include <string>
#include <cstring>

std::string copyAndPrint(const std::string& input) {
    size_t len = input.size();
    char* chunk1 = new char[len + 1];
    std::memcpy(chunk1, input.data(), len);
    chunk1[len] = '\0';

    char* chunk2 = new char[len + 1];
    std::memcpy(chunk2, chunk1, len + 1);

    std::cout << chunk2 << std::endl;

    std::string out(chunk2);
    delete[] chunk1;
    delete[] chunk2;
    return out;
}

int main() {
    std::string tests[5] = {
        "Hello",
        "",
        "Memory test 123",
        "こんにちは",
        "Emoji 😀🚀"
    };
    for (int i = 0; i < 5; ++i) {
        std::string r = copyAndPrint(tests[i]);
        std::cout << "Returned: " << r << std::endl;
    }
    return 0;
}