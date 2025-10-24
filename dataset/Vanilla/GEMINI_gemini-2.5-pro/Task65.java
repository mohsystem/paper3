import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.PrintWriter;
import java.nio.file.Files;
import java.nio.file.Paths;

public class Task65 {

    /**
     * Retrieves a filename, validates it, and returns the file's content.
     *
     * @param filename The name of the file to read.
     * @return A string containing the file content or an error message.
     */
    public static String readFileContent(String filename) {
        if (filename == null || filename.trim().isEmpty()) {
            return "Error: Filename cannot be null or empty.";
        }

        File file = new File(filename);

        if (!file.exists()) {
            return "Error: File does not exist: " + filename;
        }
        if (file.isDirectory()) {
            return "Error: Path points to a directory, not a file: " + filename;
        }
        if (!file.canRead()) {
            return "Error: Cannot read file (permission denied): " + filename;
        }

        StringBuilder content = new StringBuilder();
        try (BufferedReader reader = new BufferedReader(new FileReader(file))) {
            String line;
            while ((line = reader.readLine()) != null) {
                content.append(line).append(System.lineSeparator());
            }
        } catch (IOException e) {
            return "Error: An I/O error occurred while reading the file: " + e.getMessage();
        }

        return content.toString();
    }

    // Helper method to create a test file
    private static void createTestFile(String filename, String content) throws IOException {
        try (PrintWriter out = new PrintWriter(filename)) {
            out.println(content);
        }
    }
    
    // Helper method to delete a file or directory
    private static void cleanup(String path) {
        try {
            File file = new File(path);
            if (file.isDirectory()) {
                Files.deleteIfExists(Paths.get(path));
            } else {
                file.delete();
            }
        } catch (IOException e) {
            System.err.println("Cleanup failed for: " + path);
        }
    }

    public static void main(String[] args) {
        // Example with command-line arguments
        if (args.length > 0) {
            System.out.println("--- Reading from command line argument ---");
            String filename = args[0];
            System.out.println("Reading file: " + filename);
            String content = readFileContent(filename);
            System.out.println("Content:\n" + content);
            System.out.println("----------------------------------------\n");
        } else {
            System.out.println("Usage: java Task65 <filename>\n");
        }

        System.out.println("--- Running 5 Test Cases ---");

        // Test Case 1: Valid and existing file
        System.out.println("\n--- Test Case 1: Valid File ---");
        String validFile = "test_valid.txt";
        try {
            createTestFile(validFile, "Hello World!\nThis is a test file.");
            System.out.println("Result: \n" + readFileContent(validFile));
        } catch (IOException e) {
            System.out.println("Test setup failed: " + e.getMessage());
        } finally {
            cleanup(validFile);
        }

        // Test Case 2: Non-existent file
        System.out.println("\n--- Test Case 2: Non-existent File ---");
        System.out.println("Result: " + readFileContent("nonexistent.txt"));

        // Test Case 3: Filename is a directory
        System.out.println("\n--- Test Case 3: Path is a Directory ---");
        String dirName = "test_dir";
        File dir = new File(dirName);
        try {
            if (dir.mkdir()) {
                System.out.println("Result: " + readFileContent(dirName));
            } else {
                 System.out.println("Test setup failed: Could not create directory.");
            }
        } finally {
            cleanup(dirName);
        }
        
        // Test Case 4: Null filename
        System.out.println("\n--- Test Case 4: Null Filename ---");
        System.out.println("Result: " + readFileContent(null));

        // Test Case 5: Empty filename
        System.out.println("\n--- Test Case 5: Empty/Whitespace Filename ---");
        System.out.println("Result: " + readFileContent("  "));
        
        System.out.println("\n--- All tests completed ---");
    }
}