#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>

struct Result {
    uint64_t bytesRead;
    uint32_t checksum;
};

Result processFile(const std::string& inputPath, const std::string& outputPath) {
    std::ifstream in(inputPath, std::ios::binary);
    Result res{0, 0};
    if (!in) {
        // Still write output file indicating 0 bytes and checksum 0
        std::ofstream out(outputPath);
        out << "bytes=" << res.bytesRead << "\nchecksum=" << res.checksum << "\n";
        return res;
    }
    in.seekg(0, std::ios::end);
    std::streampos sz = in.tellg();
    in.seekg(0, std::ios::beg);

    if (sz > 0) {
        std::vector<unsigned char> buffer(static_cast<size_t>(sz));
        in.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
        res.bytesRead = static_cast<uint64_t>(buffer.size());
        uint32_t sum = 0;
        for (unsigned char b : buffer) {
            sum += static_cast<uint32_t>(b);
        }
        res.checksum = sum; // wraps modulo 2^32 automatically
    } else {
        res.bytesRead = 0;
        res.checksum = 0;
    }
    std::ofstream out(outputPath, std::ios::binary | std::ios::trunc);
    out << "bytes=" << res.bytesRead << "\nchecksum=" << res.checksum << "\n";
    return res;
}

static void writeBytes(const std::string& path, const std::vector<unsigned char>& data) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out.write(reinterpret_cast<const char*>(data.data()), data.size());
}

int main() {
    // Test case 1: Empty file
    std::string in1 = "cpp_in1.bin", out1 = "cpp_out1.txt";
    writeBytes(in1, {});
    Result r1 = processFile(in1, out1);
    std::cout << "Test1 -> bytes=" << r1.bytesRead << " checksum=" << r1.checksum << "\n";

    // Test case 2: "Hello, World!"
    std::string in2 = "cpp_in2.bin", out2 = "cpp_out2.txt";
    std::vector<unsigned char> v2({'H','e','l','l','o',',',' ','W','o','r','l','d','!'});
    writeBytes(in2, v2);
    Result r2 = processFile(in2, out2);
    std::cout << "Test2 -> bytes=" << r2.bytesRead << " checksum=" << r2.checksum << "\n";

    // Test case 3: bytes 1..100
    std::string in3 = "cpp_in3.bin", out3 = "cpp_out3.txt";
    std::vector<unsigned char> v3;
    for (int i = 1; i <= 100; ++i) v3.push_back(static_cast<unsigned char>(i));
    writeBytes(in3, v3);
    Result r3 = processFile(in3, out3);
    std::cout << "Test3 -> bytes=" << r3.bytesRead << " checksum=" << r3.checksum << "\n";

    // Test case 4: 10 zero bytes
    std::string in4 = "cpp_in4.bin", out4 = "cpp_out4.txt";
    std::vector<unsigned char> v4(10, 0);
    writeBytes(in4, v4);
    Result r4 = processFile(in4, out4);
    std::cout << "Test4 -> bytes=" << r4.bytesRead << " checksum=" << r4.checksum << "\n";

    // Test case 5: "abc" repeated 1000 times
    std::string in5 = "cpp_in5.bin", out5 = "cpp_out5.txt";
    std::vector<unsigned char> v5;
    v5.reserve(3000);
    for (int i = 0; i < 1000; ++i) { v5.push_back('a'); v5.push_back('b'); v5.push_back('c'); }
    writeBytes(in5, v5);
    Result r5 = processFile(in5, out5);
    std::cout << "Test5 -> bytes=" << r5.bytesRead << " checksum=" << r5.checksum << "\n";

    return 0;
}