#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <filesystem>
#include <random>

namespace fs = std::filesystem;
static const std::uint64_t MAX_SIZE = 50ULL * 1024ULL * 1024ULL; // 50 MB

struct Result {
    std::uint64_t bytesRead;
    std::uint32_t checksum;
    bool isEmpty;
};

bool processFile(const std::string& inputPath, const std::string& outputPath, Result& outResult) {
    try {
        fs::path in(inputPath);
        if (!fs::exists(in)) {
            throw std::runtime_error("Input file does not exist: " + inputPath);
        }
        if (fs::is_symlink(in)) {
            throw std::runtime_error("Refusing to read from a symbolic link: " + inputPath);
        }
        if (!fs::is_regular_file(in)) {
            throw std::runtime_error("Input is not a regular file: " + inputPath);
        }
        std::uint64_t size = fs::file_size(in);
        if (size > MAX_SIZE) {
            throw std::runtime_error("File too large");
        }

        std::vector<unsigned char> buffer;
        buffer.resize(static_cast<size_t>(size));
        if (size > 0) {
            std::ifstream ifs(in, std::ios::binary);
            if (!ifs) throw std::runtime_error("Failed to open input for reading");
            ifs.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
            if (!ifs && !ifs.eof()) throw std::runtime_error("Failed to read input file");
        }

        std::uint64_t bytesRead = buffer.size();
        std::uint32_t checksum = 0;
        for (unsigned char b : buffer) {
            checksum = static_cast<std::uint32_t>(checksum + static_cast<std::uint32_t>(b));
        }
        bool isEmpty = (checksum == 0);

        fs::path out(outputPath);
        if (out.has_parent_path()) {
            fs::create_directories(out.parent_path());
        }
        if (fs::exists(out) && fs::is_symlink(out)) {
            throw std::runtime_error("Refusing to write to a symbolic link: " + outputPath);
        }
        std::ofstream ofs(out, std::ios::binary | std::ios::trunc);
        if (!ofs) throw std::runtime_error("Failed to open output for writing");
        ofs << "bytes=" << bytesRead << "\n";
        ofs << "checksum=" << checksum << "\n";
        ofs.flush();

        outResult = { bytesRead, checksum, isEmpty };
        return true;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return false;
    }
}

static void writeBytes(const fs::path& p, const std::vector<unsigned char>& data) {
    if (p.has_parent_path()) fs::create_directories(p.parent_path());
    std::ofstream ofs(p, std::ios::binary | std::ios::trunc);
    ofs.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
}

int main() {
    try {
        fs::path temp = fs::temp_directory_path() / ("lse63_cpp_" + std::to_string(std::random_device{}()));
        fs::create_directories(temp);

        // Test 1: Empty file
        fs::path f1 = temp / "empty.bin";
        writeBytes(f1, {});
        Result r1{};
        processFile(f1.string(), (temp / "out1.txt").string(), r1);
        std::cout << "Test1: bytes=" << r1.bytesRead << ", checksum=" << r1.checksum << ", isEmpty=" << (r1.isEmpty ? "true" : "false") << "\n";

        // Test 2: Small text
        fs::path f2 = temp / "hello.txt";
        {
            const std::string s = "Hello, world!";
            writeBytes(f2, std::vector<unsigned char>(s.begin(), s.end()));
        }
        Result r2{};
        processFile(f2.string(), (temp / "out2.txt").string(), r2);
        std::cout << "Test2: bytes=" << r2.bytesRead << ", checksum=" << r2.checksum << ", isEmpty=" << (r2.isEmpty ? "true" : "false") << "\n";

        // Test 3: 2048 random bytes
        fs::path f3 = temp / "random.bin";
        {
            std::vector<unsigned char> data(2048);
            std::random_device rd;
            for (auto& b : data) b = static_cast<unsigned char>(rd());
            writeBytes(f3, data);
        }
        Result r3{};
        processFile(f3.string(), (temp / "out3.txt").string(), r3);
        std::cout << "Test3: bytes=" << r3.bytesRead << ", checksum=" << r3.checksum << ", isEmpty=" << (r3.isEmpty ? "true" : "false") << "\n";

        // Test 4: 10000 bytes of 0xFF
        fs::path f4 = temp / "ff.bin";
        writeBytes(f4, std::vector<unsigned char>(10000, 0xFF));
        Result r4{};
        processFile(f4.string(), (temp / "out4.txt").string(), r4);
        std::cout << "Test4: bytes=" << r4.bytesRead << ", checksum=" << r4.checksum << ", isEmpty=" << (r4.isEmpty ? "true" : "false") << "\n";

        // Test 5: UTF-8 text repeated
        fs::path f5 = temp / "utf8.txt";
        {
            std::string s;
            for (int i = 0; i < 10; ++i) s += u8"こんにちは世界";
            writeBytes(f5, std::vector<unsigned char>(s.begin(), s.end()));
        }
        Result r5{};
        processFile(f5.string(), (temp / "out5.txt").string(), r5);
        std::cout << "Test5: bytes=" << r5.bytesRead << ", checksum=" << r5.checksum << ", isEmpty=" << (r5.isEmpty ? "true" : "false") << "\n";
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}