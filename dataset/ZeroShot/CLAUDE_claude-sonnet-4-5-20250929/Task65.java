
import java.io.*;
import java.nio.file.*;
import java.util.regex.Pattern;

public class Task65 {
    private static final Pattern VALID_FILENAME_PATTERN = Pattern.compile("^[a-zA-Z0-9._-]+$");
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit
    
    public static String readFileSecurely(String filename) {
        if (filename == null || filename.trim().isEmpty()) {
            return "Error: Filename cannot be null or empty";
        }
        
        // Validate filename format (no path traversal)
        if (filename.contains("..") || filename.contains("/") || filename.contains("\\\\")) {\n            return "Error: Invalid filename format - path traversal detected";\n        }\n        \n        // Validate filename characters\n        if (!VALID_FILENAME_PATTERN.matcher(filename).matches()) {\n            return "Error: Filename contains invalid characters";\n        }\n        \n        try {\n            // Get canonical path to prevent path traversal\n            File file = new File(filename);\n            String canonicalPath = file.getCanonicalPath();\n            String currentDir = new File(".").getCanonicalPath();\n            \n            // Ensure file is in current directory\n            if (!canonicalPath.startsWith(currentDir)) {\n                return "Error: File access outside working directory is not allowed";\n            }\n            \n            // Check if file exists\n            if (!file.exists()) {\n                return "Error: File does not exist: " + filename;\n            }\n            \n            // Check if it's a regular file\n            if (!file.isFile()) {\n                return "Error: Path is not a regular file: " + filename;\n            }\n            \n            // Check file size\n            if (file.length() > MAX_FILE_SIZE) {\n                return "Error: File size exceeds maximum allowed size";\n            }\n            \n            // Check read permissions\n            if (!file.canRead()) {\n                return "Error: File is not readable: " + filename;\n            }\n            \n            // Read file content\n            StringBuilder content = new StringBuilder();\n            try (BufferedReader reader = new BufferedReader(new FileReader(file))) {\n                String line;\n                while ((line = reader.readLine()) != null) {\n                    content.append(line).append("\
");\n                }\n            }\n            \n            return content.toString();\n            \n        } catch (IOException e) {\n            return "Error: Failed to read file - " + e.getMessage();\n        } catch (SecurityException e) {\n            return "Error: Security exception - " + e.getMessage();\n        }\n    }\n    \n    public static void main(String[] args) {\n        System.out.println("=== Test Case 1: Valid file ===");\n        try {\n            // Create a test file\n            PrintWriter writer = new PrintWriter("test1.txt");\n            writer.println("Hello World!");\n            writer.println("This is a test file.");\n            writer.close();\n            System.out.println(readFileSecurely("test1.txt"));\n        } catch (Exception e) {\n            System.out.println("Could not create test file");\n        }\n        \n        System.out.println("\
=== Test Case 2: Non-existent file ===");\n        System.out.println(readFileSecurely("nonexistent.txt"));\n        \n        System.out.println("\
=== Test Case 3: Path traversal attempt ===");\n        System.out.println(readFileSecurely("../etc/passwd"));\n        \n        System.out.println("\
=== Test Case 4: Invalid characters ===");\n        System.out.println(readFileSecurely("test@#$.txt"));\n        \n        System.out.println("\
=== Test Case 5: Null/Empty filename ===");\n        System.out.println(readFileSecurely(null));\n        System.out.println(readFileSecurely(""));
    }
}
