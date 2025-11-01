#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <cctype>

struct Response {
    int status;
    std::unordered_map<std::string, std::string> headers;
    std::string body; // binary-safe
};

struct RouteEntry {
    std::regex pattern;
    Response (*handler)(const std::string& param);
};

static std::vector<RouteEntry> ROUTES;

static bool register_route(const std::string& pattern, Response (*handler)(const std::string&)) {
    // Convert /images/<name> or /images/{name} to regex
    std::string reg = "^";
    std::vector<std::string> params;
    for (size_t i = 0; i < pattern.size();) {
        char c = pattern[i];
        if (c == '<' || c == '{') {
            char closer = (c == '<') ? '>' : '}';
            size_t j = pattern.find(closer, i + 1);
            if (j == std::string::npos) throw std::runtime_error("Unmatched route token");
            reg += "([^/]+)";
            i = j + 1;
        } else {
            if (std::string(".^$*+?{}[]\\|()").find(c) != std::string::npos)
                reg += "\\";
            reg += c;
            ++i;
        }
    }
    reg += "$";
    ROUTES.push_back(RouteEntry{std::regex(reg), handler});
    return true;
}

#define ROUTE(PATH, NAME) \
    Response NAME(const std::string& param); \
    static bool __reg_##NAME = register_route(PATH, NAME); \
    Response NAME(const std::string& param)

static bool is_safe_filename(const std::string& name) {
    if (name.empty() || name.size() > 255) return false;
    if (name == "." || name == "..") return false;
    for (char ch : name) {
        if (!(std::isalnum(static_cast<unsigned char>(ch)) || ch == '.' || ch == '_' || ch == '-')) return false;
    }
    return true;
}

static std::string guess_mime(const std::string& filename) {
    std::string f;
    f.reserve(filename.size());
    for (char c : filename) f.push_back(std::tolower(static_cast<unsigned char>(c)));
    if (f.size() >= 4 && f.rfind(".png") == f.size() - 4) return "image/png";
    if ((f.size() >= 4 && f.rfind(".jpg") == f.size() - 4) || (f.size() >= 5 && f.rfind(".jpeg") == f.size() - 5)) return "image/jpeg";
    if (f.size() >= 4 && f.rfind(".gif") == f.size() - 4) return "image/gif";
    if (f.size() >= 5 && f.rfind(".webp") == f.size() - 5) return "image/webp";
    return "application/octet-stream";
}

static Response send_file_if_exists(const std::string& filename) {
    namespace fs = std::filesystem;
    if (!is_safe_filename(filename)) {
        return Response{400, {{"Content-Type","text/plain"}}, "Bad filename"};
    }
    fs::path base = fs::absolute("images");
    fs::path target = fs::weakly_canonical(base / filename);
    if (target.string().compare(0, base.string().size(), base.string()) != 0) {
        return Response{400, {{"Content-Type","text/plain"}}, "Invalid path"};
    }
    if (!fs::exists(target) || !fs::is_regular_file(target)) {
        return Response{404, {{"Content-Type","text/plain"}}, "Not Found"};
    }
    std::ifstream in(target, std::ios::binary);
    if (!in) {
        return Response{500, {{"Content-Type","text/plain"}}, "IO Error"};
    }
    std::string data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    Response r;
    r.status = 200;
    r.headers["Content-Type"] = guess_mime(filename);
    r.headers["Content-Length"] = std::to_string(data.size());
    r.body = std::move(data);
    return r;
}

// "Decorator" via registration macro
ROUTE("/images/<name>", image_handler) {
    return send_file_if_exists(param);
}

static Response serve_request(const std::string& path) {
    for (const auto& e : ROUTES) {
        std::smatch m;
        if (std::regex_match(path, m, e.pattern)) {
            if (m.size() == 2) {
                return e.handler(m[1].str());
            }
        }
    }
    return Response{404, {{"Content-Type","text/plain"}}, "Not Found"};
}

static void write_bytes(const std::filesystem::path& p, const std::string& data) {
    std::filesystem::create_directories(p.parent_path());
    std::ofstream out(p, std::ios::binary);
    out.write(data.data(), static_cast<std::streamsize>(data.size()));
}

int main() {
    namespace fs = std::filesystem;
    fs::create_directories("images");
    write_bytes("images/hello.png", std::string("\x89PNG", 4));
    write_bytes("images/cat.jpg", std::string("\xFF\xD8\xFF", 3));
    write_bytes("images/dots..png", std::string("\x01\x02\x03\x04", 4));

    std::vector<std::string> tests = {
        "/images/hello.png",
        "/images/missing.png",
        "/images/../../secret.txt",
        "/images/dots..png",
        "/images/CAT.JPG"
    };
    for (const auto& t : tests) {
        Response r = serve_request(t);
        std::cout << t << " -> status=" << r.status << ", len=" << r.body.size() << "\n";
    }
    return 0;
}