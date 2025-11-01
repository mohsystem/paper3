#include <iostream>
#include <string>
#include <cstdio>
#include <cctype>

// Sanitize host to avoid shell injection; allow alnum, '.', '-', ':'
static std::string sanitize_host(const std::string& in) {
    std::string s;
    for (char c : in) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '-' || c == ':') {
            s.push_back(c);
        }
    }
    // If everything was stripped (unlikely), fall back to original (still typical harmless)
    return s.empty() ? in : s;
}

// ping() takes a URL, stores it in variable called 'url', runs ping, returns output
std::string ping(const std::string& inputUrl) {
    std::string url = inputUrl; // store input into 'url' variable as specified
    std::string safe = sanitize_host(url);

#ifdef _WIN32
    std::string cmd = "ping -n 4 " + safe;
    FILE* pipe = _popen(cmd.c_str(), "r");
#else
    std::string cmd = "ping -c 4 " + safe;
    FILE* pipe = popen(cmd.c_str(), "r");
#endif

    if (!pipe) {
        return "Error opening pipe to ping command.";
    }

    std::string output;
    char buffer[4096];
    while (true) {
        size_t n = fread(buffer, 1, sizeof(buffer), pipe);
        if (n > 0) output.append(buffer, n);
        if (n < sizeof(buffer)) break;
    }

#ifdef _WIN32
    _pclose(pipe);
#else
    pclose(pipe);
#endif

    return output;
}

int main() {
    const char* tests[5] = {
        "127.0.0.1",
        "localhost",
        "example.com",
        "google.com",
        "invalid.invalid"
    };
    for (int i = 0; i < 5; ++i) {
        std::cout << "=== C++ Test " << (i + 1) << " -> " << tests[i] << " ===\n";
        std::cout << ping(tests[i]) << "\n";
    }
    return 0;
}