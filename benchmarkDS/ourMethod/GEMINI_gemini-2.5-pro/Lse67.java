import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.util.Properties;

// Database Schema:
// CREATE DATABASE IF NOT EXISTS image_db;
// USE image_db;
// CREATE TABLE IF NOT EXISTS images (
//     id INT AUTO_INCREMENT PRIMARY KEY,
//     filename VARCHAR(255) NOT NULL,
//     data LONGBLOB NOT NULL,
//     created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
// );

// Make sure you have the MySQL Connector/J driver in your classpath.
// e.g., for Maven:
// <dependency>
//     <groupId>mysql</groupId>
//     <artifactId>mysql-connector-java</artifactId>
//     <version>8.0.33</version>
// </dependency>

public class Lse67 {

    // --- Database Configuration ---
    // Replace with your actual database credentials.
    // It's recommended to load these from a secure configuration file or environment variables.
    private static final String DB_URL = "jdbc:mysql://localhost:3306/image_db";
    private static final String DB_USER = "your_user";
    private static final String DB_PASSWORD = "your_password";

    // --- Security Configuration ---
    // Define a base directory where images are allowed to be read from.
    private static final Path BASE_DIRECTORY = Paths.get(System.getProperty("java.io.tmpdir"), "image_uploads_safe_dir");
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 MB

    /**
     * Stores an image file in the database after performing security checks.
     *
     * @param unsafeFilePath The path to the image file provided by the user.
     * @return true if the image was stored successfully, false otherwise.
     */
    public static boolean storeImageInDb(String unsafeFilePath) {
        if (unsafeFilePath == null || unsafeFilePath.trim().isEmpty()) {
            System.err.println("Error: File path is null or empty.");
            return false;
        }

        Path filePath = Paths.get(unsafeFilePath);

        // Rule #5: Validate and sanitize all external input used in path construction.
        Path canonicalPath;
        try {
            canonicalPath = filePath.toAbsolutePath().normalize();
            if (!canonicalPath.startsWith(BASE_DIRECTORY.toAbsolutePath().normalize())) {
                System.err.println("Error: Path traversal attempt detected. File path is outside the allowed directory.");
                return false;
            }
        } catch (Exception e) {
            System.err.println("Error resolving file path: " + e.getMessage());
            return false;
        }

        // Rule #6: Check-then-use is avoided by performing checks after getting a handle.
        // Files.size() and Files.newInputStream() are safe against basic TOCTOU races on many modern filesystems.
        try {
            // Rule #1 & #3: Validate file size before reading.
            long fileSize = Files.size(canonicalPath);
            if (fileSize > MAX_FILE_SIZE) {
                System.err.println("Error: File size " + fileSize + " bytes exceeds the maximum limit of " + MAX_FILE_SIZE + " bytes.");
                return false;
            }
            if (!Files.isRegularFile(canonicalPath)) {
                System.err.println("Error: Path does not point to a regular file.");
                return false;
            }

            String fileName = canonicalPath.getFileName().toString();
            String sql = "INSERT INTO images (filename, data) VALUES (?, ?)";

            Properties props = new Properties();
            props.setProperty("user", DB_USER);
            props.setProperty("password", DB_PASSWORD);

            try (Connection conn = DriverManager.getConnection(DB_URL, props);
                 PreparedStatement pstmt = conn.prepareStatement(sql);
                 InputStream fis = Files.newInputStream(canonicalPath)) {

                // Rule #2: Use prepared statements to prevent SQL injection.
                pstmt.setString(1, fileName);
                pstmt.setBinaryStream(2, fis, fileSize);

                int affectedRows = pstmt.executeUpdate();
                if (affectedRows > 0) {
                    System.out.println("Successfully stored image: " + fileName);
                    return true;
                } else {
                    System.err.println("Error: Storing image failed, no rows affected.");
                    return false;
                }
            } catch (SQLException e) {
                System.err.println("Database error: " + e.getMessage());
                // In a real application, log the full exception stack trace to a secure log file.
            }
        } catch (IOException e) {
            System.err.println("File I/O error: " + e.getMessage());
        } catch (Exception e) {
            System.err.println("An unexpected error occurred: " + e.getMessage());
        }

        return false;
    }

    /**
     * Main method for testing purposes.
     * Before running, ensure:
     * 1. MySQL server is running.
     * 2. Database 'image_db' and table 'images' are created.
     * 3. DB_USER and DB_PASSWORD are correctly set.
     * 4. MySQL Connector/J is in the classpath.
     */
    public static void main(String[] args) {
        // Setup a test environment
        try {
            Files.createDirectories(BASE_DIRECTORY);
            System.out.println("Created base directory for tests: " + BASE_DIRECTORY);
        } catch (IOException e) {
            System.err.println("Failed to create test directory. Please check permissions.");
            e.printStackTrace();
            return;
        }

        Path validFile = BASE_DIRECTORY.resolve("test_image.jpg");
        Path largeFile = BASE_DIRECTORY.resolve("large_image.jpg");
        Path outsideFile = Paths.get(System.getProperty("java.io.tmpdir"), "outside_image.jpg");
        Path directoryPath = BASE_DIRECTORY;
        String nonExistentFilePath = BASE_DIRECTORY.resolve("nonexistent.jpg").toString();
        
        try {
            // Create dummy files for testing
            Files.write(validFile, "dummy image data".getBytes());
            Files.write(largeFile, new byte[(int) (MAX_FILE_SIZE + 1)]); // Create a file just over the limit
            Files.write(outsideFile, "should not be accessed".getBytes());
            System.out.println("Created test files.");

            // --- Test Cases ---
            System.out.println("\n--- Running Test Cases ---");

            // Test Case 1: Valid file
            System.out.println("\n1. Testing with a valid file...");
            storeImageInDb(validFile.toString());

            // Test Case 2: File is too large
            System.out.println("\n2. Testing with a file that is too large...");
            storeImageInDb(largeFile.toString());

            // Test Case 3: Path traversal attempt
            System.out.println("\n3. Testing with a file outside the allowed directory...");
            storeImageInDb(outsideFile.toString());

            // Test Case 4: Non-existent file
            System.out.println("\n4. Testing with a non-existent file...");
            storeImageInDb(nonExistentFilePath);
            
            // Test Case 5: Path is a directory, not a file
            System.out.println("\n5. Testing with a path that is a directory...");
            storeImageInDb(directoryPath.toString());

        } catch (IOException e) {
            System.err.println("Error setting up test files: " + e.getMessage());
        } finally {
            // Cleanup test files
            try {
                Files.deleteIfExists(validFile);
                Files.deleteIfExists(largeFile);
                Files.deleteIfExists(outsideFile);
                System.out.println("\nCleaned up test files.");
            } catch (IOException e) {
                System.err.println("Error cleaning up test files: " + e.getMessage());
            }
        }
    }
}