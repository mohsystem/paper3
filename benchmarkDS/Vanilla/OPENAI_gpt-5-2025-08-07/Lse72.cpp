#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

std::string upload_file(const std::string& filename, const std::string& data) {
    std::filesystem::create_directories("uploads");
    std::filesystem::path outPath = std::filesystem::path("uploads") / filename;
    std::ofstream ofs(outPath, std::ios::binary);
    ofs.write(data.data(), static_cast<std::streamsize>(data.size()));
    ofs.close();
    return std::filesystem::absolute(outPath).string();
}

int main() {
    std::cout << upload_file("cpp_test1.txt", "Hello from C++ 1") << std::endl;
    std::cout << upload_file("cpp_test2.txt", "Hello from C++ 2") << std::endl;
    std::cout << upload_file("cpp_test3.txt", "Hello from C++ 3") << std::endl;
    std::cout << upload_file("cpp_test4.txt", "Hello from C++ 4") << std::endl;
    std::cout << upload_file("cpp_test5.txt", "Hello from C++ 5") << std::endl;
    return 0;
}