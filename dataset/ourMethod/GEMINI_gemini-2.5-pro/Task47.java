import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.Statement;
import java.time.Duration;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/*
To compile and run (requires sqlite-jdbc dependency):
1. Download sqlite-jdbc jar, e.g., from https://github.com/xerial/sqlite-jdbc/releases
   (e.g., sqlite-jdbc-3.45.1.0.jar)
2. Compile:
   javac Task47.java
3. Run:
   java -cp ".:sqlite-jdbc-3.45.1.0.jar" Task47
*/
public class Task47 {

    /**
     * Extracts the content of the first <h1> tag from an HTML string.
     * @param html The HTML content.
     * @return The content of the <h1> tag, or a default string if not found.
     */
    private static String parseTitle(String html) {
        // Using regex to find the content of the first h1 tag.
        // This is a simplistic approach; a full HTML parser would be more robust.
        Pattern pattern = Pattern.compile("<h1[^>]*>(.*?)</h1>", Pattern.DOTALL | Pattern.CASE_INSENSITIVE);
        Matcher matcher = pattern.matcher(html);
        if (matcher.find()) {
            return matcher.group(1).trim();
        }
        return "No H1 Title Found";
    }

    /**
     * Initializes the database and creates the 'pages' table if it doesn't exist.
     * @param conn The database connection.
     * @throws SQLException if a database access error occurs.
     */
    private static void initializeDatabase(Connection conn) throws SQLException {
        String sql = "CREATE TABLE IF NOT EXISTS pages ("
                   + " id INTEGER PRIMARY KEY AUTOINCREMENT,"
                   + " url TEXT NOT NULL UNIQUE,"
                   + " title TEXT NOT NULL,"
                   + " scraped_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
                   + ");";
        try (Statement stmt = conn.createStatement()) {
            stmt.execute(sql);
        }
    }

    /**
     * Scrapes a website, extracts the H1 title, and stores it in a SQLite database.
     * @param url The URL of the website to scrape.
     * @param dbPath The path to the SQLite database file.
     * @return true if successful, false otherwise.
     */
    public static boolean scrapeAndStore(String url, String dbPath) {
        try {
            // 1. Fetch website content using HttpClient
            HttpClient client = HttpClient.newBuilder()
                    .version(HttpClient.Version.HTTP_2)
                    .followRedirects(HttpClient.Redirect.NORMAL)
                    .connectTimeout(Duration.ofSeconds(20))
                    .build();

            HttpRequest request = HttpRequest.newBuilder()
                    .uri(URI.create(url))
                    .timeout(Duration.ofSeconds(20))
                    .build();

            HttpResponse<String> response = client.send(request, HttpResponse.BodyHandlers.ofString());

            if (response.statusCode() != 200) {
                System.err.println("Failed to fetch " + url + ". Status code: " + response.statusCode());
                return false;
            }

            String htmlBody = response.body();

            // 2. Parse the data (extract H1 title)
            String title = parseTitle(htmlBody);

            // 3. Store data in the database
            String jdbcUrl = "jdbc:sqlite:" + dbPath;
            try (Connection conn = DriverManager.getConnection(jdbcUrl)) {
                initializeDatabase(conn);
                
                // Using a PreparedStatement to prevent SQL injection
                String sql = "INSERT OR REPLACE INTO pages (url, title) VALUES (?, ?)";
                try (PreparedStatement pstmt = conn.prepareStatement(sql)) {
                    pstmt.setString(1, url);
                    pstmt.setString(2, title);
                    pstmt.executeUpdate();
                    System.out.println("Successfully scraped and stored: " + url);
                    return true;
                }
            } catch (SQLException e) {
                System.err.println("Database error: " + e.getMessage());
                return false;
            }
        } catch (Exception e) {
            System.err.println("An error occurred during scraping: " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        String dbFile = "scraped_data_java.db";

        // Test cases
        String[] urlsToScrape = {
            "https://example.com/",
            "https://www.iana.org/domains/example",
            "https://httpbin.org/html",
            "http://info.cern.ch/hypertext/WWW/TheProject.html", // Using http to test redirects
            "https://www.w3.org/TR/html52/"
        };
        
        System.out.println("--- Starting Web Scraping Tests ---");
        int successCount = 0;
        for (String url : urlsToScrape) {
            System.out.println("Processing: " + url);
            if (scrapeAndStore(url, dbFile)) {
                successCount++;
            }
            System.out.println("--------------------");
        }
        System.out.println("--- Scraping Finished ---");
        System.out.println(successCount + " out of " + urlsToScrape.length + " pages scraped successfully.");
        System.out.println("Data stored in '" + dbFile + "'");
    }
}