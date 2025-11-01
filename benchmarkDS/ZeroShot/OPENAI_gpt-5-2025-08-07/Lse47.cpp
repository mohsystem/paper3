#include <bits/stdc++.h>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

struct HttpResult {
    int status;
    std::string contentType;
    std::string body; // raw bytes
};

using RouteFunc = HttpResult(*)(const std::string&);

static std::unordered_map<std::string, RouteFunc> ROUTES;

struct RouteRegistrar {
    RouteRegistrar(const std::string& path, RouteFunc fn) {
        ROUTES[path] = fn;
    }
};

static std::unordered_set<std::string> allowedExt = {".png", ".jpg", ".jpeg", ".gif", ".webp"};

static std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
    return s;
}

static std::string guessContentType(const std::string& extLower) {
    if (extLower == ".png") return "image/png";
    if (extLower == ".jpg" || extLower == ".jpeg") return "image/jpeg";
    if (extLower == ".gif") return "image/gif";
    if (extLower == ".webp") return "image/webp";
    return "application/octet-stream";
}

HttpResult serve_image(const std::string& filename) {
    try {
        if (filename.empty()) {
            return {400, "text/plain; charset=utf-8", "Bad Request"};
        }
        fs::path imagesDir = fs::absolute("images");
        if (!fs::exists(imagesDir)) {
            return {500, "text/plain; charset=utf-8", "Internal Server Error"};
        }
        std::string lower = toLower(filename);
        std::string ext = fs::path(lower).extension().string();
        if (allowedExt.find(ext) == allowedExt.end()) {
            return {403, "text/plain; charset=utf-8", "Forbidden"};
        }
        fs::path base = fs::canonical(imagesDir);
        fs::path combined = fs::path(filename);
        combined = combined.relative_path(); // strip any root
        fs::path requested = fs::weakly_canonical(base / combined);
        auto baseStr = base.string();
        auto reqStr = requested.string();
        if (reqStr.size() < baseStr.size() || reqStr.compare(0, baseStr.size(), baseStr) != 0) {
            return {400, "text/plain; charset=utf-8", "Bad Request"};
        }
        if (!fs::exists(requested) || !fs::is_regular_file(requested)) {
            return {404, "text/plain; charset=utf-8", "Not Found"};
        }
        std::ifstream ifs(requested, std::ios::binary);
        std::ostringstream oss;
        oss << ifs.rdbuf();
        std::string body = oss.str();
        return {200, guessContentType(ext), body};
    } catch (...) {
        return {500, "text/plain; charset=utf-8", "Internal Server Error"};
    }
}

#define REGISTER_ROUTE(PATH, FUNC) \
    static RouteRegistrar registrar_##FUNC(PATH, FUNC);

REGISTER_ROUTE("/images", serve_image)

HttpResult handle_request(const std::string& url) {
    // Very small router
    for (const auto& kv : ROUTES) {
        const std::string& base = kv.first;
        const auto& fn = kv.second;
        std::string prefix = base;
        if (prefix.back() != '/') prefix.push_back('/');
        if (url.rfind(prefix, 0) == 0) {
            std::string filename = url.substr(prefix.size());
            return fn(filename);
        }
    }
    return {404, "text/plain; charset=utf-8", "Not Found"};
}

static void ensure_test_images() {
    fs::create_directories("images");
    fs::path png = fs::path("images") / "sample.png";
    fs::path jpg = fs::path("images") / "photo.jpg";
    if (!fs::exists(png)) {
        std::ofstream(png, std::ios::binary).write("\x89PNG\x00\x00\x00\x00", 8);
    }
    if (!fs::exists(jpg)) {
        std::ofstream(jpg, std::ios::binary).write("JPEGDATA", 8);
    }
}

int main() {
    ensure_test_images();
    // 5 test cases
    std::cout << handle_request("/images/sample.png").status << "\n";   // 200
    std::cout << handle_request("/images/none.png").status << "\n";     // 404
    std::cout << handle_request("/images/../secret.txt").status << "\n"; // 400
    std::cout << handle_request("/images/bad.txt").status << "\n";      // 403
    std::cout << handle_request("/images/photo.jpg").status << "\n";    // 200
    return 0;
}