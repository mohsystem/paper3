#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <string>

struct Result {
    uint64_t bytes_read;
    uint32_t checksum;
};

static void write_bytes(const std::string& path, const std::vector<uint8_t>& data) {
    std::ofstream ofs(path, std::ios::binary);
    ofs.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
}

Result processFile(const std::string& inputPath, const std::string& outputPath) {
    std::ifstream ifs(inputPath, std::ios::binary);
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    uint64_t bytes = data.size();
    uint32_t checksum = 0;
    for (uint8_t b : data) {
        checksum += b;
    }
    // Ensure 32-bit wrap
    checksum &= 0xFFFFFFFFu;

    std::ofstream ofs(outputPath, std::ios::binary);
    ofs << "bytes=" << bytes << "\nchecksum=" << checksum << "\n";
    return Result{bytes, checksum};
}

int main() {
    // Test case 1: Empty file
    std::string in1 = "cpp_in1.bin", out1 = "cpp_out1.txt";
    write_bytes(in1, {});
    Result r1 = processFile(in1, out1);
    std::cout << "Test 1 -> bytes=" << r1.bytes_read << ", checksum=" << r1.checksum << ", saved: " << out1 << "\n";

    // Test case 2: "Hello, world!"
    std::string in2 = "cpp_in2.bin", out2 = "cpp_out2.txt";
    std::vector<uint8_t> data2 = {'H','e','l','l','o',',',' ','w','o','r','l','d','!'};
    write_bytes(in2, data2);
    Result r2 = processFile(in2, out2);
    std::cout << "Test 2 -> bytes=" << r2.bytes_read << ", checksum=" << r2.checksum << ", saved: " << out2 << "\n";

    // Test case 3: 1024 bytes 0..255 repeating
    std::string in3 = "cpp_in3.bin", out3 = "cpp_out3.txt";
    std::vector<uint8_t> data3(1024);
    for (size_t i = 0; i < data3.size(); ++i) data3[i] = static_cast<uint8_t>(i & 0xFF);
    write_bytes(in3, data3);
    Result r3 = processFile(in3, out3);
    std::cout << "Test 3 -> bytes=" << r3.bytes_read << ", checksum=" << r3.checksum << ", saved: " << out3 << "\n";

    // Test case 4: Non-ASCII pattern repeated
    std::string in4 = "cpp_in4.bin", out4 = "cpp_out4.txt";
    std::vector<uint8_t> pattern4 = {0x00, 0xFF, 0x10, 0x7F, 0x80};
    std::vector<uint8_t> data4(100);
    for (size_t i = 0; i < data4.size(); ++i) data4[i] = pattern4[i % pattern4.size()];
    write_bytes(in4, data4);
    Result r4 = processFile(in4, out4);
    std::cout << "Test 4 -> bytes=" << r4.bytes_read << ", checksum=" << r4.checksum << ", saved: " << out4 << "\n";

    // Test case 5: 4096 bytes deterministic generator
    std::string in5 = "cpp_in5.bin", out5 = "cpp_out5.txt";
    std::vector<uint8_t> data5(4096);
    for (size_t i = 0; i < data5.size(); ++i) data5[i] = static_cast<uint8_t>((i * 31 + 7) & 0xFF);
    write_bytes(in5, data5);
    Result r5 = processFile(in5, out5);
    std::cout << "Test 5 -> bytes=" << r5.bytes_read << ", checksum=" << r5.checksum << ", saved: " << out5 << "\n";

    return 0;
}