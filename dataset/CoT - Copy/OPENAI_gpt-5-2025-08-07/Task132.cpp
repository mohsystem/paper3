#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <mutex>
#include <cctype>
#include <filesystem>
#include <random>

enum class SeverityCPP { INFO = 0, WARNING = 1, ALERT = 2, CRITICAL = 3 };

struct EventCPP {
    std::string id;
    long long epochMillis;
    std::string iso8601;
    SeverityCPP severity;
    std::string source;
    std::string action;
    std::string target;
    std::string status;
    std::string details;

    std::string toJson() const {
        auto jsonEscape = [](const std::string &s) {
            std::string out;
            out.reserve(s.size() + 16);
            for (char c : s) {
                switch (c) {
                    case '"': out += "\\\""; break;
                    case '\\': out += "\\\\"; break;
                    case '\b': out += "\\b"; break;
                    case '\f': out += "\\f"; break;
                    case '\n': out += "\\n"; break;
                    case '\r': out += "\\r"; break;
                    case '\t': out += "\\t"; break;
                    default:
                        if (static_cast<unsigned char>(c) < 0x20) {
                            std::ostringstream oss;
                            oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)(unsigned char)c;
                            out += oss.str();
                        } else {
                            out += c;
                        }
                }
            }
            return out;
        };
        std::ostringstream oss;
        oss << "{"
            << "\"id\":\"" << jsonEscape(id) << "\","
            << "\"ts\":\"" << jsonEscape(iso8601) << "\","
            << "\"epochMillis\":" << epochMillis << ","
            << "\"severity\":\"" << (severity == SeverityCPP::INFO ? "INFO" :
                                      severity == SeverityCPP::WARNING ? "WARNING" :
                                      severity == SeverityCPP::ALERT ? "ALERT" : "CRITICAL") << "\","
            << "\"source\":\"" << jsonEscape(source) << "\","
            << "\"action\":\"" << jsonEscape(action) << "\","
            << "\"target\":\"" << jsonEscape(target) << "\","
            << "\"status\":\"" << jsonEscape(status) << "\","
            << "\"details\":\"" << jsonEscape(details) << "\""
            << "}";
        return oss.str();
    }
};

class SecurityLoggerCPP {
public:
    SecurityLoggerCPP(const std::string &path = "security.log", size_t maxBytes = 256 * 1024, size_t maxRecent = 1000)
        : logPath(std::filesystem::absolute(path)), rotatedPath(logPath.string() + ".1"),
          maxLogBytes(maxBytes), maxRecentEvents(maxRecent) {
        counts.fill(0);
    }

    EventCPP logEvent(const std::string &source, const std::string &action, const std::string &target,
                      const std::string &status, const std::string &details, SeverityCPP severity) {
        std::string src = sanitize(source);
        std::string act = sanitize(action);
        std::string tgt = sanitize(target);
        std::string stat = sanitize(status);
        std::string det = redactSecrets(sanitize(details));
        if (src.empty()) src = "unknown";
        if (act.empty()) act = "unknown";
        if (tgt.empty()) tgt = "unknown";
        if (stat.empty()) stat = "unknown";

        auto now = std::chrono::system_clock::now();
        long long epochMs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm gmt{};
    #if defined(_WIN32)
        gmtime_s(&gmt, &t);
    #else
        gmt = *std::gmtime(&t);
    #endif
        std::ostringstream iso;
        iso << std::put_time(&gmt, "%Y-%m-%dT%H:%M:%S");
        auto ms = epochMs % 1000;
        iso << "." << std::setw(3) << std::setfill('0') << ms << "Z";

        EventCPP ev{
            generateId(), epochMs, iso.str(), severity, src, act, tgt, stat, det
        };

        {
            std::lock_guard<std::mutex> lg(mu);
            rotateIfNeeded();
            std::ofstream ofs(logPath, std::ios::app);
            if (ofs.good()) {
                ofs << ev.toJson() << "\n";
            }
            recent.push_back(ev);
            if (recent.size() > maxRecentEvents) recent.pop_front();
            counts[static_cast<size_t>(severity)]++;
        }
        return ev;
    }

    std::vector<EventCPP> getRecentEvents(size_t n) {
        std::vector<EventCPP> out;
        std::lock_guard<std::mutex> lg(mu);
        if (n == 0) return out;
        size_t k = std::min(n, recent.size());
        out.reserve(k);
        for (size_t i = 0; i < k; ++i) {
            out.push_back(recent[recent.size() - 1 - i]);
        }
        return out;
    }

    std::array<int, 4> getSeverityCounts() {
        std::lock_guard<std::mutex> lg(mu);
        return counts;
    }

