#include <bits/stdc++.h>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

// Chain-of-Through process in code generation:
// 1) Problem understanding: safely read a file from a safe directory only.
// 2) Security requirements: validate input, block absolute paths, traversal, symlinks, and limit file size.
// 3) Secure coding generation: implemented in readFromSafeDir.
// 4) Code review: uses canonicalization, component-wise symlink checks, and size limits.
// 5) Secure code output: final function and main tests.

static const std::uintmax_t MAX_SIZE_BYTES = 1048576; // 1 MiB

std::string readFromSafeDir(const std::string& requestedPath) {
    // Validate input
    if (requestedPath.empty()) return std::string();
    if (requestedPath.find('\0') != std::string::npos) return std::string();

    // Allowed characters only
    static const std::regex safeRe("^[A-Za-z0-9._/\\-]+$");
    if (!std::regex_match(requestedPath, safeRe)) return std::string();

    // Determine safe directory
    const char* home = std::getenv("HOME");
    fs::path safeDir = (home && *home) ? fs::path(home) / "safe_dir" : fs::current_path() / "safe_dir";

    try {
        fs::create_directories(safeDir);
        fs::path safeReal = fs::canonical(safeDir);

        fs::path req(requestedPath);
        if (req.is_absolute()) return std::string();

        fs::path target = (safeDir / req).lexically_normal();

        // Ensure target remains under safeDir (lexical first)
        if (target.string().compare(0, safeDir.string().size(), safeDir.string()) != 0) return std::string();

        // Ensure exists and regular file
        if (!fs::exists(target) || !fs::is_regular_file(target)) return std::string();

        // Component-wise symlink check
        fs::path cur = safeDir;
        fs::path rel = fs::relative(target, safeDir);
        for (const auto& part : rel) {
            cur /= part;
            if (fs::exists(cur) && fs::is_symlink(cur)) return std::string();
        }

        // Final canonical containment check
        fs::path targetReal = fs::canonical(target);
        std::string safeRealStr = safeReal.string();
        std::string targetRealStr = targetReal.string();
        if (targetRealStr.compare(0, safeRealStr.size(), safeRealStr) != 0) return std::string();

        // Size check
        std::uintmax_t size = fs::file_size(targetReal);
        if (size == static_cast<std::uintmax_t>(-1) || size > MAX_SIZE_BYTES) return std::string();

        // Read file
        std::string content;
        content.resize(static_cast<size_t>(size));
        std::ifstream in(targetReal, std::ios::binary);
        if (!in) return std::string();
        in.read(&content[0], static_cast<std::streamsize>(size));
        if (!in || in.gcount() != static_cast<std::streamsize>(size)) return std::string();
        return content;
    } catch (...) {
        return std::string();
    }
}

int main() {
    std::vector<std::string> tests = {
        "example.txt",
        "../secret.txt",
        "/etc/passwd",
        "subdir/notes.txt",
        "subdir/../../evil"
    };

    for (const auto& t : tests) {
        std::string res = readFromSafeDir(t);
        std::cout << "Request: " << t << "\n";
        if (!res.empty()) {
            std::cout << "Read OK, length=" << res.size() << "\n";
        } else {
            std::cout << "Read FAILED\n";
        }
        std::cout << "---\n";
    }
    return 0;
}