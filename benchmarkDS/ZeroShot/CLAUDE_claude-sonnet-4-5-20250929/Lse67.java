
import java.io.*;
import java.nio.file.*;
import java.sql.*;
import java.security.MessageDigest;
import java.util.Arrays;
import java.util.List;

public class Lse67 {
    private static final long MAX_FILE_SIZE = 5 * 1024 * 1024; // 5MB
    private static final List<String> ALLOWED_EXTENSIONS = Arrays.asList("jpg", "jpeg", "png", "gif");
    private static final List<String> ALLOWED_MIME_TYPES = Arrays.asList("image/jpeg", "image/png", "image/gif");
    
    public static boolean storeImage(String imagePath, String dbUrl, String dbUser, String dbPassword) {
        Connection conn = null;
        PreparedStatement pstmt = null;
        
        try {
            // Validate file exists
            File imageFile = new File(imagePath);
            if (!imageFile.exists() || !imageFile.isFile()) {
                System.err.println("Error: File does not exist or is not a file");
                return false;
            }
            
            // Check file size
            if (imageFile.length() > MAX_FILE_SIZE) {
                System.err.println("Error: File size exceeds maximum allowed size");
                return false;
            }
            
            // Validate file extension
            String fileName = imageFile.getName();
            String extension = fileName.substring(fileName.lastIndexOf(".") + 1).toLowerCase();
            if (!ALLOWED_EXTENSIONS.contains(extension)) {
                System.err.println("Error: Invalid file extension");
                return false;
            }
            
            // Read file content
            byte[] imageData = Files.readAllBytes(imageFile.toPath());
            
            // Verify it's actually an image by checking magic bytes\n            if (!isValidImageFile(imageData)) {\n                System.err.println("Error: File is not a valid image");\n                return false;\n            }\n            \n            // Calculate file hash for integrity\n            String fileHash = calculateSHA256(imageData);\n            \n            // Connect to database\n            conn = DriverManager.getConnection(dbUrl, dbUser, dbPassword);\n            \n            // Create table if not exists\n            createTableIfNotExists(conn);\n            \n            // Insert image into database\n            String sql = "INSERT INTO images (file_name, file_size, file_hash, image_data, upload_date) VALUES (?, ?, ?, ?, NOW())";\n            pstmt = conn.prepareStatement(sql);\n            pstmt.setString(1, fileName);\n            pstmt.setLong(2, imageFile.length());\n            pstmt.setString(3, fileHash);\n            pstmt.setBytes(4, imageData);\n            \n            int rowsAffected = pstmt.executeUpdate();\n            return rowsAffected > 0;\n            \n        } catch (Exception e) {\n            System.err.println("Error: " + e.getMessage());\n            return false;\n        } finally {\n            try {\n                if (pstmt != null) pstmt.close();\n                if (conn != null) conn.close();\n            } catch (SQLException e) {\n                System.err.println("Error closing resources: " + e.getMessage());\n            }\n        }\n    }\n    \n    private static void createTableIfNotExists(Connection conn) throws SQLException {\n        String sql = "CREATE TABLE IF NOT EXISTS images (" +\n                     "id INT AUTO_INCREMENT PRIMARY KEY, " +\n                     "file_name VARCHAR(255) NOT NULL, " +\n                     "file_size BIGINT NOT NULL, " +\n                     "file_hash VARCHAR(64) NOT NULL, " +\n                     "image_data LONGBLOB NOT NULL, " +\n                     "upload_date DATETIME NOT NULL)";\n        Statement stmt = conn.createStatement();\n        stmt.execute(sql);\n        stmt.close();\n    }\n    \n    private static boolean isValidImageFile(byte[] data) {\n        if (data.length < 4) return false;\n        \n        // Check for common image magic bytes\n        // JPEG: FF D8 FF\n        if (data[0] == (byte)0xFF && data[1] == (byte)0xD8 && data[2] == (byte)0xFF) {\n            return true;\n        }\n        // PNG: 89 50 4E 47\n        if (data[0] == (byte)0x89 && data[1] == (byte)0x50 && \n            data[2] == (byte)0x4E && data[3] == (byte)0x47) {\n            return true;\n        }\n        // GIF: 47 49 46 38\n        if (data[0] == (byte)0x47 && data[1] == (byte)0x49 && \n            data[2] == (byte)0x46 && data[3] == (byte)0x38) {\n            return true;\n        }\n        return false;\n    }\n    \n    private static String calculateSHA256(byte[] data) throws Exception {\n        MessageDigest digest = MessageDigest.getInstance("SHA-256");\n        byte[] hash = digest.digest(data);\n        StringBuilder hexString = new StringBuilder();\n        for (byte b : hash) {\n            String hex = Integer.toHexString(0xff & b);\n            if (hex.length() == 1) hexString.append('0');
            hexString.append(hex);
        }
        return hexString.toString();
    }
    
    public static void main(String[] args) {
        // Database configuration - CHANGE THESE VALUES
        String dbUrl = "jdbc:mysql://localhost:3306/imagedb";
        String dbUser = "root";
        String dbPassword = "password";
        
        // Test cases (create dummy test files or use existing ones)
        System.out.println("Test Case 1: Valid JPEG image");
        boolean result1 = storeImage("test1.jpg", dbUrl, dbUser, dbPassword);
        System.out.println("Result: " + result1 + "\\n");
        
        System.out.println("Test Case 2: Valid PNG image");
        boolean result2 = storeImage("test2.png", dbUrl, dbUser, dbPassword);
        System.out.println("Result: " + result2 + "\\n");
        
        System.out.println("Test Case 3: Non-existent file");
        boolean result3 = storeImage("nonexistent.jpg", dbUrl, dbUser, dbPassword);
        System.out.println("Result: " + result3 + "\\n");
        
        System.out.println("Test Case 4: Invalid extension");
        boolean result4 = storeImage("test.txt", dbUrl, dbUser, dbPassword);
        System.out.println("Result: " + result4 + "\\n");
        
        System.out.println("Test Case 5: Valid GIF image");
        boolean result5 = storeImage("test5.gif", dbUrl, dbUser, dbPassword);
        System.out.println("Result: " + result5);
    }
}
