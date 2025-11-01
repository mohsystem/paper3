import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

public class Lse72 {

    /**
     * Handles the upload of a file by saving content to a specified filename
     * in an 'uploads' directory. It creates the directory if it doesn't exist
     * and sanitizes the filename to prevent path traversal attacks.
     *
     * @param filename The desired name of the file. Path information will be stripped.
     * @param content  The string content to write to the file.
     * @return true if the file was saved successfully, false otherwise.
     */
    public static boolean upload_file(String filename, String content) {
        if (filename == null || filename.trim().isEmpty() || content == null) {
            System.err.println("Error: Filename or content is null/empty.");
            return false;
        }

        // --- Security: Path Traversal Prevention ---
        // Use Path object to safely extract only the filename, stripping any directory info.
        Path path = Paths.get(filename);
        String sanitizedFilename = path.getFileName().toString();

        // Additional check to ensure the sanitized name isn't empty or a dot-file.
        if (sanitizedFilename.isEmpty() || sanitizedFilename.equals(".") || sanitizedFilename.equals("..")) {
            System.err.println("Error: Invalid filename provided: " + filename);
            return false;
        }

        String uploadDirName = "uploads";
        File uploadDir = new File(uploadDirName);

        // Create the uploads directory if it doesn't exist.
        if (!uploadDir.exists()) {
            if (!uploadDir.mkdir()) {
                System.err.println("Error: Could not create directory: " + uploadDirName);
                return false;
            }
        }

        File destinationFile = new File(uploadDir, sanitizedFilename);
        
        // --- Security: Final check to prevent writing outside the target directory ---
        // This is a redundant but good practice check.
        try {
            if (!destinationFile.getCanonicalPath().startsWith(uploadDir.getCanonicalPath() + File.separator)) {
                 System.err.println("Error: Final path check failed. Attempted write outside of uploads directory.");
                 return false;
            }
        } catch (IOException e) {
            System.err.println("Error resolving canonical path: " + e.getMessage());
            return false;
        }


        // Write the content to the file using try-with-resources for automatic closing.
        try (FileWriter writer = new FileWriter(destinationFile)) {
            writer.write(content);
            System.out.println("Successfully saved file: " + destinationFile.getPath());
            return true;
        } catch (IOException e) {
            System.err.println("Error writing to file: " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Java Test Cases ---");

        // Test Case 1: Valid simple filename
        upload_file("test1.txt", "This is a valid test file.");

        // Test Case 2: Another valid filename
        upload_file("mydocument.log", "Log entry content.");

        // Test Case 3: Path traversal attempt (should fail)
        upload_file("../../etc/passwd", "malicious content");

        // Test Case 4: Absolute path attempt (should fail)
        upload_file("/tmp/hacked.txt", "more malicious content");

        // Test Case 5: Invalid filename
        upload_file("..", "invalid filename content");
        
        System.out.println("-----------------------\n");
    }
}