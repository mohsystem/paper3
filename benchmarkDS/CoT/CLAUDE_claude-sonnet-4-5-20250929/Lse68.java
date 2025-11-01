
import java.io.*;
import java.sql.*;
import java.util.Base64;
import java.nio.file.Files;
import java.util.Arrays;
import java.util.List;

public class Lse68 {
    private static final long MAX_FILE_SIZE = 5 * 1024 * 1024; // 5MB
    private static final List<String> ALLOWED_EXTENSIONS = Arrays.asList("jpg", "jpeg", "png", "gif");
    
    public static String upload_image(String filePath, Connection conn) throws Exception {
        // Validate file exists
        File file = new File(filePath);
        if (!file.exists() || !file.isFile()) {
            throw new IllegalArgumentException("Invalid file path");
        }
        
        // Validate file size
        if (file.length() > MAX_FILE_SIZE) {
            throw new IllegalArgumentException("File size exceeds maximum allowed size");
        }
        
        // Validate file extension
        String fileName = file.getName();
        String extension = getFileExtension(fileName).toLowerCase();
        if (!ALLOWED_EXTENSIONS.contains(extension)) {
            throw new IllegalArgumentException("Invalid file type. Allowed: " + ALLOWED_EXTENSIONS);
        }
        
        // Sanitize filename
        String sanitizedFileName = sanitizeFileName(fileName);
        
        // Read and encode file
        byte[] fileContent = Files.readAllBytes(file.toPath());
        String base64String = Base64.getEncoder().encodeToString(fileContent);
        
        // Insert into database using parameterized query
        String sql = "INSERT INTO images (name, data) VALUES (?, ?)";
        try (PreparedStatement pstmt = conn.prepareStatement(sql)) {
            pstmt.setString(1, sanitizedFileName);
            pstmt.setString(2, base64String);
            pstmt.executeUpdate();
        }
        
        return "Image uploaded successfully: " + sanitizedFileName;
    }
    
    private static String getFileExtension(String fileName) {
        int lastDot = fileName.lastIndexOf('.');
        return (lastDot == -1) ? "" : fileName.substring(lastDot + 1);
    }
    
    private static String sanitizeFileName(String fileName) {
        return fileName.replaceAll("[^a-zA-Z0-9._-]", "_");
    }
    
    public static void main(String[] args) {
        try {
            // Database connection (replace with actual credentials)
            String url = "jdbc:sqlite:test.db";
            Connection conn = DriverManager.getConnection(url);
            
            // Create table
            Statement stmt = conn.createStatement();
            stmt.execute("CREATE TABLE IF NOT EXISTS images (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, data TEXT)");
            
            // Test cases (create sample files for testing)
            System.out.println("Test Case 1: Valid image");
            createTestFile("test1.jpg", 100);
            System.out.println(upload_image("test1.jpg", conn));
            
            System.out.println("\\nTest Case 2: Valid PNG");
            createTestFile("test2.png", 200);
            System.out.println(upload_image("test2.png", conn));
            
            System.out.println("\\nTest Case 3: Invalid extension");
            try {
                createTestFile("test3.exe", 100);
                upload_image("test3.exe", conn);
            } catch (Exception e) {
                System.out.println("Expected error: " + e.getMessage());
            }
            
            System.out.println("\\nTest Case 4: Non-existent file");
            try {
                upload_image("nonexistent.jpg", conn);
            } catch (Exception e) {
                System.out.println("Expected error: " + e.getMessage());
            }
            
            System.out.println("\\nTest Case 5: File with special characters");
            createTestFile("test@#$5.jpg", 100);
            System.out.println(upload_image("test@#$5.jpg", conn));
            
            conn.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    private static void createTestFile(String fileName, int size) throws IOException {
        byte[] data = new byte[size];
        Arrays.fill(data, (byte) 0xFF);
        Files.write(new File(fileName).toPath(), data);
    }
}
