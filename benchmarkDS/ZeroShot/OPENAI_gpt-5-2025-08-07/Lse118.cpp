#include <iostream>
#include <fstream>
#include <random>
#include <sstream>
#include <string>
#include <iomanip>

class Lse118 {
public:
    static std::string generateAndWrite(const std::string& filePath) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);

        float a = dist(gen);
        float b = dist(gen);
        float c = dist(gen);

        std::ostringstream oss;
        oss.imbue(std::locale::classic());
        oss << std::setprecision(9) << a
            << std::setprecision(9) << b
            << std::setprecision(9) << c;

        std::string result = oss.str();

        std::ofstream ofs(filePath, std::ios::out | std::ios::trunc | std::ios::binary);
        if (!ofs) {
            throw std::runtime_error("Failed to open file: " + filePath);
        }
        ofs << result;
        ofs.close();
        return result;
    }
};

int main() {
    const char* files[5] = {"cpp_out1.txt", "cpp_out2.txt", "cpp_out3.txt", "cpp_out4.txt", "cpp_out5.txt"};
    for (const char* f : files) {
        try {
            std::string s = Lse118::generateAndWrite(f);
            std::cout << "Wrote to " << f << ": " << s << "\n";
        } catch (const std::exception& ex) {
            std::cerr << "Error writing to " << f << ": " << ex.what() << "\n";
        }
    }
    return 0;
}