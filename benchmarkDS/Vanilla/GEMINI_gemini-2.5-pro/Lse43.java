import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.NoSuchFileException;

public class Lse43 {

    /**
     * Reads the content of a file from the /safe/ directory.
     *
     * @param fileName The name of the file to read (not the full path).
     * @return The content of the file as a String, or an error message if it fails.
     */
    public static String readFileFromSafeDir(String fileName) {
        // Ensure the filename does not contain path traversal characters for basic safety.
        if (fileName == null || fileName.contains("/") || fileName.contains("\\") || fileName.contains("..")) {
            return "[ERROR] Invalid filename.";
        }
        
        Path path = Paths.get("/safe/", fileName);
        try {
            return Files.readString(path);
        } catch (NoSuchFileException e) {
            return "[ERROR] File not found: " + path;
        } catch (IOException e) {
            return "[ERROR] Could not read file '" + path + "'. It might be a directory or you lack permissions.";
        } catch (Exception e) {
            return "[ERROR] An unexpected error occurred: " + e.getMessage();
        }
    }

    public static void main(String[] args) {
        // --- Test Cases ---
        // To run these tests, you must first set up the directory and files.
        // On a Unix-like system, you can use these commands:
        // sudo mkdir /safe
        // sudo chown $(whoami) /safe
        // echo "This is a test file." > /safe/existing.txt
        // touch /safe/empty.txt
        // mkdir /safe/test_dir
        
        System.out.println("--- Running 5 Test Cases ---");

        // Test Case 1: Read an existing file with content.
        System.out.println("Test Case 1: Reading 'existing.txt'");
        String content1 = readFileFromSafeDir("existing.txt");
        System.out.println("Content: \"" + content1 + "\"\n");

        // Test Case 2: Read a non-existent file.
        System.out.println("Test Case 2: Reading 'non_existent.txt'");
        String content2 = readFileFromSafeDir("non_existent.txt");
        System.out.println("Content: \"" + content2 + "\"\n");

        // Test Case 3: Read an empty file.
        System.out.println("Test Case 3: Reading 'empty.txt'");
        String content3 = readFileFromSafeDir("empty.txt");
        System.out.println("Content: \"" + content3 + "\"\n");
        
        // Test Case 4: Attempt to read a directory.
        System.out.println("Test Case 4: Reading directory 'test_dir'");
        String content4 = readFileFromSafeDir("test_dir");
        System.out.println("Content: \"" + content4 + "\"\n");

        // Test Case 5: Attempt to use an invalid filename (path traversal).
        System.out.println("Test Case 5: Reading invalid filename '../etc/passwd'");
        String content5 = readFileFromSafeDir("../etc/passwd");
        System.out.println("Content: \"" + content5 + "\"\n");
    }
}