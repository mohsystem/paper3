#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <filesystem>
#include <sstream>
#include <cstring>

#ifdef _WIN32
  #include <io.h>
  #include <fcntl.h>
  #include <share.h>
  #include <sys/stat.h>
  #define CLOSEFD _close
  #define WRITEFD _write
#else
  #include <unistd.h>
  #include <fcntl.h>
  #include <sys/stat.h>
  #define CLOSEFD ::close
  #define WRITEFD ::write
#endif

int binary_search(const std::vector<int>& arr, int target) {
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

std::string write_execution_time_to_temp(long long nanos) {
    namespace fs = std::filesystem;
    fs::path tempdir = fs::temp_directory_path();
    std::string tmpl = (tempdir / "task87_bsearch_XXXXXX").string();
    std::string pathStr;

#ifdef _WIN32
    // Attempt several times to avoid races between name generation and open
    const int attempts = 10;
    for (int i = 0; i < attempts; ++i) {
        std::string buf = tmpl;
        errno_t err = _mktemp_s(buf.data(), buf.size() + 1);
        if (err != 0) continue;

        int fd;
        // _O_BINARY for Windows, create exclusively, owner read/write
        if (_sopen_s(&fd, buf.c_str(), _O_WRONLY | _O_CREAT | _O_EXCL | _O_BINARY, _SH_DENYRW, _S_IREAD | _S_IWRITE) == 0) {
            std::string content = std::to_string(nanos) + "\n";
            if (WRITEFD(fd, content.data(), static_cast<unsigned int>(content.size())) < 0) {
                CLOSEFD(fd);
                throw std::runtime_error("Failed to write to temp file");
            }
            CLOSEFD(fd);
            pathStr = buf;
            break;
        }
    }
    if (pathStr.empty()) {
        throw std::runtime_error("Failed to create secure temp file");
    }
#else
    std::vector<char> buf(tmpl.begin(), tmpl.end());
    buf.push_back('\0');
    int fd = mkstemp(buf.data());
    if (fd == -1) {
        throw std::runtime_error("mkstemp failed");
    }
    // Set permissions to 0600
    fchmod(fd, S_IRUSR | S_IWUSR);
    std::string content = std::to_string(nanos) + "\n";
    if (WRITEFD(fd, content.data(), content.size()) < 0) {
        CLOSEFD(fd);
        throw std::runtime_error("Failed to write to temp file");
    }
    CLOSEFD(fd);
    pathStr = std::string(buf.data());
#endif
    return pathStr;
}

int main() {
    auto start = std::chrono::steady_clock::now();

    std::cout << "Test 1: " << binary_search({1,3,5,7,9}, 7) << "\n";                // expected 3
    std::cout << "Test 2: " << binary_search({}, 1) << "\n";                         // expected -1
    std::cout << "Test 3: " << binary_search({-10,-3,0,0,5,8}, 0) << "\n";           // expected 2 or 3
    std::cout << "Test 4: " << binary_search({1}, 1) << "\n";                        // expected 0
    std::cout << "Test 5: " << binary_search({2,4,6,8,10,12,14,16}, 15) << "\n";     // expected -1

    auto end = std::chrono::steady_clock::now();
    long long elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    try {
        std::string path = write_execution_time_to_temp(elapsed);
        std::cout << "Execution time (ns) written to: " << path << "\n";
    } catch (const std::exception& ex) {
        std::cerr << "Failed to write execution time: " << ex.what() << "\n";
    }

    return 0;
}