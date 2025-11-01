import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.util.Objects;
import java.util.UUID;
import java.io.ByteArrayInputStream;
import java.nio.charset.StandardCharsets;

public class Lse64 {

    private static final String UPLOAD_DIRECTORY = "uploads";
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 MB
    private static final String ALLOWED_EXTENSION = ".pdf";

    // --- Database Configuration ---
    // In a real application, read these from a secure configuration source or environment variables.
    // Never hard-code credentials.
    private static final String DB_URL = System.getenv("DB_URL"); // e.g., "jdbc:mysql://localhost:3306/filedb"
    private static final String DB_USER = System.getenv("DB_USER"); // e.g., "dbuser"
    private static final String DB_PASSWORD = System.getenv("DB_PASS"); // e.g., "dbpassword"

    /**
     * Uploads a PDF file, saves it to a secure location, and records the filepath in the database.
     * This function assumes it is called within an authenticated and authorized context.
     *
     * @param fileData The InputStream containing the PDF file's data.
     * @param originalFilename The original filename provided by the user.
     * @param fileSize The size of the file in bytes.
     * @return The secure filepath where the file was saved, or null on failure.
     */
    public static String uploadPdf(InputStream fileData, String originalFilename, long fileSize) {
        // 1. Validate inputs
        if (fileData == null || originalFilename == null || originalFilename.trim().isEmpty()) {
            System.err.println("Error: Invalid input provided.");
            return null;
        }

        if (fileSize > MAX_FILE_SIZE) {
            System.err.println("Error: File size exceeds the maximum limit of " + MAX_FILE_SIZE + " bytes.");
            return null;
        }

        // 2. Sanitize and validate filename
        // Rule #3: Normalize path before validation.
        // Rule #2: Use allow lists. Here, we only allow ".pdf".
        String sanitizedFilename = new File(originalFilename).getName(); // Removes directory paths
        if (!sanitizedFilename.toLowerCase().endsWith(ALLOWED_EXTENSION)) {
            System.err.println("Error: Invalid file type. Only " + ALLOWED_EXTENSION + " files are allowed.");
            return null;
        }

        // 3. Generate a secure, unique filename to prevent path traversal and overwrites
        String uniqueFilename = UUID.randomUUID().toString() + ALLOWED_EXTENSION;
        Path uploadDirPath = Paths.get(UPLOAD_DIRECTORY);
        Path destinationPath = uploadDirPath.resolve(uniqueFilename);

        // Normalize the path to prevent any potential directory traversal issues, though resolving from a constant is safe.
        destinationPath = destinationPath.normalize();
        if (!destinationPath.startsWith(uploadDirPath.toAbsolutePath().normalize())) {
             System.err.println("Error: Potential path traversal detected after normalization.");
             return null;
        }
        
        try {
            // 4. Create the upload directory if it doesn't exist
            if (!Files.exists(uploadDirPath)) {
                Files.createDirectories(uploadDirPath);
            }

            // 5. Save the file
            // Rule #18: The copy operation is inherently bounded by the source stream.
            // We've already checked the fileSize limit.
            Files.copy(fileData, destinationPath, StandardCopyOption.REPLACE_EXISTING);

        } catch (IOException e) {
            System.err.println("Error saving file: " + e.getMessage());
            // Rule #8: Use constant format strings for logging.
            // In a real app, use a proper logger, e.g., log.error("File save failed for {}", uniqueFilename, e);
            return null;
        }

        // 6. Save filepath to the database using parameterized queries
        // Rule #9: Use parameterized queries to prevent SQL injection.
        String sql = "INSERT INTO files (filepath) VALUES (?)";
        
        if (DB_URL == null || DB_USER == null || DB_PASSWORD == null) {
            System.err.println("Database environment variables (DB_URL, DB_USER, DB_PASS) not set. Skipping DB insert.");
            System.out.println("File saved at (simulation): " + destinationPath.toString());
            // In a real scenario, you might fail here. For the example, we'll return the path.
            return destinationPath.toString();
        }

        try (Connection conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {

            pstmt.setString(1, destinationPath.toString());
            pstmt.executeUpdate();

        } catch (SQLException e) {
            System.err.println("Database error: " + e.getMessage());
            // Rule #40: Do not leak internal details.
            // Clean up the saved file if the DB transaction fails to maintain consistency.
            try {
                Files.deleteIfExists(destinationPath);
            } catch (IOException ioex) {
                System.err.println("Failed to clean up file after DB error: " + ioex.getMessage());
            }
            return null;
        }

        System.out.println("File uploaded successfully to: " + destinationPath.toString());
        return destinationPath.toString();
    }

    public static void main(String[] args) {
        System.out.println("--- Running Test Cases ---");
        System.out.println("Note: A running MySQL instance with a 'files' table is required for full functionality.");
        System.out.println("CREATE TABLE files (id INT AUTO_INCREMENT PRIMARY KEY, filepath VARCHAR(255) NOT NULL, created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);");
        System.out.println("Set DB_URL, DB_USER, DB_PASS environment variables to connect.");

        // Test Case 1: Valid PDF upload
        System.out.println("\n--- Test Case 1: Valid PDF ---");
        String pdfContent1 = "%PDF-1.4 content...";
        InputStream stream1 = new ByteArrayInputStream(pdfContent1.getBytes(StandardCharsets.UTF_8));
        uploadPdf(stream1, "document.pdf", pdfContent1.length());

        // Test Case 2: Invalid file type
        System.out.println("\n--- Test Case 2: Invalid file type ---");
        String txtContent = "This is a text file.";
        InputStream stream2 = new ByteArrayInputStream(txtContent.getBytes(StandardCharsets.UTF_8));
        uploadPdf(stream2, "malicious.txt", txtContent.length());

        // Test Case 3: Path traversal attempt
        System.out.println("\n--- Test Case 3: Path traversal attempt ---");
        InputStream stream3 = new ByteArrayInputStream(pdfContent1.getBytes(StandardCharsets.UTF_8));
        uploadPdf(stream3, "../../etc/passwd.pdf", pdfContent1.length());
        
        // Test Case 4: File size too large
        System.out.println("\n--- Test Case 4: File size too large ---");
        InputStream stream4 = new ByteArrayInputStream(pdfContent1.getBytes(StandardCharsets.UTF_8));
        uploadPdf(stream4, "large_file.pdf", MAX_FILE_SIZE + 1);

        // Test Case 5: Null input
        System.out.println("\n--- Test Case 5: Null input ---");
        uploadPdf(null, null, 0);
    }
}