#include <bits/stdc++.h>
using namespace std;

class Task132 {
public:
    enum class Level { INFO, WARN, ERROR, CRITICAL, ALERT };

    struct Event {
        long long id;
        long long timestampMs;
        Level level;
        string type;
        string source;
        string message;
    };

    class SecurityLoggerMonitor {
    private:
        deque<Event> events;
        size_t maxEvents;
        array<int, 5> stats;
        unordered_map<string, deque<long long>> failedBySource;
        long long bruteWindowMs;
        int bruteThreshold;
        mutex mtx;
        long long nextId;

        static string sanitize(const string& s, size_t maxLen) {
            if (s.empty()) return "";
            string out;
            out.reserve(min(maxLen, s.size()));
            bool lastSpace = false;
            for (char ch : s) {
                if (out.size() >= maxLen) break;
                unsigned char u = static_cast<unsigned char>(ch);
                if (ch == '\r' || ch == '\n') {
                    if (!lastSpace) {
                        out.push_back(' ');
                        lastSpace = true;
                    }
                } else if (u >= 32 && u < 127) {
                    out.push_back(ch);
                    lastSpace = false;
                } else if (isspace(u)) {
                    if (!lastSpace) {
                        out.push_back(' ');
                        lastSpace = true;
                    }
                }
            }
            return out;
        }

        static size_t levelIndex(Level l) {
            switch (l) {
                case Level::INFO: return 0;
                case Level::WARN: return 1;
                case Level::ERROR: return 2;
                case Level::CRITICAL: return 3;
                case Level::ALERT: return 4;
            }
            return 0;
        }

        void appendEventLocked(const Event& e) {
            events.push_back(e);
            stats[levelIndex(e.level)]++;
            while (events.size() > maxEvents) {
                Event rem = events.front(); events.pop_front();
                size_t idx = levelIndex(rem.level);
                stats[idx] = max(0, stats[idx] - 1);
            }
        }

        void checkBruteForceLocked(const string& source, long long ts) {
            auto& dq = failedBySource[source];
            long long cutoff = ts - bruteWindowMs;
            while (!dq.empty() && dq.front() < cutoff) dq.pop_front();
            if ((int)dq.size() >= bruteThreshold) {
                Event alert;
                alert.id = nextId++;
                alert.timestampMs = ts;
                alert.level = Level::ALERT;
                alert.type = "alert_bruteforce";
                alert.source = source;
                alert.message = sanitize(string("Brute force suspected for source=") + source + " count=" + to_string(dq.size()), 256);
                appendEventLocked(alert);
            }
        }

    public:
        SecurityLoggerMonitor(size_t maxEvents_, long long bruteWindowMs_, int bruteThreshold_)
        : maxEvents(max<size_t>(1, maxEvents_)), bruteWindowMs(bruteWindowMs_), bruteThreshold(max(1, bruteThreshold_)), nextId(1) {
            stats.fill(0);
        }

        long long logEvent(const string& source, const string& type, Level level, const string& message, long long timestampMs) {
            string sSource = sanitize(source, 64);
            string sType = sanitize(type, 64);
            string sMsg = sanitize(message, 1024);
            long long ts = timestampMs > 0 ? timestampMs : chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
            lock_guard<mutex> lock(mtx);
            Event e;
            e.id = nextId++;
            e.timestampMs = ts;
            e.level = level;
            e.type = sType;
            e.source = sSource;
            e.message = sMsg;
            appendEventLocked(e);

            if (sType == "failed_login") {
                auto& dq = failedBySource[sSource];
                dq.push_back(ts);
                checkBruteForceLocked(sSource, ts);
            }
            return e.id;
        }

        vector<Event> getRecentEvents(int limit) {
            int lim = max(0, limit);
            lock_guard<mutex> lock(mtx);
            vector<Event> res;
            if (lim <= 0) return res;
            int start = (int)max(0, (int)events.size() - lim);
            res.reserve(events.size() - start);
            for (int i = start; i < (int)events.size(); ++i) res.push_back(events[i]);
            return res;
        }

        array<int,5> getStats() {
            lock_guard<mutex> lock(mtx);
            return stats;
        }

        vector<Event> getAlertsSince(long long sinceTs) {
            lock_guard<mutex> lock(mtx);
            vector<Event> res;
            for (auto& e : events) {
                if (e.level == Level::ALERT && e.timestampMs >= sinceTs) res.push_back(e);
            }
            return res;
        }

        static string levelToString(Level l) {
            switch (l) {
                case Level::INFO: return "INFO";
                case Level::WARN: return "WARN";
                case Level::ERROR: return "ERROR";
                case Level::CRITICAL: return "CRITICAL";
                case Level::ALERT: return "ALERT";
            }
            return "INFO";
        }
    };
};

int main() {
    using Level = Task132::Level;
    using Event = Task132::Event;
    Task132::SecurityLoggerMonitor mon(1024, 5LL * 60LL * 1000LL, 4);
    long long now = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();

    // Test case 1
    mon.logEvent("user1", "login_success", Level::INFO, "User logged in", now + 10);
    // Test case 2
    mon.logEvent("userX", "failed_login", Level::ERROR, "Bad password attempt 1", now + 20);
    // Test case 3
    mon.logEvent("userX", "failed_login", Level::ERROR, "Bad password attempt 2", now + 30);
    // Test case 4
    mon.logEvent("userX", "failed_login", Level::ERROR, "Bad password attempt 3", now + 40);
    // Test case 5 triggers alert
    mon.logEvent("userX", "failed_login", Level::ERROR, "Bad password attempt 4", now + 50);

    auto stats = mon.getStats();
    cout << "Stats:\n";
    cout << "INFO " << stats[0] << "\n";
    cout << "WARN " << stats[1] << "\n";
    cout << "ERROR " << stats[2] << "\n";
    cout << "CRITICAL " << stats[3] << "\n";
    cout << "ALERT " << stats[4] << "\n";

    cout << "Recent events:\n";
    auto recents = mon.getRecentEvents(10);
    for (auto& e : recents) {
        cout << "Event{id=" << e.id << ", ts=" << e.timestampMs
             << ", level=" << Task132::SecurityLoggerMonitor::levelToString(e.level)
             << ", type='" << e.type << "', source='" << e.source
             << "', message='" << e.message << "'}\n";
    }

    cout << "Alerts since now:\n";
    auto alerts = mon.getAlertsSince(now);
    for (auto& e : alerts) {
        cout << "Event{id=" << e.id << ", ts=" << e.timestampMs
             << ", level=" << Task132::SecurityLoggerMonitor::levelToString(e.level)
             << ", type='" << e.type << "', source='" << e.source
             << "', message='" << e.message << "'}\n";
    }
    return 0;
}