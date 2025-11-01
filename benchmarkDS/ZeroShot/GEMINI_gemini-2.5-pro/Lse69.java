import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.util.Base64;

public class Lse69 {

    /**
     * Reads an image file, encodes it to Base64, and saves the encoded string to a new file.
     * This function is secured against path traversal attacks.
     *
     * @param imagePath The path to the image file to upload. Must be within the UPLOAD_DIR.
     * @return true if the operation was successful, false otherwise.
     */
    public static boolean uploadImage(String imagePath) {
        // Define a safe base directory for uploads.
        final Path uploadDir = Paths.get("uploads").toAbsolutePath();

        if (imagePath == null || imagePath.trim().isEmpty()) {
            System.err.println("Error: Image path cannot be null or empty.");
            return false;
        }

        Path inputPath = Paths.get(imagePath);
        Path absoluteInputPath;

        try {
            // Create the upload directory if it doesn't exist for the check to work
            if (!Files.exists(uploadDir)) {
                 Files.createDirectories(uploadDir);
            }
            absoluteInputPath = inputPath.toAbsolutePath().normalize();

            // Security Check: Ensure the resolved path is within the designated upload directory.
            if (!absoluteInputPath.startsWith(uploadDir)) {
                System.err.println("Error: Path traversal attempt detected. File access denied.");
                return false;
            }

            // Security Check: Ensure the path points to a regular, readable file.
            if (!Files.isRegularFile(absoluteInputPath) || !Files.isReadable(absoluteInputPath)) {
                System.err.println("Error: The specified path is not a regular, readable file: " + absoluteInputPath);
                return false;
            }

            // Read all bytes from the image file
            byte[] imageBytes = Files.readAllBytes(absoluteInputPath);

            // Encode the byte array to a Base64 string
            String encodedImage = Base64.getEncoder().encodeToString(imageBytes);

            // Define the output file path
            String outputFileName = absoluteInputPath.getFileName().toString() + ".b64";
            Path outputPath = uploadDir.resolve(outputFileName);

            // Write the encoded string to the output file
            Files.write(outputPath, encodedImage.getBytes(), StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);

            System.out.println("Image successfully encoded and saved to: " + outputPath);
            return true;

        } catch (IOException | SecurityException e) {
            System.err.println("An error occurred during file processing: " + e.getMessage());
            return false;
        }
    }

    /**
     * Main method with test cases.
     */
    public static void main(String[] args) {
        // Setup for test cases
        try {
            System.out.println("--- Setting up test environment ---");
            Path uploadDir = Paths.get("uploads");
            Path sensitiveFile = Paths.get("sensitive.txt");

            // Create directories and files for testing
            Files.createDirectories(uploadDir);
            Files.write(uploadDir.resolve("test_image.jpg"), "dummy image data".getBytes());
            Files.write(sensitiveFile, "secret data".getBytes());
            System.out.println("Test environment created.");
            System.out.println("------------------------------------");
        } catch (IOException e) {
            System.err.println("Failed to set up test environment: " + e.getMessage());
            return;
        }

        System.out.println("\n--- Running Test Cases ---");

        // Test Case 1: Valid image file
        System.out.println("\n[Test 1] Processing a valid image file...");
        uploadImage("uploads/test_image.jpg");

        // Test Case 2: File does not exist
        System.out.println("\n[Test 2] Processing a non-existent file...");
        uploadImage("uploads/not_found.jpg");

        // Test Case 3: Path traversal attempt
        System.out.println("\n[Test 3] Processing with a path traversal attempt...");
        uploadImage("uploads/../sensitive.txt");
        
        // Test Case 4: Accessing a file outside the allowed directory
        System.out.println("\n[Test 4] Processing a file outside the upload directory...");
        uploadImage("sensitive.txt");

        // Test Case 5: Path is a directory, not a file
        System.out.println("\n[Test 5] Processing a directory path...");
        uploadImage("uploads");
        
        System.out.println("\n--- Test Cases Finished ---");
    }
}