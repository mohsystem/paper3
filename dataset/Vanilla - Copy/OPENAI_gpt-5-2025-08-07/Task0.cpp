#include <bits/stdc++.h>
#include <filesystem>
#include <fstream>
#include <random>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace fs = std::filesystem;

std::unordered_map<std::string, unsigned long long>
process_directory(const std::string& dir_path, int thread_count) {
    if (!fs::is_directory(dir_path)) {
        throw std::runtime_error("Not a directory: " + dir_path);
    }

    std::queue<fs::path> q;
    for (auto& p : fs::directory_iterator(dir_path)) {
        if (fs::is_regular_file(p.path())) q.push(p.path());
    }

    std::mutex q_mtx;
    std::unordered_map<std::string, unsigned long long> results;
    std::mutex res_mtx;

    auto worker = [&]() {
        while (true) {
            fs::path p;
            {
                std::lock_guard<std::mutex> lk(q_mtx);
                if (q.empty()) break;
                p = q.front();
                q.pop();
            }
            unsigned long long sum = 0;
            std::ifstream in(p, std::ios::binary);
            if (!in.good()) {
                sum = static_cast<unsigned long long>(-1);
            } else {
                std::vector<char> buf(8192);
                while (in) {
                    in.read(buf.data(), buf.size());
                    std::streamsize r = in.gcount();
                    for (std::streamsize i = 0; i < r; ++i) {
                        sum += static_cast<unsigned char>(buf[static_cast<size_t>(i)]);
                    }
                }
            }
            {
                std::lock_guard<std::mutex> lk(res_mtx);
                results[p.filename().string()] = sum;
            }
        }
    };

    int n = std::max(1, thread_count);
    std::vector<std::thread> threads;
    threads.reserve(static_cast<size_t>(n));
    for (int i = 0; i < n; ++i) threads.emplace_back(worker);
    for (auto& t : threads) t.join();

    return results;
}

static fs::path create_test_dir(const std::string& prefix, int files, int minKB = 32, int maxKB = 128) {
    fs::path dir = fs::temp_directory_path() / fs::path(prefix + std::to_string(std::random_device{}()));
    fs::create_directories(dir);
    std::mt19937_64 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(minKB, maxKB);
    std::uniform_int_distribution<int> byteDist(0, 255);
    for (int i = 0; i < files; ++i) {
        int sizeKB = dist(rng);
        fs::path file = dir / fs::path("file_" + std::to_string(i) + ".dat");
        std::ofstream out(file, std::ios::binary);
        int bytesToWrite = sizeKB * 1024;
        std::vector<char> buf(8192);
        while (bytesToWrite > 0) {
            int chunk = std::min(bytesToWrite, static_cast<int>(buf.size()));
            for (int j = 0; j < chunk; ++j) buf[static_cast<size_t>(j)] = static_cast<char>(byteDist(rng));
            out.write(buf.data(), chunk);
            bytesToWrite -= chunk;
        }
    }
    return dir;
}

static void run_test_case(const std::string& name, int files, int threads) {
    auto dir = create_test_dir(name + "_", files);
    std::cout << "Test " << name << " - dir: " << dir.string() << " threads: " << threads << "\n";
    auto res = process_directory(dir.string(), threads);
    std::vector<std::string> keys;
    keys.reserve(res.size());
    for (auto& kv : res) keys.push_back(kv.first);
    std::sort(keys.begin(), keys.end());
    for (auto& k : keys) {
        std::cout << "  " << k << " -> checksum=" << res[k] << "\n";
    }
    std::cout << "\n";
}

int main() {
    run_test_case("case1", 5, 1);
    run_test_case("case2", 6, 2);
    run_test_case("case3", 7, 3);
    run_test_case("case4", 8, 4);
    run_test_case("case5", 9, 5);
    return 0;
}