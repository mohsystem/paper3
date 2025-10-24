
import java.io.*;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.*;
import java.security.MessageDigest;
import java.nio.charset.StandardCharsets;

class Task132 {
    private static final String LOG_FILE = "security_events.log";
    private static final DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss");
    
    enum EventType {
        LOGIN_SUCCESS, LOGIN_FAILURE, UNAUTHORIZED_ACCESS, 
        DATA_MODIFICATION, PRIVILEGE_ESCALATION, SYSTEM_ERROR
    }
    
    enum Severity {
        LOW, MEDIUM, HIGH, CRITICAL
    }
    
    static class SecurityEvent {
        private String eventId;
        private LocalDateTime timestamp;
        private EventType eventType;
        private Severity severity;
        private String userId;
        private String ipAddress;
        private String description;
        private String resourceAccessed;
        
        public SecurityEvent(EventType eventType, Severity severity, String userId, 
                           String ipAddress, String description, String resourceAccessed) {
            this.eventId = generateEventId();
            this.timestamp = LocalDateTime.now();
            this.eventType = eventType;
            this.severity = severity;
            this.userId = sanitizeInput(userId);
            this.ipAddress = sanitizeInput(ipAddress);
            this.description = sanitizeInput(description);
            this.resourceAccessed = sanitizeInput(resourceAccessed);
        }
        
        private String generateEventId() {
            return "EVT-" + UUID.randomUUID().toString().substring(0, 8).toUpperCase();
        }
        
        private String sanitizeInput(String input) {
            if (input == null) return "N/A";
            return input.replaceAll("[\\\\r\\\\n]", " ").trim();
        }
        
        @Override
        public String toString() {
            return String.format("[%s] [%s] [%s] [%s] User: %s | IP: %s | Resource: %s | Description: %s",
                    timestamp.format(formatter), eventId, severity, eventType, 
                    userId, ipAddress, resourceAccessed, description);
        }
        
        public Severity getSeverity() {
            return severity;
        }
    }
    
    static class SecurityLogger {
        private List<SecurityEvent> eventBuffer;
        private int maxBufferSize;
        
        public SecurityLogger(int maxBufferSize) {
            this.eventBuffer = new ArrayList<>();
            this.maxBufferSize = maxBufferSize;
        }
        
        public synchronized void logEvent(SecurityEvent event) {
            eventBuffer.add(event);
            writeToFile(event);
            
            if (event.getSeverity() == Severity.CRITICAL || event.getSeverity() == Severity.HIGH) {
                System.out.println("ALERT: " + event);
            }
            
            if (eventBuffer.size() >= maxBufferSize) {
                flushBuffer();
            }
        }
        
        private void writeToFile(SecurityEvent event) {
            try (FileWriter fw = new FileWriter(LOG_FILE, true);
                 BufferedWriter bw = new BufferedWriter(fw);
                 PrintWriter out = new PrintWriter(bw)) {
                out.println(event.toString());
            } catch (IOException e) {
                System.err.println("Error writing to log file: " + e.getMessage());
            }
        }
        
        private void flushBuffer() {
            eventBuffer.clear();
        }
        
        public List<SecurityEvent> getRecentEvents(int count) {
            int size = eventBuffer.size();
            int start = Math.max(0, size - count);
            return new ArrayList<>(eventBuffer.subList(start, size));
        }
        
        public void generateReport() {
            Map<EventType, Integer> eventCounts = new HashMap<>();
            Map<Severity, Integer> severityCounts = new HashMap<>();
            
            for (SecurityEvent event : eventBuffer) {
                eventCounts.put(event.eventType, eventCounts.getOrDefault(event.eventType, 0) + 1);
                severityCounts.put(event.severity, severityCounts.getOrDefault(event.severity, 0) + 1);
            }
            
            System.out.println("\\n=== Security Event Report ===");
            System.out.println("Total Events: " + eventBuffer.size());
            System.out.println("\\nEvents by Type:");
            eventCounts.forEach((type, count) -> System.out.println("  " + type + ": " + count));
            System.out.println("\\nEvents by Severity:");
            severityCounts.forEach((sev, count) -> System.out.println("  " + sev + ": " + count));
            System.out.println("============================\\n");
        }
    }
    
    public static void main(String[] args) {
        SecurityLogger logger = new SecurityLogger(100);
        
        // Test Case 1: Login Success
        SecurityEvent event1 = new SecurityEvent(
            EventType.LOGIN_SUCCESS, Severity.LOW, "user123", 
            "192.168.1.100", "User logged in successfully", "/login"
        );
        logger.logEvent(event1);
        
        // Test Case 2: Failed Login Attempt
        SecurityEvent event2 = new SecurityEvent(
            EventType.LOGIN_FAILURE, Severity.MEDIUM, "admin", 
            "10.0.0.50", "Failed login attempt - invalid password", "/login"
        );
        logger.logEvent(event2);
        
        // Test Case 3: Unauthorized Access Attempt
        SecurityEvent event3 = new SecurityEvent(
            EventType.UNAUTHORIZED_ACCESS, Severity.HIGH, "user456", 
            "172.16.0.200", "Attempted to access restricted resource", "/admin/config"
        );
        logger.logEvent(event3);
        
        // Test Case 4: Critical Privilege Escalation
        SecurityEvent event4 = new SecurityEvent(
            EventType.PRIVILEGE_ESCALATION, Severity.CRITICAL, "user789", 
            "203.0.113.42", "Attempted privilege escalation detected", "/admin/users"
        );
        logger.logEvent(event4);
        
        // Test Case 5: Data Modification
        SecurityEvent event5 = new SecurityEvent(
            EventType.DATA_MODIFICATION, Severity.MEDIUM, "user123", 
            "192.168.1.100", "Modified user profile settings", "/profile/edit"
        );
        logger.logEvent(event5);
        
        // Generate and display report
        logger.generateReport();
        
        System.out.println("Recent Events:");
        List<SecurityEvent> recent = logger.getRecentEvents(3);
        for (SecurityEvent event : recent) {
            System.out.println(event);
        }
    }
}
