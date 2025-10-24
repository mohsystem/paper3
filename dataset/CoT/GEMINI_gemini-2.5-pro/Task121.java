import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.file.InvalidPathException;
import java.nio.file.Paths;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

public class Task121 {

    // Configuration for file uploads
    private static final String UPLOAD_DIRECTORY = "uploads";
    private static final long MAX_FILE_SIZE = 5 * 1024 * 1024; // 5 MB
    private static final Set<String> ALLOWED_EXTENSIONS = new HashSet<>(Arrays.asList(".txt", ".pdf", ".jpg", ".png"));

    /**
     * Simulates securely saving an uploaded file.
     * It performs security checks for file size, path traversal, and file extension.
     *
     * @param originalFilename The filename provided by the user.
     * @param fileContent The byte content of the file.
     * @return true if the file was saved successfully, false otherwise.
     */
    public static boolean saveUploadedFile(String originalFilename, byte[] fileContent) {
        // 1. Security Check: File Size
        if (fileContent.length > MAX_FILE_SIZE) {
            System.err.println("Error: File size exceeds the " + (MAX_FILE_SIZE / 1024 / 1024) + "MB limit.");
            return false;
        }

        // 2. Security Check: Path Traversal
        // Normalize the path and get the filename part. This strips directory info.
        String sanitizedFilename;
        try {
            sanitizedFilename = Paths.get(originalFilename).getFileName().toString();
            if (sanitizedFilename.isEmpty()) {
                 System.err.println("Error: Invalid filename provided.");
                 return false;
            }
        } catch (InvalidPathException e) {
            System.err.println("Error: Invalid characters in filename.");
            return false;
        }

        // 3. Security Check: File Extension Whitelist
        int dotIndex = sanitizedFilename.lastIndexOf('.');
        if (dotIndex == -1 || dotIndex == 0) {
            System.err.println("Error: File has no extension or is a hidden file.");
            return false;
        }
        String extension = sanitizedFilename.substring(dotIndex).toLowerCase();
        if (!ALLOWED_EXTENSIONS.contains(extension)) {
            System.err.println("Error: File type '" + extension + "' is not allowed.");
            return false;
        }

        // 4. Construct the final safe path
        File uploadDir = new File(UPLOAD_DIRECTORY);
        if (!uploadDir.exists() && !uploadDir.mkdirs()) {
             System.err.println("Error: Could not create upload directory.");
             return false;
        }
        File destinationFile = new File(uploadDir.getAbsolutePath() + File.separator + sanitizedFilename);

        // Prevent overwriting existing files, a good practice.
        if (destinationFile.exists()) {
            System.err.println("Error: A file with this name already exists.");
            return false;
        }
        
        // 5. Write the file to the secure location
        try (FileOutputStream fos = new FileOutputStream(destinationFile)) {
            fos.write(fileContent);
            System.out.println("Success: File '" + sanitizedFilename + "' uploaded to '" + UPLOAD_DIRECTORY + "' directory.");
            return true;
        } catch (IOException e) {
            System.err.println("Error: Could not save the file. " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Running Java File Upload Tests ---");
        byte[] smallContent = "This is a test file content.".getBytes();
        byte[] largeContent = new byte[(int)MAX_FILE_SIZE + 1]; // Content that is too large

        // Test Case 1: Valid file upload
        System.out.println("\n[Test Case 1: Valid file]");
        saveUploadedFile("document.txt", smallContent);

        // Test Case 2: Invalid file extension
        System.out.println("\n[Test Case 2: Invalid extension]");
        saveUploadedFile("malicious_script.exe", smallContent);

        // Test Case 3: Path Traversal attempt
        System.out.println("\n[Test Case 3: Path Traversal]");
        saveUploadedFile("../../etc/passwd", smallContent);
        
        // Test Case 4: File too large
        System.out.println("\n[Test Case 4: File too large]");
        saveUploadedFile("large_file.pdf", largeContent);

        // Test Case 5: No extension
        System.out.println("\n[Test Case 5: No extension]");
        saveUploadedFile("filewithnoextension", smallContent);
        
        // Cleanup the created file for consistency
        new File(UPLOAD_DIRECTORY + File.separator + "document.txt").delete();
    }
}