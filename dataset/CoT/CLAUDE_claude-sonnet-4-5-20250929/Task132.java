
import java.io.*;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.locks.ReentrantReadWriteLock;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class Task132 {
    private static final int MAX_LOG_SIZE = 10000;
    private static final int MAX_EVENT_LENGTH = 1000;
    private static final String LOG_FILE = "security_events.log";
    
    private final List<SecurityEvent> events;
    private final Map<String, Integer> eventCounters;
    private final ReentrantReadWriteLock lock;
    private final DateTimeFormatter formatter;
    
    public Task132() {
        this.events = Collections.synchronizedList(new ArrayList<>());
        this.eventCounters = new ConcurrentHashMap<>();
        this.lock = new ReentrantReadWriteLock();
        this.formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss");
    }
    
    // Sanitize input to prevent log injection
    private String sanitizeInput(String input) {
        if (input == null) {
            return "";
        }
        
        if (input.length() > MAX_EVENT_LENGTH) {
            input = input.substring(0, MAX_EVENT_LENGTH);
        }
        
        return input.replaceAll("[\\n\\r\\t]", " ")
                   .replaceAll("[^\\\\x20-\\\\x7E]", "")
                   .trim();
    }
    
    // Log security event with validation
    public boolean logEvent(String eventType, String description, String severity, String sourceIP) {
        lock.writeLock().lock();
        try {
            // Validate inputs
            if (eventType == null || eventType.isEmpty() || 
                description == null || description.isEmpty() ||
                severity == null || severity.isEmpty()) {
                return false;
            }
            
            // Sanitize all inputs
            String cleanEventType = sanitizeInput(eventType);
            String cleanDescription = sanitizeInput(description);
            String cleanSeverity = sanitizeInput(severity);
            String cleanSourceIP = sanitizeInput(sourceIP);
            
            // Validate severity level
            if (!isValidSeverity(cleanSeverity)) {
                return false;
            }
            
            // Validate IP format if provided
            if (cleanSourceIP != null && !cleanSourceIP.isEmpty() && !isValidIP(cleanSourceIP)) {
                return false;
            }
            
            LocalDateTime timestamp = LocalDateTime.now();
            SecurityEvent event = new SecurityEvent(
                timestamp, cleanEventType, cleanDescription, cleanSeverity, cleanSourceIP
            );
            
            // Prevent log overflow
            if (events.size() >= MAX_LOG_SIZE) {
                events.remove(0);
            }
            
            events.add(event);
            eventCounters.merge(cleanEventType, 1, Integer::sum);
            
            // Write to file securely
            writeToFile(event);
            
            return true;
        } catch (Exception e) {
            System.err.println("Error logging event: " + e.getMessage());
            return false;
        } finally {
            lock.writeLock().unlock();
        }
    }
    
    // Validate severity level
    private boolean isValidSeverity(String severity) {
        Set<String> validLevels = new HashSet<>(Arrays.asList("LOW", "MEDIUM", "HIGH", "CRITICAL"));
        return validLevels.contains(severity.toUpperCase());
    }
    
    // Validate IP address format
    private boolean isValidIP(String ip) {
        if (ip == null || ip.isEmpty()) {
            return false;
        }
        String ipPattern = "^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$";
        return ip.matches(ipPattern);
    }
    
    // Securely write to log file
    private void writeToFile(SecurityEvent event) {
        try (BufferedWriter writer = new BufferedWriter(new FileWriter(LOG_FILE, true))) {
            writer.write(event.toString());
            writer.newLine();
        } catch (IOException e) {
            System.err.println("Error writing to log file: " + e.getMessage());
        }
    }
    
    // Get events by severity
    public List<SecurityEvent> getEventsBySeverity(String severity) {
        lock.readLock().lock();
        try {
            String cleanSeverity = sanitizeInput(severity);
            List<SecurityEvent> result = new ArrayList<>();
            for (SecurityEvent event : events) {
                if (event.getSeverity().equalsIgnoreCase(cleanSeverity)) {
                    result.add(event);
                }
            }
            return result;
        } finally {
            lock.readLock().unlock();
        }
    }
    
    // Get event count by type
    public int getEventCount(String eventType) {
        String cleanEventType = sanitizeInput(eventType);
        return eventCounters.getOrDefault(cleanEventType, 0);
    }
    
    // Get all events
    public List<SecurityEvent> getAllEvents() {
        lock.readLock().lock();
        try {
            return new ArrayList<>(events);
        } finally {
            lock.readLock().unlock();
        }
    }
    
    // Generate hash for event integrity
    private String generateHash(String data) {
        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            byte[] hash = digest.digest(data.getBytes());
            StringBuilder hexString = new StringBuilder();
            for (byte b : hash) {
                String hex = Integer.toHexString(0xff & b);
                if (hex.length() == 1) hexString.append('0');
                hexString.append(hex);
            }
            return hexString.toString();
        } catch (NoSuchAlgorithmException e) {
            return "";
        }
    }
    
    // Inner class for security events
    static class SecurityEvent {
        private final LocalDateTime timestamp;
        private final String eventType;
        private final String description;
        private final String severity;
        private final String sourceIP;
        
        public SecurityEvent(LocalDateTime timestamp, String eventType, 
                           String description, String severity, String sourceIP) {
            this.timestamp = timestamp;
            this.eventType = eventType;
            this.description = description;
            this.severity = severity;
            this.sourceIP = sourceIP;
        }
        
        public String getSeverity() {
            return severity;
        }
        
        @Override
        public String toString() {
            DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss");
            return String.format("[%s] [%s] [%s] %s - Source: %s",
                timestamp.format(formatter), severity, eventType, description, 
                sourceIP != null ? sourceIP : "N/A");
        }
    }
    
    public static void main(String[] args) {
        Task132 monitor = new Task132();
        
        System.out.println("=== Security Event Logging System ===\\n");
        
        // Test Case 1: Valid login failure event
        System.out.println("Test Case 1: Login Failure");
        boolean result1 = monitor.logEvent("LOGIN_FAILURE", "Failed login attempt for user admin", 
                                          "HIGH", "192.168.1.100");
        System.out.println("Event logged: " + result1);
        
        // Test Case 2: Valid unauthorized access event
        System.out.println("\\nTest Case 2: Unauthorized Access");
        boolean result2 = monitor.logEvent("UNAUTHORIZED_ACCESS", 
                                          "Attempted access to restricted resource", 
                                          "CRITICAL", "10.0.0.50");
        System.out.println("Event logged: " + result2);
        
        // Test Case 3: Invalid severity level (should fail)
        System.out.println("\\nTest Case 3: Invalid Severity");
        boolean result3 = monitor.logEvent("DATA_BREACH", "Sensitive data accessed", 
                                          "INVALID_LEVEL", "172.16.0.1");
        System.out.println("Event logged: " + result3);
        
        // Test Case 4: Log injection attempt (should be sanitized)
        System.out.println("\\nTest Case 4: Log Injection Prevention");
        boolean result4 = monitor.logEvent("MALICIOUS_INPUT", 
                                          "Test\\nFake Log Entry\\r\\nAnother Line", 
                                          "MEDIUM", "192.168.1.50");
        System.out.println("Event logged: " + result4);
        
        // Test Case 5: Valid firewall block event
        System.out.println("\\nTest Case 5: Firewall Block");
        boolean result5 = monitor.logEvent("FIREWALL_BLOCK", 
                                          "Blocked suspicious traffic", 
                                          "LOW", "203.0.113.10");
        System.out.println("Event logged: " + result5);
        
        // Display all logged events
        System.out.println("\\n=== All Logged Security Events ===");
        List<Task132.SecurityEvent> allEvents = monitor.getAllEvents();
        for (Task132.SecurityEvent event : allEvents) {
            System.out.println(event);
        }
        
        // Display high severity events
        System.out.println("\\n=== High Severity Events ===");
        List<Task132.SecurityEvent> highEvents = monitor.getEventsBySeverity("HIGH");
        for (Task132.SecurityEvent event : highEvents) {
            System.out.println(event);
        }
        
        // Display event counts
        System.out.println("\\n=== Event Counts ===");
        System.out.println("LOGIN_FAILURE: " + monitor.getEventCount("LOGIN_FAILURE"));
        System.out.println("UNAUTHORIZED_ACCESS: " + monitor.getEventCount("UNAUTHORIZED_ACCESS"));
    }
}
