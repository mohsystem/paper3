import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Arrays;
import java.util.regex.Pattern;

public class Lse66 {

    // Define a secure location for uploads
    private static final String UPLOAD_DIRECTORY = "uploads";
    // Define a maximum file size to prevent Denial of Service (DoS) attacks (e.g., 10MB)
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024;
    // Pattern for allowed characters in a filename to prevent special character exploits
    private static final Pattern-allowedFilenamePattern = Pattern.compile("^[a-zA-Z0-9._-]+$");

    /**
     * Accepts a PDF file's name and content, validates it, and saves it to a
     * predefined 'uploads' folder.
     *
     * @param originalFilename The original filename from the user.
     * @param content The file content as a byte array.
     * @return A message indicating the result of the operation.
     */
    public static String savePdfFile(String originalFilename, byte[] content) {
        // 1. Security Check: File Size
        if (content.length > MAX_FILE_SIZE) {
            return "Error: File size exceeds the limit of " + MAX_FILE_SIZE / (1024 * 1024) + " MB.";
        }
        if (content.length == 0) {
            return "Error: File content cannot be empty.";
        }

        // 2. Security Check: Filename Validation and Sanitization
        if (originalFilename == null || originalFilename.trim().isEmpty()) {
            return "Error: Filename is missing.";
        }
        
        // Strip path information to prevent Path Traversal attacks (e.g., "../../etc/passwd")
        String filename = new File(originalFilename).getName();

        // 3. Security Check: File Type
        // Ensure the file is a PDF by checking the extension.
        if (!filename.toLowerCase().endsWith(".pdf")) {
            return "Error: Invalid file type. Only PDF files are allowed.";
        }

        // 4. Security Check: Sanitize filename characters
        // Although we strip paths, the filename itself could contain problematic characters.
        // A stricter approach is to whitelist characters.
        String sanitizedFilename = filename.replaceAll("[^a-zA-Z0-9._-]", "");
        if (!sanitizedFilename.equals(filename)) {
             // Or reject if a more strict policy is needed.
             System.out.println("Warning: Filename contained invalid characters. Sanitized to: " + sanitizedFilename);
        }

        if (sanitizedFilename.isEmpty() || sanitizedFilename.equals(".pdf")) {
            return "Error: Invalid filename after sanitization.";
        }

        try {
            // Create the upload directory if it doesn't exist.
            Path uploadPath = Paths.get(UPLOAD_DIRECTORY);
            if (!Files.exists(uploadPath)) {
                Files.createDirectories(uploadPath);
            }

            // Construct the final destination path securely.
            Path destinationPath = uploadPath.resolve(sanitizedFilename);

            // Security check: ensure the resolved path is still within the upload directory
            if (!destinationPath.toAbsolutePath().startsWith(uploadPath.toAbsolutePath())) {
                return "Error: Directory traversal attempt detected after path resolution.";
            }

            // Write the file. This will overwrite if the file already exists.
            try (FileOutputStream fos = new FileOutputStream(destinationPath.toFile())) {
                fos.write(content);
            }

            return "Success: File '" + sanitizedFilename + "' saved successfully in '" + UPLOAD_DIRECTORY + "' folder.";

        } catch (IOException e) {
            // Log the exception properly in a real application
            e.printStackTrace();
            return "Error: Could not save the file due to an I/O error.";
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Running Java Test Cases ---");
        byte[] validContent = "This is a dummy PDF content.".getBytes();
        byte[] largeContent = new byte[(int) (MAX_FILE_SIZE + 1)];
        Arrays.fill(largeContent, (byte)'a');

        // Test Case 1: Valid PDF file
        System.out.println("Test 1 (Valid): " + savePdfFile("MyReport_2023.pdf", validContent));

        // Test Case 2: Path Traversal attempt
        System.out.println("Test 2 (Path Traversal): " + savePdfFile("../../etc/passwd.pdf", validContent));

        // Test Case 3: Invalid file type
        System.out.println("Test 3 (Invalid Type): " + savePdfFile("image.jpg", validContent));

        // Test Case 4: Filename with malicious/invalid characters
        System.out.println("Test 4 (Invalid Chars): " + savePdfFile("a<b>c|d/e?f.pdf", validContent));

        // Test Case 5: File too large
        System.out.println("Test 5 (Too Large): " + savePdfFile("large_document.pdf", largeContent));
        System.out.println("--------------------------------\n");
    }
}