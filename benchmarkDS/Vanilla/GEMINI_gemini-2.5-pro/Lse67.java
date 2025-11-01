/*
 * Prerequisites for Java:
 * 1. MySQL Server running.
 * 2. A database created. For example: `CREATE DATABASE testdb;`
 * 3. A table for storing images. For example:
 *    CREATE TABLE images (
 *        id INT AUTO_INCREMENT PRIMARY KEY,
 *        name VARCHAR(255) NOT NULL,
 *        data LONGBLOB NOT NULL
 *    );
 * 4. MySQL Connector/J library. You need to add the JAR to your classpath.
 *    If using Maven, add this to pom.xml:
 *    <dependency>
 *        <groupId>mysql</groupId>
 *        <artifactId>mysql-connector-java</artifactId>
 *        <version>8.0.28</version>
 *    </dependency>
 */
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.SQLException;

public class Lse67 {

    // --- IMPORTANT: UPDATE DATABASE CONNECTION DETAILS ---
    private static final String DB_URL = "jdbc:mysql://localhost:3306/testdb";
    private static final String USER = "your_username";
    private static final String PASS = "your_password";

    /**
     * Stores an image file in the MySQL database.
     *
     * @param filePath The path to the image file.
     * @return true if the image was stored successfully, false otherwise.
     */
    public static boolean storeImageInDB(String filePath) {
        String sql = "INSERT INTO images (name, data) VALUES (?, ?)";
        File imageFile = new File(filePath);

        try (Connection conn = DriverManager.getConnection(DB_URL, USER, PASS);
             PreparedStatement pstmt = conn.prepareStatement(sql);
             FileInputStream fis = new FileInputStream(imageFile)) {

            pstmt.setString(1, imageFile.getName());
            pstmt.setBinaryStream(2, fis, (int) imageFile.length());
            int rowsAffected = pstmt.executeUpdate();
            return rowsAffected > 0;

        } catch (SQLException | IOException e) {
            System.err.println("Error storing image " + filePath + ": " + e.getMessage());
            return false;
        }
    }
    
    /**
     * Helper function to create a dummy file for testing.
     * @param fileName The name of the file to create.
     */
    private static void createDummyFile(String fileName) {
        try (FileOutputStream fos = new FileOutputStream(fileName)) {
            fos.write(("This is a dummy content for " + fileName).getBytes());
        } catch (IOException e) {
            System.err.println("Could not create dummy file: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        // --- Test Cases ---
        String[] testFiles = {
            "test_image_1.jpg",
            "test_image_2.png",
            "document.pdf",
            "archive.zip",
            "data.bin"
        };

        for (int i = 0; i < testFiles.length; i++) {
            String fileName = testFiles[i];
            
            // Create a dummy file for the test
            createDummyFile(fileName);
            
            System.out.println("--- Test Case " + (i + 1) + " ---");
            System.out.println("Attempting to store: " + fileName);
            boolean success = storeImageInDB(fileName);
            if (success) {
                System.out.println("Successfully stored '" + fileName + "' in the database.");
            } else {
                System.out.println("Failed to store '" + fileName + "' in the database.");
            }
            
            // Clean up the dummy file
            new File(fileName).delete();
            System.out.println();
        }
    }
}