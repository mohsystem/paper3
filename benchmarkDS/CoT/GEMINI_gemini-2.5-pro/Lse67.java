/*
Prerequisites for Java:
1. MySQL Server running.
2. A database named 'testdb'.
3. A table created with the following SQL command:
   CREATE TABLE images (
       id INT AUTO_INCREMENT PRIMARY KEY,
       name VARCHAR(255) NOT NULL,
       data LONGBLOB NOT NULL
   );
4. MySQL Connector/J library. You need to add the JAR file to your project's classpath.
   You can download it from the official MySQL website or add it as a Maven/Gradle dependency.
   <!-- Example Maven Dependency -->
   <dependency>
       <groupId>mysql</groupId>
       <artifactId>mysql-connector-java</artifactId>
       <version>8.0.33</version>
   </dependency>
*/
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.FileOutputStream;
import java.nio.file.Files;
import java.nio.file.Paths;

public class Lse67 {

    // --- Database Configuration ---
    // WARNING: Storing credentials in code is insecure. Use environment variables
    // or a secrets management system in a production environment.
    private static final String DB_URL = "jdbc:mysql://localhost:3306/testdb";
    private static final String DB_USER = "user"; // Replace with your DB username
    private static final String DB_PASSWORD = "password"; // Replace with your DB password

    /**
     * Stores an image file in the MySQL database.
     *
     * @param dbUrl      The database connection URL.
     * @param user       The database username.
     * @param password   The database password.
     * @param filePath   The path to the image file.
     * @return true if the image was stored successfully, false otherwise.
     */
    public static boolean storeImage(String dbUrl, String user, String password, String filePath) {
        String sql = "INSERT INTO images (name, data) VALUES (?, ?)";
        File imageFile = new File(filePath);

        if (!imageFile.exists()) {
            System.err.println("Error: File not found at " + filePath);
            return false;
        }

        // Using try-with-resources to ensure connection and statement are closed
        try (Connection conn = DriverManager.getConnection(dbUrl, user, password);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {

            byte[] fileContent = Files.readAllBytes(imageFile.toPath());
            
            pstmt.setString(1, imageFile.getName());
            pstmt.setBytes(2, fileContent);

            int affectedRows = pstmt.executeUpdate();
            return affectedRows > 0;

        } catch (SQLException e) {
            System.err.println("Database error: " + e.getMessage());
            e.printStackTrace();
            return false;
        } catch (IOException e) {
            System.err.println("File reading error: " + e.getMessage());
            e.printStackTrace();
            return false;
        }
    }

    // Main method with 5 test cases
    public static void main(String[] args) {
        // --- Test Setup ---
        // Create dummy files for testing
        String dummyImagePath = "test_image.png";
        String emptyImagePath = "empty_image.png";
        try {
            // Create a small binary file to simulate an image
            FileOutputStream fos = new FileOutputStream(dummyImagePath);
            fos.write(new byte[]{ (byte)0x89, 'P', 'N', 'G', 1, 2, 3, 4});
            fos.close();
            // Create an empty file
            new File(emptyImagePath).createNewFile();
        } catch (IOException e) {
            System.err.println("Failed to create test files.");
            e.printStackTrace();
            return;
        }

        System.out.println("Running Java Image Storage Tests...");

        // Test Case 1: Store a valid image file.
        System.out.println("\n--- Test Case 1: Store a valid image ---");
        boolean success1 = storeImage(DB_URL, DB_USER, DB_PASSWORD, dummyImagePath);
        System.out.println("Result: " + (success1 ? "SUCCESS" : "FAILURE"));

        // Test Case 2: Store the same image with a different (non-existent) path but copied content.
        System.out.println("\n--- Test Case 2: Store another valid image ---");
        String anotherImagePath = "another_image.jpg";
        try {
            Files.copy(Paths.get(dummyImagePath), Paths.get(anotherImagePath));
            boolean success2 = storeImage(DB_URL, DB_USER, DB_PASSWORD, anotherImagePath);
            System.out.println("Result: " + (success2 ? "SUCCESS" : "FAILURE"));
        } catch (IOException e) {
            System.out.println("Result: FAILURE (Could not copy file for test)");
        }


        // Test Case 3: Try to store a non-existent file.
        System.out.println("\n--- Test Case 3: Store a non-existent file ---");
        boolean success3 = storeImage(DB_URL, DB_USER, DB_PASSWORD, "non_existent_file.jpg");
        System.out.println("Result: " + (success3 ? "SUCCESS" : "FAILURE"));

        // Test Case 4: Store a file with a long name.
        System.out.println("\n--- Test Case 4: Store a file with a long name ---");
        String longNamePath = "a_very_long_file_name_to_test_database_field_limits_and_handling_of_such_cases_0123456789.bin";
         try {
            Files.copy(Paths.get(dummyImagePath), Paths.get(longNamePath));
            boolean success4 = storeImage(DB_URL, DB_USER, DB_PASSWORD, longNamePath);
            System.out.println("Result: " + (success4 ? "SUCCESS" : "FAILURE"));
        } catch (IOException e) {
            System.out.println("Result: FAILURE (Could not copy file for test)");
        }

        // Test Case 5: Store an empty file.
        System.out.println("\n--- Test Case 5: Store an empty file ---");
        boolean success5 = storeImage(DB_URL, DB_USER, DB_PASSWORD, emptyImagePath);
        System.out.println("Result: " + (success5 ? "SUCCESS" : "FAILURE"));

        // --- Cleanup ---
        System.out.println("\nCleaning up test files...");
        new File(dummyImagePath).delete();
        new File(anotherImagePath).delete();
        new File(longNamePath).delete();
        new File(emptyImagePath).delete();
        System.out.println("Done.");
    }
}