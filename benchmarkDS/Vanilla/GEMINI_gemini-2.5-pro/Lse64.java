import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.util.Properties;

public class Lse64 {

    // --- DATABASE CONFIGURATION (Update with your details) ---
    private static final String DB_URL = "jdbc:mysql://localhost:3306/file_db";
    private static final String DB_USER = "your_username";
    private static final String DB_PASSWORD = "your_password";
    private static final String UPLOAD_DIR = "uploads";

    /**
     * Simulates uploading a PDF by copying it to an 'uploads' folder
     * and saving its new path to a MySQL database.
     *
     * Pre-requisites:
     * 1. MySQL server is running.
     * 2. A database named 'file_db' exists.
     * 3. A table named 'files' exists in 'file_db'.
     *    SQL to create table:
     *    CREATE TABLE files (
     *        id INT AUTO_INCREMENT PRIMARY KEY,
     *        filepath VARCHAR(255) NOT NULL,
     *        uploaded_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
     *    );
     * 4. MySQL Connector/J JAR is in the classpath.
     *
     * @param sourceFilePath The path to the local PDF file to be "uploaded".
     * @return true if the upload and database insert were successful, false otherwise.
     */
    public static boolean upload_pdf(String sourceFilePath) {
        File sourceFile = new File(sourceFilePath);
        if (!sourceFile.exists() || sourceFile.isDirectory()) {
            System.err.println("Error: Source file does not exist or is a directory: " + sourceFilePath);
            return false;
        }

        // Create the uploads directory if it doesn't exist
        File uploadDir = new File(UPLOAD_DIR);
        if (!uploadDir.exists()) {
            if (!uploadDir.mkdirs()) {
                System.err.println("Error: Could not create upload directory: " + UPLOAD_DIR);
                return false;
            }
        }

        Path destPath = Paths.get(UPLOAD_DIR, sourceFile.getName());
        String dbFilePath = destPath.toString().replace('\\', '/'); // Normalize path for DB

        try {
            // 1. Save the file to the uploads folder
            Files.copy(sourceFile.toPath(), destPath, StandardCopyOption.REPLACE_EXISTING);
            System.out.println("File saved to: " + destPath);

            // 2. Save the filepath to the MySQL database
            Properties props = new Properties();
            props.setProperty("user", DB_USER);
            props.setProperty("password", DB_PASSWORD);

            try (Connection conn = DriverManager.getConnection(DB_URL, props)) {
                String sql = "INSERT INTO files (filepath) VALUES (?)";
                try (PreparedStatement pstmt = conn.prepareStatement(sql)) {
                    pstmt.setString(1, dbFilePath);
                    int affectedRows = pstmt.executeUpdate();
                    if (affectedRows > 0) {
                        System.out.println("Filepath saved to database successfully.");
                        return true;
                    } else {
                         System.err.println("Error: Failed to save filepath to database.");
                         // Rollback file copy by deleting it
                         Files.deleteIfExists(destPath);
                         return false;
                    }
                }
            } catch (SQLException e) {
                System.err.println("Database Error: " + e.getMessage());
                // Rollback file copy by deleting it
                Files.deleteIfExists(destPath);
                return false;
            }

        } catch (IOException e) {
            System.err.println("File I/O Error: " + e.getMessage());
            return false;
        }
    }

    // Main method with 5 test cases
    public static void main(String[] args) {
        // Create a directory for test files
        File testDir = new File("test_files");
        if (!testDir.exists()) {
            testDir.mkdirs();
        }

        // Create 5 dummy files to act as PDFs
        String[] testFiles = {
            "test_files/report-q1.pdf",
            "test_files/invoice-123.pdf",
            "test_files/manual_v2.pdf",
            "test_files/presentation.pdf",
            "test_files/non_existent_file.pdf"
        };
        
        try {
            for (int i = 0; i < 4; i++) { // Create first 4 files
                 new File(testFiles[i]).createNewFile();
            }
        } catch (IOException e) {
            System.err.println("Could not create dummy test files.");
            return;
        }

        System.out.println("--- Running 5 Test Cases ---");
        for (int i = 0; i < testFiles.length; i++) {
            System.out.println("\n--- Test Case " + (i + 1) + ": Uploading " + testFiles[i] + " ---");
            boolean result = upload_pdf(testFiles[i]);
            System.out.println("Result: " + (result ? "SUCCESS" : "FAILURE"));
        }
    }
}