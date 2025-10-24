import java.io.IOException;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.time.Duration;

// Note: This code requires third-party libraries: Jsoup for HTML parsing and SQLite-JDBC for database access.
// Add the following dependencies to your project (e.g., in pom.xml for Maven or build.gradle for Gradle).
// If running from the command line, download the JAR files and include them in your classpath.
// Maven Dependencies:
// <dependency>
//     <groupId>org.jsoup</groupId>
//     <artifactId>jsoup</artifactId>
//     <version>1.15.4</version>
// </dependency>
// <dependency>
//     <groupId>org.xerial</groupId>
//     <artifactId>sqlite-jdbc</artifactId>
//     <version>3.41.2.2</version>
// </dependency>
//
// How to compile and run from command line:
// 1. Download jsoup-1.15.4.jar and sqlite-jdbc-3.41.2.2.jar
// 2. Compile: javac -cp ".:jsoup-1.15.4.jar:sqlite-jdbc-3.41.2.2.jar" Task47.java
// 3. Run: java -cp ".:jsoup-1.15.4.jar:sqlite-jdbc-3.41.2.2.jar" Task47

import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;

public class Task47 {

    private static final String DB_PATH = "jdbc:sqlite:scraping_results_java.db";

    /**
     * Creates the database and table if they don't exist.
     */
    public static void setupDatabase() {
        String createTableSql = "CREATE TABLE IF NOT EXISTS pages ("
                + "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                + "url TEXT NOT NULL UNIQUE,"
                + "title TEXT NOT NULL,"
                + "scraped_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
                + ");";

        try (Connection conn = DriverManager.getConnection(DB_PATH);
             Statement stmt = conn.createStatement()) {
            stmt.execute(createTableSql);
        } catch (SQLException e) {
            System.err.println("Database setup error: " + e.getMessage());
        }
    }

    /**
     * Scrapes a website, extracts its title, and stores it in the database.
     *
     * @param url The URL of the website to scrape.
     */
    public static void scrapeAndStore(String url) {
        System.out.println("Scraping: " + url);
        try {
            // 1. Fetch HTML content
            HttpClient client = HttpClient.newBuilder()
                    .version(HttpClient.Version.HTTP_2)
                    .followRedirects(HttpClient.Redirect.NORMAL)
                    .connectTimeout(Duration.ofSeconds(20))
                    .build();

            HttpRequest request = HttpRequest.newBuilder()
                    .uri(URI.create(url))
                    .header("User-Agent", "Java-Scraper-Bot/1.0")
                    .build();

            HttpResponse<String> response = client.send(request, HttpResponse.BodyHandlers.ofString());

            if (response.statusCode() != 200) {
                System.err.println("Failed to fetch URL: " + url + " | Status: " + response.statusCode());
                return;
            }

            // 2. Parse HTML and extract title using Jsoup
            Document doc = Jsoup.parse(response.body());
            String title = doc.title();

            if (title == null || title.trim().isEmpty()) {
                System.err.println("Could not find title for URL: " + url);
                return;
            }
            title = title.trim();

            // 3. Store data in the database
            String insertSql = "INSERT OR IGNORE INTO pages(url, title) VALUES(?, ?)";

            try (Connection conn = DriverManager.getConnection(DB_PATH);
                 PreparedStatement pstmt = conn.prepareStatement(insertSql)) {
                pstmt.setString(1, url);
                pstmt.setString(2, title);
                pstmt.executeUpdate();
                System.out.println("Successfully stored title: \"" + title + "\"");
            } catch (SQLException e) {
                System.err.println("Database insertion error: " + e.getMessage());
            }

        } catch (IOException | InterruptedException e) {
            System.err.println("Error during scraping of " + url + ": " + e.getMessage());
            Thread.currentThread().interrupt();
        }
    }
    
    /**
     * Prints all records from the 'pages' table.
     */
    public static void printDatabaseContents() {
        System.out.println("\n--- Database Contents ---");
        String sql = "SELECT id, url, title, scraped_at FROM pages";

        try (Connection conn = DriverManager.getConnection(DB_PATH);
             Statement stmt  = conn.createStatement();
             ResultSet rs    = stmt.executeQuery(sql)){
            
            while (rs.next()) {
                System.out.println(rs.getInt("id") + "\t" +
                                   rs.getString("url") + "\t" +
                                   rs.getString("title") + "\t" +
                                   rs.getString("scraped_at"));
            }
        } catch (SQLException e) {
            System.out.println(e.getMessage());
        }
        System.out.println("-------------------------\n");
    }

    public static void main(String[] args) {
        setupDatabase();
        
        String[] testUrls = {
            "http://books.toscrape.com/",
            "http://books.toscrape.com/catalogue/category/books/travel_2/index.html",
            "http://books.toscrape.com/catalogue/a-light-in-the-attic_1000/index.html",
            "http://quotes.toscrape.com/",
            "http://quotes.toscrape.com/tag/humor/"
        };

        for (String url : testUrls) {
            scrapeAndStore(url);
        }
        
        printDatabaseContents();
    }
}