    std::vector<std::string> detectAnomalies(int windowSeconds, int thresholdFailuresPerSource) {
        long long nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
        long long floor = nowMs - std::max(1, windowSeconds) * 1000LL;
        std::unordered_map<std::string, int> failures;
        {
            std::lock_guard<std::mutex> lg(mu);
            for (const auto &e : recent) {
                if (e.epochMillis >= floor && strCaseEq(e.status, "FAILURE")) {
                    failures[e.source]++;
                }
            }
        }
        std::vector<std::string> alerts;
        for (const auto &kv : failures) {
            if (kv.second >= thresholdFailuresPerSource) {
                std::ostringstream oss;
                oss << "Anomaly: source=" << kv.first << " failures=" << kv.second
                    << " withinLastSec=" << windowSeconds;
                alerts.push_back(oss.str());
            }
        }
        return alerts;
    }

private:
    std::filesystem::path logPath;
    std::filesystem::path rotatedPath;
    size_t maxLogBytes;
    size_t maxRecentEvents;
    std::deque<EventCPP> recent;
    std::array<int, 4> counts;
    std::mutex mu;

    static bool strCaseEq(const std::string &a, const std::string &b) {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i) {
            if (std::tolower((unsigned char)a[i]) != std::tolower((unsigned char)b[i])) return false;
        }
        return true;
    }

    static std::string sanitize(const std::string &s) {
        std::string out;
        out.reserve(std::min<size_t>(s.size(), 1024));
        for (char c : s) {
            if (c == '\n' || c == '\r' || c == '\t' || static_cast<unsigned char>(c) >= 0x20) {
                out.push_back(c);
            }
            if (out.size() >= 1024) break;
        }
        // trim
        while (!out.empty() && std::isspace((unsigned char)out.back())) out.pop_back();
        size_t start = 0;
        while (start < out.size() && std::isspace((unsigned char)out[start])) start++;
        return out.substr(start);
    }

    static std::string redactKey(const std::string &input, const std::string &key) {
        std::string lower = input;
        std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c){ return std::tolower(c); });
        std::string k = key;
        std::transform(k.begin(), k.end(), k.begin(), [](unsigned char c){ return std::tolower(c); });

        std::string out;
        size_t i = 0;
        while (i < input.size()) {
            size_t idx = lower.find(k, i);
            if (idx == std::string::npos) {
                out.append(input, i, input.size() - i);
                break;
            }
            out.append(input, i, idx - i);
            size_t j = idx + k.size();
            while (j < input.size() && std::isspace((unsigned char)input[j])) j++;
            if (j < input.size() && input[j] == '=') {
                j++;
                while (j < input.size() && std::isspace((unsigned char)input[j])) j++;
                out.append(input, idx, j - idx);
                out += "******";
                while (j < input.size()) {
                    char c = input[j];
                    if (c == '&' || c == ',' || c == ';' || std::isspace((unsigned char)c)) break;
                    j++;
                }
                i = j;
            } else {
                out.append(input, idx, k.size());
                i = idx + k.size();
            }
        }
        return out;
    }

    static std::string redactSecrets(const std::string &s) {
        std::string r = s;
        const char* keys[] = {"password", "token", "secret", "apikey", "api_key", "authorization", "auth"};
        for (auto key : keys) {
            r = redactKey(r, key);
        }
        return r;
    }

    static std::string generateId() {
        std::random_device rd;
        std::array<unsigned char, 16> bytes{};
        for (auto &b : bytes) b = static_cast<unsigned char>(rd());
        static const char* hex = "0123456789abcdef";
        std::string out;
        out.resize(32);
        for (size_t i = 0; i < 16; ++i) {
            out[2 * i] = hex[(bytes[i] >> 4) & 0xF];
            out[2 * i + 1] = hex[bytes[i] & 0xF];
        }
        return out;
    }

    void rotateIfNeeded() {
        try {
            if (std::filesystem::exists(logPath)) {
                auto size = std::filesystem::file_size(logPath);
                if (size >= maxLogBytes) {
                    std::error_code ec;
                    std::filesystem::remove(rotatedPath, ec);
                    std::filesystem::rename(logPath, rotatedPath, ec);
                }
            }
        } catch (...) {
            // ignore rotation failures
        }
    }
};

int main() {
    SecurityLoggerCPP logger;

    logger.logEvent("auth-service", "LOGIN", "user:alice", "SUCCESS", "login via SSO", SeverityCPP::INFO);
    logger.logEvent("webapp1", "LOGIN", "user:bob", "FAILURE", "bad credentials password=supersecret", SeverityCPP::WARNING);
    logger.logEvent("webapp1", "LOGIN", "user:bob", "FAILURE", "ip=10.0.0.5 token=abcd1234", SeverityCPP::WARNING);
    logger.logEvent("webapp1", "LOGIN", "user:bob", "FAILURE", "too many attempts", SeverityCPP::ALERT);
    logger.logEvent("ids", "INTRUSION", "host:db1", "DETECTED", "signature matched secret=mysecret; action=blocked", SeverityCPP::CRITICAL);

    auto counts = logger.getSeverityCounts();
    std::cout << "Severity counts: "
              << "{INFO=" << counts[0] << ", WARNING=" << counts[1] << ", ALERT=" << counts[2] << ", CRITICAL=" << counts[3] << "}\n";

    auto last5 = logger.getRecentEvents(5);
    std::cout << "Recent events:\n";
    for (const auto &e : last5) {
        std::cout << e.toJson() << "\n";
    }

    auto anomalies = logger.detectAnomalies(300, 3);
    std::cout << "Anomalies:\n";
    for (const auto &a : anomalies) {
        std::cout << a << "\n";
    }

    return 0;
}