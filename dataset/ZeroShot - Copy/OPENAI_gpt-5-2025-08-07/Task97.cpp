#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <filesystem>

static bool is_regular_file_no_symlink(const std::filesystem::path& p) {
    std::error_code ec;
    if (std::filesystem::is_symlink(p, ec)) return false;
    return std::filesystem::is_regular_file(p, ec);
}

static std::string read_file(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("Failed to open file");
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static void skip_spaces(const std::string& s, size_t& i) {
    while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r')) ++i;
}

static bool parse_id_from_header(const std::string& header, std::string& outId) {
    size_t i = 0;
    // Expect header starts with "<tag"
    size_t start = header.find("<tag");
    if (start == std::string::npos) return false;
    i = start + 4;
    while (i < header.size()) {
        skip_spaces(header, i);
        if (i >= header.size()) break;
        if (header[i] == '>' || (header[i] == '/' && i + 1 < header.size() && header[i+1] == '>')) break;

        // read attribute name
        size_t nameStart = i;
        while (i < header.size()) {
            char c = header[i];
            if (c == '=' || c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '/' || c == '>') break;
            ++i;
        }
        std::string name = header.substr(nameStart, i - nameStart);
        skip_spaces(header, i);
        if (i >= header.size() || header[i] != '=') {
            // malformed, try to continue
            while (i < header.size() && header[i] != '>' && header[i] != ' ') ++i;
            continue;
        }
        ++i; // skip '='
        skip_spaces(header, i);
        if (i >= header.size()) break;
        char quote = header[i];
        if (quote != '"' && quote != '\'') {
            // malformed attribute value, skip token
            while (i < header.size() && header[i] != '>' && header[i] != ' ') ++i;
            continue;
        }
        ++i;
        size_t valStart = i;
        size_t valEnd = header.find(quote, i);
        if (valEnd == std::string::npos) break;
        std::string value = header.substr(valStart, valEnd - valStart);
        i = valEnd + 1;

        if (name == "id") {
            outId = value;
            return true;
        }
    }
    return false;
}

std::vector<std::string> queryById(const std::string& xmlFilePath, const std::string& idValue) {
    if (!is_regular_file_no_symlink(std::filesystem::path(xmlFilePath))) {
        throw std::runtime_error("Invalid file: not a regular file or is a symbolic link");
    }
    std::string xml = read_file(xmlFilePath);
    std::vector<std::string> results;

    size_t pos = 0;
    while (true) {
        size_t start = xml.find("<tag", pos);
        if (start == std::string::npos) break;
        size_t gt = xml.find('>', start);
        if (gt == std::string::npos) break;

        bool selfClosing = (gt > start && xml[gt - 1] == '/');
        std::string header = xml.substr(start, gt - start + 1);

        std::string foundId;
        if (!parse_id_from_header(header, foundId)) {
            pos = gt + 1;
            continue;
        }
        if (foundId != idValue) {
            pos = gt + 1;
            continue;
        }

        if (selfClosing) {
            results.emplace_back(header);
            pos = gt + 1;
            continue;
        }

        // Find matching closing tag considering nested <tag>
        size_t searchPos = gt + 1;
        int depth = 1;
        while (depth > 0) {
            size_t nextOpen = xml.find("<tag", searchPos);
            size_t nextClose = xml.find("</tag>", searchPos);
            if (nextClose == std::string::npos) break;
            if (nextOpen != std::string::npos && nextOpen < nextClose) {
                // move past this opening's '>'
                size_t nextGt = xml.find('>', nextOpen);
                if (nextGt == std::string::npos) break;
                depth++;
                searchPos = nextGt + 1;
            } else {
                depth--;
                searchPos = nextClose + 6; // length of "</tag>"
            }
        }
        if (depth == 0) {
            std::string element = xml.substr(start, searchPos - start);
            results.emplace_back(std::move(element));
            pos = searchPos;
        } else {
            // malformed; stop
            break;
        }
    }

    return results;
}

int main(int argc, char** argv) {
    try {
        if (argc >= 3) {
            auto res = queryById(argv[1], argv[2]);
            for (const auto& s : res) {
                std::cout << s << "\n";
            }
            return 0;
        }

        const char* sample =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<root>\n"
            "  <tag id=\"1\">Alpha</tag>\n"
            "  <tag id=\"2\"><inner>Beta</inner></tag>\n"
            "  <tag id=\"3\">Gamma</tag>\n"
            "  <tag id=\"42\">FortyTwo</tag>\n"
            "  <tag id=\"x\">X-Value</tag>\n"
            "</root>\n";
        const std::string path = "sample97_cpp.xml";
        {
            std::ofstream out(path, std::ios::binary | std::ios::trunc);
            out << sample;
        }

        std::vector<std::string> testIds = {"1", "2", "3", "42", "nope"};
        for (const auto& id : testIds) {
            auto res = queryById(path, id);
            std::cout << "ID=" << id << " count=" << res.size() << "\n";
            for (const auto& s : res) {
                std::cout << s << "\n";
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}