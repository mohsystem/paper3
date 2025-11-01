#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <ctime>
#include <iomanip>

struct Result {
    uint64_t bytesRead;
    uint32_t checksum;
    bool isEmpty;
};

Result processFile(const std::string& inputPath, const std::string& outputPath) {
    std::ifstream in(inputPath, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Failed to open input file: " + inputPath);
    }

    std::vector<unsigned char> buffer((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    uint64_t sum = 0;
    for (unsigned char b : buffer) {
        sum += static_cast<uint64_t>(b);
        sum &= 0xFFFFFFFFull;
    }
    uint32_t checksum = static_cast<uint32_t>(sum & 0xFFFFFFFFull);
    bool empty = (checksum == 0u);

    std::ofstream out(outputPath, std::ios::binary | std::ios::trunc);
    if (!out) {
        throw std::runtime_error("Failed to open output file: " + outputPath);
    }
    out << "bytes_read=" << buffer.size() << "\n";
    out << "checksum=" << checksum << "\n";
    out.close();

    Result r{ static_cast<uint64_t>(buffer.size()), checksum, empty };
    return r;
}

int main() {
    try {
        // Create unique base name using time
        std::time_t t = std::time(nullptr);
        std::string base = std::string("lse63_cpp_demo_") + std::to_string(static_cast<long long>(t)) + "_";

        struct Test { std::string name; std::vector<unsigned char> content; };
        std::vector<Test> tests;
        tests.push_back({"empty.bin", {}});
        tests.push_back({"hello.txt", std::vector<unsigned char>({'h','e','l','l','o'})});
        tests.push_back({"repeatA.bin", std::vector<unsigned char>(1024, static_cast<unsigned char>('a'))});
        tests.push_back({"binary.bin", std::vector<unsigned char>({0x01, 0x02, 0xFF, 0x10, 0x20, 0x7F})});
        // UTF-8 for "こんにちは"
        tests.push_back({"unicode.txt", std::vector<unsigned char>({0xE3,0x81,0x93,0xE3,0x82,0x93,0xE3,0x81,0xAB,0xE3,0x81,0xA1,0xE3,0x81,0xAF})});

        for (size_t i = 0; i < tests.size(); ++i) {
            std::string inPath = base + tests[i].name;
            std::string outPath = inPath + ".out.txt";
            // Write input file
            {
                std::ofstream f(inPath, std::ios::binary | std::ios::trunc);
                if (!f) throw std::runtime_error("Failed to create input file: " + inPath);
                if (!tests[i].content.empty()) {
                    f.write(reinterpret_cast<const char*>(tests[i].content.data()), static_cast<std::streamsize>(tests[i].content.size()));
                }
            }

            Result r = processFile(inPath, outPath);
            std::cout << "Test " << (i + 1) << ": " << inPath
                      << " -> bytes=" << r.bytesRead
                      << ", checksum=" << r.checksum
                      << ", empty=" << (r.isEmpty ? "true" : "false")
                      << " | out=" << outPath << "\n";
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}