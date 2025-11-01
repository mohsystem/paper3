import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.NoSuchFileException;
import java.nio.file.Paths;
import java.util.Arrays;

public class Lse48 {

    /**
     * Reads the entire content of a file in binary mode.
     *
     * @param fileName The path to the file.
     * @return A byte array containing the file's content, or null if the file cannot be read.
     */
    public static byte[] img(String fileName) {
        try {
            return Files.readAllBytes(Paths.get(fileName));
        } catch (IOException e) {
            // e.g., NoSuchFileException, SecurityException
            return null;
        }
    }

    // Helper function to create files for testing
    private static void setupTestFiles() {
        try {
            // Test case 1: A simple text file
            try (FileOutputStream fos = new FileOutputStream("test1.txt")) {
                fos.write("Hello".getBytes());
            }

            // Test case 2: A binary file with specific byte values
            try (FileOutputStream fos = new FileOutputStream("test2.bin")) {
                fos.write(new byte[]{0x01, 0x02, 0x03, 0x04});
            }

            // Test case 3: An empty file
            new File("empty.txt").createNewFile();

            // Test case 5: A binary file with a null character in the middle
            try (FileOutputStream fos = new FileOutputStream("test3.bin")) {
                fos.write(new byte[]{'A', 0, 'B'});
            }

        } catch (IOException e) {
            System.err.println("Failed to set up test files: " + e.getMessage());
        }
    }
    
    // Helper function to clean up test files
    private static void cleanupTestFiles() {
        new File("test1.txt").delete();
        new File("test2.bin").delete();
        new File("empty.txt").delete();
        new File("test3.bin").delete();
    }

    public static void main(String[] args) {
        setupTestFiles();

        String[] testFiles = {
            "test1.txt",        // Case 1: Simple text file
            "test2.bin",        // Case 2: Simple binary file
            "empty.txt",        // Case 3: Empty file
            "nonexistent.file", // Case 4: File that does not exist
            "test3.bin"         // Case 5: File with a null byte
        };

        for (int i = 0; i < testFiles.length; i++) {
            String file = testFiles[i];
            System.out.println("--- Test Case " + (i + 1) + ": " + file + " ---");
            byte[] content = img(file);

            if (content != null) {
                System.out.println("Success: Read " + content.length + " bytes.");
                System.out.println("Content (as bytes): " + Arrays.toString(content));
            } else {
                System.out.println("Failure: File could not be read (or does not exist).");
            }
            System.out.println();
        }
        
        cleanupTestFiles();
    }
}