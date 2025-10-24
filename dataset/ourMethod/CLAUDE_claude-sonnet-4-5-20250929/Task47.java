
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.URI;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Pattern;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLContext;

public class Task47 {
    
    private static final int MAX_URL_LENGTH = 2048;
    private static final int MAX_RESPONSE_SIZE = 10 * 1024 * 1024; // 10 MB
    private static final int MAX_CONTENT_LENGTH = 50000;
    private static final int CONNECTION_TIMEOUT = 30000; // 30 seconds
    private static final Pattern URL_PATTERN = Pattern.compile(
        "^https://(?:[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?\\\\.)*" +
        "[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?" +
        "(?::[0-9]{1,5})?(?:/[^\\\\s]*)?$"
    );
    
    /**
     * Validates URL format and ensures HTTPS protocol.
     * Prevents injection attacks and enforces secure protocol (CWE-20, CWE-319).
     */
    private static boolean validateUrl(String url) {
        if (url == null || url.isEmpty()) {
            return false;
        }
        
        // Check length to prevent DoS (CWE-400)
        if (url.length() > MAX_URL_LENGTH) {
            return false;
        }
        
        // Must use HTTPS only (CWE-319)
        if (!url.startsWith("https://")) {
            return false;
        }
        
        // Validate format
        return URL_PATTERN.matcher(url).matches();
    }
    
    /**
     * Scrapes website content using secure HTTPS connection.
     * Enforces certificate validation and TLS settings (CWE-295, CWE-297, CWE-327).
     */
    private static List<String> scrapeWebsite(String urlString) {
        // Validate URL (CWE-20)
        if (!validateUrl(urlString)) {
            System.err.println("Error: Invalid URL. Must be HTTPS and properly formatted.");
            return null;
        }
        
        List<String> scrapedData = new ArrayList<>();
        HttpsURLConnection connection = null;
        
        try {
            // Create URI and URL with validation
            URI uri = new URI(urlString);
            URL url = uri.toURL();
            
            // Ensure HTTPS protocol
            if (!"https".equalsIgnoreCase(url.getProtocol())) {
                System.err.println("Error: Only HTTPS protocol is allowed");
                return null;
            }
            
            // Create secure SSL context (CWE-327)
            SSLContext sslContext = SSLContext.getInstance("TLSv1.2");
            sslContext.init(null, null, null);
            
            // Open HTTPS connection
            connection = (HttpsURLConnection) url.openConnection();
            
            // Configure secure connection settings
            connection.setSSLSocketFactory(sslContext.getSocketFactory());
            connection.setConnectTimeout(CONNECTION_TIMEOUT);
            connection.setReadTimeout(CONNECTION_TIMEOUT);
            
            // Set secure headers
            connection.setRequestProperty("User-Agent", "SecureScraper/1.0");
            connection.setRequestProperty("Accept", "text/html");
            connection.setRequestMethod("GET");
            
            // Enforce certificate validation (CWE-295, CWE-297)
            connection.setHostnameVerifier((hostname, session) -> {
                try {
                    // Use default hostname verifier for proper validation
                    return HttpsURLConnection.getDefaultHostnameVerifier()
                        .verify(hostname, session);
                } catch (Exception e) {
                    return false;
                }
            });
            
            // Connect
            connection.connect();
            
            // Validate response code
            int responseCode = connection.getResponseCode();
            if (responseCode != 200) {
                System.err.println("Error: HTTP " + responseCode);
                return null;
            }
            
            // Read response with size limit (CWE-400)
            StringBuilder content = new StringBuilder();
            try (BufferedReader reader = new BufferedReader(
                    new InputStreamReader(connection.getInputStream(), 
                                        StandardCharsets.UTF_8))) {
                
                String line;
                int totalBytes = 0;
                
                while ((line = reader.readLine()) != null) {
                    totalBytes += line.length();
                    
                    // Prevent excessive memory usage (CWE-400)
                    if (totalBytes > MAX_RESPONSE_SIZE) {
                        System.err.println("Warning: Response truncated at size limit");
                        break;
                    }
                    
                    content.append(line).append("\\n");
                }
            }
            
            // Parse HTML and extract text (simple parsing, avoids code execution)
            String htmlContent = content.toString();
            String textContent = htmlContent
                .replaceAll("<script[^>]*>.*?</script>", "") // Remove scripts
                .replaceAll("<style[^>]*>.*?</style>", "")   // Remove styles
                .replaceAll("<[^>]+>", "")                    // Remove tags
                .replaceAll("&[^;]+;", " ")                   // Remove entities
                .trim();
            
            // Split into lines and sanitize
            String[] lines = textContent.split("\\n");
            for (String textLine : lines) {
                String cleaned = textLine.trim();
                
                if (!cleaned.isEmpty()) {
                    // Remove control characters (CWE-20)
                    cleaned = cleaned.replaceAll("[\\\\x00-\\\\x08\\\\x0B-\\\\x0C\\\\x0E-\\\\x1F\\\\x7F]", "");
                    
                    if (!cleaned.isEmpty()) {
                        scrapedData.add(cleaned);
                    }
                }
            }
            
            return scrapedData;
            
        } catch (java.net.URISyntaxException e) {
            System.err.println("Error: Invalid URI syntax");
            return null;
        } catch (java.net.MalformedURLException e) {
            System.err.println("Error: Malformed URL");
            return null;
        } catch (javax.net.ssl.SSLException e) {
            System.err.println("Error: SSL/TLS error");
            return null;
        } catch (java.io.IOException e) {
            System.err.println("Error: I/O error");
            return null;
        } catch (Exception e) {
            System.err.println("Error: " + e.getClass().getSimpleName());
            return null;
        } finally {
            // Always disconnect (CWE-404)
            if (connection != null) {
                connection.disconnect();
            }
        }
    }
    
