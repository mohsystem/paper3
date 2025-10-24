#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <vector>
#include <string>

std::vector<std::string> searchInFile(const std::string& pattern, const std::string& filename) {
    std::vector<std::string> matches;
    try {
        std::ifstream ifs(filename, std::ios::in | std::ios::binary);
        if (!ifs) return matches;
        std::ostringstream ss;
        ss << ifs.rdbuf();
        std::string content = ss.str();

        std::regex re(pattern);
        auto begin = std::sregex_iterator(content.begin(), content.end(), re);
        auto end = std::sregex_iterator();
        for (auto it = begin; it != end; ++it) {
            matches.push_back((*it).str());
        }
    } catch (const std::regex_error&) {
        // return empty on regex error
    } catch (...) {
        // return empty on other errors
    }
    return matches;
}

int main(int argc, char** argv) {
    if (argc >= 3) {
        auto res = searchInFile(argv[1], argv[2]);
        std::cout << "Matches: " << res.size() << "\n";
        for (const auto& s : res) std::cout << s << "\n";
        return 0;
    }

    const std::string filename = "test_input.txt";
    const std::string content =
        "foo bar baz 123 foo42\n"
        "New line with Foo and BAR.\n"
        "email: test@example.com; ip: 192.168.0.1; dates: 2025-10-04;\n"
        "ababa bab abab ababab\n";
    {
        std::ofstream ofs(filename, std::ios::out | std::ios::binary | std::ios::trunc);
        ofs << content;
    }

    std::vector<std::string> patterns = {
        "foo",
        "[0-9]+",
        "(ab)+",
        "[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}",
        "192\\.168\\.[0-9]+\\.[0-9]+",
    };

    for (size_t i = 0; i < patterns.size(); ++i) {
        auto res = searchInFile(patterns[i], filename);
        std::cout << "Test " << (i + 1) << " Pattern: " << patterns[i] << "\n";
        std::cout << "Matches: " << res.size() << "\n";
        for (const auto& s : res) std::cout << s << "\n";
        std::cout << "----\n";
    }

    return 0;
}