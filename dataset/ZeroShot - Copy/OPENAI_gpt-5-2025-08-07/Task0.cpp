#include <iostream>
#include <filesystem>
#include <vector>
#include <thread>
#include <mutex>
#include <fstream>
#include <sstream>
#include <atomic>
#include <map>

namespace fs = std::filesystem;

// Compute FNV-1a 64-bit
static uint64_t fnv1a_64_stream(std::ifstream &ifs) {
    const uint64_t FNV_OFFSET = 1469598103934665603ULL;
    const uint64_t FNV_PRIME = 1099511628211ULL;
    uint64_t hash = FNV_OFFSET;
    const size_t BUF = 64 * 1024;
    std::vector<char> buffer(BUF);
    while (ifs) {
        ifs.read(buffer.data(), buffer.size());
        std::streamsize n = ifs.gcount();
        for (std::streamsize i = 0; i < n; ++i) {
            hash ^= static_cast<unsigned char>(buffer[static_cast<size_t>(i)]);
            hash *= FNV_PRIME;
        }
    }
    return hash;
}

static std::string to_hex64(uint64_t v) {
    std::ostringstream oss;
    oss.setf(std::ios::hex, std::ios::basefield);
    oss.fill('0');
    oss.width(16);
    oss << std::nouppercase << std::hex << v;
    return oss.str();
}

// Secure, concurrent processing of files in a directory.
// Returns map of absolute path -> hex FNV-1a-64 checksum.
std::map<std::string, std::string> process_directory(const std::string &dirPath, int threads) {
    fs::path dir = fs::absolute(fs::path(dirPath));
    if (!fs::exists(dir) || !fs::is_directory(dir)) {
        throw std::invalid_argument("Not a directory: " + dir.string());
    }
    if (fs::is_symlink(dir)) {
        throw std::invalid_argument("Refusing to process a symlinked directory: " + dir.string());
    }

    std::vector<fs::path> files;
    for (auto it = fs::directory_iterator(dir, fs::directory_options::skip_permission_denied);
         it != fs::directory_iterator(); ++it) {
        std::error_code ec;
        auto st = it->symlink_status(ec);
        if (ec) continue;
        if (fs::is_symlink(st)) continue;
        if (fs::is_regular_file(st)) {
            files.push_back(fs::absolute(it->path()));
        }
    }

    const unsigned hw = std::thread::hardware_concurrency();
    int max_threads = std::max(1, std::min(threads, (hw ? (int)hw : 4) * 4));

    std::map<std::string, std::string> results;
    std::mutex results_mtx;

    std::atomic<size_t> index{0};
    std::vector<std::thread> pool;
    pool.reserve((size_t)max_threads);

    for (int t = 0; t < max_threads; ++t) {
        pool.emplace_back([&]() {
            while (true) {
                size_t i = index.fetch_add(1, std::memory_order_relaxed);
                if (i >= files.size()) break;
                const fs::path &p = files[i];
                std::string hex;
                try {
                    std::ifstream ifs(p.string(), std::ios::binary);
                    if (!ifs) {
                        hex = "ERROR:open";
                    } else {
                        uint64_t h = fnv1a_64_stream(ifs);
                        hex = to_hex64(h);
                    }
                } catch (...) {
                    hex = "ERROR:exception";
                }
                std::lock_guard<std::mutex> lk(results_mtx);
                results[p.string()] = hex;
            }
        });
    }
    for (auto &th : pool) th.join();
    return results;
}

// Test helpers
static void write_file(const fs::path &dir, const std::string &name, const std::string &content) {
    fs::path p = fs::absolute(dir / name);
    if (p.string().find(fs::absolute(dir).string()) != 0) {
        throw std::runtime_error("Invalid file path");
    }
    std::ofstream ofs(p.string(), std::ios::binary);
    ofs.write(content.data(), (std::streamsize)content.size());
}

static fs::path make_temp_dir(const std::string &prefix) {
    fs::path base = fs::temp_directory_path();
    for (int i = 0; i < 1000; ++i) {
        fs::path candidate = base / (prefix + std::to_string(std::rand()));
        std::error_code ec;
        if (fs::create_directory(candidate, ec)) {
            return candidate;
        }
    }
    throw std::runtime_error("Failed to create temp directory");
}

static void populate(const fs::path &dir, int count, const std::string &seed) {
    for (int i = 0; i < count; ++i) {
        std::ostringstream oss;
        oss << "file_" << (i < 10 ? "0" : "") << i << ".dat";
        std::string content = "seed:" + seed + "; index:" + std::to_string(i) + "; data:" + std::string((size_t)((i % 17) + 1), 'X');
        write_file(dir, oss.str(), content);
    }
}

int main() {
    try {
        // 1) Empty directory
        fs::path d1 = make_temp_dir("cfp1_");
        auto r1 = process_directory(d1.string(), 4);
        std::cout << "Test1 files=" << r1.size() << "\n";

        // 2) Small set, 2 threads
        fs::path d2 = make_temp_dir("cfp2_");
        populate(d2, 5, "A");
        auto r2 = process_directory(d2.string(), 2);
        std::cout << "Test2 files=" << r2.size() << "\n";

        // 3) Larger set, 4 threads
        fs::path d3 = make_temp_dir("cfp3_");
        populate(d3, 20, "B");
        auto r3 = process_directory(d3.string(), 4);
        std::cout << "Test3 files=" << r3.size() << "\n";

        // 4) Larger set, 8 threads
        fs::path d4 = make_temp_dir("cfp4_");
        populate(d4, 50, "C");
        auto r4 = process_directory(d4.string(), 8);
        std::cout << "Test4 files=" << r4.size() << "\n";

        // 5) Single-threaded run
        fs::path d5 = make_temp_dir("cfp5_");
        populate(d5, 10, "D");
        auto r5 = process_directory(d5.string(), 1);
        std::cout << "Test5 files=" << r5.size() << "\n";
    } catch (const std::exception &ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}