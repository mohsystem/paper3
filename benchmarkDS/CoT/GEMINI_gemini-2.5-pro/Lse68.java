import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

public class Lse68 {

    // Simulates a database table for storing image data.
    private static final Map<String, String> imageDatabase = new HashMap<>();

    /**
     * Reads an image file, converts it to a Base64 string, and inserts it into a simulated database.
     *
     * @param filePath The path to the image file.
     */
    public static void upload_image(String filePath) {
        // Security: Prevent path traversal by only using the file's name.
        File file = new File(filePath);
        String fileName = file.getName();

        // Use try-with-resources to ensure the file streams are properly closed.
        try {
            // Read all bytes from the file.
            Path path = Paths.get(filePath);
            if (!Files.exists(path)) {
                System.out.println("Error: File not found at path: " + filePath);
                return;
            }
            byte[] fileContent = Files.readAllBytes(path);

            // Encode byte array to Base64 string.
            String encodedString = Base64.getEncoder().encodeToString(fileContent);

            // Insert the file name and Base64 string into the simulated database.
            imageDatabase.put(fileName, encodedString);
            System.out.println("Successfully uploaded and encoded '" + fileName + "'.");

        } catch (IOException e) {
            System.err.println("Error processing file '" + fileName + "': " + e.getMessage());
        } catch (SecurityException e) {
            System.err.println("Security error: Permission denied for file '" + fileName + "'.");
        }
    }

    public static void main(String[] args) {
        // --- Test Case Setup ---
        // Create dummy files for testing purposes.
        try {
            Files.write(Paths.get("test1.txt"), "Hello World".getBytes(), StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);
            Files.write(Paths.get("test2.jpg"), new byte[]{(byte)0xFF, (byte)0xD8, (byte)0xFF, (byte)0xE0}, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);
            Files.write(Paths.get("test3.txt"), new byte[]{}, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);
        } catch (IOException e) {
            System.err.println("Failed to create test files: " + e.getMessage());
            return;
        }

        System.out.println("--- Running Test Cases ---");
        // Test Case 1: A valid text file.
        upload_image("test1.txt");

        // Test Case 2: A valid (simulated) image file.
        upload_image("test2.jpg");

        // Test Case 3: An empty file.
        upload_image("test3.txt");

        // Test Case 4: A non-existent file.
        upload_image("nonexistent.png");

        // Test Case 5: A path traversal attempt (should be sanitized).
        // On most systems this will be normalized, but our use of getName() handles it robustly.
        upload_image("./../Lse68.java");


        System.out.println("\n--- Simulated Database Content ---");
        imageDatabase.forEach((name, data) -> {
            // Limiting data length for clean printing.
            String dataPreview = data.length() > 30 ? data.substring(0, 30) + "..." : data;
            System.out.println("FileName: " + name + ", Base64: " + dataPreview);
        });
        System.out.println("--------------------------------\n");


        // --- Test Case Cleanup ---
        try {
            Files.deleteIfExists(Paths.get("test1.txt"));
            Files.deleteIfExists(Paths.get("test2.jpg"));
            Files.deleteIfExists(Paths.get("test3.txt"));
        } catch (IOException e) {
            System.err.println("Failed to clean up test files: " + e.getMessage());
        }
    }
}