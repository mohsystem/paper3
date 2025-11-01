import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Base64;

public class Lse69 {

    /**
     * Reads an image file, encodes it to Base64, and writes the encoded string to a text file.
     *
     * @param imagePath  The path to the input image file.
     * @param outputPath The path to the output file where the Base64 string will be saved.
     */
    public static void uploadImage(String imagePath, String outputPath) {
        try {
            // Read all bytes from the image file
            Path path = Paths.get(imagePath);
            if (!Files.exists(path)) {
                System.err.println("Error: Input file not found at " + imagePath);
                return;
            }
            byte[] imageBytes = Files.readAllBytes(path);

            // Encode the byte array to a Base64 string
            String encodedImage = Base64.getEncoder().encodeToString(imageBytes);

            // Write the encoded string to the output file
            Path outPath = Paths.get(outputPath);
            Files.write(outPath, encodedImage.getBytes());

        } catch (IOException e) {
            System.err.println("An error occurred during file processing.");
            e.printStackTrace();
        }
    }

    // Helper function to create a dummy file with specific content for testing
    private static void createTestFile(String fileName, byte[] content) throws IOException {
        try (FileOutputStream fos = new FileOutputStream(fileName)) {
            fos.write(content);
        }
    }

    public static void main(String[] args) {
        // --- Test Cases ---

        String[] testInputs = {
            "Man",          // Test case 1: Length is a multiple of 3
            "Ma",           // Test case 2: Length % 3 == 2
            "M",            // Test case 3: Length % 3 == 1
            "",             // Test case 4: Empty content
            "Hello, World!" // Test case 5: A longer string
        };

        for (int i = 0; i < testInputs.length; i++) {
            String inputFileName = "test_input_" + i + ".bin";
            String outputFileName = "test_output_" + i + ".txt";
            
            try {
                System.out.println("--- Running Test Case " + (i + 1) + " ---");
                System.out.println("Input data: \"" + testInputs[i] + "\"");

                // 1. Create a dummy input file
                createTestFile(inputFileName, testInputs[i].getBytes());

                // 2. Call the function to be tested
                uploadImage(inputFileName, outputFileName);
                System.out.println("uploadImage() called. Encoded data written to " + outputFileName);

                // 3. (Optional) Verify the output
                String encodedContent = new String(Files.readAllBytes(Paths.get(outputFileName)));
                System.out.println("Encoded output: " + encodedContent);
                System.out.println("Test Case " + (i + 1) + " completed.\n");

            } catch (IOException e) {
                System.err.println("Error during test case " + (i + 1) + ": " + e.getMessage());
            } finally {
                // 4. Clean up the created files
                new File(inputFileName).delete();
                new File(outputFileName).delete();
            }
        }
    }
}