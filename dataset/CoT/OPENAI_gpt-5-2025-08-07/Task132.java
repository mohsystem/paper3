import java.io.BufferedWriter;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.security.SecureRandom;
import java.time.Instant;
import java.time.ZoneOffset;
import java.time.format.DateTimeFormatter;
import java.util.*;

public class Task132 {
    public enum Severity {
        INFO, WARNING, ALERT, CRITICAL
    }

    public static final class Event {
        public final String id;
        public final long epochMillis;
        public final String iso8601;
        public final Severity severity;
        public final String source;
        public final String action;
        public final String target;
        public final String status;
        public final String details;

        Event(String id, long epochMillis, String iso8601, Severity sev,
              String source, String action, String target, String status, String details) {
            this.id = id;
            this.epochMillis = epochMillis;
            this.iso8601 = iso8601;
            this.severity = sev;
            this.source = source;
            this.action = action;
            this.target = target;
            this.status = status;
            this.details = details;
        }

        public String toJson() {
            StringBuilder sb = new StringBuilder(256);
            sb.append("{");
            sb.append("\"id\":\"").append(jsonEscape(id)).append("\",");
            sb.append("\"ts\":\"").append(jsonEscape(iso8601)).append("\",");
            sb.append("\"epochMillis\":").append(epochMillis).append(",");
            sb.append("\"severity\":\"").append(severity.name()).append("\",");
            sb.append("\"source\":\"").append(jsonEscape(source)).append("\",");
            sb.append("\"action\":\"").append(jsonEscape(action)).append("\",");
            sb.append("\"target\":\"").append(jsonEscape(target)).append("\",");
            sb.append("\"status\":\"").append(jsonEscape(status)).append("\",");
            sb.append("\"details\":\"").append(jsonEscape(details)).append("\"");
            sb.append("}");
            return sb.toString();
        }

        private static String jsonEscape(String s) {
            StringBuilder out = new StringBuilder(s.length() + 16);
            for (int i = 0; i < s.length(); i++) {
                char c = s.charAt(i);
                switch (c) {
                    case '"': out.append("\\\""); break;
                    case '\\': out.append("\\\\"); break;
                    case '\b': out.append("\\b"); break;
                    case '\f': out.append("\\f"); break;
                    case '\n': out.append("\\n"); break;
                    case '\r': out.append("\\r"); break;
                    case '\t': out.append("\\t"); break;
                    default:
                        if (c < 0x20) {
                            out.append(String.format("\\u%04x", (int) c));
                        } else {
                            out.append(c);
                        }
                }
            }
            return out.toString();
        }
    }

    public static final class SecurityLogger {
        private static final Path LOG_PATH = Paths.get("security.log").normalize();
        private static final Path LOG_ROTATED = Paths.get("security.log.1").normalize();
        private static final long MAX_LOG_BYTES = 256 * 1024;
        private static final int MAX_RECENT = 1000;

        private static final Object LOCK = new Object();
        private static final Deque<Event> RECENT = new ArrayDeque<>();
        private static final EnumMap<Severity, Integer> COUNTS = new EnumMap<>(Severity.class);
        private static final SecureRandom RNG = new SecureRandom();
        private static final DateTimeFormatter ISO_FMT = DateTimeFormatter.ISO_OFFSET_DATE_TIME;

        static {
            for (Severity s : Severity.values()) {
                COUNTS.put(s, 0);
            }
        }

        public static Event logEvent(String source, String action, String target, String status, String details, Severity severity) {
            Objects.requireNonNull(severity, "severity");
            String src = sanitize(source);
            String act = sanitize(action);
            String tgt = sanitize(target);
            String stat = sanitize(status);
            String det = redactSecrets(sanitize(details));
            if (src.isEmpty()) src = "unknown";
            if (act.isEmpty()) act = "unknown";
            if (tgt.isEmpty()) tgt = "unknown";
            if (stat.isEmpty()) stat = "unknown";

            long nowMs = System.currentTimeMillis();
            String iso = ISO_FMT.format(Instant.ofEpochMilli(nowMs).atOffset(ZoneOffset.UTC));
            String id = generateId();

            Event ev = new Event(id, nowMs, iso, severity, src, act, tgt, stat, det);

            synchronized (LOCK) {
                rotateIfNeeded();
                try (Writer w = new BufferedWriter(new OutputStreamWriter(
                        Files.newOutputStream(LOG_PATH, StandardOpenOption.CREATE, StandardOpenOption.APPEND),
                        StandardCharsets.UTF_8))) {
                    w.write(ev.toJson());
                    w.write(System.lineSeparator());
                } catch (IOException e) {
                    // Swallowing exceptions after logging can hide issues; here we fallback to in-memory only.
                }
                RECENT.addLast(ev);
                if (RECENT.size() > MAX_RECENT) {
                    RECENT.removeFirst();
                }
                COUNTS.put(severity, COUNTS.get(severity) + 1);
            }
            return ev;
        }

        public static List<Event> getRecentEvents(int n) {
            if (n <= 0) return Collections.emptyList();
            List<Event> out = new ArrayList<>(Math.min(n, RECENT.size()));
            synchronized (LOCK) {
                Iterator<Event> it = RECENT.descendingIterator();
                while (it.hasNext() && out.size() < n) {
                    out.add(it.next());
                }
            }
            return out;
        }

        public static Map<Severity, Integer> getSeverityCounts() {
            EnumMap<Severity, Integer> copy = new EnumMap<>(Severity.class);
            synchronized (LOCK) {
                copy.putAll(COUNTS);
            }
            return copy;
        }

