#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstring>

int binarySearch(const std::vector<int>& arr, int target) {
    if (arr.empty()) return -1;
    int low = 0;
    int high = static_cast<int>(arr.size()) - 1;
    while (low <= high) {
        int mid = low + ((high - low) >> 1);
        int val = arr[mid];
        if (val == target) return mid;
        if (val < target) low = mid + 1;
        else high = mid - 1;
    }
    return -1;
}

std::string measureAndWriteTime(const std::vector<int>& arr, int target) {
    auto start = std::chrono::steady_clock::now();
    int idx = binarySearch(arr, target);
    auto end = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    char tmpl[] = "/tmp/task87_time_XXXXXX";
    int fd = mkstemp(tmpl);
    if (fd == -1) {
        return std::string();
    }

    std::string content = "elapsed_nanos=" + std::to_string(elapsed) + ", index=" + std::to_string(idx) + "\n";
    ssize_t written = write(fd, content.c_str(), content.size());
    (void)written; // suppress unused warning; ignoring partial write for brevity
    close(fd);
    return std::string(tmpl);
}

int main() {
    std::vector<std::vector<int>> tests(5);
    std::vector<int> targets(5);

    tests[0] = {1, 3, 5, 7, 9}; targets[0] = 7;
    tests[1] = {2, 4, 6, 8, 10}; targets[1] = 5;
    tests[2] = {}; targets[2] = 1;
    tests[3] = {-10, -5, 0, 5, 10}; targets[3] = -10;
    tests[4].resize(100000); for (int i = 0; i < 100000; ++i) tests[4][i] = i; targets[4] = 12345;

    for (int i = 0; i < 5; ++i) {
        int idx = binarySearch(tests[i], targets[i]);
        std::string path = measureAndWriteTime(tests[i], targets[i]);
        std::cout << "Test " << (i + 1) << ": target=" << targets[i] << ", index=" << idx << ", timeFile=" << path << "\n";
    }
    return 0;
}