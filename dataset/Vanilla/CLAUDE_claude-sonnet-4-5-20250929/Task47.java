
import java.io.*;
import java.net.*;
import java.sql.*;
import java.util.regex.*;

public class Task47 {
    private static final String DB_URL = "jdbc:sqlite:scraped_data.db";
    
    public static void scrapeAndStore(String url) {
        try {
            // Create database and table
            createDatabase();
            
            // Scrape website
            String content = scrapeWebsite(url);
            
            // Extract titles and links
            Pattern pattern = Pattern.compile("<a[^>]+href=\\"([^\\"]*)\\"[^>]*>([^<]*)</a>");
            Matcher matcher = pattern.matcher(content);
            
            // Store in database
            Connection conn = DriverManager.getConnection(DB_URL);
            PreparedStatement pstmt = conn.prepareStatement(
                "INSERT INTO scraped_data (url, title, content) VALUES (?, ?, ?)"
            );
            
            while (matcher.find()) {
                String link = matcher.group(1);
                String title = matcher.group(2);
                
                pstmt.setString(1, link);
                pstmt.setString(2, title);
                pstmt.setString(3, content.substring(0, Math.min(500, content.length())));
                pstmt.executeUpdate();
            }
            
            pstmt.close();
            conn.close();
            
            System.out.println("Data scraped and stored successfully from: " + url);
            
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
        }
    }
    
    private static void createDatabase() throws SQLException {
        Connection conn = DriverManager.getConnection(DB_URL);
        Statement stmt = conn.createStatement();
        stmt.execute("CREATE TABLE IF NOT EXISTS scraped_data (" +
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, " +
                    "url TEXT, " +
                    "title TEXT, " +
                    "content TEXT, " +
                    "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP)");
        stmt.close();
        conn.close();
    }
    
    private static String scrapeWebsite(String urlString) throws IOException {
        URL url = new URL(urlString);
        HttpURLConnection conn = (HttpURLConnection) url.openConnection();
        conn.setRequestMethod("GET");
        conn.setRequestProperty("User-Agent", "Mozilla/5.0");
        
        BufferedReader in = new BufferedReader(new InputStreamReader(conn.getInputStream()));
        StringBuilder content = new StringBuilder();
        String line;
        
        while ((line = in.readLine()) != null) {
            content.append(line).append("\\n");
        }
        
        in.close();
        return content.toString();
    }
    
    public static void main(String[] args) {
        System.out.println("Web Scraper Test Cases:");
        System.out.println("========================\\n");
        
        // Test case 1
        System.out.println("Test Case 1: Scraping example.com");
        scrapeAndStore("http://example.com");
        
        // Test case 2
        System.out.println("\\nTest Case 2: Scraping example.org");
        scrapeAndStore("http://example.org");
        
        // Test case 3
        System.out.println("\\nTest Case 3: Scraping example.net");
        scrapeAndStore("http://example.net");
        
        // Test case 4
        System.out.println("\\nTest Case 4: Invalid URL test");
        scrapeAndStore("http://invalid-url-that-does-not-exist-12345.com");
        
        // Test case 5
        System.out.println("\\nTest Case 5: Another valid URL");
        scrapeAndStore("http://info.cern.ch");
    }
}
