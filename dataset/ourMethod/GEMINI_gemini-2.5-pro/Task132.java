import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.channels.FileLock;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.time.Instant;
import java.time.ZoneOffset;
import java.time.format.DateTimeFormatter;
import java.util.Collections;
import java.util.List;

public final class Task132 {

    private static final String LOG_FILE_NAME = "security_events.log";
    private static final int MAX_DESCRIPTION_LENGTH = 256;
    private static final Path LOG_PATH = Paths.get(LOG_FILE_NAME);

    public enum Severity {
        INFO,
        WARN,
        CRITICAL
    }

    /**
     * Sanitizes a string input by removing newline characters to prevent log injection.
     * @param input The string to sanitize.
     * @return The sanitized string.
     */
    private static String sanitize(String input) {
        if (input == null) {
            return "";
        }
        return input.replace('\n', '_').replace('\r', '_');
    }

    /**
     * Logs a security event to a file with proper locking and sanitization.
     *
     * @param eventType   A short string identifying the type of event.
     * @param severity    The severity level of the event.
     * @param description A detailed description of the event.
     * @return true if logging was successful, false otherwise.
     */
    public static boolean logEvent(String eventType, Severity severity, String description) {
        // 1. Input Validation
        if (eventType == null || eventType.trim().isEmpty() || severity == null || description == null) {
            System.err.println("Error: Event details cannot be null or empty.");
            return false;
        }
        if (description.length() > MAX_DESCRIPTION_LENGTH) {
            System.err.println("Error: Description exceeds maximum length of " + MAX_DESCRIPTION_LENGTH);
            return false;
        }

        // 2. Sanitize inputs
        String sanitizedEventType = sanitize(eventType);
        String sanitizedDescription = sanitize(description);

        // 3. Create log entry
        String timestamp = DateTimeFormatter.ISO_INSTANT.format(Instant.now());
        String logEntry = String.format("[%s] [%s] [%s]: %s%n",
                timestamp, severity, sanitizedEventType, sanitizedDescription);

        // 4. Write to file with exclusive lock
        try (RandomAccessFile raf = new RandomAccessFile(LOG_PATH.toFile(), "rw");
             FileChannel channel = raf.getChannel();
             FileLock lock = channel.lock()) { // Exclusive lock

            channel.position(channel.size()); // Move to the end of the file
            channel.write(ByteBuffer.wrap(logEntry.getBytes(StandardCharsets.UTF_8)));

        } catch (IOException e) {
            System.err.println("Failed to write to log file: " + e.getMessage());
            return false;
        }
        return true;
    }

    /**
     * Reads all logged events from the security log file.
     *
     * @return A list of log entries as strings, or an empty list on failure.
     */
    public static List<String> readLogs() {
        if (!Files.exists(LOG_PATH)) {
            return Collections.emptyList();
        }

        // Use a shared lock to allow reading while preventing modification
        try (RandomAccessFile raf = new RandomAccessFile(LOG_PATH.toFile(), "r");
             FileChannel channel = raf.getChannel();
             FileLock lock = channel.lock(0, Long.MAX_VALUE, true)) { // Shared lock
             
            return Files.readAllLines(LOG_PATH, StandardCharsets.UTF_8);

        } catch (IOException e) {
            System.err.println("Failed to read log file: " + e.getMessage());
            return Collections.emptyList();
        }
    }

    public static void main(String[] args) {
        // Clean up previous log file for a fresh run
        try {
            Files.deleteIfExists(LOG_PATH);
        } catch (IOException e) {
            System.err.println("Could not delete old log file: " + e.getMessage());
        }

        System.out.println("--- Running Security Logger Test Cases ---");

        // Test Case 1: Successful login event
        System.out.println("\n1. Logging a successful login event...");
        logEvent("LOGIN_SUCCESS", Severity.INFO, "User 'admin' logged in from 192.168.1.1");

        // Test Case 2: Failed login attempt
        System.out.println("\n2. Logging a failed login attempt...");
        logEvent("LOGIN_FAILURE", Severity.WARN, "Failed login for user 'guest' from 10.0.0.5");

        // Test Case 3: Critical system event
        System.out.println("\n3. Logging a critical event...");
        logEvent("CONFIG_MODIFIED", Severity.CRITICAL, "Critical system file '/etc/shadow' was modified.");

        // Test Case 4: Attempted log injection
        System.out.println("\n4. Logging an event with newline characters (should be sanitized)...");
        logEvent("INPUT_VALIDATION_FAIL", Severity.WARN, "User input contained malicious payload:\n<script>alert(1)</script>");

        // Test Case 5: Event description too long
        System.out.println("\n5. Logging an event with a description that is too long (should fail)...");
        String longDescription = "A".repeat(MAX_DESCRIPTION_LENGTH + 1);
        logEvent("DATA_OVERFLOW", Severity.WARN, longDescription);

        // Read and display all logs
        System.out.println("\n--- Current Security Logs ---");
        List<String> logs = readLogs();
        if (logs.isEmpty()) {
            System.out.println("No logs found or unable to read log file.");
        } else {
            logs.forEach(System.out::println);
        }
        System.out.println("--- End of Logs ---");
    }
}