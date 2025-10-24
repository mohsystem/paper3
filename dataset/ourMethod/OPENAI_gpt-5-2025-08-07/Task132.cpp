#include <bits/stdc++.h>
using namespace std;

struct Event {
    uint64_t id;
    int64_t timestampSec;
    string type;
    string severity;
    string source;
    string message;
};

class EventLogger {
public:
    explicit EventLogger(size_t capacity) : capacity_(capacity), nextId_(1) {
        if (capacity_ < 1 || capacity_ > 10000) {
            throw invalid_argument("Invalid capacity");
        }
        // Pre-populate allowed sets
        allowedTypes_ = {
            "LOGIN_SUCCESS", "LOGIN_FAILURE", "ACCESS_DENIED", "DATA_EXPORT", "MALWARE_ALERT", "CONFIG_CHANGE"
        };
        allowedSeverities_ = {"INFO", "LOW", "MEDIUM", "HIGH", "CRITICAL"};
    }

    bool logEvent(const string& type, const string& severity, const string& source, const string& message, int64_t timestampSec) {
        if (!isAllowedType(type) || !isAllowedSeverity(severity)) return false;
        if (!isValidTimestamp(timestampSec)) return false;
        if (!isValidSource(source, 1, 64)) return false;
        if (!isValidMessage(message, 1, 256)) return false;

        if (events_.size() >= capacity_) {
            // drop oldest
            events_.erase(events_.begin());
        }
        Event ev{nextId_++, timestampSec, type, severity, source, message};
        events_.push_back(std::move(ev));
        return true;
    }

    vector<Event> getEventsBySeverity(const string& severity) const {
        if (!isAllowedSeverity(severity)) return {};
        vector<Event> out;
        out.reserve(events_.size());
        for (const auto& e : events_) {
            if (e.severity == severity) out.push_back(e);
        }
        return out;
    }

    vector<Event> searchByKeyword(const string& keyword) const {
        if (!isValidKeyword(keyword, 1, 32)) return {};
        string k = toAsciiLower(keyword);
        vector<Event> out;
        for (const auto& e : events_) {
            string m = toAsciiLower(e.message);
            if (m.find(k) != string::npos) {
                out.push_back(e);
            }
        }
        return out;
    }

    map<string, int> countByType() const {
        map<string, int> m;
        for (const auto& t : allowedTypes_) m[t] = 0;
        for (const auto& e : events_) {
            m[e.type] += 1;
        }
        return m;
    }

    map<string, int> countBySeverity() const {
        map<string, int> m;
        for (const auto& s : allowedSeverities_) m[s] = 0;
        for (const auto& e : events_) {
            m[e.severity] += 1;
        }
        return m;
    }

    vector<Event> getRecent(size_t n) const {
        if (n > capacity_) return {};
        if (n >= events_.size()) return events_;
        return vector<Event>(events_.end() - static_cast<long>(n), events_.end());
    }

private:
    size_t capacity_;
    vector<Event> events_;
    uint64_t nextId_;
    unordered_set<string> allowedTypes_;
    unordered_set<string> allowedSeverities_;
    static constexpr int64_t MIN_TS = 946684800;   // 2000-01-01
    static constexpr int64_t MAX_TS = 4102444800;  // 2100-01-01

    bool isAllowedType(const string& t) const {
        return allowedTypes_.find(t) != allowedTypes_.end();
    }
    bool isAllowedSeverity(const string& s) const {
        return allowedSeverities_.find(s) != allowedSeverities_.end();
    }
    static bool isValidTimestamp(int64_t ts) {
        return ts >= MIN_TS && ts <= MAX_TS;
    }
    static bool isAlphaNum(char c) {
        return (c >= 'A' && c <= 'Z') ||
               (c >= 'a' && c <= 'z') ||
               (c >= '0' && c <= '9');
    }
    static bool isValidSource(const string& s, size_t minLen, size_t maxLen) {
        if (s.size() < minLen || s.size() > maxLen) return false;
        for (char c : s) {
            if (!(isAlphaNum(c) || c == '.' || c == '_' || c == '-')) return false;
        }
        return true;
    }
    static bool isPrintableAscii(char c) {
        return c >= 32 && c <= 126;
    }
    static bool isValidMessage(const string& s, size_t minLen, size_t maxLen) {
        if (s.size() < minLen || s.size() > maxLen) return false;
        for (char c : s) if (!isPrintableAscii(c)) return false;
        return true;
    }
    static bool isValidKeyword(const string& s, size_t minLen, size_t maxLen) {
        return isValidMessage(s, minLen, maxLen);
    }
    static string toAsciiLower(const string& s) {
        string out;
        out.reserve(s.size());
        for (char c : s) {
            if (c >= 'A' && c <= 'Z') out.push_back(static_cast<char>(c + 32));
            else out.push_back(c);
        }
        return out;
    }
};

static string formatCounts(const map<string, int>& m) {
    string out;
    for (const auto& kv : m) {
        out += kv.first + ": " + to_string(kv.second) + "; ";
    }
    return out;
}

int main() {
    try {
        EventLogger logger(100);
        int64_t baseTs = 1700000000;

        // Test 1: valid event
        bool t1 = logger.logEvent("LOGIN_SUCCESS", "INFO", "authsvc", "User alice logged in", baseTs);
        cout << "Test1 success=" << (t1 ? "true" : "false") << "\n";

        // Test 2: invalid type
        bool t2 = logger.logEvent("UNKNOWN_TYPE", "LOW", "guard", "Unknown event", baseTs + 1);
        cout << "Test2 success=" << (t2 ? "true" : "false") << "\n";

        // Test 3: high severity
        bool t3 = logger.logEvent("MALWARE_ALERT", "CRITICAL", "scanner01", "Malware signature detected", baseTs + 2);
        cout << "Test3 success=" << (t3 ? "true" : "false") << "\n";

        // Test 4: search
        (void)logger.logEvent("ACCESS_DENIED", "MEDIUM", "gateway01", "Access denied to resource /admin", baseTs + 3);
        vector<Event> searchRes = logger.searchByKeyword("denied");
        cout << "Test4 search count=" << searchRes.size() << "\n";

        // Test 5: counts and recent
        (void)logger.logEvent("DATA_EXPORT", "HIGH", "exporter", "Data export initiated", baseTs + 4);
        auto byType = logger.countByType();
        auto bySev = logger.countBySeverity();
        cout << "Test5 countByType: " << formatCounts(byType) << "\n";
        cout << "Test5 countBySeverity: " << formatCounts(bySev) << "\n";
        auto recent = logger.getRecent(3);
        cout << "Recent last 3 size=" << recent.size() << "\n";
    } catch (const exception& ex) {
        // Avoid leaking details; print generic error
        cout << "Fatal error\n";
        return 1;
    }
    return 0;
}