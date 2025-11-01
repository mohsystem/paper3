import java.io.IOException;
import java.nio.file.*;
import java.nio.file.attribute.FileAttribute;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.Arrays;
import java.util.regex.Pattern;
import java.nio.charset.StandardCharsets;

public class Lse66 {

    // Define a safe location for uploads. In a real app, this should be configurable and outside the web root.
    private static final String UPLOADS_DIR_NAME = "uploads";
    private static final Path UPLOADS_DIR = Paths.get(UPLOADS_DIR_NAME).toAbsolutePath();
    
    // Set a reasonable maximum file size (e.g., 10 MB) to prevent denial of service.
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024;

    // A strict regex for valid filenames. Allows letters, numbers, underscore, hyphen, and period.
    private static final Pattern FILENAME_PATTERN = Pattern.compile("^[a-zA-Z0-9_.-]+$");

    /**
     * Accepts a PDF file's content and saves it securely to an 'uploads' folder.
     *
     * @param fileName The name of the file provided by the user.
     * @param fileContent The byte content of the file.
     * @return true if the file was saved successfully, false otherwise.
     */
    public static boolean savePdf(String fileName, byte[] fileContent) {
        // 1. Input validation: size, nulls, and content.
        if (fileName == null || fileName.trim().isEmpty() || fileContent == null || fileContent.length == 0) {
            System.err.println("Error: File name or content is empty.");
            return false;
        }

        if (fileContent.length > MAX_FILE_SIZE) {
            System.err.println("Error: File size exceeds the maximum limit of " + MAX_FILE_SIZE + " bytes.");
            return false;
        }

        // Basic magic byte check for PDF (%PDF-).
        byte[] pdfMagicBytes = new byte[]{'%', 'P', 'D', 'F', '-'};
        if (fileContent.length < pdfMagicBytes.length || 
            !Arrays.equals(Arrays.copyOfRange(fileContent, 0, pdfMagicBytes.length), pdfMagicBytes)) {
            System.err.println("Error: File is not a valid PDF.");
            return false;
        }

        // 2. Sanitize filename to prevent path traversal and other attacks.
        // Get only the base name of the file, stripping any directory info.
        String sanitizedFileName = Paths.get(fileName).getFileName().toString();

        // Validate against a strict whitelist and ensure it ends with .pdf.
        if (!FILENAME_PATTERN.matcher(sanitizedFileName).matches() || !sanitizedFileName.toLowerCase().endsWith(".pdf")) {
            System.err.println("Error: Invalid filename. It contains illegal characters or is not a .pdf file.");
            return false;
        }

        try {
            // Create the uploads directory if it doesn't exist.
            // On POSIX systems, create with specific permissions (e.g., rwxr-x---).
            if (System.getProperty("os.name").toLowerCase().contains("win")) {
                 Files.createDirectories(UPLOADS_DIR);
            } else {
                FileAttribute<?> permissions = PosixFilePermissions.asFileAttribute(PosixFilePermissions.fromString("rwxr-x---"));
                Files.createDirectories(UPLOADS_DIR, permissions);
            }
           
            // 3. Construct the final destination path and verify it's within the upload directory.
            Path destinationPath = UPLOADS_DIR.resolve(sanitizedFileName).normalize();
            if (!destinationPath.startsWith(UPLOADS_DIR)) {
                System.err.println("Error: Path traversal attempt detected.");
                return false;
            }

            // 4. Secure write: Write to a temporary file and then atomically move it.
            // This prevents race conditions and partial file reads.
            Path tempFile = null;
            try {
                // Create a temporary file in the same directory to ensure atomic move is possible.
                tempFile = Files.createTempFile(UPLOADS_DIR, "upload-", ".tmp");
                Files.write(tempFile, fileContent, StandardOpenOption.WRITE);
                
                // Atomically move the temporary file to the final destination.
                // This will fail if the destination file already exists, which is a safe default.
                Files.move(tempFile, destinationPath, StandardCopyOption.ATOMIC_MOVE, StandardCopyOption.NOSHARE_DELETE);
                System.out.println("Successfully saved file: " + destinationPath);
                return true;
            } catch (FileAlreadyExistsException e) {
                 System.err.println("Error: File already exists at destination: " + destinationPath);
                 return false;
            } finally {
                // Ensure temporary file is deleted on failure.
                if (tempFile != null && Files.exists(tempFile)) {
                    Files.delete(tempFile);
                }
            }
        } catch (IOException | SecurityException e) {
            System.err.println("An error occurred during file processing: " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        System.out.println("Running test cases...");

        // Test Case 1: Valid PDF file
        System.out.println("\n--- Test Case 1: Valid PDF ---");
        byte[] validPdfContent = "%PDF-1.4\n... a valid pdf content ...".getBytes(StandardCharsets.UTF_8);
        savePdf("report.pdf", validPdfContent);

        // Test Case 2: Path traversal attempt
        System.out.println("\n--- Test Case 2: Path Traversal ---");
        savePdf("../../../etc/passwd", validPdfContent);

        // Test Case 3: Non-PDF file
        System.out.println("\n--- Test Case 3: Non-PDF File ---");
        byte[] textContent = "This is just a text file.".getBytes(StandardCharsets.UTF_8);
        savePdf("not_a_pdf.txt", textContent);

        // Test Case 4: File too large
        System.out.println("\n--- Test Case 4: File Too Large ---");
        byte[] largeContent = new byte[(int) MAX_FILE_SIZE + 1];
        Arrays.fill(largeContent, (byte) 'a');
        System.arraycopy(validPdfContent, 0, largeContent, 0, validPdfContent.length); // make it a "pdf"
        savePdf("large_file.pdf", largeContent);

        // Test Case 5: Invalid filename
        System.out.println("\n--- Test Case 5: Invalid Filename ---");
        savePdf("file_with_null\0.pdf", validPdfContent);
    }
}