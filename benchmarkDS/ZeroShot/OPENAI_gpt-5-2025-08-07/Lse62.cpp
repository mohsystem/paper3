#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <random>

struct Result {
    uint64_t bytes_read;
    uint64_t checksum;
};

// Reads input file, computes checksum (unsigned 64-bit sum of bytes), saves results to output file.
// Output format:
//   bytes_read=<num>
//   checksum=<num>
Result processFile(const std::string& inputPath, const std::string& outputPath) {
    Result res{0u, 0u};

    std::ifstream in(inputPath, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Failed to open input file: " + inputPath);
    }

    std::vector<unsigned char> buf(8192);
    while (in) {
        in.read(reinterpret_cast<char*>(buf.data()), static_cast<std::streamsize>(buf.size()));
        std::streamsize got = in.gcount();
        if (got <= 0) break;
        res.bytes_read += static_cast<uint64_t>(got);
        for (std::streamsize i = 0; i < got; ++i) {
            res.checksum = (res.checksum + static_cast<uint64_t>(buf[static_cast<size_t>(i)])) & 0xFFFFFFFFFFFFFFFFULL;
        }
    }
    if (!in.eof() && in.fail()) {
        throw std::runtime_error("Error while reading input file: " + inputPath);
    }

    // atomic-like replace: write to temp then rename
    std::filesystem::path outPath(outputPath);
    std::filesystem::path dir = outPath.parent_path();
    if (dir.empty()) dir = std::filesystem::current_path();
    std::filesystem::create_directories(dir);

    std::filesystem::path tmp = outPath;
    tmp += ".tmp";

    {
        std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
        if (!out) {
            throw std::runtime_error("Failed to open temp output file: " + tmp.string());
        }
        out << "bytes_read=" << res.bytes_read << "\n";
        out << "checksum=" << res.checksum << "\n";
        out.flush();
        if (!out) {
            throw std::runtime_error("Failed to write to temp output file: " + tmp.string());
        }
    }
    std::error_code ec;
    std::filesystem::rename(tmp, outPath, ec);
    if (ec) {
        // fallback: replace existing
        std::filesystem::remove(outPath, ec);
        std::filesystem::rename(tmp, outPath, ec);
        if (ec) {
            std::filesystem::remove(tmp, ec);
            throw std::runtime_error("Failed to move temp file into place: " + outPath.string());
        }
    }

    return res;
}

static void writeBytes(const std::filesystem::path& p, const std::vector<unsigned char>& data) {
    std::filesystem::create_directories(p.parent_path());
    std::ofstream out(p, std::ios::binary | std::ios::trunc);
    if (!out) throw std::runtime_error("Cannot create file: " + p.string());
    if (!data.empty()) out.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    if (!out) throw std::runtime_error("Write failed: " + p.string());
}

int main() {
    try {
        namespace fs = std::filesystem;
        // Create a unique temp directory
        std::random_device rd;
        std::mt19937_64 gen(rd());
        uint64_t suffix = gen();
        fs::path base = fs::temp_directory_path() / ("lse62_cpp_tests_" + std::to_string(suffix));
        fs::create_directories(base);

        fs::path in1 = base / "in1_empty.bin";
        fs::path in2 = base / "in2_hello.txt";
        fs::path in3 = base / "in3_zeros.bin";
        fs::path in4 = base / "in4_pattern.bin";
        fs::path in5 = base / "in5_mixed.bin";

        writeBytes(in1, {});
        writeBytes(in2, std::vector<unsigned char>{'h','e','l','l','o'});
        writeBytes(in3, std::vector<unsigned char>(2048, 0u));
        std::vector<unsigned char> pattern(200000);
        for (size_t i = 0; i < pattern.size(); ++i) pattern[i] = static_cast<unsigned char>(i & 0xFFu);
        writeBytes(in4, pattern);
        writeBytes(in5, std::vector<unsigned char>{0xFFu, 0x01u, 0x02u, 0x7Fu, 0x80u, 0x55u, 0x00u, 0xAAu});

        fs::path out1 = base / "out1.txt";
        fs::path out2 = base / "out2.txt";
        fs::path out3 = base / "out3.txt";
        fs::path out4 = base / "out4.txt";
        fs::path out5 = base / "out5.txt";

        Result r1 = processFile(in1.string(), out1.string());
        Result r2 = processFile(in2.string(), out2.string());
        Result r3 = processFile(in3.string(), out3.string());
        Result r4 = processFile(in4.string(), out4.string());
        Result r5 = processFile(in5.string(), out5.string());

        std::cout << "Test1 bytes=" << r1.bytes_read << " checksum=" << r1.checksum << " -> " << out1 << "\n";
        std::cout << "Test2 bytes=" << r2.bytes_read << " checksum=" << r2.checksum << " -> " << out2 << "\n";
        std::cout << "Test3 bytes=" << r3.bytes_read << " checksum=" << r3.checksum << " -> " << out3 << "\n";
        std::cout << "Test4 bytes=" << r4.bytes_read << " checksum=" << r4.checksum << " -> " << out4 << "\n";
        std::cout << "Test5 bytes=" << r5.bytes_read << " checksum=" << r5.checksum << " -> " << out5 << "\n";
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}