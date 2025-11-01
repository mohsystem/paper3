import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.InvalidPathException;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.Base64;
import java.util.Random;

public class Lse68 {

    private static final long MAX_FILE_SIZE = 5 * 1024 * 1024; // 5 MB

    /**
     * Reads an image, converts it to Base64, and inserts it into the database.
     *
     * @param conn The database connection object.
     * @param imagePathStr The path to the image file.
     * @return true if the upload was successful, false otherwise.
     */
    public static boolean uploadImage(Connection conn, String imagePathStr) {
        Path imagePath;
        try {
            imagePath = Paths.get(imagePathStr).normalize();
            // Basic path validation: ensure the path component is just a simple filename.
            if (imagePath.getParent() != null || !imagePath.getFileName().toString().equals(imagePathStr)) {
                System.err.println("Error: Invalid file path. Path traversal detected.");
                return false;
            }
        } catch (InvalidPathException e) {
            System.err.println("Error: Invalid file path format. " + e.getMessage());
            return false;
        }

        File imageFile = imagePath.toFile();
        if (!imageFile.exists() || !imageFile.isFile()) {
            System.err.println("Error: File does not exist or is not a regular file.");
            return false;
        }

        try {
            long fileSize = Files.size(imagePath);
            if (fileSize == 0) {
                System.err.println("Error: File is empty.");
                return false;
            }
            if (fileSize > MAX_FILE_SIZE) {
                System.err.println("Error: File size " + fileSize + " bytes exceeds the limit of " + MAX_FILE_SIZE + " bytes.");
                return false;
            }

            byte[] fileContent = Files.readAllBytes(imagePath);
            String encodedString = Base64.getEncoder().encodeToString(fileContent);

            String sql = "INSERT INTO images(name, data) VALUES(?, ?)";
            try (PreparedStatement pstmt = conn.prepareStatement(sql)) {
                pstmt.setString(1, imagePath.getFileName().toString());
                pstmt.setString(2, encodedString);
                pstmt.executeUpdate();
                System.out.println("Successfully uploaded " + imagePath.getFileName());
                return true;
            }

        } catch (IOException e) {
            System.err.println("Error reading file: " + e.getMessage());
            return false;
        } catch (SQLException e) {
            System.err.println("Database error: " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        String dbUrl = "jdbc:sqlite::memory:";
        try (Connection conn = DriverManager.getConnection(dbUrl)) {
            System.out.println("Connected to in-memory SQLite database.");
            String createTableSql = "CREATE TABLE IF NOT EXISTS images ("
                                  + "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                  + "name TEXT NOT NULL,"
                                  + "data TEXT NOT NULL);";
            try (Statement stmt = conn.createStatement()) {
                stmt.execute(createTableSql);
                System.out.println("Table 'images' created.");
            }

            // Test cases
            runTestCases(conn);

        } catch (SQLException e) {
            System.err.println("Failed to connect or setup database: " + e.getMessage());
        }
    }

    private static void runTestCases(Connection conn) {
        System.out.println("\n--- Running Test Cases ---");
        // Test Case 1: Valid small PNG file
        String validFile = createDummyFile("test.png", 1024);
        System.out.println("\n[Test 1] Uploading a valid small image...");
        uploadImage(conn, validFile);

        // Test Case 2: Valid small JPG file
        String validFile2 = createDummyFile("image.jpg", 2048);
        System.out.println("\n[Test 2] Uploading another valid small image...");
        uploadImage(conn, validFile2);

        // Test Case 3: Zero-byte file
        String zeroByteFile = createDummyFile("empty.gif", 0);
        System.out.println("\n[Test 3] Uploading a zero-byte file (should fail)...");
        uploadImage(conn, zeroByteFile);

        // Test Case 4: File too large
        String largeFile = createDummyFile("large_file.bmp", (int) (MAX_FILE_SIZE + 1));
        System.out.println("\n[Test 4] Uploading a file that is too large (should fail)...");
        uploadImage(conn, largeFile);
        
        // Test Case 5: Non-existent file
        System.out.println("\n[Test 5] Uploading a non-existent file (should fail)...");
        uploadImage(conn, "nonexistent.jpg");

        // Verify database content
        verifyDbContent(conn, 2);

        // Cleanup
        new File(validFile).delete();
        new File(validFile2).delete();
        new File(zeroByteFile).delete();
        new File(largeFile).delete();
        System.out.println("\n--- Test Cases Finished ---");
    }

    private static String createDummyFile(String fileName, int size) {
        File file = new File(fileName);
        try (FileOutputStream fos = new FileOutputStream(file)) {
            if (size > 0) {
                byte[] bytes = new byte[size];
                new Random().nextBytes(bytes);
                fos.write(bytes);
            }
        } catch (IOException e) {
            System.err.println("Failed to create dummy file: " + e.getMessage());
        }
        file.deleteOnExit();
        return fileName;
    }

    private static void verifyDbContent(Connection conn, int expectedRows) {
        String sql = "SELECT COUNT(*) FROM images";
        try (Statement stmt = conn.createStatement();
             ResultSet rs = stmt.executeQuery(sql)) {
            if (rs.next()) {
                int count = rs.getInt(1);
                System.out.println("\nVerification: Found " + count + " rows in the database. Expected " + expectedRows + ".");
                if (count == expectedRows) {
                    System.out.println("Verification successful.");
                } else {
                    System.out.println("Verification failed.");
                }
            }
        } catch (SQLException e) {
            System.err.println("Failed to verify database content: " + e.getMessage());
        }
    }
}