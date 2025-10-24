import java.io.File;
import java.io.IOException;

public class Task69 {

    /**
     * Deletes a file at the given file path.
     *
     * @param filePath The path of the file to delete.
     * @return true if the file was successfully deleted, false otherwise.
     */
    public static boolean deleteFile(String filePath) {
        File file = new File(filePath);
        if (file.exists()) {
            return file.delete();
        }
        // File doesn't exist, so it cannot be deleted.
        return false;
    }

    public static void main(String[] args) {
        // If a command-line argument is provided, use it as the file path.
        if (args.length > 0) {
            String filePath = args[0];
            System.out.println("Attempting to delete file from command line: " + filePath);
            if (deleteFile(filePath)) {
                System.out.println("File deleted successfully.");
            } else {
                System.out.println("Failed to delete the file. It may not exist or you may not have permission.");
            }
        } else {
            // If no arguments, run the built-in test cases.
            runTestCases();
        }
    }

    /**
     * Creates and runs 5 test cases for the deleteFile function.
     */
    public static void runTestCases() {
        System.out.println("No command line arguments provided. Running test cases...");
        String[] testFiles = {"test1.tmp", "test2.tmp", "test3.tmp", "test4.tmp", "test5.tmp"};
        
        // Test Cases 1-5: Create and then delete a file.
        for (int i = 0; i < testFiles.length; i++) {
            String fileName = testFiles[i];
            System.out.println("\n--- Test Case " + (i + 1) + ": Delete " + fileName + " ---");
            
            // 1. Create a dummy file for the test.
            try {
                File testFile = new File(fileName);
                if (testFile.createNewFile()) {
                    System.out.println("  [Setup] Created temporary file: " + fileName);
                } else {
                    System.out.println("  [Setup] Could not create temporary file: " + fileName);
                    continue; // Skip this test if setup fails
                }
            } catch (IOException e) {
                System.out.println("  [Setup] An error occurred while creating file: " + e.getMessage());
                continue;
            }

            // 2. Attempt to delete the file.
            if (deleteFile(fileName)) {
                System.out.println("  [Result] SUCCESS: File deleted.");
            } else {
                System.out.println("  [Result] FAILED: File not deleted.");
            }
        }
    }
}