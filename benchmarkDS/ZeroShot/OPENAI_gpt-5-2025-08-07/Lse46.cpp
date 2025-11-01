#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <cctype>

struct ImgResult {
    int statusCode;               // 200, 400, 403, 404, 500
    std::string contentType;
    std::vector<unsigned char> data;
    std::string message;
};

static bool isAllowedName(const std::string& name) {
    if (name.empty() || name.size() > 255) return false;
    if (name[0] == '.') return false;
    for (char c : name) {
        if (!(std::isalnum((unsigned char)c) || c == '.' || c == '_' || c == '-')) return false;
    }
    return true;
}

static std::string guessContentType(const std::string& name) {
    std::string lower = name;
    for (auto& ch : lower) ch = (char)std::tolower((unsigned char)ch);
    if (lower.size() >= 4 && lower.rfind(".png") == lower.size() - 4) return "image/png";
    if ((lower.size() >= 4 && lower.rfind(".jpg") == lower.size() - 4) ||
        (lower.size() >= 5 && lower.rfind(".jpeg") == lower.size() - 5)) return "image/jpeg";
    if (lower.size() >= 4 && lower.rfind(".gif") == lower.size() - 4) return "image/gif";
    if (lower.size() >= 5 && lower.rfind(".webp") == lower.size() - 5) return "image/webp";
    if (lower.size() >= 4 && lower.rfind(".svg") == lower.size() - 4) return "image/svg+xml";
    if (lower.size() >= 4 && lower.rfind(".txt") == lower.size() - 4) return "text/plain; charset=utf-8";
    return "application/octet-stream";
}

ImgResult img(const std::string& name) {
    namespace fs = std::filesystem;
    ImgResult res{500, "text/plain; charset=utf-8", {}, "Internal Server Error"};

    if (!isAllowedName(name)) {
        res.statusCode = 400;
        res.message = "Invalid file name";
        return res;
    }

    try {
        fs::path base = fs::absolute("images");
        if (!fs::exists(base)) {
            fs::create_directories(base);
        }
        fs::path target = fs::weakly_canonical(base / name);

        // Prevent path traversal
        auto baseStr = base.generic_string();
        auto targetStr = target.generic_string();
        if (targetStr.compare(0, baseStr.size(), baseStr) != 0 || (targetStr.size() > baseStr.size() && targetStr[baseStr.size()] != '/')) {
            res.statusCode = 400;
            res.message = "Invalid path";
            return res;
        }

        if (!fs::exists(target) || fs::is_directory(target)) {
            res.statusCode = 404;
            res.message = "Not found";
            return res;
        }

        if (fs::is_symlink(target)) {
            res.statusCode = 403;
            res.message = "Forbidden";
            return res;
        }

        std::ifstream in(target, std::ios::binary);
        if (!in) {
            res.statusCode = 500;
            res.message = "Internal Server Error";
            return res;
        }
        std::vector<unsigned char> buffer((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        res.statusCode = 200;
        res.contentType = guessContentType(target.filename().string());
        res.data = std::move(buffer);
        res.message = "OK";
        return res;
    } catch (...) {
        res.statusCode = 500;
        res.message = "Internal Server Error";
        return res;
    }
}

int main() {
    namespace fs = std::filesystem;
    try {
        fs::path base = fs::absolute("images");
        fs::create_directories(base);
        fs::path sample = base / "hello.txt";
        if (!fs::exists(sample)) {
            std::ofstream out(sample, std::ios::binary);
            out << "Hello world";
        }
    } catch (...) {
        // ignore
    }

    // 5 tests
    ImgResult r1 = img("hello.txt");     // 200
    ImgResult r2 = img("");              // 400
    ImgResult r3 = img("../secret");     // 400
    ImgResult r4 = img("bad/name.png");  // 400
    ImgResult r5 = img("nope.png");      // 404

    auto print = [](const char* label, const ImgResult& r){
        std::cout << label << " -> status=" << r.statusCode
                  << ", type=" << r.contentType
                  << ", bytes=" << r.data.size() << "\n";
    };

    print("valid existing", r1);
    print("missing param", r2);
    print("path traversal", r3);
    print("invalid chars", r4);
    print("not found", r5);

    return 0;
}