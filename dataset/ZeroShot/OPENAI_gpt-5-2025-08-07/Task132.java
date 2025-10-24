import java.util.*;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class Task132 {

    public enum Level {
        INFO, WARN, ERROR, CRITICAL, ALERT
    }

    public static final class Event {
        public final long id;
        public final long timestampMs;
        public final Level level;
        public final String type;
        public final String source;
        public final String message;

        public Event(long id, long timestampMs, Level level, String type, String source, String message) {
            this.id = id;
            this.timestampMs = timestampMs;
            this.level = level;
            this.type = type;
            this.source = source;
            this.message = message;
        }

        @Override
        public String toString() {
            return "Event{id=" + id +
                    ", ts=" + timestampMs +
                    ", level=" + level +
                    ", type='" + type + '\'' +
                    ", source='" + source + '\'' +
                    ", message='" + message + '\'' +
                    '}';
        }
    }

    public static final class SecurityLoggerMonitor {
        private final Deque<Event> events = new ArrayDeque<>();
        private final int maxEvents;
        private final EnumMap<Level, Integer> stats = new EnumMap<>(Level.class);
        private final Map<String, Deque<Long>> failedLoginBySource = new HashMap<>();
        private final long bruteForceWindowMs;
        private final int bruteForceThreshold;
        private final Lock lock = new ReentrantLock(true);
        private long nextId = 1;

        public SecurityLoggerMonitor(int maxEvents, long bruteForceWindowMs, int bruteForceThreshold) {
            this.maxEvents = Math.max(1, maxEvents);
            this.bruteForceWindowMs = bruteForceWindowMs;
            this.bruteForceThreshold = Math.max(1, bruteForceThreshold);
            for (Level l : Level.values()) stats.put(l, 0);
        }

        private static String sanitize(String s, int maxLen) {
            if (s == null) return "";
            StringBuilder sb = new StringBuilder(Math.min(s.length(), maxLen));
            int n = 0;
            for (int i = 0; i < s.length() && n < maxLen; i++) {
                char c = s.charAt(i);
                if (c == '\r' || c == '\n') {
                    if (n == 0 || sb.charAt(n - 1) != ' ') {
                        sb.append(' ');
                        n++;
                    }
                } else if (c >= 32 && c < 127) {
                    sb.append(c);
                    n++;
                } else if (Character.isWhitespace(c)) {
                    if (n == 0 || sb.charAt(n - 1) != ' ') {
                        sb.append(' ');
                        n++;
                    }
                }
            }
            return sb.toString();
        }

        private void appendEventLocked(Event e) {
            events.addLast(e);
            stats.put(e.level, stats.get(e.level) + 1);
            while (events.size() > maxEvents) {
                Event removed = events.removeFirst();
                stats.put(removed.level, Math.max(0, stats.get(removed.level) - 1));
            }
        }

        private void checkBruteForceLocked(String source, long timestampMs) {
            Deque<Long> dq = failedLoginBySource.get(source);
            if (dq == null) {
                dq = new ArrayDeque<>();
                failedLoginBySource.put(source, dq);
            }
            long cutoff = timestampMs - bruteForceWindowMs;
            while (!dq.isEmpty() && dq.peekFirst() < cutoff) {
                dq.removeFirst();
            }
            if (dq.size() >= bruteForceThreshold) {
                String msg = "Brute force suspected for source=" + source + " count=" + dq.size();
                Event alert = new Event(nextId++, timestampMs, Level.ALERT,
                        "alert_bruteforce", source, sanitize(msg, 256));
                appendEventLocked(alert);
            }
        }

        public long logEvent(String source, String type, Level level, String message, long timestampMs) {
            String sSource = sanitize(source, 64);
            String sType = sanitize(type, 64);
            String sMsg = sanitize(message, 1024);
            long ts = timestampMs > 0 ? timestampMs : System.currentTimeMillis();

            lock.lock();
            try {
                Event e = new Event(nextId++, ts, level, sType, sSource, sMsg);
                appendEventLocked(e);
                if ("failed_login".equals(sType)) {
                    Deque<Long> dq = failedLoginBySource.get(sSource);
                    if (dq == null) {
                        dq = new ArrayDeque<>();
                        failedLoginBySource.put(sSource, dq);
                    }
                    dq.addLast(ts);
                    checkBruteForceLocked(sSource, ts);
                }
                return e.id;
            } finally {
                lock.unlock();
            }
        }

        public List<Event> getRecentEvents(int limit) {
            int lim = Math.max(0, limit);
            lock.lock();
            try {
                List<Event> list = new ArrayList<>(Math.min(lim, events.size()));
                Iterator<Event> it = events.descendingIterator();
                while (it.hasNext() && list.size() < lim) {
                    list.add(it.next());
                }
                Collections.reverse(list);
                return list;
            } finally {
                lock.unlock();
            }
        }

        public Map<Level, Integer> getStats() {
            lock.lock();
            try {
                return new EnumMap<>(stats);
            } finally {
                lock.unlock();
            }
        }

        public List<Event> getAlertsSince(long sinceTimestampMs) {
            lock.lock();
            try {
                List<Event> res = new ArrayList<>();
                for (Event e : events) {
                    if (e.level == Level.ALERT && e.timestampMs >= sinceTimestampMs) {
                        res.add(e);
                    }
                }
                return res;
            } finally {
                lock.unlock();
            }
        }
    }

    public static void main(String[] args) {
        SecurityLoggerMonitor mon = new SecurityLoggerMonitor(1024, 5 * 60_000L, 4);
        long now = System.currentTimeMillis();

        // Test case 1: INFO login success
        mon.logEvent("user1", "login_success", Level.INFO, "User logged in", now + 10);

        // Test case 2: failed_login #1
        mon.logEvent("userX", "failed_login", Level.ERROR, "Bad password attempt 1", now + 20);

        // Test case 3: failed_login #2
        mon.logEvent("userX", "failed_login", Level.ERROR, "Bad password attempt 2", now + 30);

        // Test case 4: failed_login #3
        mon.logEvent("userX", "failed_login", Level.ERROR, "Bad password attempt 3", now + 40);

        // Test case 5: failed_login #4 triggers alert
        mon.logEvent("userX", "failed_login", Level.ERROR, "Bad password attempt 4", now + 50);

        Map<Level, Integer> stats = mon.getStats();
        System.out.println("Stats:");
        for (Level l : Level.values()) {
            System.out.println(l + ": " + stats.get(l));
        }

        System.out.println("Recent events:");
        for (Task132.Event e : mon.getRecentEvents(10)) {
            System.out.println(e);
        }

        System.out.println("Alerts since 'now':");
        for (Task132.Event e : mon.getAlertsSince(now)) {
            System.out.println(e);
        }
    }
}