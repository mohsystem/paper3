#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>

std::string readFromSafe(const std::string& filename) {
    namespace fs = std::filesystem;
    try {
        fs::path base("/safe");
        fs::path target = (base / filename).lexically_normal();

        // Ensure target is within base using component-wise comparison
        auto itb = base.begin();
        auto itt = target.begin();
        for (; itb != base.end() && itt != target.end(); ++itb, ++itt) {
            if (*itb != *itt) {
                return "ERROR: Access outside /safe is not allowed";
            }
        }
        if (itb != base.end()) {
            return "ERROR: Access outside /safe is not allowed";
        }

        std::ifstream in(target, std::ios::in | std::ios::binary);
        if (!in) {
            return std::string("ERROR: Unable to open file: ") + target.string();
        }
        std::ostringstream ss;
        ss << in.rdbuf();
        return ss.str();
    } catch (const std::exception& e) {
        return std::string("ERROR: ") + e.what();
    }
}

int main() {
    std::string tests[5] = {
        "example.txt",
        "notes.md",
        "data.json",
        "subdir/file.txt",
        "../etc/passwd"
    };
    for (const auto& t : tests) {
        std::cout << "== " << t << " ==" << std::endl;
        std::cout << readFromSafe(t) << std::endl << std::endl;
    }
    return 0;
}