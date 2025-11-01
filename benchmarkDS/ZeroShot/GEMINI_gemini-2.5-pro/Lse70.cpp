// To compile and run this code:
// 1. You need a C++17 compliant compiler (like g++ or clang++).
// 2. You need the Crow C++ microframework library.
//    Download 'crow_all.h' from https://github.com/CrowCpp/Crow and place it in your include path.
// 3. Compile the code, linking necessary libraries.
//    Example: g++ -std=c++17 -o Lse70 Lse70.cpp -I/path/to/crow/include -lpthread -lssl -lcrypto -lboost_system
// 4. Run the executable: ./Lse70
// 5. The server will start on http://127.0.0.1:18080

#define CROW_MAIN
#include "crow_all.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <sstream>
#include <random>
#include <unordered_set>

std::string generate_uuid() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis2(8, 11);

    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < 8; i++) ss << dis(gen); ss << "-";
    for (int i = 0; i < 4; i++) ss << dis(gen); ss << "-4";
    for (int i = 0; i < 3; i++) ss << dis(gen); ss << "-";
    ss << dis2(gen);
    for (int i = 0; i < 3; i++) ss << dis(gen); ss << "-";
    for (int i = 0; i < 12; i++) ss << dis(gen);
    return ss.str();
}

std::string get_file_extension(const std::string& filename) {
    std::filesystem::path p(filename);
    if (p.has_extension()) {
        std::string ext = p.extension().string();
        if (!ext.empty()) return ext.substr(1);
    }
    return "";
}

void upload_file(const crow::request& req, crow::response& res) {
    const std::string UPLOAD_DIR = "uploads";
    const std::unordered_set<std::string> ALLOWED_EXTENSIONS = {"txt", "jpg", "jpeg", "png", "pdf"};
    const size_t MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 MB

    if (!std::filesystem::exists(UPLOAD_DIR)) {
        std::filesystem::create_directory(UPLOAD_DIR);
    }

    crow::multipart::message msg(req);
    auto file_part = msg.get_part_by_name("file");

    if (!file_part) {
        res.code = 400;
        res.body = "No file part in the request.";
        return res.end();
    }
    
    if (file_part->body.length() > MAX_FILE_SIZE) {
        res.code = 413;
        res.body = "File is too large.";
        return res.end();
    }

    // Sanitize filename by taking only the basename and validate extension
    std::string original_filename = std::filesystem::path(file_part->get_header_value("filename")).filename().string();
    if(original_filename.empty()) {
        res.code = 400;
        res.body = "Invalid file name.";
        return res.end();
    }

    std::string extension = get_file_extension(original_filename);
    if (ALLOWED_EXTENSIONS.find(extension) == ALLOWED_EXTENSIONS.end()) {
        res.code = 400;
        res.body = "File type not allowed.";
        return res.end();
    }
    
    std::string new_filename = generate_uuid() + "." + extension;
    std::string file_path = UPLOAD_DIR + "/" + new_filename;

    std::ofstream out_file(file_path, std::ios::binary);
    if (!out_file) {
        res.code = 500;
        res.body = "Failed to open file for writing on server.";
        return res.end();
    }

    out_file.write(file_part->body.data(), file_part->body.size());
    out_file.close();

    res.code = 200;
    res.body = "File uploaded successfully as " + new_filename;
    res.end();
}

int main() {
    std::cout << "Starting C++ Crow server on http://127.0.0.1:18080" << std::endl;
    std::cout << "Use the curl commands below for testing." << std::endl;
    std::cout << "\n--- 5 Test Cases (using curl) ---" << std::endl;

    std::cout << "\n1. Test Case: Successful upload of an allowed file type (.txt)" << std::endl;
    std::cout << "echo \"This is a test file.\" > test.txt" << std::endl;
    std::cout << "curl -X POST -F 'file=@test.txt' http://127.0.0.1:18080/upload" << std::endl;

    std::cout << "\n2. Test Case: Attempted upload of a disallowed file type (.exe)" << std::endl;
    std::cout << "echo \"dummy exe\" > test.exe" << std::endl;
    std::cout << "curl -X POST -F 'file=@test.exe' http://127.0.0.1:18080/upload" << std::endl;

    std::cout << "\n3. Test Case: Attempted upload of a file that is too large (e.g., > 10MB)" << std::endl;
    std::cout << "fallocate -l 11M largefile.dat" << std::endl;
    std::cout << "curl -X POST -F 'file=@largefile.dat' http://127.0.0.1:18080/upload" << std::endl;

    std::cout << "\n4. Test Case: Attempted upload with a malicious filename (path traversal)" << std::endl;
    std::cout << "echo \"malicious content\" > evil.txt" << std::endl;
    std::cout << "curl -X POST -F 'file=@evil.txt;filename=\"../../evil.txt\"' http://127.0.0.1:18080/upload" << std::endl;

    std::cout << "\n5. Test Case: Successful upload of another allowed file type (.jpg)" << std::endl;
    std::cout << "# (Assuming you have a file named 'image.jpg' in the current directory)" << std::endl;
    std::cout << "curl -X POST -F 'file=@image.jpg' http://127.0.0.1:18080/upload\n" << std::endl;
    
    crow::SimpleApp Lse70;
    
    CROW_ROUTE(Lse70, "/upload").methods(crow::HTTPMethod::Post)(upload_file);

    Lse70.port(18080).run();
    
    return 0;
}