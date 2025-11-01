#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sys/stat.h>

static bool file_exists(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

static std::string content_type_for(const std::string& filename) {
    std::string f = filename;
    for (auto& c : f) c = (char)tolower(c);
    if (f.size() >= 5 && (f.rfind(".jpeg") == f.size()-5 || f.rfind(".jpg") == f.size()-4)) return "image/jpeg";
    if (f.size() >= 4 && f.rfind(".png") == f.size()-4) return "image/png";
    if (f.size() >= 4 && f.rfind(".gif") == f.size()-4) return "image/gif";
    if (f.size() >= 4 && f.rfind(".bmp") == f.size()-4) return "image/bmp";
    if (f.size() >= 5 && f.rfind(".webp") == f.size()-5) return "image/webp";
    return "application/octet-stream";
}

// Loads an image from images/ folder into outData and sets outContentType.
// Returns true if the file exists and was loaded, false otherwise.
bool load_image_from_images(const std::string& filename, std::vector<unsigned char>& outData, std::string& outContentType) {
    if (filename.find("..") != std::string::npos) return false;
    std::string path = "images/" + filename;
    // naive normalize: reject absolute paths
    if (!path.empty() && (path[0] == '/' || path[0] == '\\')) return false;

    if (!file_exists(path)) return false;
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    in.seekg(0, std::ios::end);
    std::streampos len = in.tellg();
    in.seekg(0, std::ios::beg);
    if (len < 0) return false;
    outData.resize(static_cast<size_t>(len));
    if (len > 0) in.read(reinterpret_cast<char*>(outData.data()), len);
    outContentType = content_type_for(filename);
    return true;
}

int main() {
    // 5 test cases for the core function
    std::string tests[5] = {"sample.jpg", "logo.png", "missing.png", "test.gif", "photos/pic.jpg"};
    for (const auto& t : tests) {
        std::vector<unsigned char> data;
        std::string ctype;
        bool ok = load_image_from_images(t, data, ctype);
        std::cout << "Test '" << t << "': exists=" << (ok ? "true" : "false");
        if (ok) std::cout << ", bytes=" << data.size() << ", type=" << ctype;
        std::cout << std::endl;
    }
    return 0;
}