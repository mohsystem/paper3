import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;

public class Task36 {

    /**
     * Reads a file specified by the file path and prints its contents to the console.
     * It handles potential IOExceptions, such as the file not being found.
     * This implementation uses try-with-resources to ensure the file reader is closed automatically.
     *
     * @param filePath The path to the file to be read.
     */
    public static void readFileAndPrint(String filePath) {
        // Security: Basic input validation to prevent NullPointerException.
        if (filePath == null || filePath.trim().isEmpty()) {
            System.err.println("Error: File path cannot be null or empty.");
            return;
        }

        // Using try-with-resources to ensure the BufferedReader is closed automatically,
        // preventing resource leaks.
        try (BufferedReader reader = new BufferedReader(new FileReader(filePath))) {
            String line;
            while ((line = reader.readLine()) != null) {
                System.out.println(line);
            }
        } catch (IOException e) {
            // Security: Catching IOException handles FileNotFoundException, access errors, etc.
            // and informs the user gracefully without crashing.
            System.err.println("Error reading file: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Java Test Cases ---");
        String testFileName = "java_test_file.txt";
        File testFile = new File(testFileName);

        // Setup: Create a temporary file for testing
        try (BufferedWriter writer = new BufferedWriter(new FileWriter(testFile))) {
            writer.write("Hello from the Java test file.\n");
            writer.write("This is the second line.\n");
            writer.write("End of file.\n");
        } catch (IOException e) {
            System.err.println("Failed to create test file: " + e.getMessage());
            return;
        }

        // Test Case 1: Read a valid, existing file.
        System.out.println("\n[Test Case 1: Reading a valid file]");
        readFileAndPrint(testFileName);

        // Test Case 2: Attempt to read a non-existent file.
        System.out.println("\n[Test Case 2: Reading a non-existent file]");
        readFileAndPrint("non_existent_file.txt");

        // Test Case 3: Attempt to read with a null path.
        System.out.println("\n[Test Case 3: Reading a null path]");
        readFileAndPrint(null);

        // Test Case 4: Attempt to read with an empty path.
        System.out.println("\n[Test Case 4: Reading an empty path]");
        readFileAndPrint("");

        // Test Case 5: Attempt to read a directory.
        System.out.println("\n[Test Case 5: Reading a directory]");
        readFileAndPrint("."); // "." represents the current directory

        // Cleanup: Delete the temporary test file
        if (!testFile.delete()) {
            System.err.println("Failed to delete test file.");
        }
    }
}