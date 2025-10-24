import java.io.File;
import java.io.IOException;

public class Task69 {

    /**
     * Deletes a file specified by the file path.
     * @param filePath The path to the file to be deleted.
     */
    public static void deleteFile(String filePath) {
        if (filePath == null || filePath.trim().isEmpty()) {
            System.err.println("Error: File path cannot be null or empty.");
            return;
        }

        File file = new File(filePath);

        try {
            // Check if the file exists before attempting deletion
            if (file.exists()) {
                if (file.delete()) {
                    System.out.println("Success: File '" + filePath + "' was deleted successfully.");
                } else {
                    // This can happen due to permissions, or if it's a non-empty directory
                    System.err.println("Error: Failed to delete the file '" + filePath + "'. Check permissions or if it is a directory.");
                }
            } else {
                System.err.println("Error: File '" + filePath + "' not found.");
            }
        } catch (SecurityException e) {
            System.err.println("Security Error: Permission denied to delete the file '" + filePath + "'.");
            e.printStackTrace();
        }
    }

    public static void main(String[] args) {
        // --- Test Cases ---
        System.out.println("--- Running Test Cases ---");
        String testFile1 = "test1.tmp";
        String testFile2 = "test2.tmp";
        String nonExistentFile = "nonexistent.tmp";
        String alreadyDeletedFile = "test1.tmp";
        String testDir = "testdir.tmp";

        try {
            // Setup test files and directory
            new File(testFile1).createNewFile();
            new File(testFile2).createNewFile();
            new File(testDir).mkdir();

            // Test Case 1: Delete an existing file
            System.out.println("\n[Test Case 1: Delete an existing file]");
            deleteFile(testFile1);

            // Test Case 2: Delete another existing file
            System.out.println("\n[Test Case 2: Delete another existing file]");
            deleteFile(testFile2);

            // Test Case 3: Attempt to delete a non-existent file
            System.out.println("\n[Test Case 3: Attempt to delete a non-existent file]");
            deleteFile(nonExistentFile);

            // Test Case 4: Attempt to delete a file that was already deleted
            System.out.println("\n[Test Case 4: Attempt to delete an already deleted file]");
            deleteFile(alreadyDeletedFile);
            
            // Test Case 5: Attempt to delete a directory with file deletion method
            System.out.println("\n[Test Case 5: Attempt to delete a directory]");
            deleteFile(testDir);
            // Cleanup the directory after test
            new File(testDir).delete();

        } catch (IOException e) {
            System.err.println("Failed to setup test cases: " + e.getMessage());
        }
        System.out.println("\n--- Test Cases Finished ---");

        // --- Main Logic for Command Line ---
        if (args.length > 0) {
            System.out.println("\n--- Deleting file from command line argument ---");
            deleteFile(args[0]);
        } else {
            System.out.println("\nUsage: java Task69 <file_path_to_delete>");
            System.out.println("Example: java Task69 my_file.txt");
        }
    }
}