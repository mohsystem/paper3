import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Comparator;

public class Task70 {

    /**
     * Securely reads the content of a file specified by a path.
     * The path is validated to prevent directory traversal attacks. The file must
     * exist within the current working directory.
     *
     * @param userPath The path to the file provided by the user.
     * @return The content of the file as a String, or an error message if the operation fails.
     */
    public static String readFileContent(String userPath) {
        if (userPath == null || userPath.isEmpty()) {
            return "Error: File path cannot be null or empty.";
        }

        // Define the allowed base directory (current working directory)
        File baseDir = new File(System.getProperty("user.dir"));
        File requestedFile = new File(baseDir, userPath);

        try {
            // Get canonical paths to resolve ".." and symbolic links
            String canonicalBasePath = baseDir.getCanonicalPath();
            String canonicalRequestPath = requestedFile.getCanonicalPath();

            // Security Check 1: Ensure the requested file path is within the base directory
            if (!canonicalRequestPath.startsWith(canonicalBasePath)) {
                return "Error: Directory traversal attempt detected. Access denied.";
            }

            // Security Check 2: Ensure the path points to a regular file and not a directory or other type
            if (!requestedFile.exists()) {
                return "Error: File does not exist.";
            }
            if (!requestedFile.isFile()) {
                return "Error: Path does not point to a regular file.";
            }
            if (!requestedFile.canRead()) {
                return "Error: File cannot be read (permission denied).";
            }
            
            // Limit file size to prevent DoS attacks (e.g., 10MB)
            long fileSize = requestedFile.length();
            long maxSize = 10 * 1024 * 1024; // 10 MB
            if (fileSize > maxSize) {
                return "Error: File is too large.";
            }

            // Read the file content
            StringBuilder contentBuilder = new StringBuilder();
            try (BufferedReader br = new BufferedReader(new FileReader(requestedFile))) {
                String line;
                while ((line = br.readLine()) != null) {
                    contentBuilder.append(line).append(System.lineSeparator());
                }
            }
            return contentBuilder.toString();

        } catch (IOException e) {
            // This can happen if getCanonicalPath fails (e.g., file doesn't exist during resolution)
            // or if there's an I/O error during reading.
            return "Error: An I/O error occurred: " + e.getMessage();
        } catch (SecurityException e) {
            return "Error: A security violation occurred: " + e.getMessage();
        }
    }

    // Main method with test cases
    public static void main(String[] args) {
        // Handle command-line argument if provided
        if (args.length > 0) {
            System.out.println("Reading file from command line argument: " + args[0]);
            String content = readFileContent(args[0]);
            System.out.println("--- File Content ---");
            System.out.println(content);
            System.out.println("--- End of Content ---");
            return; // Exit after processing command-line arg
        }
        
        // Setup for integrated test cases
        System.out.println("Running integrated test cases...");
        Path testSubDir = Paths.get("test_dir");
        Path safeFile = Paths.get("safe.txt");
        Path nestedFile = testSubDir.resolve("nested.txt");
        
        try {
            // Create test files and directories
            Files.createDirectories(testSubDir);
            Files.write(safeFile, "This is a safe file.".getBytes());
            Files.write(nestedFile, "This is a nested file.".getBytes());

            // --- Test Cases ---
            System.out.println("\n--- Test Case 1: Valid file in CWD ---");
            System.out.println("Input: " + safeFile.toString());
            System.out.println("Output: \n" + readFileContent(safeFile.toString()));

            System.out.println("\n--- Test Case 2: Valid nested file ---");
            System.out.println("Input: " + nestedFile.toString());
            System.out.println("Output: \n" + readFileContent(nestedFile.toString()));

            System.out.println("\n--- Test Case 3: Path Traversal attempt ---");
            String traversalPath = "../safe.txt"; // A simple traversal example
            System.out.println("Input: " + traversalPath);
            System.out.println("Output: \n" + readFileContent(traversalPath));

            System.out.println("\n--- Test Case 4: Non-existent file ---");
            String nonExistentFile = "nonexistent.txt";
            System.out.println("Input: " + nonExistentFile);
            System.out.println("Output: \n" + readFileContent(nonExistentFile));

            System.out.println("\n--- Test Case 5: Attempt to read a directory ---");
            System.out.println("Input: " + testSubDir.toString());
            System.out.println("Output: \n" + readFileContent(testSubDir.toString()));

        } catch (IOException e) {
            System.err.println("Failed to set up test environment: " + e.getMessage());
        } finally {
            // Clean up created files and directories
            try {
                if (Files.exists(testSubDir)) {
                    Files.walk(testSubDir)
                        .sorted(Comparator.reverseOrder())
                        .map(Path::toFile)
                        .forEach(File::delete);
                }
                Files.deleteIfExists(safeFile);
            } catch (IOException e) {
                System.err.println("Failed to clean up test environment: " + e.getMessage());
            }
        }
    }
}