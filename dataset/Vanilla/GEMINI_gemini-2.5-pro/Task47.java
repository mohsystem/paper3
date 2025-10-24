/*
 * Instructions for running this Java code:
 * 1. You need two external libraries (JAR files):
 *    - Jsoup: for parsing HTML. Download from https://jsoup.org/download
 *    - SQLite-JDBC: for interacting with the SQLite database. Download from https://github.com/xerial/sqlite-jdbc/releases
 * 2. Place the downloaded JAR files (e.g., jsoup-1.15.3.jar, sqlite-jdbc-3.39.3.0.jar) in the same directory as this Java file.
 * 3. Compile the code from your terminal, including the JARs in the classpath.
 *    On Windows:
 *    javac -cp ".;jsoup-1.15.3.jar;sqlite-jdbc-3.39.3.0.jar" Task47.java
 *    On Linux/macOS:
 *    javac -cp ".:jsoup-1.15.3.jar:sqlite-jdbc-3.39.3.0.jar" Task47.java
 *    (Adjust JAR file names to match the versions you downloaded).
 * 4. Run the compiled code, again including the JARs in the classpath.
 *    On Windows:
 *    java -cp ".;jsoup-1.15.3.jar;sqlite-jdbc-3.39.3.0.jar" Task47
 *    On Linux/macOS:
 *    java -cp ".:jsoup-1.15.3.jar:sqlite-jdbc-3.39.3.0.jar" Task47
 */
import java.sql.*;
import java.util.ArrayList;
import java.util.List;

import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;
import org.jsoup.nodes.Element;
import org.jsoup.select.Elements;

class Book {
    private final String title;
    private final String price;

    public Book(String title, String price) {
        this.title = title;
        this.price = price;
    }

    public String getTitle() { return title; }
    public String getPrice() { return price; }

    @Override
    public String toString() {
        return "Book{title='" + title + "', price='" + price + "'}";
    }
}

public class Task47 {

    private static Connection connect(String dbName) {
        String url = "jdbc:sqlite:" + dbName;
        Connection conn = null;
        try {
            conn = DriverManager.getConnection(url);
        } catch (SQLException e) {
            System.out.println(e.getMessage());
        }
        return conn;
    }

    public static void setupDatabase(String dbName) {
        String sql = "CREATE TABLE IF NOT EXISTS books (\n"
                + " id integer PRIMARY KEY AUTOINCREMENT,\n"
                + " title text NOT NULL,\n"
                + " price text NOT NULL\n"
                + ");";
        try (Connection conn = connect(dbName);
             Statement stmt = conn.createStatement()) {
            stmt.execute(sql);
        } catch (SQLException e) {
            System.out.println("Error setting up database: " + e.getMessage());
        }
    }

    public static List<Book> scrapeData(String url) {
        List<Book> bookList = new ArrayList<>();
        try {
            Document doc = Jsoup.connect(url).get();
            Elements articles = doc.select("article.product_pod");
            for (Element article : articles) {
                String title = article.select("h3 a").attr("title");
                String price = article.select("p.price_color").text();
                if (!title.isEmpty() && !price.isEmpty()) {
                    bookList.add(new Book(title, price));
                }
            }
        } catch (Exception e) {
            System.out.println("Error scraping URL " + url + ": " + e.getMessage());
        }
        return bookList;
    }

    public static void storeData(String dbName, List<Book> books) {
        String sql = "INSERT INTO books(title, price) VALUES(?, ?)";
        try (Connection conn = connect(dbName);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            for (Book book : books) {
                pstmt.setString(1, book.getTitle());
                pstmt.setString(2, book.getPrice());
                pstmt.executeUpdate();
            }
        } catch (SQLException e) {
            System.out.println("Error storing data: " + e.getMessage());
        }
    }

    public static void printData(String dbName) {
        String sql = "SELECT id, title, price FROM books";
        try (Connection conn = connect(dbName);
             Statement stmt = conn.createStatement();
             ResultSet rs = stmt.executeQuery(sql)) {
            System.out.println("\n--- Data in " + dbName + " ---");
            boolean found = false;
            while (rs.next()) {
                found = true;
                System.out.println("ID: " + rs.getInt("id") + "\t" +
                                   "Title: " + rs.getString("title") + "\t" +
                                   "Price: " + rs.getString("price"));
            }
            if (!found) {
                System.out.println("No data found in the 'books' table.");
            }
            System.out.println("--------------------------");
        } catch (SQLException e) {
            System.out.println("Error printing data: " + e.getMessage());
        }
    }

    public static void clearTable(String dbName) {
        String sql = "DELETE FROM books";
        try (Connection conn = connect(dbName);
             Statement stmt = conn.createStatement()) {
            stmt.execute(sql);
            System.out.println("\nTable 'books' cleared.");
        } catch (SQLException e) {
            System.out.println("Error clearing table: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        String dbName = "scraper_db.sqlite";
        String url1 = "http://books.toscrape.com/";
        String url2 = "http://books.toscrape.com/catalogue/page-2.html";
        
        setupDatabase(dbName);

        // Test Case 1: Scrape data from the first URL
        System.out.println("--- Test Case 1: Scraping " + url1 + " ---");
        List<Book> books1 = scrapeData(url1);
        storeData(dbName, books1);
        System.out.println("Scraped and stored " + books1.size() + " books.");

        // Test Case 2: Print the stored data
        System.out.println("\n--- Test Case 2: Printing data ---");
        printData(dbName);

        // Test Case 3: Clear the database table
        System.out.println("\n--- Test Case 3: Clearing table ---");
        clearTable(dbName);

        // Test Case 4: Print the data again to confirm it's empty
        System.out.println("\n--- Test Case 4: Printing empty table ---");
        printData(dbName);

        // Test Case 5: Scrape data from the second URL and print it
        System.out.println("\n--- Test Case 5: Scraping " + url2 + " and printing ---");
        List<Book> books2 = scrapeData(url2);
        storeData(dbName, books2);
        System.out.println("Scraped and stored " + books2.size() + " books.");
        printData(dbName);
    }
}