        public static List<String> detectAnomalies(int windowSeconds, int thresholdFailuresPerSource) {
            long now = System.currentTimeMillis();
            long floor = now - Math.max(1, windowSeconds) * 1000L;
            Map<String, Integer> failureBySource = new HashMap<>();
            synchronized (LOCK) {
                for (Event e : RECENT) {
                    if (e.epochMillis >= floor && "FAILURE".equalsIgnoreCase(e.status)) {
                        failureBySource.merge(e.source, 1, Integer::sum);
                    }
                }
            }
            List<String> alerts = new ArrayList<>();
            for (Map.Entry<String, Integer> en : failureBySource.entrySet()) {
                if (en.getValue() >= thresholdFailuresPerSource) {
                    alerts.add("Anomaly: source=" + en.getKey() + " failures=" + en.getValue() +
                            " withinLastSec=" + windowSeconds);
                }
            }
            return alerts;
        }

        private static String generateId() {
            byte[] b = new byte[16];
            RNG.nextBytes(b);
            StringBuilder sb = new StringBuilder(32);
            for (byte x : b) sb.append(String.format("%02x", x));
            return sb.toString();
        }

        private static void rotateIfNeeded() {
            try {
                long size = Files.exists(LOG_PATH) ? Files.size(LOG_PATH) : 0L;
                if (size >= MAX_LOG_BYTES) {
                    // Best-effort rotate
                    try {
                        if (Files.exists(LOG_ROTATED)) {
                            Files.delete(LOG_ROTATED);
                        }
                    } catch (IOException ignored) {}
                    try {
                        if (Files.exists(LOG_PATH)) {
                            Files.move(LOG_PATH, LOG_ROTATED, StandardCopyOption.ATOMIC_MOVE);
                        }
                    } catch (IOException e) {
                        // Fallback non-atomic move
                        try {
                            if (Files.exists(LOG_PATH)) {
                                Files.move(LOG_PATH, LOG_ROTATED, StandardCopyOption.REPLACE_EXISTING);
                            }
                        } catch (IOException ignored) {}
                    }
                }
            } catch (IOException ignored) {
            }
        }

        private static String sanitize(String s) {
            if (s == null) return "";
            StringBuilder sb = new StringBuilder(Math.min(1024, s.length()));
            for (int i = 0; i < s.length() && sb.length() < 1024; i++) {
                char c = s.charAt(i);
                if (c == '\n' || c == '\r' || c == '\t' || c >= 0x20) {
                    sb.append(c);
                }
            }
            return sb.toString().trim();
        }

        private static String redactSecrets(String s) {
            if (s.isEmpty()) return s;
            // Simple patterns like password=..., token=..., secret=...
            String redacted = s;
            String[] keys = new String[]{"password", "token", "secret", "apikey", "api_key", "authorization", "auth"};
            for (String k : keys) {
                redacted = redactKey(redacted, k);
            }
            return redacted;
        }

        private static String redactKey(String input, String key) {
            String lower = input.toLowerCase(Locale.ROOT);
            String k = key.toLowerCase(Locale.ROOT);
            StringBuilder out = new StringBuilder(input.length());
            int i = 0;
            while (i < input.length()) {
                int idx = lower.indexOf(k, i);
                if (idx < 0) {
                    out.append(input, i, input.length());
                    break;
                }
                out.append(input, i, idx);
                int j = idx + k.length();
                // optional spaces
                while (j < input.length() && Character.isWhitespace(input.charAt(j))) j++;
                if (j < input.length() && input.charAt(j) == '=') {
                    j++;
                    // optional spaces
                    while (j < input.length() && Character.isWhitespace(input.charAt(j))) j++;
                    out.append(input, idx, j);
                    out.append("******");
                    // consume until delimiter
                    while (j < input.length()) {
                        char c = input.charAt(j);
                        if (c == '&' || c == ',' || c == ';' || Character.isWhitespace(c)) break;
                        j++;
                    }
                    i = j;
                } else {
                    out.append(input, idx, idx + k.length());
                    i = idx + k.length();
                }
            }
            return out.toString();
        }
    }

    // Demonstration with 5 test cases
    public static void main(String[] args) {
        SecurityLogger.logEvent("auth-service", "LOGIN", "user:alice", "SUCCESS",
                "login via SSO", Severity.INFO);
        SecurityLogger.logEvent("webapp1", "LOGIN", "user:bob", "FAILURE",
                "bad credentials password=supersecret", Severity.WARNING);
        SecurityLogger.logEvent("webapp1", "LOGIN", "user:bob", "FAILURE",
                "ip=10.0.0.5 token=abcd1234", Severity.WARNING);
        SecurityLogger.logEvent("webapp1", "LOGIN", "user:bob", "FAILURE",
                "too many attempts", Severity.ALERT);
        SecurityLogger.logEvent("ids", "INTRUSION", "host:db1", "DETECTED",
                "signature matched secret=mysecret; action=blocked", Severity.CRITICAL);

        Map<Severity, Integer> counts = SecurityLogger.getSeverityCounts();
        System.out.println("Severity counts: " + counts);

        List<Event> last5 = SecurityLogger.getRecentEvents(5);
        System.out.println("Recent events:");
        for (Event e : last5) {
            System.out.println(e.toJson());
        }

        List<String> anomalies = SecurityLogger.detectAnomalies(300, 3);
        System.out.println("Anomalies:");
        for (String a : anomalies) {
            System.out.println(a);
        }
    }
}