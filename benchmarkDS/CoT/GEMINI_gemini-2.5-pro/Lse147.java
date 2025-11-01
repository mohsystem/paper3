import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.Statement;
import java.sql.SQLException;

public class Lse147 {

    /**
     * In a real web application, this method would be called by a route handler
     * like a Servlet or a Spring Controller. The stockName and quantity would
     * come from the HTTP request. The function would then perform a redirect.
     *
     * This simulation focuses on the secure database insertion logic.
     *
     * @param conn The database connection.
     * @param stockName The name of the stock to buy.
     * @param quantity The quantity of the stock to buy.
     * @return true if insertion was successful, false otherwise.
     */
    public static boolean buyOrder(Connection conn, String stockName, int quantity) {
        // SQL query with placeholders (?) to prevent SQL injection.
        String sql = "INSERT INTO orders(stock_name, quantity) VALUES(?, ?)";

        // Use try-with-resources to ensure PreparedStatement is closed automatically.
        try (PreparedStatement pstmt = conn.prepareStatement(sql)) {
            // Bind the parameters to the placeholders.
            // This safely handles escaping special characters.
            pstmt.setString(1, stockName);
            pstmt.setInt(2, quantity);

            // Execute the update.
            pstmt.executeUpdate();
            System.out.println("Successfully inserted order for " + quantity + " of " + stockName);
            // In a real web app, you would redirect here:
            // response.sendRedirect("/stock_view");
            return true;
        } catch (SQLException e) {
            System.err.println("Error inserting order: " + e.getMessage());
            return false;
        }
    }

    // Helper method to display the contents of the orders table.
    public static void viewOrders(Connection conn) {
        String sql = "SELECT id, stock_name, quantity, order_date FROM orders";
        System.out.println("\n--- Current Orders in Database ---");
        try (Statement stmt = conn.createStatement();
             ResultSet rs = stmt.executeQuery(sql)) {
            while (rs.next()) {
                System.out.println("ID: " + rs.getInt("id") + ", " +
                                   "Stock: " + rs.getString("stock_name") + ", " +
                                   "Quantity: " + rs.getInt("quantity") + ", " +
                                   "Date: " + rs.getString("order_date"));
            }
        } catch (SQLException e) {
            System.err.println("Error viewing orders: " + e.getMessage());
        }
        System.out.println("--------------------------------\n");
    }

    public static void main(String[] args) {
        // Using an in-memory SQLite database for simplicity.
        // A file-based DB would use a URL like "jdbc:sqlite:stocks.db".
        String url = "jdbc:sqlite::memory:";
        Connection conn = null;

        try {
            // Establish the database connection.
            conn = DriverManager.getConnection(url);
            System.out.println("Connection to SQLite has been established.");

            // Create the orders table.
            String createTableSql = "CREATE TABLE IF NOT EXISTS orders ("
                                  + " id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                  + " stock_name TEXT NOT NULL,"
                                  + " quantity INTEGER NOT NULL,"
                                  + " order_date DATETIME DEFAULT CURRENT_TIMESTAMP"
                                  + ");";
            try (Statement stmt = conn.createStatement()) {
                stmt.execute(createTableSql);
                System.out.println("Table 'orders' is ready.");
            }

            // --- 5 Test Cases ---
            System.out.println("\n--- Running 5 Test Cases ---");
            buyOrder(conn, "AAPL", 10);
            buyOrder(conn, "GOOGL", 5);
            buyOrder(conn, "MSFT", 15);
            buyOrder(conn, "AMZN", 2);
            // Test case with a potentially malicious name to show safety
            buyOrder(conn, "TSLA'); DROP TABLE orders; --", 1); 
            System.out.println("--- Test Cases Finished ---\n");


            // View all orders to verify the inserts.
            viewOrders(conn);

        } catch (SQLException e) {
            System.err.println(e.getMessage());
        } finally {
            try {
                if (conn != null) {
                    conn.close();
                    System.out.println("Connection to SQLite has been closed.");
                }
            } catch (SQLException ex) {
                System.err.println(ex.getMessage());
            }
        }
    }
}