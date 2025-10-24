import java.util.*;
import java.nio.charset.StandardCharsets;

public class Task132 {

    // Event model
    public static final class Event {
        public final long id;
        public final long timestampSec;
        public final String type;
        public final String severity;
        public final String source;
        public final String message;

        public Event(long id, long timestampSec, String type, String severity, String source, String message) {
            this.id = id;
            this.timestampSec = timestampSec;
            this.type = type;
            this.severity = severity;
            this.source = source;
            this.message = message;
        }

        @Override
        public String toString() {
            return "Event{id=" + id + ", ts=" + timestampSec + ", type=" + type + ", sev=" + severity +
                    ", source=" + source + ", msg=" + message + "}";
        }
    }

    public static final class EventLogger {
        private final int capacity;
        private final ArrayDeque<Event> events;
        private long nextId;

        private static final long MIN_TS = 946684800L;   // 2000-01-01
        private static final long MAX_TS = 4102444800L;  // 2100-01-01

        private static final Set<String> ALLOWED_TYPES = Set.of(
                "LOGIN_SUCCESS", "LOGIN_FAILURE", "ACCESS_DENIED", "DATA_EXPORT", "MALWARE_ALERT", "CONFIG_CHANGE"
        );
        private static final Set<String> ALLOWED_SEVERITIES = Set.of(
                "INFO", "LOW", "MEDIUM", "HIGH", "CRITICAL"
        );

        public EventLogger(int capacity) {
            if (capacity < 1 || capacity > 10000) {
                throw new IllegalArgumentException("Invalid capacity");
            }
            this.capacity = capacity;
            this.events = new ArrayDeque<>(capacity);
            this.nextId = 1L;
        }

        public boolean logEvent(String type, String severity, String source, String message, long timestampSec) {
            if (!isAllowedType(type) || !isAllowedSeverity(severity)) {
                return false;
            }
            if (!isValidTimestamp(timestampSec)) {
                return false;
            }
            if (!isValidSource(source, 1, 64)) {
                return false;
            }
            if (!isValidMessage(message, 1, 256)) {
                return false;
            }
            // Trim to capacity (oldest first)
            if (events.size() >= capacity) {
                events.pollFirst();
            }
            Event ev = new Event(nextId++, timestampSec, type, severity, source, message);
            events.addLast(ev);
            return true;
        }

        public List<Event> getEventsBySeverity(String severity) {
            if (!isAllowedSeverity(severity)) {
                return List.of();
            }
            ArrayList<Event> out = new ArrayList<>();
            for (Event e : events) {
                if (e.severity.equals(severity)) {
                    out.add(e);
                }
            }
            return out;
        }

        public List<Event> searchByKeyword(String keyword) {
            if (!isValidKeyword(keyword, 1, 32)) {
                return List.of();
            }
            String k = toAsciiLower(keyword);
            ArrayList<Event> out = new ArrayList<>();
            for (Event e : events) {
                String msgLower = toAsciiLower(e.message);
                if (msgLower.contains(k)) {
                    out.add(e);
                }
            }
            return out;
        }

        public Map<String, Integer> countByType() {
            HashMap<String, Integer> map = new HashMap<>();
            for (String t : ALLOWED_TYPES) {
                map.put(t, 0);
            }
            for (Event e : events) {
                map.put(e.type, map.getOrDefault(e.type, 0) + 1);
            }
            return map;
        }

        public Map<String, Integer> countBySeverity() {
            HashMap<String, Integer> map = new HashMap<>();
            for (String s : ALLOWED_SEVERITIES) {
                map.put(s, 0);
            }
            for (Event e : events) {
                map.put(e.severity, map.getOrDefault(e.severity, 0) + 1);
            }
            return map;
        }

