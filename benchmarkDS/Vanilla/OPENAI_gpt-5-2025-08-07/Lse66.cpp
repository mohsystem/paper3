#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <cctype>

namespace fs = std::filesystem;

static std::string sanitize_filename(const std::string& name) {
    // keep only last segment
    std::string s = name;
    for (char& c : s) if (c == '\\') c = '/';
    auto pos = s.find_last_of('/');
    if (pos != std::string::npos) s = s.substr(pos + 1);
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '_' || c == '-') out.push_back(c);
        else out.push_back('_');
    }
    // ensure .pdf extension present
    std::string low = out;
    for (auto& c : low) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (low.size() < 4 || low.substr(low.size() - 4) != ".pdf") {
        out += ".pdf";
    }
    return out;
}

static bool is_pdf(const std::vector<unsigned char>& data) {
    return data.size() >= 4 && data[0] == '%' && data[1] == 'P' && data[2] == 'D' && data[3] == 'F';
}

static std::string save_pdf(const std::vector<unsigned char>& data, const std::string& filename) {
    if (!is_pdf(data)) {
        throw std::runtime_error("Not a valid PDF (bad signature)");
    }
    std::string sanitized = sanitize_filename(filename);
    std::string low = sanitized;
    for (auto& c : low) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (low.size() < 4 || low.substr(low.size() - 4) != ".pdf") {
        throw std::runtime_error("Filename must end with .pdf");
    }
    fs::create_directories("uploads");
    fs::path target = fs::path("uploads") / sanitized;
    if (fs::exists(target)) {
        std::string base = sanitized.substr(0, sanitized.size() - 4);
        int i = 1;
        while (true) {
            fs::path cand = fs::path("uploads") / (base + "_" + std::to_string(i) + ".pdf");
            if (!fs::exists(cand)) { target = cand; break; }
            ++i;
        }
    }
    std::ofstream ofs(target, std::ios::binary);
    if (!ofs) throw std::runtime_error("Cannot open file for writing");
    ofs.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    ofs.close();
    return target.string();
}

int main() {
    std::vector<unsigned char> valid = {'%', 'P', 'D', 'F', '-', '1', '.', '7', '\n'};
    std::vector<unsigned char> invalidSig = {'X', 'X', 'X', 'X'};
    try {
        std::cout << "Test1: " << save_pdf(valid, "report.pdf") << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test1 failed: " << e.what() << std::endl;
    }
    try {
        std::cout << "Test2: " << save_pdf(invalidSig, "bad.pdf") << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test2 failed: " << e.what() << std::endl;
    }
    try {
        std::cout << "Test3: " << save_pdf(valid, "notes.txt") << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test3 failed: " << e.what() << std::endl;
    }
    try {
        std::cout << "Test4: " << save_pdf(valid, "../../escape.pdf") << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test4 failed: " << e.what() << std::endl;
    }
    try {
        std::string p1 = save_pdf(valid, "duplicate.pdf");
        std::string p2 = save_pdf(valid, "duplicate.pdf");
        std::cout << "Test5: " << p1 << " and " << p2 << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test5 failed: " << e.what() << std::endl;
    }
    // Note: Minimal file-saving logic and tests are implemented.
    // A full HTTP server is not included here.
    return 0;
}