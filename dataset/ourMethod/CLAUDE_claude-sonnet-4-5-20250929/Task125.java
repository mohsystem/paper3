
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.HashMap;
import java.util.Map;
import java.util.Scanner;

public class Task125 {
    private static final Map<String, String> RESOURCE_MAP = new HashMap<>();
    private static final Path BASE_DIRECTORY;
    
    static {
        // Initialize allowed resources with their relative paths
        RESOURCE_MAP.put("config", "config.txt");
        RESOURCE_MAP.put("data", "data.txt");
        RESOURCE_MAP.put("log", "log.txt");
        RESOURCE_MAP.put("report", "report.txt");
        RESOURCE_MAP.put("info", "info.txt");
        
        // Set base directory to current working directory
        BASE_DIRECTORY = Paths.get(System.getProperty("user.dir")).toAbsolutePath().normalize();
    }
    
    public static String accessResource(String resourceName) {
        if (resourceName == null || resourceName.trim().isEmpty()) {
            return "ERROR: Resource name cannot be null or empty";
        }
        
        // Validate resource name format (alphanumeric and underscores only)
        if (!resourceName.matches("^[a-zA-Z0-9_]+$")) {
            return "ERROR: Invalid resource name format";
        }
        
        // Check if resource exists in whitelist
        if (!RESOURCE_MAP.containsKey(resourceName)) {
            return "ERROR: Resource not found or access denied";
        }
        
        String relativePath = RESOURCE_MAP.get(resourceName);
        
        try {
            // Construct path safely
            Path requestedPath = BASE_DIRECTORY.resolve(relativePath).normalize();
            
            // Verify the resolved path is still within base directory
            if (!requestedPath.startsWith(BASE_DIRECTORY)) {
                return "ERROR: Path traversal attempt detected";
            }
            
            // Check if path is a regular file (not symlink or directory)
            if (!Files.isRegularFile(requestedPath) || Files.isSymbolicLink(requestedPath)) {
                return "ERROR: Resource is not a regular file";
            }
            
            // Read file content safely
            byte[] content = Files.readAllBytes(requestedPath);
            return new String(content, StandardCharsets.UTF_8);
            
        } catch (IOException e) {
            return "ERROR: Unable to access resource";
        } catch (SecurityException e) {
            return "ERROR: Security violation";
        }
    }
    
    public static void main(String[] args) {
        // Create test files for demonstration
        setupTestFiles();
        
        // Test case 1: Valid resource access
        System.out.println("Test 1 - Access 'config':");
        System.out.println(accessResource("config"));
        System.out.println();
        
        // Test case 2: Valid resource access
        System.out.println("Test 2 - Access 'data':");
        System.out.println(accessResource("data"));
        System.out.println();
        
        // Test case 3: Non-existent resource
        System.out.println("Test 3 - Access 'secret':");
        System.out.println(accessResource("secret"));
        System.out.println();
        
        // Test case 4: Null input
        System.out.println("Test 4 - Null input:");
        System.out.println(accessResource(null));
        System.out.println();
        
        // Test case 5: Invalid format (path traversal attempt)
        System.out.println("Test 5 - Path traversal attempt:");
        System.out.println(accessResource("../etc/passwd"));
        System.out.println();
    }
    
    private static void setupTestFiles() {
        try {
            Files.write(BASE_DIRECTORY.resolve("config.txt"), 
                       "Configuration data".getBytes(StandardCharsets.UTF_8));
            Files.write(BASE_DIRECTORY.resolve("data.txt"), 
                       "Application data".getBytes(StandardCharsets.UTF_8));
            Files.write(BASE_DIRECTORY.resolve("log.txt"), 
                       "Log entries".getBytes(StandardCharsets.UTF_8));
            Files.write(BASE_DIRECTORY.resolve("report.txt"), 
                       "Report content".getBytes(StandardCharsets.UTF_8));
            Files.write(BASE_DIRECTORY.resolve("info.txt"), 
                       "Information text".getBytes(StandardCharsets.UTF_8));
        } catch (IOException e) {
            System.err.println("Warning: Could not create test files");
        }
    }
}
