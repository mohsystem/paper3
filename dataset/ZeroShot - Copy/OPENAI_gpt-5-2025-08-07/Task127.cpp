#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;

class Task127 {
public:
    static std::string safeCopyToTemp(const std::string& baseDir, const std::string& relPath, const std::string& tempDir = "") {
        try {
            fs::path base(baseDir);
            if (!fs::exists(base) || !fs::is_directory(base)) {
                std::cerr << "Base directory does not exist or is not a directory." << std::endl;
                return std::string();
            }

            fs::path baseReal;
            try {
                baseReal = fs::canonical(base);
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Unable to resolve base directory: " << e.what() << std::endl;
                return std::string();
            }

            fs::path candidate = baseReal / fs::path(relPath);
            fs::path candidateReal;
            try {
                candidateReal = fs::canonical(candidate);
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Source file cannot be resolved: " << e.what() << std::endl;
                return std::string();
            }

            // Check candidateReal is inside baseReal
            auto bIt = baseReal.begin();
            auto cIt = candidateReal.begin();
            for (; bIt != baseReal.end() && cIt != candidateReal.end(); ++bIt, ++cIt) {
                if (*bIt != *cIt) {
                    std::cerr << "Security violation: Attempted path traversal outside base directory." << std::endl;
                    return std::string();
                }
            }
            if (bIt != baseReal.end()) {
                std::cerr << "Security violation: Attempted path traversal outside base directory." << std::endl;
                return std::string();
            }

            if (!fs::exists(candidateReal) || !fs::is_regular_file(candidateReal)) {
                std::cerr << "Source is not a regular file." << std::endl;
                return std::string();
            }

            fs::path tempBase = tempDir.empty() ? fs::temp_directory_path() : fs::path(tempDir);
            std::error_code ec;
            fs::create_directories(tempBase, ec);
            if (ec) {
                std::cerr << "Unable to create/access temp directory: " << ec.message() << std::endl;
                return std::string();
            }

            fs::path tempFile = tempBase / fs::unique_path("safe_copy_%%%%-%%%%-%%%%-%%%%.tmp");
            {
                std::ifstream in(candidateReal, std::ios::binary);
                if (!in) {
                    std::cerr << "Failed to open source file." << std::endl;
                    return std::string();
                }
                std::ofstream out(tempFile, std::ios::binary | std::ios::trunc);
                if (!out) {
                    std::cerr << "Failed to create temporary file." << std::endl;
                    return std::string();
                }

                char buffer[8192];
                while (in) {
                    in.read(buffer, sizeof(buffer));
                    std::streamsize n = in.gcount();
                    if (n > 0) out.write(buffer, n);
                    if (!out) {
                        std::cerr << "Write error to temporary file." << std::endl;
                        out.close();
                        fs::remove(tempFile, ec);
                        return std::string();
                    }
                }
                out.flush();
            }

            // Set restrictive permissions if supported
            try {
                fs::permissions(tempFile, fs::perms::owner_read | fs::perms::owner_write,
                                fs::perm_options::replace);
            } catch (...) {
                // Ignore on unsupported platforms
            }

            return tempFile.string();
        } catch (const std::exception& ex) {
            std::cerr << "Error: " << ex.what() << std::endl;
            return std::string();
        }
    }
};

static void writeFile(const fs::path& p, const std::string& content) {
    fs::create_directories(p.parent_path());
    std::ofstream out(p, std::ios::binary | std::ios::trunc);
    out.write(content.data(), static_cast<std::streamsize>(content.size()));
    out.close();
}

int main() {
    fs::path base = fs::temp_directory_path() / fs::unique_path("task127_cpp_base_%%%%-%%%%");
    fs::create_directories(base);
    fs::path a = base / "a.txt";
    fs::path b = base / "sub" / "b.txt";
    writeFile(a, "Hello A");
    writeFile(b, "Hello B");

    // 5 Test cases
    std::string r1 = Task127::safeCopyToTemp(base.string(), "a.txt");
    std::cout << "Test1: " << (r1.empty() ? "NULL" : r1) << std::endl;

    std::string r2 = Task127::safeCopyToTemp(base.string(), "sub/b.txt");
    std::cout << "Test2: " << (r2.empty() ? "NULL" : r2) << std::endl;

    std::string r3 = Task127::safeCopyToTemp(base.string(), "../../etc/passwd");
    std::cout << "Test3: " << (r3.empty() ? "NULL" : r3) << std::endl;

    std::string r4 = Task127::safeCopyToTemp(base.string(), "missing.txt");
    std::cout << "Test4: " << (r4.empty() ? "NULL" : r4) << std::endl;

    std::string r5 = Task127::safeCopyToTemp(a.string(), "a.txt");
    std::cout << "Test5: " << (r5.empty() ? "NULL" : r5) << std::endl;

    return 0;
}