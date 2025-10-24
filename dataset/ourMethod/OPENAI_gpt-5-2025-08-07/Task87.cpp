#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

static int binary_search(const std::vector<int>& arr, int target) {
    int left = 0;
    int right = static_cast<int>(arr.size()) - 1;
    while (left <= right) {
        int mid = left + ((right - left) >> 1);
        int val = arr[mid];
        if (val == target) return mid;
        if (val < target) left = mid + 1;
        else right = mid - 1;
    }
    return -1;
}

static std::string sanitize_label(const std::string& label) {
    std::string s = label.substr(0, 128);
    for (char& c : s) {
        if (!((c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') ||
              c == ' ' || c == '-' || c == '_')) {
            c = '_';
        }
    }
    return s;
}

static bool robust_write(int fd, const char* buf, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t w = ::write(fd, buf + total, len - total);
        if (w < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        total += static_cast<size_t>(w);
    }
    return true;
}

static std::string write_execution_time_to_temp_file(long long nanos, const std::string& label) {
    std::string content = "label=" + sanitize_label(label) + "\nexecution_time_ns=" + std::to_string(nanos) + "\n";

    char tmpl[] = "/tmp/task87_XXXXXX";
    int fd = ::mkstemp(tmpl);
    if (fd < 0) {
        return std::string();
    }

    // Restrict permissions (best effort)
    ::fchmod(fd, 0600);

    // Truncate and write
    if (::ftruncate(fd, 0) != 0) {
        ::close(fd);
        return std::string();
    }

    if (!robust_write(fd, content.c_str(), content.size())) {
        ::close(fd);
        return std::string();
    }

    ::fsync(fd);
    ::close(fd);

    return std::string(tmpl);
}

int main() {
    std::vector<std::vector<int>> arrays = {
        {1, 3, 5, 7, 9},
        {2, 4, 6, 8, 10},
        {0},
        {-10, -5, 0, 5, 10, 15},
        {1,2,3,4,5,6,7,8,9,10}
    };
    std::vector<int> targets = {7, 1, 0, 15, 11};

    auto start = std::chrono::steady_clock::now();
    std::vector<int> results;
    results.reserve(targets.size());
    for (size_t i = 0; i < targets.size(); ++i) {
        results.push_back(binary_search(arrays[i], targets[i]));
    }
    auto end = std::chrono::steady_clock::now();
    long long total_nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    std::string path = write_execution_time_to_temp_file(total_nanos, "total_run");
    std::cout << "Binary search results (indices): [";
    for (size_t i = 0; i < results.size(); ++i) {
        std::cout << results[i] << (i + 1 < results.size() ? ", " : "");
    }
    std::cout << "]\n";
    std::cout << "Execution time written to: " << path << "\n";
    return 0;
}