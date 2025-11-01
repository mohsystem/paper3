import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.UUID;
import java.util.logging.Level;
import java.util.logging.Logger;

public class Lse64 {

    private static final String UPLOAD_DIR = "uploads";
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 MB
    private static final byte[] PDF_MAGIC_BYTES = {0x25, 0x50, 0x44, 0x46, 0x2D}; // %PDF-

    /**
     * Simulates uploading a PDF file.
     *
     * @param originalFilename The original filename from the user.
     * @param fileContent The byte content of the file.
     * @return The sanitized filepath where the file was saved, or null on failure.
     */
    public static String uploadPdf(String originalFilename, byte[] fileContent) {
        if (originalFilename == null || originalFilename.trim().isEmpty() || fileContent == null) {
            System.out.println("Error: Invalid input provided.");
            return null;
        }

        // 1. Security: Check file size
        if (fileContent.length > MAX_FILE_SIZE) {
            System.out.println("Error: File size exceeds the limit of " + MAX_FILE_SIZE + " bytes.");
            return null;
        }

        // 2. Security: Validate file type using magic bytes
        if (fileContent.length < PDF_MAGIC_BYTES.length) {
             System.out.println("Error: Invalid file type. Not a PDF.");
             return null;
        }
        for (int i = 0; i < PDF_MAGIC_BYTES.length; i++) {
            if (fileContent[i] != PDF_MAGIC_BYTES[i]) {
                System.out.println("Error: Invalid file type. Not a PDF.");
                return null;
            }
        }

        // 3. Security: Prevent path traversal by extracting only the filename
        Path originalPath = Paths.get(originalFilename);
        String baseFilename = originalPath.getFileName().toString();

        // Ensure the filename ends with .pdf, but don't solely rely on it for security
        if (!baseFilename.toLowerCase().endsWith(".pdf")) {
             System.out.println("Error: Filename must end with .pdf.");
             return null;
        }

        // 4. Security: Generate a unique filename to prevent overwrites
        String uniqueFilename = UUID.randomUUID().toString() + ".pdf";
        
        try {
            Path uploadPath = Paths.get(UPLOAD_DIR);
            // Create the directory if it doesn't exist
            if (!Files.exists(uploadPath)) {
                Files.createDirectories(uploadPath);
            }

            Path destinationPath = uploadPath.resolve(uniqueFilename);

            // Save the file
            Files.write(destinationPath, fileContent);
            System.out.println("File saved successfully to: " + destinationPath);

            // 5. Security: Simulate saving the filepath to a database using a prepared statement
            saveFilePathToDatabase(destinationPath.toString());
            
            return destinationPath.toString();
        } catch (IOException e) {
            Logger.getLogger(Lse64.class.getName()).log(Level.SEVERE, "File operation failed", e);
            return null;
        }
    }
    
    /**
     * Simulates saving a filepath to a MySQL database using parameterized queries to prevent SQL Injection.
     * @param filePath The path to save.
     */
    private static void saveFilePathToDatabase(String filePath) {
        System.out.println("\n--- Simulating Database Interaction ---");
        // This is a placeholder for actual database logic
        // In a real application, you would use JDBC and a PreparedStatement
        String sql = "INSERT INTO pdf_files (filepath) VALUES (?);";
        System.out.println("SQL (Prepared Statement): " + sql);
        System.out.println("Binding parameter 1: " + filePath);
        System.out.println("Executing query...");
        System.out.println("Filepath successfully saved in the database.");
        System.out.println("-------------------------------------\n");
    }

    public static void main(String[] args) {
        System.out.println("--- Running Java Test Cases ---");

        // Test Case 1: Valid PDF upload
        System.out.println("--- Test Case 1: Valid PDF ---");
        byte[] validPdfContent = "%PDF-1.5\n...".getBytes();
        uploadPdf("document.pdf", validPdfContent);

        // Test Case 2: Invalid file type (not a PDF)
        System.out.println("--- Test Case 2: Invalid File Type (TXT) ---");
        byte[] invalidContent = "This is a text file.".getBytes();
        uploadPdf("not_a_pdf.txt", invalidContent);
        
        // Test Case 3: Path traversal attempt
        System.out.println("--- Test Case 3: Path Traversal Attempt ---");
        uploadPdf("../../etc/passwd.pdf", validPdfContent);

        // Test Case 4: File too large
        System.out.println("--- Test Case 4: File Too Large ---");
        byte[] largeContent = new byte[(int) MAX_FILE_SIZE + 1];
        System.arraycopy(validPdfContent, 0, largeContent, 0, validPdfContent.length);
        uploadPdf("large_file.pdf", largeContent);

        // Test Case 5: Another valid PDF to show unique naming
        System.out.println("--- Test Case 5: Another Valid PDF ---");
        uploadPdf("report.pdf", validPdfContent);
    }
}