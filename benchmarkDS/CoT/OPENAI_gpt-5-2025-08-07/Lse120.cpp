#include <iostream>
#include <vector>
#include <cstring>

std::vector<uint8_t> copyChunk(const std::vector<uint8_t>& input) {
    size_t n = input.size();
    uint8_t* chunk1 = n ? new uint8_t[n] : nullptr;
    if (n) std::memcpy(chunk1, input.data(), n);

    uint8_t* chunk2 = n ? new uint8_t[n] : nullptr;
    if (n) std::memcpy(chunk2, chunk1, n);

    std::vector<uint8_t> out;
    out.reserve(n);
    if (n) out.insert(out.end(), chunk2, chunk2 + n);

    delete[] chunk1;
    delete[] chunk2;
    return out;
}

void printBytes(const std::vector<uint8_t>& data) {
    std::cout << "[";
    for (size_t i = 0; i < data.size(); ++i) {
        std::cout << static_cast<unsigned int>(data[i]);
        if (i + 1 < data.size()) std::cout << ", ";
    }
    std::cout << "]\n";
}

int main() {
    std::vector<std::vector<uint8_t>> tests;

    std::string s1 = "Hello, World!";
    tests.emplace_back(s1.begin(), s1.end());

    std::string s2 = "foo";
    tests.emplace_back(s2.begin(), s2.end());

    std::string s3 = "The quick brown fox";
    tests.emplace_back(s3.begin(), s3.end());

    tests.emplace_back(); // empty

    tests.push_back(std::vector<uint8_t>{0, 1, 2, 3, 127, 255});

    for (const auto& t : tests) {
        auto out = copyChunk(t);
        printBytes(out);
    }
    return 0;
}