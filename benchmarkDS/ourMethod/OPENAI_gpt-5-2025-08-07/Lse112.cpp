#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <cstdlib>
#include <cstring>

#if defined(_WIN32)
  #include <windows.h>
#else
  #include <unistd.h>
  #include <sys/wait.h>
  #include <fcntl.h>
#endif

static bool is_digits(const std::string& s) {
    if (s.empty()) return false;
    for (char c : s) if (!std::isdigit(static_cast<unsigned char>(c))) return false;
    return true;
}

static bool is_ipv4(const std::string& s) {
    int dots = 0;
    for (char c : s) if (c == '.') dots++;
    if (dots != 3) return false;
    size_t start = 0;
    for (int i = 0; i < 4; i++) {
        size_t pos = s.find('.', start);
        std::string part = (pos == std::string::npos) ? s.substr(start) : s.substr(start, pos - start);
        if (!is_digits(part) || part.size() > 3) return false;
        int val = std::stoi(part);
        if (val < 0 || val > 255) return false;
        start = (pos == std::string::npos) ? s.size() : pos + 1;
    }
    return true;
}

static bool is_hostname_label(const std::string& label) {
    if (label.empty() || label.size() > 63) return false;
    if (!std::isalnum(static_cast<unsigned char>(label.front()))) return false;
    if (!std::isalnum(static_cast<unsigned char>(label.back()))) return false;
    for (char c : label) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '-')) return false;
    }
    return true;
}

static bool is_hostname(const std::string& s) {
    if (s.empty() || s.size() > 253) return false;
    if (s == "localhost" || s == "LOCALHOST") return true;
    std::string label;
    for (size_t i = 0; i <= s.size(); i++) {
        if (i == s.size() || s[i] == '.') {
            if (!is_hostname_label(label)) return false;
            label.clear();
        } else {
            char c = s[i];
            if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '.')) return false;
            if (c != '.') label.push_back(c);
        }
    }
    return true;
}

static std::string extract_host(const std::string& input) {
    std::string s = input;
    // trim
    auto ltrim = [](const std::string& x) {
        size_t i = 0; while (i < x.size() && std::isspace(static_cast<unsigned char>(x[i]))) i++; return x.substr(i);
    };
    auto rtrim = [](const std::string& x) {
        if (x.empty()) return x;
        size_t i = x.size(); while (i > 0 && std::isspace(static_cast<unsigned char>(x[i-1]))) i--; return x.substr(0, i);
    };
    s = rtrim(ltrim(s));
    if (s.empty()) return "";
    size_t scheme = s.find("://");
    std::string hostPart = (scheme != std::string::npos) ? s.substr(scheme + 3) : s;
    size_t cut = hostPart.find_first_of("/?#");
    if (cut != std::string::npos) hostPart = hostPart.substr(0, cut);
    // final trim
    hostPart = rtrim(ltrim(hostPart));
    return hostPart;
}

static bool is_valid_host(const std::string& host) {
    if (host.empty() || host.size() > 253) return false;
    for (char c : host) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '-')) {
            return false;
        }
    }
    if (host == "localhost" || host == "LOCALHOST") return true;
    if (is_ipv4(host)) return true;
    return is_hostname(host);
}

static bool run_ping(const std::string& host) {
#if defined(_WIN32)
    std::string cmd = "ping -n 1 -w 2000 " + host;
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    // Redirect to NUL by creating inheritable handles is complex; simplest: let it print.
    // For cleanliness, we won't attach console output here.
    ZeroMemory(&pi, sizeof(pi));
    // Create mutable command line
    std::vector<char> cmdline(cmd.begin(), cmd.end());
    cmdline.push_back('\0');
    BOOL ok = CreateProcessA(
        NULL, cmdline.data(), NULL, NULL, FALSE,
        CREATE_NO_WINDOW, NULL, NULL, &si, &pi
    );
    if (!ok) {
        return false;
    }
    DWORD waitRes = WaitForSingleObject(pi.hProcess, 3000);
    if (waitRes == WAIT_TIMEOUT) {
        TerminateProcess(pi.hProcess, 1);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return false;
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return true;
#else
    pid_t pid = fork();
    if (pid < 0) {
        return false;
    } else if (pid == 0) {
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull >= 0) {
            dup2(devnull, STDOUT_FILENO);
            dup2(devnull, STDERR_FILENO);
            close(devnull);
        }
        execlp("ping", "ping", "-c", "1", "-W", "2", host.c_str(), (char*)NULL);
        _exit(127);
    } else {
        int status = 0;
        pid_t w = waitpid(pid, &status, 0);
        if (w < 0) return false;
        return true;
    }
#endif
}

std::string ping_url(const std::string& input) {
    std::string host = extract_host(input);
    if (!is_valid_host(host)) {
        return "Error: invalid or unsafe URL";
    }
    if (!run_ping(host)) {
        return "Error: failed to run ping or timeout";
    }
    return "URL: " + host;
}

int main() {
    // Five test cases
    std::vector<std::string> tests = {
        "http://example.com",
        "example.com",
        "127.0.0.1",
        "bad; rm -rf /",
        "https://localhost/path?q=1"
    };
    for (const auto& t : tests) {
        std::cout << "Input: " << t << " -> " << ping_url(t) << "\n";
    }
    return 0;
}