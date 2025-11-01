
import java.io.*;
import java.sql.*;
import java.nio.file.*;
import java.util.UUID;

public class Lse64 {
    private static final String DB_URL = "jdbc:mysql://localhost:3306/pdfdb";
    private static final String DB_USER = "root";
    private static final String DB_PASSWORD = "password";
    private static final String UPLOAD_DIR = "uploads/";
    
    public static String upload_pdf(InputStream fileContent, String originalFilename) {
        // Validate file extension
        if (!originalFilename.toLowerCase().endsWith(".pdf")) {
            return "Error: Only PDF files are allowed";
        }
        
        // Create uploads directory if it doesn't exist
        File uploadDir = new File(UPLOAD_DIR);
        if (!uploadDir.exists()) {
            uploadDir.mkdirs();
        }
        
        // Generate unique filename to prevent overwrites
        String uniqueFilename = UUID.randomUUID().toString() + "_" + originalFilename;
        String filepath = UPLOAD_DIR + uniqueFilename;
        
        try {
            // Save file to uploads folder
            Files.copy(fileContent, Paths.get(filepath), StandardCopyOption.REPLACE_EXISTING);
            
            // Save filepath to database
            saveFilepathToDatabase(filepath, originalFilename);
            
            return "Success: " + filepath;
        } catch (Exception e) {
            e.printStackTrace();
            return "Error: " + e.getMessage();
        }
    }
    
    private static void saveFilepathToDatabase(String filepath, String originalFilename) throws SQLException {
        String sql = "INSERT INTO pdf_files (filepath, original_name, upload_date) VALUES (?, ?, NOW())";
        
        try (Connection conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            
            pstmt.setString(1, filepath);
            pstmt.setString(2, originalFilename);
            pstmt.executeUpdate();
        }
    }
    
    public static byte[] download_pdf(String filepath) {
        // Validate filepath to prevent directory traversal
        if (filepath.contains("..") || !filepath.startsWith(UPLOAD_DIR)) {
            return null;
        }
        
        try {
            return Files.readAllBytes(Paths.get(filepath));
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid PDF upload
        try {
            String testContent1 = "PDF content test 1";
            InputStream is1 = new ByteArrayInputStream(testContent1.getBytes());
            String result1 = upload_pdf(is1, "test1.pdf");
            System.out.println("Test 1: " + result1);
        } catch (Exception e) {
            System.out.println("Test 1 failed: " + e.getMessage());
        }
        
        // Test case 2: Valid PDF upload
        try {
            String testContent2 = "PDF content test 2";
            InputStream is2 = new ByteArrayInputStream(testContent2.getBytes());
            String result2 = upload_pdf(is2, "document.pdf");
            System.out.println("Test 2: " + result2);
        } catch (Exception e) {
            System.out.println("Test 2 failed: " + e.getMessage());
        }
        
        // Test case 3: Invalid file extension
        try {
            String testContent3 = "Not a PDF";
            InputStream is3 = new ByteArrayInputStream(testContent3.getBytes());
            String result3 = upload_pdf(is3, "malicious.exe");
            System.out.println("Test 3: " + result3);
        } catch (Exception e) {
            System.out.println("Test 3 failed: " + e.getMessage());
        }
        
        // Test case 4: Download valid file
        System.out.println("Test 4: Attempting to download uploads/test.pdf");
        byte[] downloadResult = download_pdf("uploads/test.pdf");
        System.out.println("Test 4: " + (downloadResult != null ? "Success" : "Failed"));
        
        // Test case 5: Directory traversal attempt
        System.out.println("Test 5: Attempting directory traversal");
        byte[] traversalResult = download_pdf("../../../etc/passwd");
        System.out.println("Test 5: " + (traversalResult == null ? "Blocked (Success)" : "Failed"));
    }
}