        public List<Event> getRecent(int n) {
            if (n < 0 || n > capacity) {
                return List.of();
            }
            ArrayList<Event> out = new ArrayList<>(Math.min(n, events.size()));
            int skip = Math.max(0, events.size() - n);
            int i = 0;
            for (Event e : events) {
                if (i++ >= skip) out.add(e);
            }
            return out;
        }

        private static boolean isAllowedType(String t) {
            return t != null && ALLOWED_TYPES.contains(t);
        }

        private static boolean isAllowedSeverity(String s) {
            return s != null && ALLOWED_SEVERITIES.contains(s);
        }

        private static boolean isValidTimestamp(long ts) {
            return ts >= MIN_TS && ts <= MAX_TS;
        }

        private static boolean isValidSource(String s, int minLen, int maxLen) {
            if (s == null) return false;
            int len = s.length();
            if (len < minLen || len > maxLen) return false;
            for (int i = 0; i < len; i++) {
                char c = s.charAt(i);
                if (!(isAlphaNum(c) || c == '.' || c == '_' || c == '-')) {
                    return false;
                }
            }
            return true;
        }

        private static boolean isValidMessage(String s, int minLen, int maxLen) {
            if (s == null) return false;
            int len = s.length();
            if (len < minLen || len > maxLen) return false;
            for (int i = 0; i < len; i++) {
                char c = s.charAt(i);
                if (c < 32 || c > 126) {
                    return false;
                }
            }
            return true;
        }

        private static boolean isValidKeyword(String s, int minLen, int maxLen) {
            // keyword for search: printable ASCII only
            return isValidMessage(s, minLen, maxLen);
        }

        private static boolean isAlphaNum(char c) {
            return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
        }

        private static String toAsciiLower(String s) {
            byte[] b = s.getBytes(StandardCharsets.US_ASCII);
            for (int i = 0; i < b.length; i++) {
                if (b[i] >= 'A' && b[i] <= 'Z') {
                    b[i] = (byte) (b[i] + 32);
                }
            }
            return new String(b, StandardCharsets.US_ASCII);
        }
    }

    // Simple formatted summary utility
    public static String formatCounts(Map<String, Integer> map) {
        StringBuilder sb = new StringBuilder();
        ArrayList<String> keys = new ArrayList<>(map.keySet());
        Collections.sort(keys);
        for (String k : keys) {
            sb.append(k).append(": ").append(map.get(k)).append("; ");
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        EventLogger logger = new EventLogger(100);

        long baseTs = 1700000000L; // a safe fixed timestamp (2023)

        // Test 1: valid login success
        boolean t1 = logger.logEvent("LOGIN_SUCCESS", "INFO", "authsvc", "User alice logged in", baseTs);
        System.out.println("Test1 success=" + t1);

        // Test 2: invalid type
        boolean t2 = logger.logEvent("UNKNOWN_TYPE", "LOW", "guard", "Unknown event", baseTs + 1);
        System.out.println("Test2 success=" + t2);

        // Test 3: valid high severity event
        boolean t3 = logger.logEvent("MALWARE_ALERT", "CRITICAL", "scanner01", "Malware signature detected", baseTs + 2);
        System.out.println("Test3 success=" + t3);

        // Test 4: search by keyword
        boolean t4a = logger.logEvent("ACCESS_DENIED", "MEDIUM", "gateway01", "Access denied to resource /admin", baseTs + 3);
        List<Event> searchRes = logger.searchByKeyword("denied");
        System.out.println("Test4 search count=" + searchRes.size());

        // Test 5: counts and recent
        logger.logEvent("DATA_EXPORT", "HIGH", "exporter", "Data export initiated", baseTs + 4);
        Map<String, Integer> byType = logger.countByType();
        Map<String, Integer> bySev = logger.countBySeverity();
        System.out.println("Test5 countByType: " + formatCounts(byType));
        System.out.println("Test5 countBySeverity: " + formatCounts(bySev));
        List<Event> recent = logger.getRecent(3);
        System.out.println("Recent last 3 size=" + recent.size());
    }
}