import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.ArrayList;
import java.util.List;

public class Task132 {

    // Represents a single security event
    static class SecurityEvent {
        private final String timestamp;
        private final String eventType;
        private final String severity;
        private final String description;

        public SecurityEvent(String eventType, String severity, String description) {
            this.timestamp = LocalDateTime.now().format(DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss"));
            this.eventType = eventType;
            this.severity = severity;
            this.description = description;
        }

        @Override
        public String toString() {
            return String.format("[%s] [%s] [%s] %s", timestamp, severity, eventType, description);
        }
    }

    // Manages logging and monitoring of security events
    static class SecurityLogger {
        private final List<SecurityEvent> eventLog;

        public SecurityLogger() {
            this.eventLog = new ArrayList<>();
        }

        /**
         * Creates and logs a new security event.
         * @param eventType The type of the event (e.g., "LOGIN", "FILE_ACCESS").
         * @param severity The severity level (e.g., "INFO", "WARNING", "CRITICAL").
         * @param description A detailed description of the event.
         */
        public void logEvent(String eventType, String severity, String description) {
            SecurityEvent newEvent = new SecurityEvent(eventType, severity, description);
            this.eventLog.add(newEvent);
            // In a real system, this might also write to a file or a remote logging service.
            System.out.println("Logged: " + newEvent);
        }

        /**
         * Displays all the logged events to the console.
         */
        public void displayLogs() {
            System.out.println("\n--- Security Event Log ---");
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

    public static void main(String[] args) {
        SecurityLogger logger = new SecurityLogger();

        System.out.println("--- Running 5 Test Cases ---");

        // Test Case 1: Successful login
        logger.logEvent("LOGIN_SUCCESS", "INFO", "User 'admin' logged in successfully from IP 192.168.1.10");

        // Test Case 2: Failed login attempt
        logger.logEvent("LOGIN_FAILURE", "WARNING", "Failed login attempt for user 'root' from IP 10.0.0.5");

        // Test Case 3: Unauthorized access attempt
        logger.logEvent("UNAUTHORIZED_ACCESS", "CRITICAL", "Attempt to access '/etc/shadow' by user 'guest'");

        // Test Case 4: File modification
        logger.logEvent("FILE_MODIFICATION", "INFO", "User 'dev' modified '/var/www/index.html'");

        // Test Case 5: Firewall rule change
        logger.logEvent("FIREWALL_UPDATE", "WARNING", "Firewall rule added to block port 8080");

        // Display all logged events at the end
        logger.displayLogs();
    }
}