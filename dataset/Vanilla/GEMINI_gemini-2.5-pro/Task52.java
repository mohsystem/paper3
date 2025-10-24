import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.Arrays;

public class Task52 {

    /**
     * Encrypts or decrypts a file using a simple XOR cipher with a given key.
     * The operation is symmetric; applying it twice with the same key restores the original file.
     *
     * @param key The secret key for encryption/decryption.
     * @param inputFilePath Path to the input file.
     * @param outputFilePath Path to the output file.
     */
    public static void processFile(String key, String inputFilePath, String outputFilePath) {
        if (key == null || key.isEmpty()) {
            System.err.println("Error: Key cannot be null or empty.");
            return;
        }

        try (InputStream inputStream = new FileInputStream(inputFilePath);
             OutputStream outputStream = new FileOutputStream(outputFilePath)) {

            byte[] keyBytes = key.getBytes();
            int keyIndex = 0;
            int data;

            // Read one byte at a time from the input file
            while ((data = inputStream.read()) != -1) {
                // XOR the byte with the corresponding key byte and write to the output file
                outputStream.write(data ^ keyBytes[keyIndex]);
                // Move to the next key byte, wrapping around if necessary
                keyIndex = (keyIndex + 1) % keyBytes.length;
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    // Helper method to run a complete test case
    private static void runTestCase(String testId, String content, String key) {
        String plainFile = testId + "_plain.txt";
        String encryptedFile = testId + "_encrypted.bin";
        String decryptedFile = testId + "_decrypted.txt";

        try {
            // 1. Create the original file with specified content
            Files.write(Paths.get(plainFile), content.getBytes());
            System.out.println("\n--- Running Test Case: " + testId + " ---");
            System.out.println("Original content: \"" + content + "\"");

            // 2. Encrypt the file
            processFile(key, plainFile, encryptedFile);
            System.out.println("'" + plainFile + "' encrypted to '" + encryptedFile + "' with key '" + key + "'.");

            // 3. Decrypt the file
            processFile(key, encryptedFile, decryptedFile);
            System.out.println("'" + encryptedFile + "' decrypted to '" + decryptedFile + "'.");

            // 4. Verify the result
            byte[] originalBytes = Files.readAllBytes(Paths.get(plainFile));
            byte[] decryptedBytes = Files.readAllBytes(Paths.get(decryptedFile));
            
            if (Arrays.equals(originalBytes, decryptedBytes)) {
                System.out.println("SUCCESS: Decrypted content matches original content.");
            } else {
                System.out.println("FAILURE: Content mismatch!");
                System.out.println("Decrypted content: \"" + new String(decryptedBytes) + "\"");
            }

        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            // 5. Clean up the created files
            new File(plainFile).delete();
            new File(encryptedFile).delete();
            new File(decryptedFile).delete();
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Java File Encryption/Decryption Tests ---");

        // Test Case 1: Simple text
        runTestCase("test1", "Hello World!", "KEY1");
        
        // Test Case 2: Different key and text
        runTestCase("test2", "Java XOR Encryption/Decryption.", "SECRET");
        
        // Test Case 3: Longer text to test key wrapping
        runTestCase("test3", "This is a longer piece of text to ensure the key repeats correctly.", "LONGERKEY");

        // Test Case 4: Text with special characters and numbers
        runTestCase("test4", "12345!@#$%^&*()_+", "NUMSYM");

        // Test Case 5: Empty file
        runTestCase("test5", "", "EMPTY");

        System.out.println("\n--- Java Tests Completed ---");
    }
}