    /**
     * Validates database path to prevent path traversal attacks.
     * Ensures path is within expected directory (CWE-22).
     */
    private static boolean validateDbPath(String dbPath) {
        if (dbPath == null || dbPath.isEmpty()) {
            return false;
        }
        
        // Check length
        if (dbPath.length() > 255) {
            return false;
        }
        
        try {
            // Resolve to absolute path
            Path basePath = Paths.get(System.getProperty("user.dir")).toRealPath();
            Path targetPath = basePath.resolve(dbPath).normalize();
            
            // Ensure path is within base directory (CWE-22)
            if (!targetPath.startsWith(basePath)) {
                return false;
            }
            
            // Ensure it's not a directory
            if (Files.exists(targetPath) && Files.isDirectory(targetPath)) {
                return false;
            }
            
            // Ensure parent directory exists
            Path parent = targetPath.getParent();
            if (parent != null && !Files.exists(parent)) {
                return false;
            }
            
            return true;
        } catch (Exception e) {
            return false;
        }
    }
    
    /**
     * Stores scraped data in SQLite database using parameterized queries.
     * Prevents SQL injection (CWE-89).
     */
    private static boolean storeInDatabase(List<String> data, String dbPath, 
                                          String sourceUrl) {
        // Validate database path (CWE-22)
        if (!validateDbPath(dbPath)) {
            System.err.println("Error: Invalid database path");
            return false;
        }
        
        // Validate inputs (CWE-20)
        if (data == null || data.isEmpty()) {
            System.err.println("Error: Invalid data");
            return false;
        }
        
        if (sourceUrl == null || sourceUrl.length() > MAX_URL_LENGTH) {
            System.err.println("Error: Invalid source URL");
            return false;
        }
        
        Connection connection = null;
        
        try {
            // Connect to SQLite database
            String jdbcUrl = "jdbc:sqlite:" + dbPath;
            connection = DriverManager.getConnection(jdbcUrl);
            
            // Disable auto-commit for transaction control
            connection.setAutoCommit(false);
            
            // Create table with proper schema
            try (Statement stmt = connection.createStatement()) {
                stmt.execute(
                    "CREATE TABLE IF NOT EXISTS scraped_data (" +
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, " +
                    "source_url TEXT NOT NULL, " +
                    "content TEXT NOT NULL, " +
                    "scraped_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)"
                );
                
                // Create index for performance
                stmt.execute(
                    "CREATE INDEX IF NOT EXISTS idx_source_url " +
                    "ON scraped_data(source_url)"
                );
            }
            
            // Insert data using parameterized query (CWE-89)
            String insertSql = 
                "INSERT INTO scraped_data (source_url, content) VALUES (?, ?)";
            
            try (PreparedStatement pstmt = connection.prepareStatement(insertSql)) {
                for (String item : data) {
                    // Validate item
                    if (item == null || item.isEmpty()) {
                        continue;
                    }
                    
                    // Limit content length
                    String content = item;
                    if (content.length() > MAX_CONTENT_LENGTH) {
                        content = content.substring(0, MAX_CONTENT_LENGTH);
                    }
                    
                    // Use parameterized query to prevent SQL injection (CWE-89)
                    pstmt.setString(1, sourceUrl);
                    pstmt.setString(2, content);
                    pstmt.executeUpdate();
                }
            }
            
            // Commit transaction
            connection.commit();
            
            System.out.println("Successfully stored " + data.size() + 
                             " items in database");
            return true;
            
        } catch (java.sql.SQLException e) {
            System.err.println("Database error: " + e.getClass().getSimpleName());
            
            // Rollback on error (CWE-755)
            if (connection != null) {
                try {
                    connection.rollback();
                } catch (java.sql.SQLException ex) {
                    // Ignore rollback errors
                }
            }
            return false;
        } catch (Exception e) {
            System.err.println("Unexpected error: " + e.getClass().getSimpleName());
            
            if (connection != null) {
                try {
                    connection.rollback();
                } catch (java.sql.SQLException ex) {
                    // Ignore rollback errors
                }
            }
            return false;
        } finally {
            // Ensure connection is closed (CWE-404)
            if (connection != null) {
                try {
                    connection.close();
                } catch (java.sql.SQLException e) {
                    // Ignore close errors
                }
            }
        }
    }
    
