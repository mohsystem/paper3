import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.regex.Pattern;

public class Task132 {

    // A static nested class for the logger to keep it all in one file.
    static class SecurityLogger {

        private static final String LOG_FILE_NAME = "security.log";
        private static final DateTimeFormatter dtf = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss");
        private static final Object lock = new Object(); // for thread-safety
        // A simple regex to validate an IP address format. Not exhaustive but better than nothing.
        private static final Pattern IP_PATTERN = Pattern.compile(
                "^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$" // IPv4
                + "|^([0-9a-fA-F]{1,4}:){7}[0-9a-fA-F]{1,4}$"); // Basic IPv6

        public enum Severity {
            INFO,
            WARNING,
            CRITICAL
        }

        public enum EventType {
            LOGIN_SUCCESS,
            LOGIN_FAILURE,
            FILE_ACCESS,
            CONFIG_CHANGE,
            UNAUTHORIZED_ATTEMPT
        }

        /**
         * Logs a security event to a file in a thread-safe manner.
         * Note: In a real-world application, use a dedicated logging framework like Log4j or SLF4J.
         * The log file 'security.log' should have its permissions set to be readable/writable
         * only by the application user to prevent unauthorized access.
         *
         * @param severity The severity level of the event.
         * @param eventType The type of the event.
         * @param sourceIp The source IP address where the event originated.
         * @param description A detailed description of the event.
         */
        public static void logEvent(Severity severity, EventType eventType, String sourceIp, String description) {
            // Sanitize inputs to prevent log injection (e.g., remove newlines)
            String sanitizedDescription = description.replace('\n', '_').replace('\r', '_');
            String sanitizedIp = IP_PATTERN.matcher(sourceIp).matches() ? sourceIp : "INVALID_IP";

            String timestamp = dtf.format(LocalDateTime.now());
            String logMessage = String.format("[%s] [%s] [%s] [SRC_IP: %s] - %s%n",
                    timestamp, severity, eventType, sanitizedIp, sanitizedDescription);

            // Synchronized block to ensure thread-safe file writing
            synchronized (lock) {
                // Use try-with-resources to ensure the writer is closed automatically
                try (PrintWriter out = new PrintWriter(new FileWriter(LOG_FILE_NAME, true))) {
                    out.print(logMessage);
                    // In a real monitoring system, CRITICAL events would trigger alerts.
                    if (severity == Severity.CRITICAL) {
                        sendAlert(logMessage);
                    }
                } catch (IOException e) {
                    System.err.println("Error: Failed to write to log file: " + e.getMessage());
                }
            }
        }

        /**
         * Simulates sending an alert for critical events.
         * In a real system, this would integrate with a monitoring tool (e.g., email, PagerDuty).
         * @param alertMessage The message to be sent in the alert.
         */
        private static void sendAlert(String alertMessage) {
            System.out.println("ALERT! Critical security event detected:");
            System.out.println(">> " + alertMessage.trim());
        }
    }

    public static void main(String[] args) {
        System.out.println("Logging security events. Check 'security.log' for details.");
        System.out.println("---------------------------------------------------------");
        
        // Test Case 1: Successful login
        SecurityLogger.logEvent(SecurityLogger.Severity.INFO, SecurityLogger.EventType.LOGIN_SUCCESS, "192.168.1.10", "User 'admin' logged in successfully.");

        // Test Case 2: Failed login attempt
        SecurityLogger.logEvent(SecurityLogger.Severity.WARNING, SecurityLogger.EventType.LOGIN_FAILURE, "10.0.0.5", "Failed login attempt for user 'guest'.");

        // Test Case 3: Critical - Multiple failed attempts (Simulated)
        SecurityLogger.logEvent(SecurityLogger.Severity.CRITICAL, SecurityLogger.EventType.LOGIN_FAILURE, "203.0.113.45", "Multiple (5) failed login attempts for user 'root'. IP blocked.");

        // Test Case 4: Configuration change
        SecurityLogger.logEvent(SecurityLogger.Severity.INFO, SecurityLogger.EventType.CONFIG_CHANGE, "127.0.0.1", "User 'admin' updated 'firewall.conf'.");

        // Test Case 5: Unauthorized access attempt
        SecurityLogger.logEvent(SecurityLogger.Severity.CRITICAL, SecurityLogger.EventType.UNAUTHORIZED_ATTEMPT, "198.51.100.88", "Attempt to access '/api/admin' endpoint without authentication.");

        System.out.println("---------------------------------------------------------");
        System.out.println("Test cases executed.");
    }
}