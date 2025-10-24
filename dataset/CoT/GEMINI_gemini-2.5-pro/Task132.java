import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;

/**
 * Represents a single security event.
 * This class is immutable to ensure the integrity of log entries once created.
 */
final class SecurityEvent {
    private final long timestamp; // Unix timestamp in milliseconds
    private final String eventType;
    private final String userId;
    private final String sourceIp;
    private final String description;

    public SecurityEvent(long timestamp, String eventType, String userId, String sourceIp, String description) {
        // Basic validation to ensure required fields are not null.
        if (eventType == null || userId == null || sourceIp == null || description == null) {
            throw new IllegalArgumentException("Event fields cannot be null.");
        }
        this.timestamp = timestamp;
        this.eventType = eventType;
        this.userId = userId;
        this.sourceIp = sourceIp;
        this.description = description;
    }

    // Getters
    public long getTimestamp() { return timestamp; }
    public String getEventType() { return eventType; }
    public String getUserId() { return userId; }
    public String getSourceIp() { return sourceIp; }
    public String getDescription() { return description; }

    @Override
    public String toString() {
        return String.format("Event(time=%d, type='%s', user='%s', ip='%s', desc='%s')",
                timestamp, eventType, userId, sourceIp, description);
    }
}

/**
 * Handles the logging of security events.
 * Note: This implementation is not thread-safe. In a multi-threaded environment,
 * a synchronized list or a concurrent collection should be used.
 */
class SecurityLogger {
    private final List<SecurityEvent> eventLog;

    public SecurityLogger() {
        this.eventLog = new ArrayList<>();
    }

    /**
     * Sanitizes input strings to prevent log injection attacks by removing newlines.
     * @param input The string to sanitize.
     * @return The sanitized string.
     */
    private String sanitize(String input) {
        if (input == null) {
            return "";
        }
        // Replace newline and carriage return characters to prevent log entry splitting.
        return input.replaceAll("[\n\r]", "_");
    }

    /**
     * Logs a new security event.
     */
    public void logEvent(String eventType, String userId, String sourceIp, String description) {
        long timestamp = System.currentTimeMillis();
        // Sanitize all string inputs before creating the event object.
        SecurityEvent event = new SecurityEvent(
            timestamp,
            sanitize(eventType),
            sanitize(userId),
            sanitize(sourceIp),
            sanitize(description)
        );
        eventLog.add(event);
    }

    public List<SecurityEvent> getEventLog() {
        return new ArrayList<>(eventLog); // Return a copy to prevent external modification
    }

    public void printLogs() {
        System.out.println("--- Security Event Log ---");
        if (eventLog.isEmpty()) {
            System.out.println("No events logged.");
        } else {
            for (SecurityEvent event : eventLog) {
                System.out.println(event);
            }
        }
        System.out.println("--------------------------\n");
    }
}

/**
 * Monitors logs for suspicious patterns.
 */
class SecurityMonitor {
    // Configuration for brute-force detection
    private static final int FAILED_LOGIN_THRESHOLD = 3;
    private static final long TIME_WINDOW_SECONDS = 60;

    /**
     * Analyzes a list of security events for suspicious activity.
     * Specifically checks for multiple failed login attempts from the same IP.
     * @param events The list of events to analyze.
     */
    public void analyzeLogs(List<SecurityEvent> events) {
        System.out.println("--- Security Analysis Report ---");
        Map<String, List<Long>> failedLoginAttempts = new HashMap<>();

        // Filter for LOGIN_FAILURE events and group by source IP
        for (SecurityEvent event : events) {
            if ("LOGIN_FAILURE".equals(event.getEventType())) {
                failedLoginAttempts
                    .computeIfAbsent(event.getSourceIp(), k -> new ArrayList<>())
                    .add(event.getTimestamp());
            }
        }

        boolean alertsFound = false;
        // Analyze the timestamps for each IP
        for (Map.Entry<String, List<Long>> entry : failedLoginAttempts.entrySet()) {
            String ip = entry.getKey();
            List<Long> timestamps = entry.getValue().stream().sorted().collect(Collectors.toList());

            if (timestamps.size() >= FAILED_LOGIN_THRESHOLD) {
                 // Check for events within the time window
                for (int i = 0; i <= timestamps.size() - FAILED_LOGIN_THRESHOLD; i++) {
                    long firstTimestamp = timestamps.get(i);
                    long lastTimestamp = timestamps.get(i + FAILED_LOGIN_THRESHOLD - 1);
                    if (TimeUnit.MILLISECONDS.toSeconds(lastTimestamp - firstTimestamp) <= TIME_WINDOW_SECONDS) {
                        System.out.printf("ALERT: Potential brute-force attack detected from IP: %s. %d failed attempts within %d seconds.\n",
                                ip, FAILED_LOGIN_THRESHOLD, TIME_WINDOW_SECONDS);
                        alertsFound = true;
                        break; // Move to the next IP after finding one pattern
                    }
                }
            }
        }

        if (!alertsFound) {
            System.out.println("No suspicious activity detected.");
        }
        System.out.println("------------------------------\n");
    }
}


