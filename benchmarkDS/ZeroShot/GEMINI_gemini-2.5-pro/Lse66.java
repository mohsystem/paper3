import java.io.File;
import java.io.IOException;
import java.io.UncheckedIOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.util.Arrays;
import java.util.regex.Pattern;

public class Lse66 {

    // Define security constants
    private static final String UPLOAD_DIR = "uploads";
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 MB
    private static final byte[] PDF_MAGIC_BYTES = {0x25, 0x50, 0x44, 0x46, 0x2D}; // %PDF-
    // Whitelist for safe filename characters
    private static final Pattern SAFE_FILENAME_PATTERN = Pattern.compile("[^a-zA-Z0-9_.-]");

    /**
     * Securely saves a PDF file to the 'uploads' directory.
     *
     * @param originalFilename The original filename from the user.
     * @param fileContent      The byte content of the file.
     * @return true if the file was saved successfully, false otherwise.
     */
    public static boolean savePdf(String originalFilename, byte[] fileContent) {
        // 1. Check for null or empty inputs
        if (originalFilename == null || originalFilename.trim().isEmpty() || fileContent == null || fileContent.length == 0) {
            System.err.println("Error: Invalid filename or empty file content.");
            return false;
        }

        // 2. Security: Check file size
        if (fileContent.length > MAX_FILE_SIZE) {
            System.err.println("Error: File size exceeds the maximum limit of " + MAX_FILE_SIZE + " bytes.");
            return false;
        }

        // 3. Security: Validate file type by checking magic bytes
        if (fileContent.length < PDF_MAGIC_BYTES.length ||
            !Arrays.equals(Arrays.copyOfRange(fileContent, 0, PDF_MAGIC_BYTES.length), PDF_MAGIC_BYTES)) {
            System.err.println("Error: File is not a valid PDF.");
            return false;
        }

        // 4. Security: Prevent path traversal by extracting the base filename
        Path originalPath = Paths.get(originalFilename);
        String baseFilename = originalPath.getFileName().toString();

        // 5. Security: Sanitize the filename to remove unsafe characters
        String sanitizedFilename = SAFE_FILENAME_PATTERN.matcher(baseFilename).replaceAll("_");
        if (!sanitizedFilename.toLowerCase().endsWith(".pdf")) {
             // In case sanitization removes the extension, or it was wrong case
             int dotIndex = sanitizedFilename.lastIndexOf('.');
             if (dotIndex != -1) {
                 sanitizedFilename = sanitizedFilename.substring(0, dotIndex) + ".pdf";
             } else {
                 sanitizedFilename += ".pdf";
             }
        }


        try {
            // Ensure the upload directory exists
            Path uploadPath = Paths.get(UPLOAD_DIR);
            if (!Files.exists(uploadPath)) {
                Files.createDirectories(uploadPath);
            }

            // Construct the final, secure destination path
            Path destinationPath = uploadPath.resolve(sanitizedFilename).normalize();

            // Security: Final check to ensure the resolved path is still within the upload directory
            if (!destinationPath.startsWith(uploadPath)) {
                System.err.println("Error: Path traversal attempt detected after sanitization.");
                return false;
            }

            // Write the file
            Files.write(destinationPath, fileContent);
            System.out.println("Successfully saved file to: " + destinationPath);
            return true;

        } catch (IOException e) {
            System.err.println("Error saving file: " + e.getMessage());
            // In a real app, log the exception
            return false;
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Running Java Test Cases ---");

        // Test Case 1: Valid PDF file
        System.out.println("\n[Test 1] Valid PDF file");
        byte[] validPdfContent = "%PDF-1.7... some pdf content...".getBytes();
        savePdf("my_document.pdf", validPdfContent);

        // Test Case 2: Path traversal attempt
        System.out.println("\n[Test 2] Path traversal attempt");
        savePdf("../../etc/passwd", validPdfContent);

        // Test Case 3: Invalid file type (not a PDF)
        System.out.println("\n[Test 3] Invalid file type");
        byte[] nonPdfContent = "<?php echo 'hello'; ?>".getBytes();
        savePdf("shell.php.pdf", nonPdfContent);

        // Test Case 4: Filename with special characters
        System.out.println("\n[Test 4] Filename with special characters");
        savePdf("report<2023>*.pdf", validPdfContent);
        
        // Test Case 5: File too large
        System.out.println("\n[Test 5] File too large");
        byte[] largeContent = new byte[(int) MAX_FILE_SIZE + 1];
        // Ensure it still looks like a PDF for the magic byte check
        System.arraycopy(PDF_MAGIC_BYTES, 0, largeContent, 0, PDF_MAGIC_BYTES.length);
        savePdf("large_file.pdf", largeContent);

        System.out.println("\n--- Java Test Cases Finished ---");
    }
}