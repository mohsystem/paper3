#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <string>
#include <iostream>

static const size_t MAX_INPUT_BYTES = 2048;
static const size_t MAX_USERNAME_CHARS = 64;

std::string http_hello_page(FILE* request, FILE* response) {
    if (request == nullptr || response == nullptr) {
        throw std::invalid_argument("Invalid stream");
    }

    // Read up to first newline or MAX_INPUT_BYTES
    std::string raw;
    raw.reserve(128);
    for (size_t i = 0; i < MAX_INPUT_BYTES; ++i) {
        int c = std::fgetc(request);
        if (c == EOF) break;
        if (c == '\n' || c == '\r') break;
        raw.push_back(static_cast<char>(c));
    }

    // Sanitize: allow [A-Za-z0-9 _.-'], collapse spaces, trim, and limit length
    std::string clean;
    clean.reserve(MAX_USERNAME_CHARS);
    bool lastWasSpace = true; // to trim leading space
    for (char ch : raw) {
        if (clean.size() >= MAX_USERNAME_CHARS) break;
        bool allowed = (ch >= 'A' && ch <= 'Z') ||
                       (ch >= 'a' && ch <= 'z') ||
                       (ch >= '0' && ch <= '9') ||
                       ch == '_' || ch == '-' || ch == '.' || ch == '\'';
        if (allowed) {
            clean.push_back(ch);
            lastWasSpace = false;
        } else if (std::isspace(static_cast<unsigned char>(ch))) {
            if (!lastWasSpace && clean.size() < MAX_USERNAME_CHARS) {
                clean.push_back(' ');
            }
            lastWasSpace = true;
        } else {
            // drop disallowed
        }
    }
    if (!clean.empty() && clean.back() == ' ') {
        clean.pop_back();
    }
    if (clean.empty()) clean = "user";

    std::string responseBody = "Hello " + clean;

    // Write response
    if (!responseBody.empty()) {
        size_t wrote = std::fwrite(responseBody.data(), 1, responseBody.size(), response);
        (void)wrote; // ignore partial write for this context
        std::fflush(response);
    }

    return responseBody;
}

static void run_test(const char* input) {
    FILE* req = std::tmpfile();
    FILE* resp = std::tmpfile();
    if (!req || !resp) {
        std::cerr << "Failed to create temp files" << std::endl;
        if (req) std::fclose(req);
        if (resp) std::fclose(resp);
        return;
    }

    if (input && std::strlen(input) > 0) {
        std::fwrite(input, 1, std::strlen(input), req);
    }
    std::fflush(req);
    std::rewind(req);

    std::string result = http_hello_page(req, resp);

    std::fflush(resp);
    std::rewind(resp);

    char buffer[256];
    std::string written;
    size_t n;
    while ((n = std::fread(buffer, 1, sizeof(buffer), resp)) > 0) {
        written.append(buffer, buffer + n);
    }

    std::cout << "Returned: " << result << "\n";
    std::cout << "Written:  " << written << "\n";
    std::cout << "---" << std::endl;

    std::fclose(req);
    std::fclose(resp);
}

int main() {
    run_test("Alice");
    run_test("  Bob   Smith  \r\n");
    run_test("Eve<script>alert(1)</script>");
    run_test("");
    std::string longInput(500, 'X');
    run_test(longInput.c_str());
    return 0;
}