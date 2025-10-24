
import java.io.*;
import java.nio.file.*;
import java.security.MessageDigest;
import java.util.*;
import java.util.regex.Pattern;

public class Task129 {
    private static final String BASE_DIRECTORY = "./secure_files/";
    private static final Set<String> ALLOWED_EXTENSIONS = new HashSet<>(
        Arrays.asList(".txt", ".json", ".xml", ".csv", ".log")
    );
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB
    private static final Pattern SAFE_FILENAME_PATTERN = Pattern.compile("^[a-zA-Z0-9_\\\\-\\\\.]+$");
    
    public static String retrieveFile(String fileName) {
        try {
            // Input validation
            if (fileName == null || fileName.trim().isEmpty()) {
                return "Error: File name cannot be empty";
            }
            
            fileName = fileName.trim();
            
            // Check for safe filename pattern
            if (!SAFE_FILENAME_PATTERN.matcher(fileName).matches()) {
                return "Error: Invalid file name format. Only alphanumeric, dash, underscore and dot allowed";
            }
            
            // Check file extension
            String extension = getFileExtension(fileName);
            if (!ALLOWED_EXTENSIONS.contains(extension.toLowerCase())) {
                return "Error: File type not allowed. Allowed types: " + ALLOWED_EXTENSIONS;
            }
            
            // Prevent path traversal attacks
            Path basePath = Paths.get(BASE_DIRECTORY).toAbsolutePath().normalize();
            Path filePath = basePath.resolve(fileName).normalize();
            
            if (!filePath.startsWith(basePath)) {
                return "Error: Path traversal detected. Access denied";
            }
            
            // Check if file exists
            File file = filePath.toFile();
            if (!file.exists()) {
                return "Error: File not found";
            }
            
            // Check if it's a file (not directory)\n            if (!file.isFile()) {\n                return "Error: Invalid file type";\n            }\n            \n            // Check file size\n            if (file.length() > MAX_FILE_SIZE) {\n                return "Error: File size exceeds maximum allowed size";\n            }\n            \n            // Read file content\n            StringBuilder content = new StringBuilder();\n            try (BufferedReader reader = new BufferedReader(new FileReader(file))) {\n                String line;\n                while ((line = reader.readLine()) != null) {\n                    content.append(line).append("\\n");\n                }\n            }\n            \n            return "Success: File retrieved\\nContent:\\n" + content.toString();\n            \n        } catch (IOException e) {\n            return "Error: Unable to read file - " + e.getMessage();\n        } catch (Exception e) {\n            return "Error: Unexpected error - " + e.getMessage();\n        }\n    }\n    \n    private static String getFileExtension(String fileName) {\n        int lastDot = fileName.lastIndexOf('.');
        if (lastDot > 0 && lastDot < fileName.length() - 1) {
            return fileName.substring(lastDot);
        }
        return "";
    }
    
    private static void setupTestEnvironment() {
        try {
            File baseDir = new File(BASE_DIRECTORY);
            if (!baseDir.exists()) {
                baseDir.mkdirs();
            }
            
            // Create test files
            createTestFile("test1.txt", "This is a test file 1");
            createTestFile("test2.json", "{\\"name\\": \\"test\\", \\"value\\": 123}");
            createTestFile("test3.csv", "id,name,age\\n1,John,30\\n2,Jane,25");
        } catch (Exception e) {
            System.err.println("Setup error: " + e.getMessage());
        }
    }
    
    private static void createTestFile(String fileName, String content) throws IOException {
        File file = new File(BASE_DIRECTORY + fileName);
        try (FileWriter writer = new FileWriter(file)) {
            writer.write(content);
        }
    }
    
    public static void main(String[] args) {
        setupTestEnvironment();
        
        System.out.println("=== Secure File Retrieval System ===\\n");
        
        // Test case 1: Valid file retrieval
        System.out.println("Test 1: Valid file retrieval");
        System.out.println(retrieveFile("test1.txt"));
        System.out.println("---\\n");
        
        // Test case 2: Another valid file
        System.out.println("Test 2: Valid JSON file");
        System.out.println(retrieveFile("test2.json"));
        System.out.println("---\\n");
        
        // Test case 3: Path traversal attempt
        System.out.println("Test 3: Path traversal attempt");
        System.out.println(retrieveFile("../../../etc/passwd"));
        System.out.println("---\\n");
        
        // Test case 4: Invalid file extension
        System.out.println("Test 4: Invalid file extension");
        System.out.println(retrieveFile("test.exe"));
        System.out.println("---\\n");
        
        // Test case 5: Non-existent file
        System.out.println("Test 5: Non-existent file");
        System.out.println(retrieveFile("nonexistent.txt"));
        System.out.println("---\\n");
    }
}
