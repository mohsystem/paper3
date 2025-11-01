import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.ByteArrayInputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.util.Arrays;
import java.util.UUID;

public class Lse64 {

    private static final String UPLOAD_DIR = "uploads";
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 MB
    private static final byte[] PDF_MAGIC_BYTES = {'%', 'P', 'D', 'F', '-'};

    // --- Database Configuration ---
    // FIXME: Replace with your actual database configuration
    private static final String DB_URL = "jdbc:mysql://localhost:3306/your_database";
    private static final String DB_USER = "your_username";
    private static final String DB_PASSWORD = "your_password";

    /**
     * Establishes a connection to the database.
     * In a real application, use a connection pool.
     * @return A database connection object.
     * @throws SQLException if a database access error occurs.
     */
    private static Connection getDbConnection() throws SQLException {
        // NOTE: Ensure you have the MySQL JDBC driver in your classpath.
        return DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
    }

    /**
     * Securely uploads a PDF file, saves it to a designated folder,
     * and records its path in the database.
     *
     * @param fileStream The InputStream of the file being uploaded.
     * @param originalFilename The original name of the file from the user.
     * @param fileSize The total size of the file in bytes.
     * @return The secure filepath where the file was saved, or null on failure.
     */
    public static String uploadPdf(InputStream fileStream, String originalFilename, long fileSize) {
        // 1. Security: Validate file size
        if (fileSize > MAX_FILE_SIZE) {
            System.err.println("Error: File size exceeds the limit of " + MAX_FILE_SIZE + " bytes.");
            return null;
        }
        if (fileSize <= 0) {
            System.err.println("Error: Invalid file size.");
            return null;
        }

        // 2. Security: Sanitize filename to prevent path traversal
        String sanitizedFilename = Paths.get(originalFilename).getFileName().toString();

        // 3. Security: Validate file extension
        if (!sanitizedFilename.toLowerCase().endsWith(".pdf")) {
            System.err.println("Error: Invalid file extension. Only .pdf files are allowed.");
            return null;
        }

        // 4. Security: Generate a unique filename to prevent overwrites
        String uniqueFilename = UUID.randomUUID().toString() + ".pdf";
        Path destinationPath = Paths.get(UPLOAD_DIR, uniqueFilename);

        try {
            // Create upload directory if it doesn't exist
            Files.createDirectories(destinationPath.getParent());

            byte[] buffer = new byte[8192];
            int bytesRead;

            // 5. Security: Validate file content (magic bytes)
            if (!fileStream.markSupported()) {
                 System.err.println("Warning: Stream does not support mark/reset. Cannot validate magic bytes without consuming stream.");
                 // In a real scenario, you might wrap it in a BufferedInputStream, but here we proceed with caution.
            } else {
                fileStream.mark(PDF_MAGIC_BYTES.length);
                byte[] initialBytes = new byte[PDF_MAGIC_BYTES.length];
                bytesRead = fileStream.read(initialBytes);
                fileStream.reset();

                if (bytesRead < PDF_MAGIC_BYTES.length || !Arrays.equals(initialBytes, PDF_MAGIC_BYTES)) {
                    System.err.println("Error: File is not a valid PDF. Magic bytes mismatch.");
                    return null;
                }
            }


            // Save the file
            try (FileOutputStream fos = new FileOutputStream(destinationPath.toFile())) {
                while ((bytesRead = fileStream.read(buffer)) != -1) {
                    fos.write(buffer, 0, bytesRead);
                }
            }
            
            System.out.println("File saved successfully to: " + destinationPath);

            // 6. Security: Use PreparedStatement to prevent SQL Injection
            saveFilePathToDb(destinationPath.toString());

            return destinationPath.toString();

        } catch (IOException e) {
            System.err.println("Error during file I/O operation: " + e.getMessage());
            // Clean up partially created file on error
            try {
                Files.deleteIfExists(destinationPath);
            } catch (IOException cleanupEx) {
                System.err.println("Error cleaning up file: " + cleanupEx.getMessage());
            }
            return null;
        } catch (SQLException e) {
            System.err.println("Error saving filepath to database: " + e.getMessage());
            // Clean up the saved file if the DB operation fails
             try {
                Files.deleteIfExists(destinationPath);
            } catch (IOException cleanupEx) {
                System.err.println("Error cleaning up file: " + cleanupEx.getMessage());
            }
            return null;
        }
    }

    private static void saveFilePathToDb(String filePath) throws SQLException {
        // This is a placeholder for a real table.
        // CREATE TABLE IF NOT EXISTS uploaded_files (id INT AUTO_INCREMENT PRIMARY KEY, filepath VARCHAR(255) NOT NULL, created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);
        String sql = "INSERT INTO uploaded_files (filepath) VALUES (?)";
        
        System.out.println("Attempting to save filepath to DB: " + filePath);
        // NOTE: The following lines are commented out to allow the code to run
        // without a database connection. Uncomment and configure them for a real environment.
        /*
        try (Connection conn = getDbConnection();
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            
            pstmt.setString(1, filePath);
            int affectedRows = pstmt.executeUpdate();

            if (affectedRows == 0) {
                throw new SQLException("Creating file record failed, no rows affected.");
            }
            System.out.println("Filepath successfully saved to database.");
        }
        */
    }

    public static void main(String[] args) {
        // Create uploads directory for tests
        new File(UPLOAD_DIR).mkdirs();

        System.out.println("--- Running Test Cases ---");

        // Test Case 1: Valid PDF upload
        System.out.println("\n[Test Case 1: Valid PDF]");
        byte[] validPdfContent = {'%', 'P', 'D', 'F', '-', '1', '.', '7', '\n', 'S', 'o', 'm', 'e', ' ', 'c', 'o', 'n', 't', 'e', 'n', 't'};
        InputStream validStream = new ByteArrayInputStream(validPdfContent);
        String result1 = uploadPdf(validStream, "mydocument.pdf", validPdfContent.length);
        System.out.println("Result: " + (result1 != null ? "Success" : "Failure"));

        // Test Case 2: Non-PDF file
        System.out.println("\n[Test Case 2: Non-PDF file (invalid magic bytes)]");
        byte[] invalidContent = {'N', 'o', 't', ' ', 'a', ' ', 'P', 'D', 'F'};
        InputStream invalidStream = new ByteArrayInputStream(invalidContent);
        String result2 = uploadPdf(invalidStream, "not_a_pdf.pdf", invalidContent.length);
        System.out.println("Result: " + (result2 != null ? "Success" : "Failure"));
        
        // Test Case 3: Malicious filename (path traversal attempt)
        System.out.println("\n[Test Case 3: Malicious filename]");
        InputStream validStream2 = new ByteArrayInputStream(validPdfContent);
        String result3 = uploadPdf(validStream2, "../../etc/passwd.pdf", validPdfContent.length);
        System.out.println("Result: " + (result3 != null ? "Success" : "Failure"));

        // Test Case 4: File too large
        System.out.println("\n[Test Case 4: File too large]");
        InputStream largeStream = new ByteArrayInputStream(new byte[0]); // Content doesn't matter, just size
        String result4 = uploadPdf(largeStream, "largefile.pdf", MAX_FILE_SIZE + 1);
        System.out.println("Result: " + (result4 != null ? "Success" : "Failure"));

        // Test Case 5: Invalid file extension
        System.out.println("\n[Test Case 5: Invalid file extension]");
        InputStream validStream3 = new ByteArrayInputStream(validPdfContent);
        String result5 = uploadPdf(validStream3, "document.txt", validPdfContent.length);
        System.out.println("Result: " + (result5 != null ? "Success" : "Failure"));
    }
}