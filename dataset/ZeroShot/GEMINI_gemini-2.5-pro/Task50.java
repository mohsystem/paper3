import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.util.Arrays;
import java.util.List;
import java.util.UUID;
import java.io.File;

public class Task50 {

    private static final long MAX_FILE_SIZE_BYTES = 10 * 1024 * 1024; // 10 MB
    private static final List<String> ALLOWED_EXTENSIONS = Arrays.asList(".txt", ".jpg", ".png", ".pdf", ".docx");
    private static final String UPLOAD_DIRECTORY = "java_uploads";

    /**
     * Securely "uploads" a file by saving it to a designated directory after security checks.
     *
     * @param originalFileName The name of the file provided by the user.
     * @param fileContent The byte content of the file.
     * @return A confirmation or error message.
     */
    public static String uploadFile(String originalFileName, byte[] fileContent) {
        // 1. Check for null or empty inputs
        if (originalFileName == null || originalFileName.trim().isEmpty() || fileContent == null) {
            return "Error: Invalid file name or content.";
        }
        if (fileContent.length == 0) {
            return "Error: File content cannot be empty.";
        }
        
        // 2. Check file size
        if (fileContent.length > MAX_FILE_SIZE_BYTES) {
            return "Error: File size exceeds the limit of " + (MAX_FILE_SIZE_BYTES / (1024 * 1024)) + " MB.";
        }

        // 3. Validate file extension against a whitelist
        String extension = getFileExtension(originalFileName);
        if (extension.isEmpty() || !ALLOWED_EXTENSIONS.contains(extension.toLowerCase())) {
            return "Error: File type is not allowed. Allowed types are: " + ALLOWED_EXTENSIONS;
        }

        // 4. Sanitize filename to prevent path traversal and other attacks
        // - Get only the base name of the file, stripping any directory info
        // - Generate a unique name to prevent overwrites and hide original naming
        String sanitizedBaseName = Paths.get(originalFileName).getFileName().toString();
        String uniqueFileName = UUID.randomUUID().toString() + "_" + sanitizedBaseName;

        // Ensure the final name does not contain any null bytes
        if (uniqueFileName.indexOf('\0') != -1) {
            return "Error: Invalid characters in filename.";
        }

        try {
            // 5. Create the upload directory if it doesn't exist.
            Path uploadPath = Paths.get(UPLOAD_DIRECTORY);
            if (!Files.exists(uploadPath)) {
                Files.createDirectories(uploadPath);
            }
            
            // 6. Define the destination path securely inside the upload directory
            Path destinationPath = uploadPath.resolve(uniqueFileName);
            
            // Double-check that the resolved path is still within the upload directory
            if (!destinationPath.toAbsolutePath().startsWith(uploadPath.toAbsolutePath())) {
                return "Error: Directory traversal attempt detected.";
            }

            // 7. Write the file to the server's filesystem
            Files.write(destinationPath, fileContent);
            
            return "Success: File '" + sanitizedBaseName + "' uploaded as '" + uniqueFileName + "'.";
        } catch (IOException e) {
            // Log the exception server-side
            System.err.println("File upload failed: " + e.getMessage());
            return "Error: Could not save the file.";
        }
    }

    private static String getFileExtension(String fileName) {
        int lastIndexOfDot = fileName.lastIndexOf(".");
        if (lastIndexOfDot == -1) {
            return ""; // No extension
        }
        return fileName.substring(lastIndexOfDot);
    }

    public static void main(String[] args) {
        System.out.println("--- Running Java File Upload Tests ---");

        // Test Case 1: Successful upload of a valid text file
        String validContent1 = "This is a simple text file.";
        String result1 = uploadFile("mydocument.txt", validContent1.getBytes());
        System.out.println("Test 1 (Valid .txt): " + result1);

        // Test Case 2: Successful upload of a valid image file (simulated content)
        byte[] validImageContent = { (byte)0x89, 'P', 'N', 'G', '\r', '\n', (byte)0x1a, '\n' };
        String result2 = uploadFile("photo.png", validImageContent);
        System.out.println("Test 2 (Valid .png): " + result2);

        // Test Case 3: Rejected upload due to disallowed file type
        String invalidContent3 = "#!/bin/bash\necho 'pwned'";
        String result3 = uploadFile("malicious_script.sh", invalidContent3.getBytes());
        System.out.println("Test 3 (Invalid extension .sh): " + result3);

        // Test Case 4: Rejected upload due to path traversal attempt in filename
        String validContent4 = "some data";
        String result4 = uploadFile("../../etc/passwd", validContent4.getBytes());
        System.out.println("Test 4 (Path Traversal): " + result4);

        // Test Case 5: Rejected upload due to file size limit
        byte[] largeContent = new byte[(int)MAX_FILE_SIZE_BYTES + 1];
        String result5 = uploadFile("largefile.docx", largeContent);
        System.out.println("Test 5 (File too large): " + result5);
        
        System.out.println("--------------------------------------\n");
    }
}