import java.util.*;
import java.util.stream.Collectors;

public class Task132 {

    // Simple Event model
    static class Event {
        long timestamp; // epoch seconds
        String type;
        int severity; // 1 (low) .. 5 (critical)
        String source;
        String message;

        Event(long ts, String type, int severity, String source, String message) {
            this.timestamp = ts;
            this.type = type;
            this.severity = severity;
            this.source = source;
            this.message = message;
        }
    }

    // SecurityLogger for logging and monitoring events
    static class SecurityLogger {
        private final List<Event> events = new ArrayList<>();

        // Log a new event; returns the index/id of the logged event
        public synchronized int logEvent(long timestamp, String type, int severity, String source, String message) {
            events.add(new Event(timestamp, type, severity, source, message));
            return events.size() - 1;
        }

        public synchronized int totalEvents() {
            return events.size();
        }

        public synchronized int countByType(String type) {
            int c = 0;
            for (Event e : events) if (e.type.equals(type)) c++;
            return c;
        }

        public synchronized int countBySeverityAtLeast(int minSeverity) {
            int c = 0;
            for (Event e : events) if (e.severity >= minSeverity) c++;
            return c;
        }

        public synchronized int countFromSource(String source) {
            int c = 0;
            for (Event e : events) if (e.source.equals(source)) c++;
            return c;
        }

        // Count events in [start, end] inclusive
        public synchronized int countInRange(long start, long end) {
            int c = 0;
            for (Event e : events) if (e.timestamp >= start && e.timestamp <= end) c++;
            return c;
        }

        // Count events in the window (endTime - windowSeconds, endTime], exclusive of start bound, inclusive of end
        public synchronized int countInWindow(long endTime, long windowSeconds) {
            long startExclusive = endTime - windowSeconds;
            int c = 0;
            for (Event e : events) if (e.timestamp > startExclusive && e.timestamp <= endTime) c++;
            return c;
        }

        // Simple anomaly check: if events in window exceed threshold
        public synchronized boolean isAnomalous(long endTime, long windowSeconds, int threshold) {
            return countInWindow(endTime, windowSeconds) > threshold;
        }

        // Optional: get counts by type (not used in tests but useful)
        public synchronized Map<String, Integer> countsByType() {
            Map<String, Integer> m = new HashMap<>();
            for (Event e : events) m.put(e.type, m.getOrDefault(e.type, 0) + 1);
            return m;
        }
    }

    public static void main(String[] args) {
        SecurityLogger logger = new SecurityLogger();
        long t0 = 1700000000L;

        // Seed events
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

        // 5 Test cases
        System.out.println("Test1 totalEvents: " + logger.totalEvents()); // expect 10
        System.out.println("Test2 countByType(LOGIN_FAILURE): " + logger.countByType("LOGIN_FAILURE")); // expect 3
        System.out.println("Test3 countBySeverityAtLeast(4): " + logger.countBySeverityAtLeast(4)); // expect 5
        System.out.println("Test4 countInRange[t0, t0+60]: " + logger.countInRange(t0, t0 + 60)); // expect 7
        System.out.println("Test5 isAnomalous(window=60, end=t0+60, threshold=6): " + logger.isAnomalous(t0 + 60, 60, 6)); // expect true
    }
}