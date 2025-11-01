
import java.io.*;
import java.sql.*;
import java.nio.file.*;
import java.util.*;

public class Lse64 {
    private static final String UPLOAD_DIR = "uploads/";
    private static final String DB_URL = "jdbc:mysql://localhost:3306/pdf_db";
    private static final String DB_USER = "root";
    private static final String DB_PASSWORD = "password";
    
    // Upload PDF function
    public static String uploadPdf(String sourceFilePath, String fileName) {
        try {
            // Create uploads directory if it doesn't exist
            File uploadDir = new File(UPLOAD_DIR);
            if (!uploadDir.exists()) {
                uploadDir.mkdirs();
            }
            
            // Generate unique filename
            String uniqueFileName = System.currentTimeMillis() + "_" + fileName;
            String destinationPath = UPLOAD_DIR + uniqueFileName;
            
            // Copy file to uploads folder
            Files.copy(Paths.get(sourceFilePath), Paths.get(destinationPath), 
                      StandardCopyOption.REPLACE_EXISTING);
            
            // Save filepath to database
            saveToDatabase(destinationPath, fileName);
            
            return destinationPath;
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }
    
    // Save filepath to database
    private static void saveToDatabase(String filepath, String filename) {
        try (Connection conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD)) {
            String sql = "INSERT INTO pdf_files (filepath, filename, upload_date) VALUES (?, ?, NOW())";
            PreparedStatement pstmt = conn.prepareStatement(sql);
            pstmt.setString(1, filepath);
            pstmt.setString(2, filename);
            pstmt.executeUpdate();
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }
    
    // Download PDF function
    public static boolean downloadPdf(int fileId, String downloadPath) {
        try (Connection conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD)) {
            String sql = "SELECT filepath FROM pdf_files WHERE id = ?";
            PreparedStatement pstmt = conn.prepareStatement(sql);
            pstmt.setInt(1, fileId);
            ResultSet rs = pstmt.executeQuery();
            
            if (rs.next()) {
                String filepath = rs.getString("filepath");
                Files.copy(Paths.get(filepath), Paths.get(downloadPath), 
                          StandardCopyOption.REPLACE_EXISTING);
                return true;
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return false;
    }
    
    public static void main(String[] args) {
        System.out.println("PDF Upload/Download System Test Cases:");
        
        // Test Case 1: Upload a PDF file
        System.out.println("\\nTest Case 1: Upload PDF");
        String result1 = uploadPdf("test1.pdf", "document1.pdf");
        System.out.println("Upload result: " + (result1 != null ? "Success - " + result1 : "Failed"));
        
        // Test Case 2: Upload another PDF file
        System.out.println("\\nTest Case 2: Upload another PDF");
        String result2 = uploadPdf("test2.pdf", "document2.pdf");
        System.out.println("Upload result: " + (result2 != null ? "Success - " + result2 : "Failed"));
        
        // Test Case 3: Upload PDF with special characters
        System.out.println("\\nTest Case 3: Upload PDF with special name");
        String result3 = uploadPdf("test3.pdf", "my_document_2024.pdf");
        System.out.println("Upload result: " + (result3 != null ? "Success - " + result3 : "Failed"));
        
        // Test Case 4: Download PDF by ID
        System.out.println("\\nTest Case 4: Download PDF");
        boolean download1 = downloadPdf(1, "downloaded_file1.pdf");
        System.out.println("Download result: " + (download1 ? "Success" : "Failed"));
        
        // Test Case 5: Download non-existent PDF
        System.out.println("\\nTest Case 5: Download non-existent PDF");
        boolean download2 = downloadPdf(9999, "downloaded_file2.pdf");
        System.out.println("Download result: " + (download2 ? "Success" : "Failed (Expected)"));
    }
}
