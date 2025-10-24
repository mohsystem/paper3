#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <atomic>
#include <thread>
#include <mutex>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <random>

namespace fs = std::filesystem;

// FNV-1a 64-bit streaming hash
static std::string fnv1a64_hex(const fs::path& p) {
    const uint64_t FNV_OFFSET_BASIS = 0xcbf29ce484222325ULL;
    const uint64_t FNV_PRIME = 0x100000001b3ULL;
    uint64_t h = FNV_OFFSET_BASIS;

    std::ifstream in(p, std::ios::binary);
    if (!in) throw std::runtime_error("Failed to open file: " + p.string());
    const size_t BUFSZ = 64 * 1024;
    std::vector<char> buf(BUFSZ);
    while (in) {
        in.read(buf.data(), BUFSZ);
        std::streamsize got = in.gcount();
        for (std::streamsize i = 0; i < got; ++i) {
            h ^= static_cast<unsigned char>(buf[i]);
            h *= FNV_PRIME;
        }
    }
    std::ostringstream oss;
    oss << std::hex << std::setfill('0') << std::setw(16) << (h & 0xFFFFFFFFFFFFFFFFULL);
    return oss.str();
}

// List regular files in directory (non-recursive)
static std::vector<fs::path> list_regular_files(const fs::path& dir) {
    std::vector<fs::path> out;
    if (!fs::exists(dir) || !fs::is_directory(dir)) return out;
    for (auto const& entry : fs::directory_iterator(dir)) {
        std::error_code ec;
        if (fs::is_regular_file(entry.status(ec)) && !ec) {
            out.push_back(entry.path());
        }
    }
    return out;
}

// Core concurrent processing: returns map filename -> hash
std::map<std::string, std::string> process_directory(const std::string& dirPath, size_t threadCount) {
    if (threadCount == 0) throw std::invalid_argument("threadCount must be > 0");
    fs::path dir = fs::absolute(fs::path(dirPath));
    std::vector<fs::path> files = list_regular_files(dir);

    std::atomic<size_t> index{0};
    std::map<std::string, std::string> results;
    std::mutex results_mtx;

    size_t workers = std::min(threadCount, std::max<size_t>(1, files.size()));
    std::vector<std::thread> threads;
    threads.reserve(workers);

    for (size_t t = 0; t < workers; ++t) {
        threads.emplace_back([&]() {
            while (true) {
                size_t i = index.fetch_add(1);
                if (i >= files.size()) break;
                try {
                    std::string h = fnv1a64_hex(files[i]);
                    std::lock_guard<std::mutex> lg(results_mtx);
                    results[files[i].filename().string()] = h;
                } catch (const std::exception& ex) {
                    std::cerr << "Error: " << ex.what() << "\n";
                }
            }
        });
    }
    for (auto& th : threads) th.join();
    return results;
}

// Helpers for tests
static void write_file(const fs::path& p, uint64_t sizeBytes, unsigned char seed) {
    fs::create_directories(p.parent_path());
    std::ofstream out(p, std::ios::binary | std::ios::trunc);
    if (!out) throw std::runtime_error("Failed to create file: " + p.string());
    const size_t CHUNK = 8192;
    std::vector<unsigned char> buf(CHUNK, seed);
    uint64_t remaining = sizeBytes;
    unsigned char s = seed;
    while (remaining > 0) {
        size_t toWrite = static_cast<size_t>(std::min<uint64_t>(remaining, buf.size()));
        out.write(reinterpret_cast<const char*>(buf.data()), toWrite);
        remaining -= toWrite;
        s = static_cast<unsigned char>(s + 1);
        if (toWrite == buf.size()) std::fill(buf.begin(), buf.end(), s);
    }
}

static fs::path create_test_dir(const std::string& base) {
    fs::path tmp = fs::temp_directory_path();
    fs::path dir = tmp / (base + "-" + std::to_string(std::random_device{}()));
    fs::create_directories(dir);
    return dir;
}

static void print_results(const std::string& title, const std::map<std::string, std::string>& res) {
    std::cout << "=== " << title << " ===\n";
    for (auto const& kv : res) {
        std::cout << kv.first << " -> " << kv.second << "\n";
    }
    std::cout << "\n";
}

int main() {
    try {
        // Test 1: 3 small files, 1 thread
        auto dir1 = create_test_dir("task0-tc1");
        write_file(dir1 / "a.txt", 1024, 1);
        write_file(dir1 / "b.txt", 2048, 2);
        write_file(dir1 / "c.txt", 4096, 3);
        auto r1 = process_directory(dir1.string(), 1);
        print_results("Test 1 (3 files, 1 thread)", r1);

        // Test 2: 10 varied files, 2 threads
        auto dir2 = create_test_dir("task0-tc2");
        for (int i = 0; i < 10; ++i) {
            write_file(dir2 / ("file" + std::to_string(i) + ".bin"), (i + 1) * 1500, static_cast<unsigned char>(10 + i));
        }
        auto r2 = process_directory(dir2.string(), 2);
        print_results("Test 2 (10 files, 2 threads)", r2);

        // Test 3: 5 files incl empty, 4 threads
        auto dir3 = create_test_dir("task0-tc3");
        write_file(dir3 / "empty.dat", 0, 0);
        for (int i = 0; i < 4; ++i) {
            write_file(dir3 / ("d" + std::to_string(i) + ".dat"), (i + 1) * 8192, static_cast<unsigned char>(20 + i));
        }
        auto r3 = process_directory(dir3.string(), 4);
        print_results("Test 3 (5 files incl empty, 4 threads)", r3);

        // Test 4: empty directory, 3 threads
        auto dir4 = create_test_dir("task0-tc4");
        auto r4 = process_directory(dir4.string(), 3);
        print_results("Test 4 (0 files, 3 threads)", r4);

        // Test 5: 20 larger files, 8 threads
        auto dir5 = create_test_dir("task0-tc5");
        std::mt19937_64 rng(std::random_device{}());
        std::uniform_int_distribution<int> dist(0, 50 * 1024);
        for (int i = 0; i < 20; ++i) {
            uint64_t size = 200 * 1024 + static_cast<uint64_t>(dist(rng));
            std::ostringstream name;
            name << "large_" << std::setw(2) << std::setfill('0') << i << ".bin";
            write_file(dir5 / name.str(), size, static_cast<unsigned char>(30 + i));
        }
        auto r5 = process_directory(dir5.string(), 8);
        print_results("Test 5 (20 larger files, 8 threads)", r5);
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}