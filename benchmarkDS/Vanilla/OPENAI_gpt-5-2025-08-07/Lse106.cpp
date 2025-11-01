#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>

std::string writeSecret(const std::string& filename, const std::string& secret) {
    std::ofstream ofs(filename, std::ios::binary | std::ios::trunc);
    if (!ofs) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    ofs << secret;
    ofs.close();
    chmod(filename.c_str(), S_IRUSR); // 0400: read-only for owner
    return filename;
}

int main() {
    try {
        for (int i = 1; i <= 5; ++i) {
            std::string fname = "secret_cpp_" + std::to_string(static_cast<long long>(time(nullptr))) + "_" + std::to_string(i) + ".txt";
            std::string name = writeSecret(fname, "Super secret info #" + std::to_string(i));
            std::cout << name << std::endl;
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}