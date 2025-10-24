import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

public class Task36 {

    /**
     * Reads a file specified by the file path and prints its contents to the console.
     * Uses try-with-resources for automatic resource management and catches IOExceptions.
     *
     * @param filePath The path to the file to be read.
     */
    public static void readFileAndPrint(String filePath) {
        // Rule #7: The file path is treated as untrusted input.
        // We avoid TOCTOU vulnerabilities by directly trying to open the file
        // and handling the exception rather than checking for existence first.
        Path path = Paths.get(filePath);

        // Rule #8: Use try-with-resources to ensure the reader is closed automatically.
        try (BufferedReader reader = Files.newBufferedReader(path, StandardCharsets.UTF_8)) {
            String line;
            // Rule #1: Reading line by line avoids large buffer allocations and potential overflows.
            while ((line = reader.readLine()) != null) {
                System.out.println(line);
            }
        } catch (IOException e) {
            // Handle exceptions like file not found, permission denied, etc.
            System.err.println("Error reading file '" + filePath + "': " + e.getMessage());
        }
    }

    private static void createTestFile(String fileName, String content) {
        Path path = Paths.get(fileName);
        try (BufferedWriter writer = Files.newBufferedWriter(path, StandardCharsets.UTF_8)) {
            writer.write(content);
        } catch (IOException e) {
            System.err.println("Failed to create test file: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        // Setup test files
        createTestFile("test1.txt", "Hello, World!");
        createTestFile("test2.txt", "This is line 1.\nThis is line 2.");
        createTestFile("test3.txt", "");
        createTestFile("test4.txt", "UTF-8 characters: áéíóú ñ €");

        System.out.println("--- Test Case 1: File with a single line ---");
        readFileAndPrint("test1.txt");
        System.out.println("\n--- Test Case 2: File with multiple lines ---");
        readFileAndPrint("test2.txt");
        System.out.println("\n--- Test Case 3: Empty file ---");
        readFileAndPrint("test3.txt");
        System.out.println("\n--- Test Case 4: File with UTF-8 characters ---");
        readFileAndPrint("test4.txt");
        System.out.println("\n--- Test Case 5: Non-existent file ---");
        readFileAndPrint("nonexistent.txt");

        // Cleanup
        new File("test1.txt").delete();
        new File("test2.txt").delete();
        new File("test3.txt").delete();
        new File("test4.txt").delete();
    }
}