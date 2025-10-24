
import java.io.*;
import java.net.*;
import java.sql.*;
import java.util.*;
import java.util.regex.*;

public class Task47 {
    private static final String DB_URL = "jdbc:sqlite:scraped_data.db";
    
    public static void scrapeAndStore(String url, String tableName) {
        try {
            // Validate URL
            if (!isValidUrl(url)) {
                System.out.println("Invalid URL provided");
                return;
            }
            
            // Initialize database
            initializeDatabase(tableName);
            
            // Scrape data
            String content = scrapeWebsite(url);
            
            // Store in database
            storeData(tableName, url, content);
            
            System.out.println("Data scraped and stored successfully");
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
        }
    }
    
    private static boolean isValidUrl(String url) {
        try {
            new URL(url);
            return url.startsWith("http://") || url.startsWith("https://");
        } catch (MalformedURLException e) {
            return false;
        }
    }
    
    private static void initializeDatabase(String tableName) throws SQLException {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            
            String sql = "CREATE TABLE IF NOT EXISTS " + sanitizeTableName(tableName) + 
                        " (id INTEGER PRIMARY KEY AUTOINCREMENT, " +
                        "url TEXT NOT NULL, " +
                        "content TEXT, " +
                        "scraped_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)";
            stmt.execute(sql);
        }
    }
    
    private static String sanitizeTableName(String name) {
        return name.replaceAll("[^a-zA-Z0-9_]", "_");
    }
    
    private static String scrapeWebsite(String urlString) throws IOException {
        URL url = new URL(urlString);
        HttpURLConnection conn = (HttpURLConnection) url.openConnection();
        conn.setRequestMethod("GET");
        conn.setConnectTimeout(5000);
        conn.setReadTimeout(5000);
        conn.setRequestProperty("User-Agent", "Mozilla/5.0");
        
        StringBuilder content = new StringBuilder();
        try (BufferedReader reader = new BufferedReader(
                new InputStreamReader(conn.getInputStream()))) {
            String line;
            while ((line = reader.readLine()) != null) {
                content.append(line).append("\\n");
            }
        }
        
        return content.toString();
    }
    
    private static void storeData(String tableName, String url, String content) 
            throws SQLException {
        String sql = "INSERT INTO " + sanitizeTableName(tableName) + 
                    " (url, content) VALUES (?, ?)";
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            pstmt.setString(1, url);
            pstmt.setString(2, content);
            pstmt.executeUpdate();
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== Web Scraper Test Cases ===\\n");
        
        // Test Case 1: Valid URL
        System.out.println("Test 1: Scraping example.com");
        scrapeAndStore("http://example.com", "test_data");
        
        // Test Case 2: Invalid URL
        System.out.println("\\nTest 2: Invalid URL");
        scrapeAndStore("invalid-url", "test_data");
        
        // Test Case 3: HTTPS URL
        System.out.println("\\nTest 3: HTTPS URL");
        scrapeAndStore("https://example.com", "secure_data");
        
        // Test Case 4: Different table name
        System.out.println("\\nTest 4: Different table");
        scrapeAndStore("http://example.com", "website_content");
        
        // Test Case 5: Special characters in table name
        System.out.println("\\nTest 5: Table name sanitization");
        scrapeAndStore("http://example.com", "data-table-2024");
    }
}
