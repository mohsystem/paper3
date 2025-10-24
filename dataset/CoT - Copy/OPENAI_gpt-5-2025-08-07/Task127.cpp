#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <system_error>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

namespace fs = std::filesystem;

// Step 1: Problem understanding and function definition
// Securely copy a file from a specified base directory to a securely created temporary file.
// Returns the path to the temp file as a std::string, or empty string on failure.
std::string copyFileToTemp(const std::string& baseDir, const std::string& relativeFile) {
    // Step 2: Security requirements - validate inputs
    if (baseDir.empty() || relativeFile.empty()) {
        std::cerr << "Error: invalid parameters." << std::endl;
        return "";
    }

    try {
        std::error_code ec;
        fs::path base(baseDir);
        if (!fs::exists(base, ec) || !fs::is_directory(base, ec)) {
            std::cerr << "Error: base directory not found or not a directory." << std::endl;
            return "";
        }
        fs::path baseReal = fs::canonical(base, ec);
        if (ec) {
            std::cerr << "Error: failed to resolve base directory." << std::endl;
            return "";
        }

        fs::path candidate = baseReal / fs::path(relativeFile);
        if (!fs::exists(candidate, ec)) {
            std::cerr << "Error: source file not found." << std::endl;
            return "";
        }
        fs::path fileReal = fs::canonical(candidate, ec);
        if (ec) {
            std::cerr << "Error: source file not found." << std::endl;
            return "";
        }

        // Ensure within base directory
        auto mismatch = std::mismatch(baseReal.begin(), baseReal.end(), fileReal.begin(), fileReal.end());
        if (mismatch.first != baseReal.end()) {
            std::cerr << "Error: access outside base directory is not allowed." << std::endl;
            return "";
        }

        if (!fs::is_regular_file(fileReal, ec)) {
            std::cerr << "Error: specified path is not a regular file." << std::endl;
            return "";
        }

        // Step 3: Secure coding generation - create secure temp file
        fs::path tmpDir = fs::temp_directory_path(ec);
        if (ec) {
            std::cerr << "Error: unable to get temp directory." << std::endl;
            return "";
        }
        fs::path tmpl = tmpDir / "Task127_XXXXXX";
        std::string tmplStr = tmpl.string();
        std::vector<char> pattern(tmplStr.begin(), tmplStr.end());
        pattern.push_back('\0');

        int tfd = mkstemp(pattern.data());
        if (tfd == -1) {
            std::cerr << "Error: failed to create temporary file." << std::endl;
            return "";
        }

        // Set restrictive permissions
        (void)fchmod(tfd, S_IRUSR | S_IWUSR);

        // Copy content
        std::ifstream in(fileReal, std::ios::binary);
        if (!in) {
            std::cerr << "Error: failed to open source file." << std::endl;
            close(tfd);
            unlink(pattern.data());
            return "";
        }

        const size_t BUF_SIZE = 8192;
        std::vector<char> buf(BUF_SIZE);
        while (in) {
            in.read(buf.data(), BUF_SIZE);
            std::streamsize got = in.gcount();
            if (got > 0) {
                ssize_t written = 0;
                const char* p = buf.data();
                while (written < got) {
                    ssize_t w = write(tfd, p + written, static_cast<size_t>(got - written));
                    if (w < 0) {
                        std::cerr << "Error: write failure." << std::endl;
                        close(tfd);
                        unlink(pattern.data());
                        return "";
                    }
                    written += w;
                }
            }
        }
        if (!in.eof()) {
            std::cerr << "Error: read failure." << std::endl;
            close(tfd);
            unlink(pattern.data());
            return "";
        }

        close(tfd);
        // Step 4: Code review - validated paths, safe temp creation, copied with checks
        return std::string(pattern.data());

    } catch (const std::bad_alloc&) {
        std::cerr << "Error: memory allocation failure." << std::endl;
        return "";
    } catch (...) {
        std::cerr << "Error: unexpected failure." << std::endl;
        return "";
    }
}

static void writeFile(const fs::path& dir, const fs::path& rel, const std::string& content) {
    fs::create_directories((dir / rel).parent_path());
    std::ofstream out(dir / rel, std::ios::binary);
    out.write(content.data(), static_cast<std::streamsize>(content.size()));
}

int main() {
    // Step 5: Secure code output - run 5 test cases
    std::error_code ec;
    fs::path base = fs::temp_directory_path(ec) / ("Task127_test_cpp_" + std::to_string(::getpid()));
    fs::create_directories(base, ec);
    writeFile(base, "file1.txt", "Hello, secure world!");
    writeFile(base, "empty.txt", "");
    fs::create_directories(base / "subdir", ec);
    writeFile(base, "subdir/nested.txt", "Nested content");

    std::cout << "Test1: " << copyFileToTemp(base.string(), "file1.txt") << std::endl;
    std::cout << "Test2: " << copyFileToTemp(base.string(), "missing.txt") << std::endl;
    std::cout << "Test3: " << copyFileToTemp(base.string(), "../file1.txt") << std::endl;
    std::cout << "Test4: " << copyFileToTemp(base.string(), "subdir") << std::endl;
    std::cout << "Test5: " << copyFileToTemp(base.string(), "empty.txt") << std::endl;

    return 0;
}