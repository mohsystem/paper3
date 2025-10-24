#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <chrono>
#include <random>
#include <cctype>

static const std::string UPLOAD_DIR = "uploads";

std::string sanitizeFilename(const std::string& name) {
    if (name.empty()) return "file";
    std::string out;
    out.reserve(name.size());
    for (unsigned char c : name) {
        if (std::isalnum(c) || c == '.' || c == '_' || c == '-') out.push_back(c);
        else out.push_back('_');
    }
    if (out.empty()) out = "file";
    if (out.size() > 200) out.resize(200);
    return out;
}

std::string uploadFile(const std::string& filename, const std::string& data) {
    try {
        std::filesystem::create_directories(UPLOAD_DIR);
        auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(
                       std::chrono::system_clock::now()).time_since_epoch().count();
        std::random_device rd;
        std::mt19937_64 gen(rd());
        uint64_t rnd = gen();
        std::string safe = sanitizeFilename(filename);
        std::string unique = std::to_string(now) + "_" + std::to_string(rnd) + "_" + safe;
        std::filesystem::path path = std::filesystem::path(UPLOAD_DIR) / unique;
        std::ofstream ofs(path, std::ios::binary);
        if (!ofs) return std::string("ERROR: cannot open file for writing: ") + path.string();
        ofs.write(data.data(), static_cast<std::streamsize>(data.size()));
        if (!ofs) return std::string("ERROR: write failed for: ") + path.string();
        return "OK: stored as " + path.string() + " (name=" + filename + ", size=" + std::to_string(data.size()) + " bytes)";
    } catch (const std::exception& e) {
        return std::string("ERROR: ") + e.what();
    }
}

int main() {
    // 5 test cases
    std::cout << uploadFile("hello.txt", std::string("Hello, world!")) << "\n";

    std::string img_mock(256, '\0');
    std::random_device rd;
    for (char& c : img_mock) c = static_cast<char>(rd());
    std::cout << uploadFile("image.png", img_mock) << "\n";

    std::cout << uploadFile("../report.pdf", std::string("This is a mock PDF content.")) << "\n";

    std::string data_bin(1024, '\0');
    for (char& c : data_bin) c = static_cast<char>(rd());
    std::cout << uploadFile("data.bin", data_bin) << "\n";

    std::string long_name = "resume_with_a_very_long_name_that_should_be_sanitized_and_truncated_if_needed_because_it_might_be_too_long_for_some_file_systems.docx";
    std::cout << uploadFile(long_name, std::string("Resume content bytes")) << "\n";
    return 0;
}