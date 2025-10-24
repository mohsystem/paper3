#include <bits/stdc++.h>
using namespace std;

struct Event {
    long long timestamp;
    string type;
    int severity;
    string source;
    string message;
};

class SecurityLogger {
private:
    vector<Event> events;
public:
    int logEvent(long long timestamp, const string& type, int severity, const string& source, const string& message) {
        events.push_back(Event{timestamp, type, severity, source, message});
        return (int)events.size() - 1;
    }

    int totalEvents() const {
        return (int)events.size();
    }

    int countByType(const string& type) const {
        int c = 0;
        for (const auto& e : events) if (e.type == type) c++;
        return c;
    }

    int countBySeverityAtLeast(int minSeverity) const {
        int c = 0;
        for (const auto& e : events) if (e.severity >= minSeverity) c++;
        return c;
    }

    int countFromSource(const string& source) const {
        int c = 0;
        for (const auto& e : events) if (e.source == source) c++;
        return c;
    }

    int countInRange(long long start, long long end) const {
        int c = 0;
        for (const auto& e : events) if (e.timestamp >= start && e.timestamp <= end) c++;
        return c;
    }

    int countInWindow(long long endTime, long long windowSeconds) const {
        long long startExclusive = endTime - windowSeconds;
        int c = 0;
        for (const auto& e : events) if (e.timestamp > startExclusive && e.timestamp <= endTime) c++;
        return c;
    }

    bool isAnomalous(long long endTime, long long windowSeconds, int threshold) const {
        return countInWindow(endTime, windowSeconds) > threshold;
    }
};

int main() {
    SecurityLogger logger;
    long long t0 = 1700000000LL;

    logger.logEvent(t0 + 10, "LOGIN_FAILURE", 3, "web", "Invalid password");
    logger.logEvent(t0 + 20, "ACCESS_DENIED", 4, "db", "Unauthorized table access");
    logger.logEvent(t0 + 30, "MALWARE_DETECTED", 5, "av", "Trojan found");
    logger.logEvent(t0 + 40, "LOGIN_SUCCESS", 1, "web", "User login");
    logger.logEvent(t0 + 50, "PORT_SCAN", 4, "ids", "Scan detected");
    logger.logEvent(t0 + 55, "LOGIN_FAILURE", 3, "web", "Invalid password");
    logger.logEvent(t0 + 58, "LOGIN_FAILURE", 3, "vpn", "Failed login");
    logger.logEvent(t0 + 120, "DLP_ALERT", 4, "dlp", "Sensitive data transfer");
    logger.logEvent(t0 + 125, "MALWARE_DETECTED", 5, "av", "Worm found");
    logger.logEvent(t0 + 180, "ACCESS_DENIED", 2, "db", "Restricted view");

    cout << "Test1 totalEvents: " << logger.totalEvents() << "\n";
    cout << "Test2 countByType(LOGIN_FAILURE): " << logger.countByType("LOGIN_FAILURE") << "\n";
    cout << "Test3 countBySeverityAtLeast(4): " << logger.countBySeverityAtLeast(4) << "\n";
    cout << "Test4 countInRange[t0, t0+60]: " << logger.countInRange(t0, t0 + 60) << "\n";
    cout << "Test5 isAnomalous(window=60, end=t0+60, threshold=6): " << (logger.isAnomalous(t0 + 60, 60, 6) ? "true" : "false") << "\n";

    return 0;
}