import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;
import java.util.UUID;
import java.util.regex.Pattern;

public class Lse72 {

    private static final String UPLOAD_DIR = "uploads";
    private static final long MAX_FILE_SIZE = 5 * 1024 * 1024; // 5 MB
    private static final Set<String> ALLOWED_EXTENSIONS = new HashSet<>(Arrays.asList(".txt", ".pdf", ".jpg", ".png"));
    // A regex to check for invalid characters in a filename.
    // This is a basic whitelist for letters, numbers, dots, underscores, and hyphens.
    private static final Pattern FILENAME_PATTERN = Pattern.compile("^[a-zA-Z0-9._-]+$");

    /**
     * Securely saves an uploaded file.
     *
     * @param originalFilename The original name of the file provided by the user.
     * @param fileContent      The binary content of the file.
     * @return The path to the saved file on success, or null on failure.
     */
    public static String upload_file(String originalFilename, byte[] fileContent) {
        if (originalFilename == null || originalFilename.isEmpty() || fileContent == null) {
            System.err.println("Error: Filename or content is null/empty.");
            return null;
        }

        // 1. Check file size
        if (fileContent.length > MAX_FILE_SIZE) {
            System.err.println("Error: File size exceeds the limit of " + MAX_FILE_SIZE + " bytes.");
            return null;
        }

        // 2. Sanitize filename to prevent path traversal
        // Paths.get().getFileName() extracts the final component of the path, defeating ".." attacks.
        String baseName = Paths.get(originalFilename).getFileName().toString();

        // Check for null bytes which can terminate strings prematurely in some environments
        if (baseName.contains("\0")) {
            System.err.println("Error: Filename contains null byte.");
            return null;
        }

        // Additional check for potentially harmful characters
        if (!FILENAME_PATTERN.matcher(baseName).matches()) {
             System.err.println("Error: Filename '" + baseName + "' contains invalid characters.");
             return null;
        }

        // 3. Validate file extension
        String extension = getFileExtension(baseName);
        if (extension.isEmpty() || !ALLOWED_EXTENSIONS.contains(extension.toLowerCase())) {
            System.err.println("Error: File type '" + extension + "' is not allowed.");
            return null;
        }

        try {
            // 4. Create the upload directory if it doesn't exist
            Path uploadPath = Paths.get(UPLOAD_DIR);
            if (!Files.exists(uploadPath)) {
                Files.createDirectories(uploadPath);
                System.out.println("Created directory: " + uploadPath.toAbsolutePath());
            }

            // 5. Generate a unique filename to prevent overwrites
            String uniqueFileName = UUID.randomUUID().toString() + "_" + baseName;
            Path destinationPath = uploadPath.resolve(uniqueFileName);

            // Ensure the resolved path doesn't escape the upload directory (a final safeguard)
            if (!destinationPath.toAbsolutePath().normalize().startsWith(uploadPath.toAbsolutePath().normalize())) {
                System.err.println("Error: Final path is outside the designated upload directory.");
                return null;
            }

            // 6. Save the file
            Files.write(destinationPath, fileContent);
            System.out.println("File saved successfully to: " + destinationPath.toAbsolutePath());
            return destinationPath.toString();

        } catch (IOException | SecurityException e) {
            System.err.println("Error saving file: " + e.getMessage());
            return null;
        }
    }

    private static String getFileExtension(String filename) {
        int lastIndexOfDot = filename.lastIndexOf('.');
        if (lastIndexOfDot == -1) {
            return ""; // No extension
        }
        return filename.substring(lastIndexOfDot);
    }

    public static void main(String[] args) {
        System.out.println("--- Running Test Cases ---");

        // Test Case 1: Normal, valid file upload
        System.out.println("\n[Test 1] Normal file upload");
        upload_file("report.txt", "This is the content of the report.".getBytes());

        // Test Case 2: Path traversal attempt
        System.out.println("\n[Test 2] Path traversal attempt");
        upload_file("../../etc/passwd", "root:x:0:0:root:/root:/bin/bash".getBytes());

        // Test Case 3: File too large
        System.out.println("\n[Test 3] File too large");
        byte[] largeContent = new byte[(int)MAX_FILE_SIZE + 1];
        upload_file("largefile.jpg", largeContent);

        // Test Case 4: Disallowed file type
        System.out.println("\n[Test 4] Disallowed file type");
        upload_file("malicious_script.sh", "#!/bin/bash\nrm -rf /".getBytes());

        // Test Case 5: Filename with invalid characters
        System.out.println("\n[Test 5] Filename with invalid characters");
        upload_file("file with spaces & symbols!.png", "some image data".getBytes());
        
        System.out.println("\n--- Test Cases Finished ---");
    }
}