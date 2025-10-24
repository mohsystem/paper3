import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;
import java.nio.file.Files;
import java.nio.file.Paths;

public class Task70 {

    /**
     * Reads the entire content of a file into a string.
     * Note: This method uses Files.readString which is available from Java 11+.
     *
     * @param filePath The path to the file.
     * @return The content of the file as a string, or an error message if the file cannot be read.
     */
    public static String readFileContent(String filePath) {
        try {
            return Files.readString(Paths.get(filePath));
        } catch (IOException e) {
            return "Error: Could not read file '" + filePath + "'. Reason: " + e.getMessage();
        }
    }

    // Helper function to create a test file
    private static void createTestFile(String fileName, String content) throws IOException {
        try (PrintWriter out = new PrintWriter(fileName)) {
            out.print(content);
        }
    }

    public static void main(String[] args) {
        // Handle command-line argument if provided
        if (args.length > 0) {
            String filePath = args[0];
            System.out.println("Reading content from command line argument: " + filePath);
            System.out.println("--- File Content ---");
            System.out.println(readFileContent(filePath));
            System.out.println("--- End of Content ---");
            return; // Exit after processing command line arg
        }

        // Run built-in test cases if no command-line argument is provided
        System.out.println("No command line argument provided. Running built-in test cases...");
        System.out.println("=========================================================");

        // Test Case 1: Simple file
        System.out.println("Test Case 1: Reading a simple file (test1.txt)");
        try {
            String fileName1 = "test1.txt";
            createTestFile(fileName1, "Hello World from Java!");
            System.out.println("Content: " + readFileContent(fileName1));
            new File(fileName1).delete();
        } catch (IOException e) {
            System.out.println("Test Case 1 failed to set up: " + e.getMessage());
        }
        System.out.println("---------------------------------------------------------");

        // Test Case 2: File with multiple lines
        System.out.println("Test Case 2: Reading a multi-line file (test2.txt)");
        try {
            String fileName2 = "test2.txt";
            createTestFile(fileName2, "Line 1\nLine 2\nLine 3");
            System.out.println("Content:\n" + readFileContent(fileName2));
            new File(fileName2).delete();
        } catch (IOException e) {
            System.out.println("Test Case 2 failed to set up: " + e.getMessage());
        }
        System.out.println("---------------------------------------------------------");

        // Test Case 3: Empty file
        System.out.println("Test Case 3: Reading an empty file (test3.txt)");
        try {
            String fileName3 = "test3.txt";
            createTestFile(fileName3, "");
            System.out.println("Content: '" + readFileContent(fileName3) + "'");
            new File(fileName3).delete();
        } catch (IOException e) {
            System.out.println("Test Case 3 failed to set up: " + e.getMessage());
        }
        System.out.println("---------------------------------------------------------");

        // Test Case 4: Non-existent file
        System.out.println("Test Case 4: Attempting to read a non-existent file");
        String nonExistentFile = "nonexistentfile.txt";
        System.out.println("Content: " + readFileContent(nonExistentFile));
        System.out.println("---------------------------------------------------------");
        
        // Test Case 5: File with special characters
        System.out.println("Test Case 5: Reading a file with special characters (test5.txt)");
        try {
            String fileName5 = "test5.txt";
            createTestFile(fileName5, "UTF-8 test: áéíóú ñ €");
            System.out.println("Content: " + readFileContent(fileName5));
            new File(fileName5).delete();
        } catch (IOException e) {
            System.out.println("Test Case 5 failed to set up: " + e.getMessage());
        }
        System.out.println("=========================================================");
    }
}