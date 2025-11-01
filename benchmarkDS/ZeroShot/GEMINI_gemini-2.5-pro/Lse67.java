import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.nio.charset.StandardCharsets;

/**
 * Note: To compile and run this code, you need the MySQL JDBC driver.
 * You can download it and add it to your classpath.
 * E.g., for mysql-connector-java-8.0.28.jar:
 * javac Lse67.java
 * java -cp .:mysql-connector-java-8.0.28.jar Lse67
 *
 * SQL to create the necessary table:
 * CREATE DATABASE IF NOT EXISTS image_db;
 * USE image_db;
 * CREATE TABLE IF NOT EXISTS images (
 *     id INT AUTO_INCREMENT PRIMARY KEY,
 *     filename VARCHAR(255) NOT NULL,
 *     image_data LONGBLOB NOT NULL,
 *     upload_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP
 * );
 */
public class Lse67 {

    /**
     * Stores an image file in a MySQL database.
     *
     * @param dbUrl      The database connection URL (e.g., "jdbc:mysql://localhost:3306/image_db").
     * @param user       The database username.
     * @param password   The database password.
     * @param filePath   The path to the image file to be stored.
     * @return true if the image was stored successfully, false otherwise.
     */
    public static boolean storeImageInDB(String dbUrl, String user, String password, String filePath) {
        File imageFile = new File(filePath);
        if (!imageFile.exists() || !imageFile.isFile()) {
            System.err.println("Error: File does not exist or is not a regular file: " + filePath);
            return false;
        }

        String sql = "INSERT INTO images (filename, image_data) VALUES (?, ?)";

        try (Connection conn = DriverManager.getConnection(dbUrl, user, password);
             PreparedStatement pstmt = conn.prepareStatement(sql);
             FileInputStream fis = new FileInputStream(imageFile)) {

            pstmt.setString(1, imageFile.getName());
            pstmt.setBinaryStream(2, fis, (int) imageFile.length());

            int rowsAffected = pstmt.executeUpdate();
            return rowsAffected > 0;

        } catch (SQLException e) {
            System.err.println("Database Error: " + e.getMessage());
            return false;
        } catch (FileNotFoundException e) {
            System.err.println("File Not Found Error: " + e.getMessage());
            return false;
        } catch (IOException e) {
            System.err.println("File Read Error: " + e.getMessage());
            return false;
        }
    }

    // Helper function to create a dummy file for testing
    private static void createDummyFile(String fileName) {
        try (FileOutputStream fos = new FileOutputStream(fileName)) {
            fos.write("This is a dummy file.".getBytes(StandardCharsets.UTF_8));
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static void main(String[] args) {
        // --- Database Configuration ---
        // IMPORTANT: In a real application, do not hardcode credentials.
        // Use environment variables, a configuration file, or a secrets management service.
        String dbUrl = "jdbc:mysql://localhost:3306/image_db";
        String user = "user"; // Replace with your MySQL username
        String password = "password"; // Replace with your MySQL password

        // --- Test Cases ---
        System.out.println("Running 5 test cases...");
        String dummyFileName = "test_image.png";
        createDummyFile(dummyFileName);

        // Test Case 1: Successful Insertion
        System.out.println("\n--- Test Case 1: Successful Insertion ---");
        boolean success1 = storeImageInDB(dbUrl, user, password, dummyFileName);
        System.out.println("Result: " + (success1 ? "SUCCESS" : "FAILURE"));

        // Test Case 2: File Not Found
        System.out.println("\n--- Test Case 2: File Not Found ---");
        boolean success2 = storeImageInDB(dbUrl, user, password, "non_existent_file.jpg");
        System.out.println("Result: " + (success2 ? "SUCCESS" : "FAILURE"));

        // Test Case 3: Invalid Database Password
        System.out.println("\n--- Test Case 3: Invalid Database Password ---");
        boolean success3 = storeImageInDB(dbUrl, user, "wrong_password", dummyFileName);
        System.out.println("Result: " + (success3 ? "SUCCESS" : "FAILURE"));

        // Test Case 4: Path is a Directory (should fail)
        System.out.println("\n--- Test Case 4: Path is a Directory ---");
        boolean success4 = storeImageInDB(dbUrl, user, password, "."); // Current directory
        System.out.println("Result: " + (success4 ? "SUCCESS" : "FAILURE"));

        // Test Case 5: Another successful insertion
        System.out.println("\n--- Test Case 5: Another Successful Insertion ---");
        String anotherDummyFile = "another_test.jpg";
        createDummyFile(anotherDummyFile);
        boolean success5 = storeImageInDB(dbUrl, user, password, anotherDummyFile);
        System.out.println("Result: " + (success5 ? "SUCCESS" : "FAILURE"));

        // Cleanup dummy files
        new File(dummyFileName).delete();
        new File(anotherDummyFile).delete();
    }
}