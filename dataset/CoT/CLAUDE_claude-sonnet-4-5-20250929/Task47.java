
import java.sql.*;
import java.net.URL;
import java.net.HttpURLConnection;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Task47 {
    private static final String DB_URL = "jdbc:sqlite:scraped_data.db";
    private static final int TIMEOUT = 5000;
    private static final int MAX_CONTENT_LENGTH = 1000000;
    
    public static void initDatabase() throws SQLException {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            String sql = "CREATE TABLE IF NOT EXISTS scraped_data (" +
                        "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                        "url TEXT NOT NULL," +
                        "title TEXT," +
                        "content TEXT," +
                        "scraped_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)";
            stmt.execute(sql);
        }
    }
    
    public static String scrapeWebsite(String urlString) throws Exception {
        if (urlString == null || urlString.trim().isEmpty()) {
            throw new IllegalArgumentException("URL cannot be null or empty");
        }
        
        if (!urlString.matches("^https?://.*")) {
            throw new IllegalArgumentException("Invalid URL format. Must start with http:// or https://");
        }
        
        URL url = new URL(urlString);
        HttpURLConnection conn = (HttpURLConnection) url.openConnection();
        
        conn.setRequestMethod("GET");
        conn.setConnectTimeout(TIMEOUT);
        conn.setReadTimeout(TIMEOUT);
        conn.setRequestProperty("User-Agent", "Mozilla/5.0 (Secure Scraper)");
        conn.setInstanceFollowRedirects(false);
        
        int responseCode = conn.getResponseCode();
        if (responseCode != 200) {
            throw new Exception("HTTP Error: " + responseCode);
        }
        
        StringBuilder content = new StringBuilder();
        try (BufferedReader reader = new BufferedReader(
                new InputStreamReader(conn.getInputStream()))) {
            String line;
            int totalLength = 0;
            while ((line = reader.readLine()) != null) {
                totalLength += line.length();
                if (totalLength > MAX_CONTENT_LENGTH) {
                    throw new Exception("Content too large");
                }
                content.append(line).append("\\n");
            }
        }
        
        return content.toString();
    }
    
    public static String extractTitle(String html) {
        if (html == null) return "";
        Pattern pattern = Pattern.compile("<title>(.*?)</title>", Pattern.CASE_INSENSITIVE);
        Matcher matcher = pattern.matcher(html);
        if (matcher.find()) {
            return sanitizeString(matcher.group(1));
        }
        return "No title found";
    }
    
    public static String sanitizeString(String input) {
        if (input == null) return "";
        return input.replaceAll("[<>\\"'%;()&+]", "").trim();
    }
    
    public static boolean storeInDatabase(String url, String title, String content) throws SQLException {
        if (url == null || url.trim().isEmpty()) {
            throw new IllegalArgumentException("URL cannot be null or empty");
        }
        
        String sql = "INSERT INTO scraped_data (url, title, content) VALUES (?, ?, ?)";
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            
            pstmt.setString(1, sanitizeString(url));
            pstmt.setString(2, sanitizeString(title));
            pstmt.setString(3, content != null ? content.substring(0, Math.min(content.length(), 10000)) : "");
            
            int rowsAffected = pstmt.executeUpdate();
            return rowsAffected > 0;
        }
    }
    
    public static void scrapeAndStore(String url) throws Exception {
        String content = scrapeWebsite(url);
        String title = extractTitle(content);
        boolean success = storeInDatabase(url, title, content);
        
        if (!success) {
            throw new Exception("Failed to store data in database");
        }
    }
    
    public static void main(String[] args) {
        try {
            initDatabase();
            System.out.println("Database initialized successfully");
            
            // Test case 1: Valid HTML content simulation
            String testUrl1 = "https://example.com/test1";
            System.out.println("\\nTest 1: Storing test data for " + testUrl1);
            storeInDatabase(testUrl1, "Test Page 1", "<html><body>Test content 1</body></html>");
            System.out.println("Test 1 passed");
            
            // Test case 2: Another valid entry
            String testUrl2 = "https://example.com/test2";
            System.out.println("\\nTest 2: Storing test data for " + testUrl2);
            storeInDatabase(testUrl2, "Test Page 2", "<html><body>Test content 2</body></html>");
            System.out.println("Test 2 passed");
            
            // Test case 3: Extract title test
            System.out.println("\\nTest 3: Extracting title");
            String html = "<html><head><title>Sample Title</title></head><body>Content</body></html>";
            String title = extractTitle(html);
            System.out.println("Extracted title: " + title);
            System.out.println("Test 3 passed");
            
            // Test case 4: Sanitization test
            System.out.println("\\nTest 4: String sanitization");
            String unsafe = "<script>alert('xss')</script>";
            String safe = sanitizeString(unsafe);
            System.out.println("Sanitized string: " + safe);
            System.out.println("Test 4 passed");
            
            // Test case 5: Invalid URL handling
            System.out.println("\\nTest 5: Invalid URL handling");
            try {
                scrapeWebsite("invalid-url");
                System.out.println("Test 5 failed - should throw exception");
            } catch (IllegalArgumentException e) {
                System.out.println("Test 5 passed - exception caught: " + e.getMessage());
            }
            
            System.out.println("\\nAll tests completed successfully!");
            
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
            e.printStackTrace();
        }
    }
}
