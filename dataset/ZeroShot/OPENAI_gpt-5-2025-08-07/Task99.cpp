#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <filesystem>
#include <random>
#include <chrono>

static const std::regex kPattern(R"(^[0-9]+.*\.$)", std::regex::ECMAScript);

std::vector<std::string> match_lines_start_num_end_dot(const std::string& file_path) {
    std::vector<std::string> matches;
    std::ifstream in(file_path, std::ios::in | std::ios::binary);
    if (!in) {
        return matches;
    }
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (std::regex_match(line, kPattern)) {
            matches.push_back(line);
        }
    }
    return matches;
}

static std::string create_temp_file_with_lines(const std::vector<std::string>& lines) {
    namespace fs = std::filesystem;
    fs::path dir = fs::temp_directory_path();
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<unsigned long long> dist;

    for (int attempt = 0; attempt < 10; ++attempt) {
        auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        std::string name = "task99_" + std::to_string(now) + "_" + std::to_string(dist(gen)) + ".txt";
        fs::path p = dir / name;
        std::ofstream out(p, std::ios::out | std::ios::binary | std::ios::trunc);
        if (out) {
            for (const auto& s : lines) {
                out.write(s.data(), static_cast<std::streamsize>(s.size()));
                out.put('\n');
            }
            out.close();
            return p.string();
        }
    }
    throw std::runtime_error("Failed to create temporary file");
}

static void print_test(const std::string& title, const std::vector<std::string>& out) {
    std::cout << title << "\n";
    for (const auto& s : out) {
        std::cout << s << "\n";
    }
    std::cout << "---\n";
}

int main() {
    try {
        // Test 1
        std::string f1 = create_temp_file_with_lines({
            "123.",
            "1abc.",
            "abc.",
            "123",
            " 123.",
            "456.."
        });
        print_test("Test 1", match_lines_start_num_end_dot(f1));
        std::filesystem::remove(f1);

        // Test 2: empty file
        std::string f2 = create_temp_file_with_lines({});
        print_test("Test 2", match_lines_start_num_end_dot(f2));
        std::filesystem::remove(f2);

        // Test 3: whitespace and numbers
        std::string f3 = create_temp_file_with_lines({
            "\t789.",
            "000.",
            "000. ",
            "999.x",
            "42."
        });
        print_test("Test 3", match_lines_start_num_end_dot(f3));
        std::filesystem::remove(f3);

        // Test 4: mixed punctuation
        std::string f4 = create_temp_file_with_lines({
            "7!@#$.",
            "5text.",
            "0042..",
            "nope.",
            "9"
        });
        print_test("Test 4", match_lines_start_num_end_dot(f4));
        std::filesystem::remove(f4);

        // Test 5: long lines and edge cases
        std::string long_line(1000, 'x');
        std::string f5 = create_temp_file_with_lines({
            "1" + long_line + ".",
            long_line + ".",
            "2.",
            "3. ",
            "4.."
        });
        print_test("Test 5", match_lines_start_num_end_dot(f5));
        std::filesystem::remove(f5);
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}