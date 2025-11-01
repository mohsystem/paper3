#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

std::string readFileFromSafeDir(const std::string& safeDir, const std::string& requestedPath) {
    try {
        fs::path base = fs::absolute(fs::path(safeDir));
        if (!fs::exists(base)) fs::create_directories(base);

        fs::path req(requestedPath);
        if (req.is_absolute()) return "";

        fs::path candidate = fs::weakly_canonical(base / req);

        bool inside = false;
        for (fs::path p = candidate; !p.empty(); p = p.parent_path()) {
            if (p == base) { inside = true; break; }
        }
        if (!inside) return "";

        if (!fs::exists(candidate) || !fs::is_regular_file(candidate)) return "";

        std::ifstream in(candidate, std::ios::in | std::ios::binary);
        if (!in) return "";
        std::ostringstream ss;
        ss << in.rdbuf();
        return ss.str();
    } catch (...) {
        return "";
    }
}

int main() {
    fs::path base = fs::absolute("safe_dir_cpp");
    fs::create_directories(base / "sub");

    {
        std::ofstream(base / "hello.txt") << "Hello from C++";
        std::ofstream(base / "sub" / "inner.txt") << "Inner from C++";
    }

    std::string tests[5] = {
        "hello.txt",                               // valid
        "sub/inner.txt",                           // valid in subdir
        "../hello.txt",                            // traversal attempt
        (base / "hello.txt").string(),             // absolute path attempt
        "missing.txt"                              // non-existent
    };

    for (const auto& t : tests) {
        std::string content = readFileFromSafeDir(base.string(), t);
        std::cout << "Request: " << t << " => " << (content.empty() ? "(blocked or not found)" : content) << "\n";
    }

    return 0;
}