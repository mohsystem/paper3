#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <filesystem>
#include <string>
#include <random>

int binary_search(const std::vector<int>& arr, int target) {
    int lo = 0, hi = static_cast<int>(arr.size()) - 1;
    while (lo <= hi) {
        int mid = lo + ((hi - lo) / 2);
        int val = arr[mid];
        if (val == target) return mid;
        if (val < target) lo = mid + 1;
        else hi = mid - 1;
    }
    return -1;
}

int main() {
    auto start = std::chrono::high_resolution_clock::now();

    int idx1 = binary_search({1, 3, 5, 7, 9}, 7);
    int idx2 = binary_search({1, 3, 5, 7, 9}, 2);
    int idx3 = binary_search({}, 1);
    int idx4 = binary_search({5}, 5);
    int idx5 = binary_search({-10, -3, 0, 5, 5, 9, 12}, 5);

    std::cout << "Test1 index: " << idx1 << "\n";
    std::cout << "Test2 index: " << idx2 << "\n";
    std::cout << "Test3 index: " << idx3 << "\n";
    std::cout << "Test4 index: " << idx4 << "\n";
    std::cout << "Test5 index: " << idx5 << "\n";

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
    // Create a unique filename
    std::string filename = "Task87_" + std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count()) + ".tmp";
    std::filesystem::path temp_file = temp_dir / filename;

    std::ofstream ofs(temp_file);
    ofs << "elapsed_nanos=" << elapsed_ns << "\n";
    ofs << "elapsed_millis=" << (elapsed_ns / 1'000'000.0) << "\n";
    ofs.close();

    std::cout << "Wrote timing to: " << temp_file << "\n";
    return 0;
}