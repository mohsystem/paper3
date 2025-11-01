#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <filesystem>

namespace fs = std::filesystem;

// Validate username: allowed chars and max length
bool isValidUsername(const std::string& username) {
    static const std::regex re("^[A-Za-z0-9_-]{1,32}$");
    return std::regex_match(username, re);
}

// Simple HTML escape
std::string escapeHtml(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 16);
    for (char c : s) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#x27;"; break;
            case '/': out += "&#x2F;"; break;
            default: out += c; break;
        }
    }
    return out;
}

// Create hello.html template in a temp directory
fs::path createTemplateFile() {
    fs::path dir = fs::temp_directory_path() / fs::path("lse132_templates_cpp");
    fs::create_directories(dir);
    fs::path p = dir / "hello.html";
    std::string tpl =
        "<!doctype html>\n"
        "<html>\n"
        "<head><meta charset=\"utf-8\"><title>Hello</title></head>\n"
        "<body>\n"
        "  <h1>Hello, {{ username }}</h1>\n"
        "</body>\n"
        "</html>\n";
    std::ofstream ofs(p, std::ios::binary | std::ios::trunc);
    ofs << tpl;
    ofs.close();
    return p;
}

// Render hello.html with username
std::string renderHello(const std::string& username, const fs::path& templatePath) {
    if (!isValidUsername(username)) {
        return "Invalid username";
    }
    std::ifstream ifs(templatePath, std::ios::binary);
    if (!ifs) {
        return "Template not found";
    }
    std::string tpl((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();
    const std::string placeholder = "{{ username }}";
    std::string safeUser = escapeHtml(username);

    // Replace all occurrences
    std::string out;
    out.reserve(tpl.size() + safeUser.size());
    std::size_t start = 0;
    while (true) {
        std::size_t pos = tpl.find(placeholder, start);
        if (pos == std::string::npos) {
            out.append(tpl, start, std::string::npos);
            break;
        }
        out.append(tpl, start, pos - start);
        out.append(safeUser);
        start = pos + placeholder.size();
    }
    return out;
}

int main() {
    fs::path tmpl = createTemplateFile();

    // 5 test cases
    std::string tests[] = {"Alice", "bob_123", "user-01", "X", "Z9_9-_"};
    for (const auto& t : tests) {
        std::cout << "---- Test: " << t << " ----\n";
        std::cout << renderHello(t, tmpl) << "\n";
    }
    return 0;
}