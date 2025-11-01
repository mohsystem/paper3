// Chain-of-Through process:
// 1) Problem understanding: Read file into buffered chunks, compute 32-bit checksum (sum of bytes mod 2^32), empty if checksum==0, save metrics.
// 2) Security requirements: Streamed I/O; handle errors; avoid integer overflow by using modular arithmetic.
// 3) Secure coding generation: Use RAII for file streams; validate I/O states; no raw pointers for buffers.
// 4) Code review: Verify unsigned handling, correct loop termination, and error-safe writes.
// 5) Secure code output: Final code reliably computes and persists results.

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
    const size_t BUF_SIZE = 8192;
    std::ifstream in(inputPath, std::ios::binary);
    Result res{0u, 0u};

    if (!in) {
        // Write zeros to output to indicate failure while keeping contract simple
        std::ofstream out_fail(outputPath, std::ios::binary);
        if (out_fail) {
            out_fail << "bytesRead=" << 0 << "\nchecksum=" << 0 << "\n";
        }
        return res;
    }

    std::vector<char> buf(BUF_SIZE);
    while (in) {
        in.read(buf.data(), static_cast<std::streamsize>(buf.size()));
        std::streamsize n = in.gcount();
        if (n <= 0) break;
        res.bytesRead += static_cast<uint64_t>(n);
        for (std::streamsize i = 0; i < n; ++i) {
            res.checksum = static_cast<uint32_t>(res.checksum + static_cast<unsigned char>(buf[static_cast<size_t>(i)]));
        }
    }

    // checksum == 0 => buffer considered empty (per spec)
    std::ofstream out(outputPath, std::ios::binary | std::ios::trunc);
    if (out) {
        out << "bytesRead=" << res.bytesRead << "\nchecksum=" << res.checksum << "\n";
    }
    return res;
}

static void writeFile(const std::string& path, const std::vector<unsigned char>& data) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) return;
    if (!data.empty())
        out.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
}

int main() {
    // 5 test cases
    const std::string inFiles[5] = {
        "cpp_in_1_empty.bin",
        "cpp_in_2_hello.txt",
        "cpp_in_3_zeros.bin",
        "cpp_in_4_seq.bin",
        "cpp_in_5_large.bin"
    };
    const std::string outFiles[5] = {
        "cpp_out_1.txt",
        "cpp_out_2.txt",
        "cpp_out_3.txt",
        "cpp_out_4.txt",
        "cpp_out_5.txt"
    };

    // Prepare inputs
    writeFile(inFiles[0], {});
    writeFile(inFiles[1], std::vector<unsigned char>{'H','e','l','l','o',',',' ','w','o','r','l','d','!'});
    writeFile(inFiles[2], std::vector<unsigned char>(1024, 0));
    {
        std::vector<unsigned char> seq(4096);
        for (size_t i = 0; i < seq.size(); ++i) seq[i] = static_cast<unsigned char>(i & 0xFF);
        writeFile(inFiles[3], seq);
    }
    {
        std::vector<unsigned char> large(10000);
        for (size_t i = 0; i < large.size(); ++i) large[i] = static_cast<unsigned char>('A' + (i % 26));
        writeFile(inFiles[4], large);
    }

    for (int i = 0; i < 5; ++i) {
        Result r = processFile(inFiles[i], outFiles[i]);
        std::cout << "Test " << (i + 1) << " -> bytesRead=" << r.bytesRead << ", checksum=" << r.checksum << "\n";
    }
    return 0;
}