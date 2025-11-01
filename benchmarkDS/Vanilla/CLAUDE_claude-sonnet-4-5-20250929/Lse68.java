
import java.io.*;
import java.nio.file.Files;
import java.sql.*;
import java.util.Base64;

public class Lse68 {
    private static final String DB_URL = "jdbc:sqlite:images.db";
    
    public static void initDatabase() {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            String sql = "CREATE TABLE IF NOT EXISTS images (" +
                        "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                        "name TEXT NOT NULL," +
                        "image_data TEXT NOT NULL)";
            stmt.execute(sql);
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }
    
    public static boolean upload_image(String imagePath) {
        try {
            File imageFile = new File(imagePath);
            if (!imageFile.exists()) {
                return false;
            }
            
            byte[] imageBytes = Files.readAllBytes(imageFile.toPath());
            String base64Image = Base64.getEncoder().encodeToString(imageBytes);
            String imageName = imageFile.getName();
            
            try (Connection conn = DriverManager.getConnection(DB_URL);
                 PreparedStatement pstmt = conn.prepareStatement(
                     "INSERT INTO images (name, image_data) VALUES (?, ?)")) {
                pstmt.setString(1, imageName);
                pstmt.setString(2, base64Image);
                pstmt.executeUpdate();
                return true;
            }
        } catch (IOException | SQLException e) {
            e.printStackTrace();
            return false;
        }
    }
    
    public static void main(String[] args) {
        initDatabase();
        
        // Test case 1: Upload a valid image
        System.out.println("Test 1: " + upload_image("test_image1.jpg"));
        
        // Test case 2: Upload another valid image
        System.out.println("Test 2: " + upload_image("test_image2.png"));
        
        // Test case 3: Upload non-existent image
        System.out.println("Test 3: " + upload_image("non_existent.jpg"));
        
        // Test case 4: Upload with different path
        System.out.println("Test 4: " + upload_image("images/photo.jpg"));
        
        // Test case 5: Upload another image
        System.out.println("Test 5: " + upload_image("sample.bmp"));
    }
}
