
import java.io.*;
import java.nio.file.*;

public class Task65 {
    public static String readFileFromArgument(String filename) {
        if (filename == null || filename.trim().isEmpty()) {
            return "Error: Filename is null or empty";
        }
        
        // Validate filename format (basic validation)
        if (filename.contains("\\0") || filename.length() > 255) {
            return "Error: Invalid filename format";
        }
        
        File file = new File(filename);
        
        // Check if file exists
        if (!file.exists()) {
            return "Error: File does not exist: " + filename;
        }
        
        // Check if it's a file (not a directory)
        if (!file.isFile()) {
            return "Error: Path is not a file: " + filename;
        }
        
        // Check if file is readable
        if (!file.canRead()) {
            return "Error: File is not readable: " + filename;
        }
        
        try {
            // Read file content
            StringBuilder content = new StringBuilder();
            BufferedReader reader = new BufferedReader(new FileReader(file));
            String line;
            while ((line = reader.readLine()) != null) {
                content.append(line).append("\\n");
            }
            reader.close();
            return content.toString();
        } catch (IOException e) {
            return "Error: Failed to read file: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== Test Case 1: Valid file ===");
        try {
            // Create a test file
            FileWriter writer = new FileWriter("test1.txt");
            writer.write("Hello World!\\nThis is a test file.");
            writer.close();
            String result1 = readFileFromArgument("test1.txt");
            System.out.println(result1);
        } catch (IOException e) {
            System.out.println("Setup error: " + e.getMessage());
        }
        
        System.out.println("\\n=== Test Case 2: Non-existent file ===");
        String result2 = readFileFromArgument("nonexistent.txt");
        System.out.println(result2);
        
        System.out.println("\\n=== Test Case 3: Null filename ===");
        String result3 = readFileFromArgument(null);
        System.out.println(result3);
        
        System.out.println("\\n=== Test Case 4: Empty filename ===");
        String result4 = readFileFromArgument("");
        System.out.println(result4);
        
        System.out.println("\\n=== Test Case 5: Another valid file ===");
        try {
            FileWriter writer = new FileWriter("test2.txt");
            writer.write("Line 1\\nLine 2\\nLine 3");
            writer.close();
            String result5 = readFileFromArgument("test2.txt");
            System.out.println(result5);
        } catch (IOException e) {
            System.out.println("Setup error: " + e.getMessage());
        }
        
        // Cleanup
        try {
            Files.deleteIfExists(Paths.get("test1.txt"));
            Files.deleteIfExists(Paths.get("test2.txt"));
        } catch (IOException e) {
            // Ignore cleanup errors
        }
    }
}
