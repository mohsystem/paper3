#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;
static constexpr std::uintmax_t MAX_SIZE = 10u * 1024u * 1024u; // 10 MB

// Secure file-reading function:
// - Normalize path
// - Reject symbolic links
// - Ensure regular file and readable
// - Enforce a maximum size
// - Stream read to avoid TOCTOU issues
std::string readFile(const std::string& path) {
    try {
        fs::path p(path);
        fs::path norm = fs::absolute(p).lexically_normal();

        // Check existence without following symlinks
        fs::file_status s = fs::symlink_status(norm);
        if (!fs::exists(s)) {
            return std::string();
        }

        // Reject symlinks
        if (fs::is_symlink(s)) {
            return std::string();
        }

        // Ensure regular file
        if (!fs::is_regular_file(s)) {
            return std::string();
        }

        // Initial size check
        std::uintmax_t size = fs::file_size(norm);
        if (size > MAX_SIZE) {
            return std::string();
        }

        std::ifstream in(norm, std::ios::binary);
        if (!in) {
            return std::string();
        }

        std::string out;
        out.reserve(static_cast<size_t>(std::min<std::uintmax_t>(size, 8192)));
        char buf[8192];
        std::uintmax_t total = 0;
        while (in) {
            in.read(buf, sizeof(buf));
            std::streamsize got = in.gcount();
            if (got > 0) {
                total += static_cast<std::uintmax_t>(got);
                if (total > MAX_SIZE) {
                    return std::string();
                }
                out.append(buf, static_cast<size_t>(got));
            }
        }

        // Assume UTF-8 text; return as-is
        return out;
    } catch (...) {
        return std::string();
    }
}

static fs::path makeTempWithContent(const std::string& content) {
    fs::path tempdir = fs::temp_directory_path();
    fs::path file = tempdir / fs::unique_path("task70-%%%%-%%%%.txt");
    std::ofstream out(file, std::ios::binary | std::ios::trunc);
    out.write(content.data(), static_cast<std::streamsize>(content.size()));
    out.close();
    return file;
}

int main(int argc, char** argv) {
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            std::string res = readFile(argv[i]);
            if (!res.empty() || (fs::exists(argv[i]) && fs::is_regular_file(argv[i]) && fs::file_size(argv[i]) == 0)) {
                std::cout << res;
                if (!res.empty() && res.back() != '\n') std::cout << "\n";
            } else {
                std::cout << "ERROR\n";
            }
        }
    } else {
        // 5 test cases
        std::vector<fs::path> created;
        try {
            fs::path t1 = makeTempWithContent("Hello from test 1\n");
            fs::path t2 = makeTempWithContent("Line1\nLine2\nLine3\n");
            fs::path t3 = makeTempWithContent(""); // empty
            fs::path t4 = makeTempWithContent("Unicode: \xCE\xB1\xCE\xB2\xCE\xB3, emoji: \xF0\x9F\x99\x82\n"); // UTF-8
            std::string t5 = "this_path_should_not_exist_12345.txt";

            created = {t1, t2, t3, t4};

            std::vector<std::string> tests = { t1.string(), t2.string(), t3.string(), t4.string(), t5 };
            for (size_t i = 0; i < tests.size(); ++i) {
                std::cout << "Test " << (i + 1) << ":\n";
                std::string res = readFile(tests[i]);
                bool isEmptyRegular = false;
                try {
                    fs::file_status s = fs::symlink_status(tests[i]);
                    isEmptyRegular = fs::exists(s) && fs::is_regular_file(s) && fs::file_size(tests[i]) == 0;
                } catch (...) {}

                if (!res.empty() || isEmptyRegular) {
                    std::cout << res;
                    if (!res.empty() && res.back() != '\n') std::cout << "\n";
                } else {
                    std::cout << "ERROR\n";
                }
            }
        } catch (...) {
            // ignore
        }
        for (auto& p : created) {
            std::error_code ec;
            fs::remove(p, ec);
        }
    }
    return 0;
}