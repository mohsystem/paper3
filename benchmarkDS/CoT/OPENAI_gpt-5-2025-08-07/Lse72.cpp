#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <unordered_set>
#include <cctype>

// Chain-of-Through process:
// 1) Problem understanding: Provide upload_file to save data to 'uploads' folder next to this source file.
// 2) Security requirements: Sanitize filename, avoid traversal, restrict extensions, limit size, avoid overwrite.
// 3) Secure coding generation: Implement with checks and filesystem APIs.
// 4) Code review: Validate inputs, normalize paths, and ensure directory creation.
// 5) Secure code output: Final function below.

namespace secure_upload {

    static std::string sanitize_base(const std::string& base) {
        std::string out;
        out.reserve(base.size());
        for (unsigned char c : base) {
            if (std::isalnum(c) || c == '.' || c == '_' || c == '-') {
                out.push_back(static_cast<char>(c));
            } else {
                out.push_back('_');
            }
        }
        // remove leading dots
        while (!out.empty() && out.front() == '.') {
            out.erase(out.begin());
        }
        if (out.empty()) out = "file";
        if (out.size() > 200) out.resize(200);
        return out;
    }

    std::string upload_file(const std::string& originalFilename, const std::string& data) {
        if (originalFilename.empty()) {
            throw std::invalid_argument("filename cannot be empty");
        }
        const size_t MAX_SIZE = 5 * 1024 * 1024;
        if (data.size() > MAX_SIZE) {
            throw std::invalid_argument("file too large");
        }

        // Allowed extensions
        static const std::unordered_set<std::string> allowed = {
            "txt","pdf","png","jpg","jpeg","gif","bin"
        };

        // Strip directories
        std::string name = originalFilename;
        for (auto& ch : name) {
            if (ch == '\\') ch = '/';
        }
        auto pos = name.find_last_of('/');
        if (pos != std::string::npos) {
            name = name.substr(pos + 1);
        }

        // Extract extension
        std::string base = name;
        std::string ext;
        auto dot = name.find_last_of('.');
        if (dot != std::string::npos && dot > 0 && dot + 1 < name.size()) {
            ext = name.substr(dot + 1);
            for (auto& c : ext) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            base = name.substr(0, dot);
        }
        if (ext.empty() || allowed.find(ext) == allowed.end()) {
            throw std::invalid_argument("disallowed or missing extension");
        }

        std::string safeBase = sanitize_base(base);
        std::string safeName = safeBase + "." + ext;

        // Base directory: same as this source file (__FILE__)
        std::filesystem::path baseDir = std::filesystem::path(__FILE__).parent_path();
        if (baseDir.empty()) baseDir = std::filesystem::current_path();
        std::filesystem::path uploadDir = (baseDir / "uploads");
        std::filesystem::create_directories(uploadDir);

        // Ensure uploadDir is inside baseDir
        auto normBase = std::filesystem::weakly_canonical(baseDir);
        auto normUpload = std::filesystem::weakly_canonical(uploadDir);
        if (normUpload.string().rfind(normBase.string(), 0) != 0) {
            throw std::runtime_error("invalid upload directory");
        }

        std::filesystem::path candidate = uploadDir / safeName;
        int counter = 1;
        while (std::filesystem::exists(candidate)) {
            candidate = uploadDir / (safeBase + "_" + std::to_string(counter) + "." + ext);
            counter++;
        }

        // Write file (best effort uniqueness)
        std::ofstream ofs(candidate, std::ios::binary | std::ios::out);
        if (!ofs) {
            throw std::runtime_error("failed to open file for writing");
        }
        ofs.write(data.data(), static_cast<std::streamsize>(data.size()));
        if (!ofs) {
            throw std::runtime_error("failed to write data");
        }
        ofs.close();
        return std::filesystem::absolute(candidate).string();
    }
}

int main() {
    try {
        // Test 1: valid
        std::string p1 = secure_upload::upload_file("report.txt", std::string("Hello, World!"));
        std::cout << "Saved: " << p1 << "\n";
    } catch (const std::exception& e) {
        std::cout << "Error test1: " << e.what() << "\n";
    }

    try {
        // Test 2: traversal attempt
        std::string p2 = secure_upload::upload_file("../../secret.txt", std::string("secret"));
        std::cout << "Saved: " << p2 << "\n";
    } catch (const std::exception& e) {
        std::cout << "Blocked traversal: " << e.what() << "\n";
    }

    try {
        // Test 3: disallowed extension
        std::string p3 = secure_upload::upload_file("script.sh", std::string("#!/bin/sh\necho hi"));
        std::cout << "Saved: " << p3 << "\n";
    } catch (const std::exception& e) {
        std::cout << "Blocked extension: " << e.what() << "\n";
    }

    try {
        // Test 4: weird chars
        std::string bin(3, '\0');
        bin[0] = 1; bin[1] = 2; bin[2] = 3;
        std::string p4 = secure_upload::upload_file("inva|id:name?.png", bin);
        std::cout << "Saved: " << p4 << "\n";
    } catch (const std::exception& e) {
        std::cout << "Error test4: " << e.what() << "\n";
    }

    try {
        // Test 5: duplicate filename
        std::string p5 = secure_upload::upload_file("report.txt", std::string("Another content"));
        std::cout << "Saved: " << p5 << "\n";
    } catch (const std::exception& e) {
        std::cout << "Error test5: " << e.what() << "\n";
    }

    return 0;
}