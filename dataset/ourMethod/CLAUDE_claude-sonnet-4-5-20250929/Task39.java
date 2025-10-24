
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;

public class Task39 {
    
    // Security: Validate filename to prevent command injection and path traversal
    private static boolean isSafeFilename(String filename) {
        if (filename == null || filename.isEmpty() || filename.length() > 255) {
            return false;
        }
        
        // Reject directory traversal attempts
        if (filename.contains("..")) {
            return false;
        }
        
        // Reject absolute paths
        if (filename.startsWith("/") || filename.startsWith("\\\\") || \n            filename.contains(":")) {\n            return false;\n        }\n        \n        // Allow only alphanumeric, underscore, hyphen, and dot\n        for (char c : filename.toCharArray()) {\n            if (!Character.isLetterOrDigit(c) && c != '_' && c != '-' && c != '.') {\n                return false;\n            }\n        }\n        \n        // Reject filenames starting with dot (hidden files)\n        if (filename.startsWith(".")) {\n            return false;\n        }\n        \n        return true;\n    }\n    \n    // Security: Safe file content display without system command\n    private static void displayFileContents(String filename) {\n        // Security: Validate filename before use\n        if (!isSafeFilename(filename)) {\n            System.err.println("Error: Invalid filename format");\n            return;\n        }\n        \n        // Security: Use try-with-resources for automatic resource management\n        Path path = Paths.get(filename);\n        \n        // Security: Check if path resolves to current directory only\n        try {\n            Path normalizedPath = path.normalize();\n            Path absolutePath = normalizedPath.toAbsolutePath();\n            Path currentDir = Paths.get(".").toAbsolutePath().normalize();\n            \n            if (!absolutePath.startsWith(currentDir)) {\n                System.err.println("Error: File must be in current directory");\n                return;\n            }\n        } catch (Exception e) {\n            System.err.println("Error: Invalid path");\n            return;\n        }\n        \n        // Security: Open and read file safely - no Runtime.exec() to prevent command injection\n        try (BufferedReader reader = Files.newBufferedReader(path, StandardCharsets.UTF_8)) {\n            System.out.println("Contents of '" + filename + "':");\n            System.out.println("----------------------------------------");\n            \n            String line;\n            while ((line = reader.readLine()) != null) {\n                System.out.println(line);\n            }\n            \n            System.out.println("----------------------------------------");\n        } catch (IOException e) {\n            System.err.println("Error: Cannot read file '" + filename + "': " + e.getMessage());\n        }\n    }\n    \n    public static void main(String[] args) {\n        // Security: Use try-with-resources for automatic resource cleanup\n        try (BufferedReader consoleReader = new BufferedReader(\n                new InputStreamReader(System.in, StandardCharsets.UTF_8))) {\n            \n            System.out.print("Enter filename to display: ");\n            String filename = consoleReader.readLine();\n            \n            if (filename == null) {\n                System.err.println("Error: Failed to read input");\n                return;\n            }\n            \n            System.out.println("\
=== Test Case 1: User Input ===");\n            displayFileContents(filename.trim());\n            \n        } catch (IOException e) {\n            System.err.println("Error: Input/output error: " + e.getMessage());\n            return;\n        }\n        \n        // Create test files for automated testing\n        try {\n            Files.write(Paths.get("test1.txt"), \n                "This is test file 1.\
Line 2.\
".getBytes(StandardCharsets.UTF_8));\n            Files.write(Paths.get("test2.txt"), \n                "Test file 2 contents.\
".getBytes(StandardCharsets.UTF_8));\n            Files.write(Paths.get("test3.txt"), \n                "Another test file.\
Multiple lines.\
Third line.\
".getBytes(StandardCharsets.UTF_8));\n        } catch (IOException e) {\n            System.err.println("Warning: Could not create test files");\n        }\n        \n        System.out.println("\
=== Test Case 2: Valid filename ===");\n        displayFileContents("test1.txt");\n        \n        System.out.println("\
=== Test Case 3: Another valid filename ===");\n        displayFileContents("test2.txt");\n        \n        System.out.println("\
=== Test Case 4: Invalid filename (path traversal) ===");\n        displayFileContents("../etc/passwd");\n        \n        System.out.println("\
=== Test Case 5: Invalid filename (special chars) ===");\n        displayFileContents("file;rm -rf");\n        \n        System.out.println("\
=== Test Case 6: Non-existent file ===");\n        displayFileContents("nonexistent.txt");
    }
}
