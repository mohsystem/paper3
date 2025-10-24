
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.*;
import java.io.*;

class SecurityEvent {
    private String eventType;
    private String severity;
    private String source;
    private String description;
    private LocalDateTime timestamp;
    
    public SecurityEvent(String eventType, String severity, String source, String description) {
        this.eventType = eventType;
        this.severity = severity;
        this.source = source;
        this.description = description;
        this.timestamp = LocalDateTime.now();
    }
    
    public String getEventType() { return eventType; }
    public String getSeverity() { return severity; }
    public String getSource() { return source; }
    public String getDescription() { return description; }
    public LocalDateTime getTimestamp() { return timestamp; }
    
    @Override
    public String toString() {
        DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss");
        return String.format("[%s] [%s] [%s] %s - %s",
            timestamp.format(formatter), severity, eventType, source, description);
    }
}

class SecurityMonitor {
    private List<SecurityEvent> eventLog;
    private Map<String, Integer> eventCounts;
    private Map<String, Integer> severityCounts;
    
    public SecurityMonitor() {
        this.eventLog = new ArrayList<>();
        this.eventCounts = new HashMap<>();
        this.severityCounts = new HashMap<>();
    }
    
    public void logEvent(String eventType, String severity, String source, String description) {
        SecurityEvent event = new SecurityEvent(eventType, severity, source, description);
        eventLog.add(event);
        
        eventCounts.put(eventType, eventCounts.getOrDefault(eventType, 0) + 1);
        severityCounts.put(severity, severityCounts.getOrDefault(severity, 0) + 1);
        
        System.out.println("Logged: " + event);
    }
    
    public List<SecurityEvent> getEventsBySeverity(String severity) {
        List<SecurityEvent> result = new ArrayList<>();
        for (SecurityEvent event : eventLog) {
            if (event.getSeverity().equals(severity)) {
                result.add(event);
            }
        }
        return result;
    }
    
    public List<SecurityEvent> getEventsByType(String eventType) {
        List<SecurityEvent> result = new ArrayList<>();
        for (SecurityEvent event : eventLog) {
            if (event.getEventType().equals(eventType)) {
                result.add(event);
            }
        }
        return result;
    }
    
    public Map<String, Integer> getEventStatistics() {
        return new HashMap<>(eventCounts);
    }
    
    public Map<String, Integer> getSeverityStatistics() {
        return new HashMap<>(severityCounts);
    }
    
    public List<SecurityEvent> getAllEvents() {
        return new ArrayList<>(eventLog);
    }
    
    public int getTotalEvents() {
        return eventLog.size();
    }
    
    public void generateReport() {
        System.out.println("\\n=== Security Event Report ===");
        System.out.println("Total Events: " + getTotalEvents());
        System.out.println("\\nEvents by Type:");
        for (Map.Entry<String, Integer> entry : eventCounts.entrySet()) {
            System.out.println("  " + entry.getKey() + ": " + entry.getValue());
        }
        System.out.println("\\nEvents by Severity:");
        for (Map.Entry<String, Integer> entry : severityCounts.entrySet()) {
            System.out.println("  " + entry.getKey() + ": " + entry.getValue());
        }
        System.out.println("============================\\n");
    }
}

public class Task132 {
    public static void main(String[] args) {
        SecurityMonitor monitor = new SecurityMonitor();
        
        // Test Case 1: Log authentication failure
        System.out.println("Test Case 1: Authentication Failure");
        monitor.logEvent("AUTH_FAILURE", "HIGH", "192.168.1.100", "Failed login attempt for user 'admin'");
        
        // Test Case 2: Log successful login
        System.out.println("\\nTest Case 2: Successful Login");
        monitor.logEvent("AUTH_SUCCESS", "INFO", "192.168.1.101", "User 'john.doe' logged in successfully");
        
        // Test Case 3: Log unauthorized access attempt
        System.out.println("\\nTest Case 3: Unauthorized Access");
        monitor.logEvent("UNAUTHORIZED_ACCESS", "CRITICAL", "192.168.1.105", "Attempt to access restricted resource");
        
        // Test Case 4: Log multiple events and generate report
        System.out.println("\\nTest Case 4: Multiple Events");
        monitor.logEvent("AUTH_FAILURE", "HIGH", "192.168.1.102", "Brute force attack detected");
        monitor.logEvent("DATA_ACCESS", "MEDIUM", "192.168.1.103", "Sensitive data accessed by user 'jane.smith'");
        monitor.logEvent("AUTH_FAILURE", "HIGH", "192.168.1.100", "Another failed login for 'admin'");
        
        monitor.generateReport();
        
        // Test Case 5: Query events by severity
        System.out.println("Test Case 5: Query High Severity Events");
        List<SecurityEvent> highSeverityEvents = monitor.getEventsBySeverity("HIGH");
        System.out.println("High Severity Events: " + highSeverityEvents.size());
        for (SecurityEvent event : highSeverityEvents) {
            System.out.println("  " + event);
        }
        
        System.out.println("\\nQuery Events by Type (AUTH_FAILURE):");
        List<SecurityEvent> authFailures = monitor.getEventsByType("AUTH_FAILURE");
        System.out.println("Authentication Failures: " + authFailures.size());
    }
}
