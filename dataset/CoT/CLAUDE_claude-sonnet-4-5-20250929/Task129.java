
import java.io.*;
import java.nio.file.*;
import java.util.*;

public class Task129 {
    private static final String SAFE_DIRECTORY = "safe_files";
    private static final Set<String> ALLOWED_EXTENSIONS = new HashSet<>(
        Arrays.asList(".txt", ".log", ".csv", ".json", ".xml")
    );
    
    public static String retrieveFile(String fileName) {
        try {
            // Input validation
            if (fileName == null || fileName.trim().isEmpty()) {
                return "Error: File name cannot be empty";
            }
            
            // Remove any path traversal attempts
            fileName = fileName.replaceAll("\\\\.\\\\.", "").replaceAll("[/\\\\\\\\]", "");
            
            // Validate file extension
            if (!isAllowedExtension(fileName)) {
                return "Error: File extension not allowed";
            }
            
            // Create safe directory if it doesn't exist
            File safeDir = new File(SAFE_DIRECTORY);
            if (!safeDir.exists()) {
                safeDir.mkdir();
            }
            
            // Build safe file path
            Path basePath = Paths.get(SAFE_DIRECTORY).toRealPath();
            Path filePath = basePath.resolve(fileName).normalize();
            
            // Verify the file is within the safe directory (prevent path traversal)
            if (!filePath.startsWith(basePath)) {
                return "Error: Access denied - path traversal detected";
            }
            
            // Check if file exists
            File file = filePath.toFile();
            if (!file.exists() || !file.isFile()) {
                return "Error: File not found";
            }
            
            // Read file content with size limit (10MB)
            if (file.length() > 10 * 1024 * 1024) {
                return "Error: File too large";
            }
            
            StringBuilder content = new StringBuilder();
            try (BufferedReader reader = new BufferedReader(new FileReader(file))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    content.append(line).append("\\n");
                }
            }
            
            return "Success: File retrieved\\n" + content.toString();
            
        } catch (IOException e) {
            return "Error: " + e.getMessage();
        }
    }
    
    private static boolean isAllowedExtension(String fileName) {
        for (String ext : ALLOWED_EXTENSIONS) {
            if (fileName.toLowerCase().endsWith(ext)) {
                return true;
            }
        }
        return false;
    }
    
    public static void main(String[] args) {
        // Create test files
        setupTestFiles();
        
        // Test Case 1: Valid file retrieval
        System.out.println("Test 1 - Valid file:");
        System.out.println(retrieveFile("test1.txt"));
        
        // Test Case 2: Path traversal attempt
        System.out.println("\\nTest 2 - Path traversal attempt:");
        System.out.println(retrieveFile("../../../etc/passwd"));
        
        // Test Case 3: Invalid extension
        System.out.println("\\nTest 3 - Invalid extension:");
        System.out.println(retrieveFile("malicious.exe"));
        
        // Test Case 4: Empty filename
        System.out.println("\\nTest 4 - Empty filename:");
        System.out.println(retrieveFile(""));
        
        // Test Case 5: Non-existent file
        System.out.println("\\nTest 5 - Non-existent file:");
        System.out.println(retrieveFile("nonexistent.txt"));
    }
    
    private static void setupTestFiles() {
        try {
            File dir = new File(SAFE_DIRECTORY);
            if (!dir.exists()) dir.mkdir();
            
            File testFile = new File(SAFE_DIRECTORY + "/test1.txt");
            try (FileWriter writer = new FileWriter(testFile)) {
                writer.write("This is a test file content.\\nSecure file retrieval system.");
            }
        } catch (IOException e) {
            System.err.println("Setup error: " + e.getMessage());
        }
    }
}
