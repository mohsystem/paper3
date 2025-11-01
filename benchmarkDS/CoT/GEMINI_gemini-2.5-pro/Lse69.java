import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.InvalidPathException;
import java.util.Base64;
import java.nio.charset.StandardCharsets;

public class Lse69 {

    /**
     * Reads an image file, encodes it to Base64, and writes the result to an output file.
     * This function performs basic security checks to prevent path traversal.
     *
     * @param imagePath  The path to the input image file.
     * @param outputPath The path to the output file for the Base64 string.
     * @return The path to the output file on success, null on failure.
     */
    public static String upload_image(String imagePath, String outputPath) {
        // Security: Basic path validation to prevent path traversal.
        // A robust solution would involve canonicalizing the path and checking against a whitelisted directory.
        if (imagePath.contains("..") || outputPath.contains("..")) {
            System.err.println("Error: Path traversal attempt detected.");
            return null;
        }

        try {
            Path inputPath = Paths.get(imagePath);
            Path outPath = Paths.get(outputPath);

            // Security: Ensure the output directory exists. Creating it here for convenience.
            // In a production environment, you might only allow writing to pre-existing, whitelisted directories.
            Path parentDir = outPath.getParent();
            if (parentDir != null && !Files.exists(parentDir)) {
                Files.createDirectories(parentDir);
            }

            // Read all bytes from the image file.
            // This is not suitable for very large files as it can lead to OutOfMemoryError.
            // For large files, a streaming approach would be more secure against resource exhaustion.
            byte[] fileContent = Files.readAllBytes(inputPath);

            // Encode the byte array to a Base64 string.
            String encodedString = Base64.getEncoder().encodeToString(fileContent);

            // Write the encoded string to the output file.
            Files.write(outPath, encodedString.getBytes(StandardCharsets.UTF_8));

            return outputPath;

        } catch (InvalidPathException e) {
            System.err.println("Error: Invalid file path provided. " + e.getMessage());
            return null;
        } catch (IOException e) {
            System.err.println("Error during file operation: " + e.getMessage());
            return null;
        } catch (OutOfMemoryError e) {
            System.err.println("Error: File is too large to process. " + e.getMessage());
            return null;
        }
    }

    public static void main(String[] args) {
        // --- Test Cases ---

        // Helper to create a dummy file and directories for testing
        try {
            Path tempDir = Paths.get("temp_test_java");
            // Clean up previous runs if necessary
            if (Files.exists(tempDir)) {
                 try {
                     // Simple recursive delete
                     Files.walk(tempDir)
                         .sorted(java.util.Comparator.reverseOrder())
                         .map(Path::toFile)
                         .forEach(java.io.File::delete);
                 } catch (IOException e) {
                     // Ignore cleanup errors
                 }
            }
            Files.createDirectories(tempDir);

            // Create a dummy image file
            Path dummyImage = tempDir.resolve("test_image.jpg");
            byte[] dummyContent = { (byte)0xFF, (byte)0xD8, (byte)0xFF, (byte)0xE0, 0x1, 0x2, 0x3, 0x4 }; // Fake JPEG data
            Files.write(dummyImage, dummyContent);
            
            Path emptyImage = tempDir.resolve("empty.jpg");
            Files.createFile(emptyImage);

            System.out.println("--- Java Test Cases ---");

            // Test Case 1: Successful upload
            System.out.println("Test Case 1: Normal valid image path");
            String output1 = upload_image(dummyImage.toString(), tempDir.resolve("output1.txt").toString());
            System.out.println(output1 != null ? "Success, output at: " + output1 : "Failed");
            if (output1 != null) { System.out.println("Content: " + new String(Files.readAllBytes(Paths.get(output1)))); }
            System.out.println();

            // Test Case 2: Non-existent image path
            System.out.println("Test Case 2: Non-existent image path");
            String output2 = upload_image("non_existent_image.jpg", tempDir.resolve("output2.txt").toString());
            System.out.println(output2 != null ? "Success, output at: " + output2 : "Failed");
            System.out.println();

            // Test Case 3: Invalid output path (directory doesn't exist - function should create it)
            System.out.println("Test Case 3: Output to non-existent directory");
            String output3 = upload_image(dummyImage.toString(), tempDir.resolve("new_dir/output3.txt").toString());
            System.out.println(output3 != null ? "Success, output at: " + output3 : "Failed");
            System.out.println();
            
            // Test Case 4: Path traversal attempt
            System.out.println("Test Case 4: Path traversal attempt");
            String output4 = upload_image(dummyImage.toString(), "../output4.txt");
            System.out.println(output4 != null ? "Success, output at: " + output4 : "Failed");
            System.out.println();

            // Test Case 5: Empty input file
            System.out.println("Test Case 5: Empty input file");
            String output5 = upload_image(emptyImage.toString(), tempDir.resolve("output5.txt").toString());
            System.out.println(output5 != null ? "Success, output at: " + output5 : "Failed");
            if (output5 != null) { System.out.println("Content: " + new String(Files.readAllBytes(Paths.get(output5)))); }
            System.out.println();

        } catch (IOException e) {
            System.err.println("Failed to set up test cases: " + e.getMessage());
        }
    }
}