    /**
     * Main method with test cases.
     */
    public static void main(String[] args) {
        System.out.println("=== Secure Web Scraper Test Cases ===\\n");
        
        // Test case 1: Valid HTTPS URL
        System.out.println("Test 1: Scraping from example.com...");
        String url1 = "https://example.com";
        List<String> data1 = scrapeWebsite(url1);
        if (data1 != null) {
            boolean result1 = storeInDatabase(data1, "test_data1.db", url1);
            System.out.println("Result: " + (result1 ? "Success" : "Failed") + "\\n");
        } else {
            System.out.println("Result: Failed to scrape\\n");
        }
        
        // Test case 2: Invalid URL - HTTP instead of HTTPS (should fail)
        System.out.println("Test 2: Testing HTTP URL (should reject)...");
        String url2 = "http://example.com";
        List<String> data2 = scrapeWebsite(url2);
        System.out.println("Result: " + 
            (data2 == null ? "Rejected" : "Failed - should reject HTTP") + "\\n");
        
        // Test case 3: Invalid URL - malformed (should fail)
        System.out.println("Test 3: Testing malformed URL (should reject)...");
        String url3 = "not-a-valid-url";
        List<String> data3 = scrapeWebsite(url3);
        System.out.println("Result: " + 
            (data3 == null ? "Rejected" : "Failed - should reject malformed") + "\\n");
        
        // Test case 4: Path traversal attempt in database path (should fail)
        System.out.println("Test 4: Testing path traversal in DB path (should reject)...");
        String url4 = "https://example.com";
        List<String> data4 = new ArrayList<>();
        data4.add("test data");
        boolean result4 = storeInDatabase(data4, "../etc/passwd", url4);
        System.out.println("Result: " + 
            (!result4 ? "Rejected" : "Failed - should reject traversal") + "\\n");
        
        // Test case 5: Valid scrape and store
        System.out.println("Test 5: Complete scrape and store operation...");
        String url5 = "https://www.ietf.org";
        List<String> data5 = scrapeWebsite(url5);
        if (data5 != null) {
            boolean result5 = storeInDatabase(data5, "test_data5.db", url5);
            System.out.println("Result: " + (result5 ? "Success" : "Failed") + "\\n");
        } else {
            System.out.println("Result: Failed to scrape\\n");
        }
        
        System.out.println("=== Test Cases Complete ===");
    }
}
