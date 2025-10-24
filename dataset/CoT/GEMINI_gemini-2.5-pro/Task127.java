import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.nio.file.Files;
import java.nio.file.Paths;

public class Task127 {

    /**
     * Reads a file from sourcePath and writes its content to destPath.
     * Handles various file-related exceptions.
     *
     * @param sourcePath The path of the file to read.
     * @param destPath   The path of the file to write.
     * @return true on success, false on failure.
     */
    public static boolean copyFileWithHandling(String sourcePath, String destPath) {
        // Using try-with-resources to ensure streams are closed automatically
        try (FileInputStream fis = new FileInputStream(sourcePath);
             FileOutputStream fos = new FileOutputStream(destPath)) {

            byte[] buffer = new byte[1024];
            int length;
            while ((length = fis.read(buffer)) > 0) {
                fos.write(buffer, 0, length);
            }
            System.out.println("Successfully copied '" + sourcePath + "' to '" + destPath + "'.");
            return true;
        } catch (FileNotFoundException e) {
            System.err.println("Error: Source file not found at '" + sourcePath + "'. Details: " + e.getMessage());
        } catch (SecurityException e) {
            System.err.println("Error: Permission denied. Cannot access '" + sourcePath + "' or '" + destPath + "'. Details: " + e.getMessage());
        } catch (IOException e) {
            System.err.println("Error: An I/O error occurred. Details: " + e.getMessage());
        } catch (Exception e) {
            System.err.println("An unexpected error occurred: " + e.getClass().getSimpleName() + ". Details: " + e.getMessage());
        }
        return false;
    }

    public static void main(String[] args) {
        // Setup for test cases
        String tempDir = System.getProperty("java.io.tmpdir");
        String sourceFilePath = tempDir + File.separator + "source.txt";
        String destFilePath = tempDir + File.separator + "destination.txt";
        String nonExistentSource = tempDir + File.separator + "non_existent_file.txt";
        String writeProtectedDir = tempDir + File.separator + "protected_dir";
        String writeProtectedPath = writeProtectedDir + File.separator + "cannot_write.txt";

        // Create a dummy source file
        try {
            Files.write(Paths.get(sourceFilePath), "This is a test file.".getBytes());
        } catch (IOException e) {
            System.err.println("Failed to set up test cases: " + e.getMessage());
            return;
        }

        // Create a directory to simulate a write-protected location
        new File(writeProtectedDir).mkdir();

        System.out.println("--- Running Java Test Cases ---");

        // Test Case 1: Successful file copy
        System.out.println("\n[Test Case 1: Successful Copy]");
        copyFileWithHandling(sourceFilePath, destFilePath);

        // Test Case 2: Source file does not exist
        System.out.println("\n[Test Case 2: Source File Not Found]");
        copyFileWithHandling(nonExistentSource, destFilePath);

        // Test Case 3: Reading a directory (simulates a read error)
        System.out.println("\n[Test Case 3: Read Error (trying to read a directory)]");
        copyFileWithHandling(tempDir, destFilePath);

        // Test Case 4: Destination is not writable (trying to write a file with the same name as a directory)
        System.out.println("\n[Test Case 4: Write Error (destination is a directory)]");
        copyFileWithHandling(sourceFilePath, writeProtectedDir);

        // Test Case 5: Invalid source path (null)
        System.out.println("\n[Test Case 5: Invalid Argument (null path)]");
        try {
            copyFileWithHandling(null, destFilePath);
        } catch (Exception e) {
             System.err.println("An unexpected error occurred: " + e.getClass().getSimpleName() + ". Details: " + e.getMessage());
        }


        // Cleanup
        new File(sourceFilePath).delete();
        new File(destFilePath).delete();
        new File(writeProtectedDir).delete();
    }
}