public class Task132 {
    public static void main(String[] args) throws InterruptedException {
        SecurityLogger logger = new SecurityLogger();
        SecurityMonitor monitor = new SecurityMonitor();

        // --- Test Case 1: Normal Operations ---
        System.out.println(">>> Running Test Case 1: Normal Operations");
        logger.logEvent("LOGIN_SUCCESS", "alice", "192.168.1.10", "User alice logged in successfully.");
        TimeUnit.SECONDS.sleep(1);
        logger.logEvent("FILE_ACCESS", "alice", "192.168.1.10", "User alice accessed /data/report.docx.");
        TimeUnit.SECONDS.sleep(1);
        logger.logEvent("LOGOUT", "alice", "192.168.1.10", "User alice logged out.");
        logger.printLogs();
        monitor.analyzeLogs(logger.getEventLog());

        // --- Test Case 2: Brute-Force Attack ---
        System.out.println(">>> Running Test Case 2: Brute-Force Attack");
        logger.logEvent("LOGIN_FAILURE", "admin", "203.0.113.55", "Failed login attempt for user admin.");
        TimeUnit.MILLISECONDS.sleep(500);
        logger.logEvent("LOGIN_FAILURE", "root", "203.0.113.55", "Failed login attempt for user root.");
        TimeUnit.MILLISECONDS.sleep(500);
        logger.logEvent("LOGIN_FAILURE", "admin", "203.0.113.55", "Failed login attempt for user admin.");
        TimeUnit.MILLISECONDS.sleep(500);
        logger.logEvent("LOGIN_FAILURE", "guest", "203.0.113.55", "Failed login attempt for user guest.");
        logger.printLogs();
        monitor.analyzeLogs(logger.getEventLog());
        
        // --- Test Case 3: Distributed Attack (should not trigger alert) ---
        System.out.println(">>> Running Test Case 3: Distributed Low-and-Slow Attack");
        logger.logEvent("LOGIN_FAILURE", "user", "198.51.100.1", "Failed login.");
        TimeUnit.SECONDS.sleep(2);
        logger.logEvent("LOGIN_FAILURE", "user", "198.51.100.2", "Failed login.");
        TimeUnit.SECONDS.sleep(2);
        logger.logEvent("LOGIN_FAILURE", "user", "198.51.100.3", "Failed login.");
        logger.printLogs();
        monitor.analyzeLogs(logger.getEventLog());
        
        // --- Test Case 4: Mixed Activity & Successful Intrusion ---
        System.out.println(">>> Running Test Case 4: Mixed Activity & Eventual Success");
        logger.logEvent("LOGIN_FAILURE", "bob", "10.0.0.42", "Failed login: invalid password.");
        TimeUnit.SECONDS.sleep(1);
        logger.logEvent("LOGIN_FAILURE", "bob", "10.0.0.42", "Failed login: invalid password.");
        TimeUnit.SECONDS.sleep(1);
        logger.logEvent("LOGIN_SUCCESS", "bob", "10.0.0.42", "User bob logged in successfully.");
        logger.printLogs();
        monitor.analyzeLogs(logger.getEventLog());
        
        // --- Test Case 5: Permission Escalation Attempt ---
        System.out.println(">>> Running Test Case 5: Permission Escalation Attempt");
        logger.logEvent("LOGIN_SUCCESS", "charlie", "172.16.0.12", "User charlie logged in successfully.");
        TimeUnit.SECONDS.sleep(1);
        logger.logEvent("PERMISSION_DENIED", "charlie", "172.16.0.12", "Access denied for /etc/shadow.");
        logger.printLogs();
        monitor.analyzeLogs(logger.getEventLog());
    }
}