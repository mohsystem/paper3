import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.UUID;

public class Task50 {

    // Define a safe directory for uploads.
    // In a real application, this should be outside the web root and configured carefully.
    private static final String UPLOAD_DIRECTORY = "uploads_java";
    // Define a maximum file size to prevent denial-of-service attacks (e.g., 5MB).
    private static final long MAX_FILE_SIZE = 5 * 1024 * 1024;

    /**
     * Simulates uploading a file to the server.
     *
     * @param originalFileName The name of the file provided by the user.
     * @param fileContent      The binary content of the file.
     * @return A confirmation or error message.
     */
    public static String uploadFile(String originalFileName, byte[] fileContent) {
        // Security: Check for null or empty inputs
        if (originalFileName == null || originalFileName.trim().isEmpty()) {
            return "Error: File name cannot be empty.";
        }
        if (fileContent == null || fileContent.length == 0) {
            return "Error: File content cannot be empty.";
        }

        // Security: Check file size to prevent DoS attacks
        if (fileContent.length > MAX_FILE_SIZE) {
            return "Error: File size exceeds the " + (MAX_FILE_SIZE / (1024 * 1024)) + "MB limit.";
        }

        try {
            // Security: Prevent path traversal attacks.
            // This extracts just the filename, discarding any directory information.
            // e.g., "../../etc/passwd" becomes "passwd"
            String sanitizedFileName = Paths.get(originalFileName).getFileName().toString();

            if (sanitizedFileName.isEmpty()) {
                 return "Error: Invalid file name provided.";
            }

            // Security: Generate a unique filename to prevent overwriting existing files
            // and to avoid issues with file names containing special characters.
            String fileExtension = "";
            int i = sanitizedFileName.lastIndexOf('.');
            if (i > 0) {
                fileExtension = sanitizedFileName.substring(i);
            }
            String uniqueFileName = UUID.randomUUID().toString() + fileExtension;

            // Create the upload directory if it doesn't exist
            Path uploadPath = Paths.get(UPLOAD_DIRECTORY);
            if (!Files.exists(uploadPath)) {
                Files.createDirectories(uploadPath);
            }

            // Construct the final, safe destination path
            Path destinationPath = uploadPath.resolve(uniqueFileName);

            // Write the file using try-with-resources to ensure the stream is closed
            try (FileOutputStream fos = new FileOutputStream(destinationPath.toFile())) {
                fos.write(fileContent);
            }

            return "Success: File '" + originalFileName + "' uploaded as '" + uniqueFileName + "'.";

        } catch (IOException e) {
            // Log the exception in a real application
            // e.printStackTrace();
            return "Error: Could not save the file. " + e.getMessage();
        } catch (Exception e) {
            // Catch any other unexpected errors
            return "Error: An unexpected error occurred. " + e.getMessage();
        }
    }

    public static void main(String[] args) {
        // Create the upload directory for the test cases
        new File(UPLOAD_DIRECTORY).mkdirs();

        System.out.println("--- Running Java Test Cases ---");

        // Test Case 1: Normal successful upload
        String content1 = "This is a test file.";
        System.out.println("Test 1 (Normal): " + uploadFile("report.txt", content1.getBytes()));

        // Test Case 2: Uploading a file with the same name (should not overwrite)
        String content2 = "This is another test file.";
        System.out.println("Test 2 (Duplicate Name): " + uploadFile("report.txt", content2.getBytes()));

        // Test Case 3: Path traversal attack attempt
        String content3 = "malicious content";
        System.out.println("Test 3 (Path Traversal): " + uploadFile("../../etc/passwd", content3.getBytes()));

        // Test Case 4: File size too large
        byte[] largeContent = new byte[(int) MAX_FILE_SIZE + 1];
        System.out.println("Test 4 (Oversized File): " + uploadFile("large_file.zip", largeContent));

        // Test Case 5: File with spaces and special characters in name
        String content5 = "Final document content.";
        System.out.println("Test 5 (Special Chars): " + uploadFile("My Report (Final Version).docx", content5.getBytes()));
        
        System.out.println("---------------------------------");
    }
}