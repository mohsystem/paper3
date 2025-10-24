import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;

public class Task121 {

    /**
     * Simulates a file upload by copying a file from a source path to a destination path.
     * The destination path should be the full path of the new file, including the filename.
     *
     * @param sourceFilePath      The path to the file to be "uploaded".
     * @param destinationFilePath The path where the file will be saved.
     * @return true if the upload (copy) was successful, false otherwise.
     */
    public static boolean uploadFile(String sourceFilePath, String destinationFilePath) {
        Path sourcePath = Paths.get(sourceFilePath);
        Path destinationPath = Paths.get(destinationFilePath);

        if (!Files.exists(sourcePath)) {
            System.err.println("Error: Source file does not exist: " + sourceFilePath);
            return false;
        }

        try {
            // Ensure the destination directory exists
            Path parentDir = destinationPath.getParent();
            if (parentDir != null && !Files.exists(parentDir)) {
                Files.createDirectories(parentDir);
            }
            // Copy the file, overwriting if it already exists
            Files.copy(sourcePath, destinationPath, StandardCopyOption.REPLACE_EXISTING);
            System.out.println("File uploaded successfully from '" + sourceFilePath + "' to '" + destinationFilePath + "'");
            return true;
        } catch (IOException e) {
            System.err.println("File upload failed: " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        // Setup for test cases
        String testDir = "java_upload_test_dir";
        String sourceFile = "java_source.txt";
        String sourceFilePath = sourceFile;
        String destDir = testDir;

        try {
            // Create a dummy source file
            Files.write(Paths.get(sourceFile), "This is a test file for upload.".getBytes());
            // Create destination directory
            Files.createDirectories(Paths.get(destDir));

            System.out.println("--- Running Test Cases ---");

            // Test Case 1: Successful upload
            System.out.println("\n--- Test Case 1: Successful Upload ---");
            uploadFile(sourceFilePath, destDir + File.separator + "uploaded_file1.txt");

            // Test Case 2: Uploading a non-existent source file
            System.out.println("\n--- Test Case 2: Non-existent Source File ---");
            uploadFile("non_existent_file.txt", destDir + File.separator + "uploaded_file2.txt");

            // Test Case 3: Overwriting an existing file
            System.out.println("\n--- Test Case 3: Overwriting an Existing File ---");
            String overwriteFile = destDir + File.separator + "overwrite.txt";
            uploadFile(sourceFilePath, overwriteFile); // First time
            System.out.println("Uploading again to the same destination...");
            uploadFile(sourceFilePath, overwriteFile); // Second time, should overwrite

            // Test Case 4: Uploading to a nested directory that doesn't exist
            System.out.println("\n--- Test Case 4: Upload to a non-existent nested directory ---");
            uploadFile(sourceFilePath, destDir + File.separator + "new" + File.separator + "nested" + File.separator + "uploaded_file4.txt");

            // Test Case 5: Source and destination are the same
            System.out.println("\n--- Test Case 5: Source and Destination are the same ---");
            String sameFilePath = destDir + File.separator + "same_file.txt";
            uploadFile(sourceFilePath, sameFilePath); // create it first
            uploadFile(sameFilePath, sameFilePath); // try to copy to itself


        } catch (IOException e) {
            System.err.println("Setup for tests failed: " + e.getMessage());
        } finally {
            // Cleanup
            System.out.println("\n--- Cleaning up test files and directories ---");
            try {
                // Delete source file
                Files.deleteIfExists(Paths.get(sourceFile));
                // Delete test directory and its contents
                if (Files.exists(Paths.get(testDir))) {
                    Files.walk(Paths.get(testDir))
                         .sorted((a, b) -> b.compareTo(a)) // reverse order for deletion
                         .forEach(path -> {
                             try {
                                 Files.delete(path);
                             } catch (IOException e) {
                                 System.err.println("Failed to delete " + path + ": " + e.getMessage());
                             }
                         });
                }
                System.out.println("Cleanup complete.");
            } catch (IOException e) {
                System.err.println("Cleanup failed: " + e.getMessage());
            }
        }
    }
}