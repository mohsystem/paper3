import java.io.IOException;
import java.nio.file.*;

public class Task69 {

    /**
     * Deletes a file at the given path after performing security checks.
     * It will not delete directories or symbolic links.
     *
     * @param filePathString The path to the file to be deleted.
     * @return true if the file was successfully deleted, false otherwise.
     */
    public static boolean deleteFile(String filePathString) {
        if (filePathString == null || filePathString.trim().isEmpty()) {
            System.err.println("Error: File path cannot be null or empty.");
            return false;
        }

        try {
            Path path = Paths.get(filePathString);

            // Security Check: Do not follow symbolic links for checks.
            // This helps mitigate TOCTOU vulnerabilities where a file is replaced by a link.
            if (Files.isSymbolicLink(path)) {
                System.err.println("Error: Deleting symbolic links is not allowed for security reasons. Path: " + path);
                return false;
            }

            // Check existence after symlink check to avoid race conditions on the type.
            if (!Files.exists(path)) {
                System.err.println("Error: File does not exist. Path: " + path);
                return false;
            }

            // Security Check: Ensure the path is not a directory.
            if (Files.isDirectory(path)) {
                System.err.println("Error: Path is a directory, not a file. Path: " + path);
                return false;
            }

            // Security Check: Ensure it is a regular file.
            if (!Files.isRegularFile(path)) {
                System.err.println("Error: Path is not a regular file. Path: " + path);
                return false;
            }

            Files.delete(path);
            System.out.println("Successfully deleted file: " + path);
            return true;

        } catch (InvalidPathException e) {
            System.err.println("Error: Invalid file path provided. " + e.getMessage());
        } catch (NoSuchFileException e) {
            System.err.println("Error: File not found for deletion. It may have been removed by another process. Path: " + e.getFile());
        } catch (SecurityException e) {
            System.err.println("Error: Permission denied. Could not delete file. " + e.getMessage());
        } catch (IOException e) {
            System.err.println("An I/O error occurred: " + e.getMessage());
        }
        return false;
    }

    private static void runTest(String testName, String path) {
        System.out.println("--- " + testName + " ---");
        System.out.println("Attempting to delete: " + path);
        deleteFile(path);
        System.out.println();
    }

    public static void main(String[] args) {
        if (args.length == 1) {
            deleteFile(args[0]);
        } else {
            System.out.println("Usage: java Task69 <file_path>");
            System.out.println("\n--- No file path provided, running built-in test cases ---");

            // Setup for tests
            Path testFile = Paths.get("test_file_java.txt");
            Path testDir = Paths.get("test_dir_java");
            Path testLink = Paths.get("test_link_java.txt");
            Path nonExistentFile = Paths.get("non_existent_file.txt");
            
            try {
                Files.deleteIfExists(testFile);
                Files.deleteIfExists(testDir);
                Files.deleteIfExists(testLink);

                Files.createFile(testFile);
                Files.createDirectory(testDir);
                try {
                    Files.createSymbolicLink(testLink, testFile);
                } catch (IOException | UnsupportedOperationException e) {
                    System.err.println("Warning: Could not create symbolic link for test. " + e.getMessage());
                }
                
                // --- 5 Test Cases ---
                runTest("Test Case 1: Delete existing file", testFile.toString());
                runTest("Test Case 2: Delete non-existent file", nonExistentFile.toString());
                runTest("Test Case 3: Delete a directory", testDir.toString());
                if (Files.exists(testLink, LinkOption.NOFOLLOW_LINKS)) {
                    runTest("Test Case 4: Delete a symbolic link", testLink.toString());
                } else {
                    System.out.println("--- Skipping Test Case 4: Symbolic link was not created ---");
                }
                runTest("Test Case 5: Re-delete already deleted file", testFile.toString());
                
            } catch (IOException e) {
                System.err.println("Error during test setup: " + e.getMessage());
            } finally {
                // Cleanup
                try {
                    Files.deleteIfExists(testDir);
                    Files.deleteIfExists(testLink);
                    Files.deleteIfExists(testFile);
                    System.out.println("--- Test cleanup complete ---");
                } catch (IOException e) {
                    System.err.println("Error during cleanup: " + e.getMessage());
                }
            }
        }
    }
}