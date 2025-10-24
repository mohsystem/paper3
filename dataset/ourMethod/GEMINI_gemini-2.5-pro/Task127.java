import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;

public class Task127 {

    /**
     * Reads a file from a base directory and writes its content to a new temporary file.
     * This function is secured against path traversal attacks.
     *
     * @param baseDirStr  The trusted base directory.
     * @param fileNameStr The name of the file to be read (untrusted).
     * @return The path to the temporary file, or null if an error occurred.
     */
    public static String copyFileToTemp(String baseDirStr, String fileNameStr) {
        Path baseDir;
        try {
            baseDir = Paths.get(baseDirStr).toRealPath();
        } catch (IOException e) {
            System.err.println("Error: Base directory is invalid or does not exist. " + e.getMessage());
            return null;
        }

        // Validate that fileNameStr does not contain path separators
        if (fileNameStr.contains("/") || fileNameStr.contains("\\")) {
             System.err.println("Error: File name cannot contain path separators.");
             return null;
        }
        
        Path inputFile = baseDir.resolve(fileNameStr);

        try {
            // Get the canonical path to prevent directory traversal attacks (e.g., using '..')
            Path canonicalInputFile = inputFile.toRealPath();

            // Security check: Ensure the canonical path of the input file is within the base directory
            if (!canonicalInputFile.startsWith(baseDir)) {
                System.err.println("Error: Path traversal attempt detected. Access denied.");
                return null;
            }

            // Security check: Ensure we are reading a regular file, not a directory or symlink
            if (!Files.isRegularFile(canonicalInputFile)) {
                System.err.println("Error: The specified path is not a regular file.");
                return null;
            }

            // Create a temporary file
            Path tempFile = Files.createTempFile("temp-copy-", ".tmp");

            // Copy the contents
            Files.copy(canonicalInputFile, tempFile, StandardCopyOption.REPLACE_EXISTING);

            return tempFile.toString();

        } catch (NoSuchFileException e) {
            System.err.println("Error: File not found: " + inputFile);
        } catch (IOException e) {
            System.err.println("Error processing file: " + e.getMessage());
        } catch (SecurityException e) {
            System.err.println("Error: Security manager denied access. " + e.getMessage());
        }
        
        return null;
    }

    public static void main(String[] args) {
        // Setup a temporary directory for tests
        Path testBaseDir = null;
        try {
            testBaseDir = Files.createTempDirectory("test-basedir-");
            File validFile = new File(testBaseDir.toFile(), "valid.txt");
            Files.write(validFile.toPath(), "Hello, World!".getBytes(StandardCharsets.UTF_8));
            
            File subDir = new File(testBaseDir.toFile(), "subdir");
            if (!subDir.mkdir()) {
                System.err.println("Failed to create subdir for testing.");
            }

            System.out.println("Test environment set up in: " + testBaseDir);
            System.out.println("------------------------------------------");

            // Test Case 1: Valid file
            System.out.println("Test Case 1: Valid file 'valid.txt'");
            String tempPath1 = copyFileToTemp(testBaseDir.toString(), "valid.txt");
            if (tempPath1 != null) {
                System.out.println("Success! Copied to: " + tempPath1);
                Files.deleteIfExists(Paths.get(tempPath1)); // Clean up
            } else {
                System.out.println("Failed unexpectedly.");
            }
            System.out.println();

            // Test Case 2: Non-existent file
            System.out.println("Test Case 2: Non-existent file 'nonexistent.txt'");
            String tempPath2 = copyFileToTemp(testBaseDir.toString(), "nonexistent.txt");
            if (tempPath2 == null) {
                System.out.println("Success! Operation failed as expected.");
            } else {
                System.out.println("Failed! An unexpected temporary file was created: " + tempPath2);
                 Files.deleteIfExists(Paths.get(tempPath2));
            }
            System.out.println();

            // Test Case 3: Path traversal attempt
            System.out.println("Test Case 3: Path traversal attempt '../test.txt'");
            String tempPath3 = copyFileToTemp(testBaseDir.toString(), "../test.txt");
            if (tempPath3 == null) {
                System.out.println("Success! Path traversal blocked as expected.");
            } else {
                System.out.println("Failed! Path traversal was successful: " + tempPath3);
                 Files.deleteIfExists(Paths.get(tempPath3));
            }
            System.out.println();
            
            // Test Case 4: Accessing a directory
            System.out.println("Test Case 4: Attempting to copy a directory 'subdir'");
            String tempPath4 = copyFileToTemp(testBaseDir.toString(), "subdir");
            if (tempPath4 == null) {
                System.out.println("Success! Directory access blocked as expected.");
            } else {
                System.out.println("Failed! Directory was copied: " + tempPath4);
                 Files.deleteIfExists(Paths.get(tempPath4));
            }
            System.out.println();

            // Test Case 5: Invalid base directory
            System.out.println("Test Case 5: Using a non-existent base directory");
            String nonExistentBaseDir = testBaseDir.resolve("nonexistent_dir").toString();
            String tempPath5 = copyFileToTemp(nonExistentBaseDir, "valid.txt");
            if (tempPath5 == null) {
                System.out.println("Success! Invalid base directory handled correctly.");
            } else {
                System.out.println("Failed! An unexpected file was created: " + tempPath5);
                Files.deleteIfExists(Paths.get(tempPath5));
            }

        } catch (IOException e) {
            System.err.println("Failed to set up test environment: " + e.getMessage());
        } finally {
            // Cleanup test directory
            if (testBaseDir != null) {
                try {
                    Files.walk(testBaseDir)
                         .map(Path::toFile)
                         .sorted((o1, o2) -> -o1.compareTo(o2))
                         .forEach(File::delete);
                } catch (IOException e) {
                    System.err.println("Failed to clean up test directory: " + testBaseDir);
                }
            }
        }
    }
}