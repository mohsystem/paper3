import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.Statement;
import java.io.IOException;

import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;
import org.jsoup.safety.Safelist;

/*
 * NOTE: This code requires external libraries (JAR files) to be included in the classpath.
 * 1. Jsoup: for web scraping (e.g., jsoup-1.15.3.jar)
 * 2. SQLite JDBC Driver: for database interaction (e.g., sqlite-jdbc-3.36.0.3.jar)
 *
 * Compilation and Execution from command line:
 * (Assuming JARs are in a 'libs' folder)
 * javac -cp ".;libs/*" Task47.java
 * java -cp ".;libs/*" Task47
 */
public class Task47 {

    /**
     * Initializes the database and creates the necessary table if it doesn't exist.
     * @param conn The database connection object.
     */
    private static void initializeDatabase(Connection conn) {
        String sql = "CREATE TABLE IF NOT EXISTS scraped_data (\n"
                   + " id integer PRIMARY KEY AUTOINCREMENT,\n"
                   + " url text NOT NULL UNIQUE,\n"
                   + " title text NOT NULL,\n"
                   + " h1_content text\n"
                   + ");";
        try (Statement stmt = conn.createStatement()) {
            stmt.execute(sql);
        } catch (SQLException e) {
            System.err.println("Database initialization error: " + e.getMessage());
        }
    }

    /**
     * Scrapes the title and the first H1 tag from a given URL and stores it in an SQLite database.
     * @param urlString The URL to scrape.
     * @param dbPath The path to the SQLite database file.
     */
    public static void scrapeAndStore(String urlString, String dbPath) {
        // Input validation: Basic check for a valid URL pattern.
        if (urlString == null || !urlString.matches("^https?://.*")) {
            System.err.println("Invalid or null URL provided: " + urlString);
            return;
        }

        String sql = "INSERT OR REPLACE INTO scraped_data(url, title, h1_content) VALUES(?,?,?)";
        
        try (Connection conn = DriverManager.getConnection("jdbc:sqlite:" + dbPath)) {
            if (conn != null) {
                initializeDatabase(conn);

                // Use Jsoup to fetch and parse the document with a timeout
                Document doc = Jsoup.connect(urlString)
                                    .userAgent("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/98.0.4758.82 Safari/537.36")
                                    .timeout(10000) // 10-second timeout
                                    .get();

                // Sanitize user-controlled data before using it. Jsoup's text() method helps by extracting only text.
                String title = doc.title();
                String h1Content = doc.selectFirst("h1") != null ? doc.selectFirst("h1").text() : "No H1 tag found";

                // Use PreparedStatement to prevent SQL injection vulnerabilities.
                try (PreparedStatement pstmt = conn.prepareStatement(sql)) {
                    pstmt.setString(1, urlString);
                    pstmt.setString(2, title);
                    pstmt.setString(3, h1Content);
                    pstmt.executeUpdate();
                    System.out.println("Successfully scraped and stored data from: " + urlString);
                }
            }
        } catch (SQLException e) {
            System.err.println("Database error: " + e.getMessage());
        } catch (IOException e) {
            System.err.println("Could not fetch the URL. Error: " + e.getMessage());
        } catch (Exception e) {
            System.err.println("An unexpected error occurred: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        String dbPath = "scraped_data.db";

        // 5 Test Cases
        String[] testUrls = {
            "http://example.com/",
            "https://www.oracle.com/java/",
            "https://www.python.org/",
            "https://isocpp.org/",
            "https://www.w3.org/TR/c-char-mod/" // A non-existent page might fail, which is a good test
        };

        for (String url : testUrls) {
            scrapeAndStore(url, dbPath);
            System.out.println("---------------------------------");
        }
    }
}