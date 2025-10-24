
import java.sql.*;
import java.util.*;

public class Task137 {
    private static final String DB_URL = "jdbc:sqlite::memory:";
    
    static class Product {
        int id;
        String name;
        double price;
        String category;
        
        Product(int id, String name, double price, String category) {
            this.id = id;
            this.name = name;
            this.price = price;
            this.category = category;
        }
        
        @Override
        public String toString() {
            return String.format("Product{id=%d, name='%s', price=%.2f, category='%s'}", 
                               id, name, price, category);
        }
    }
    
    public static Connection initializeDatabase() throws SQLException {
        Connection conn = DriverManager.getConnection(DB_URL);
        Statement stmt = conn.createStatement();
        
        stmt.execute("CREATE TABLE IF NOT EXISTS products (" +
                    "id INTEGER PRIMARY KEY, " +
                    "name TEXT NOT NULL, " +
                    "price REAL NOT NULL, " +
                    "category TEXT NOT NULL)");
        
        stmt.execute("INSERT INTO products VALUES (1, 'Laptop', 999.99, 'Electronics')");
        stmt.execute("INSERT INTO products VALUES (2, 'Mouse', 29.99, 'Electronics')");
        stmt.execute("INSERT INTO products VALUES (3, 'Desk Chair', 199.99, 'Furniture')");
        stmt.execute("INSERT INTO products VALUES (4, 'Notebook', 4.99, 'Stationery')");
        stmt.execute("INSERT INTO products VALUES (5, 'Monitor', 299.99, 'Electronics')");
        
        return conn;
    }
    
    public static List<Product> queryProductById(Connection conn, int productId) throws SQLException {
        List<Product> results = new ArrayList<>();
        String query = "SELECT * FROM products WHERE id = ?";
        
        try (PreparedStatement pstmt = conn.prepareStatement(query)) {
            pstmt.setInt(1, productId);
            ResultSet rs = pstmt.executeQuery();
            
            while (rs.next()) {
                results.add(new Product(
                    rs.getInt("id"),
                    rs.getString("name"),
                    rs.getDouble("price"),
                    rs.getString("category")
                ));
            }
        }
        
        return results;
    }
    
    public static List<Product> queryProductByName(Connection conn, String productName) throws SQLException {
        List<Product> results = new ArrayList<>();
        String query = "SELECT * FROM products WHERE name LIKE ?";
        
        try (PreparedStatement pstmt = conn.prepareStatement(query)) {
            pstmt.setString(1, "%" + productName + "%");
            ResultSet rs = pstmt.executeQuery();
            
            while (rs.next()) {
                results.add(new Product(
                    rs.getInt("id"),
                    rs.getString("name"),
                    rs.getDouble("price"),
                    rs.getString("category")
                ));
            }
        }
        
        return results;
    }
    
    public static List<Product> queryProductByCategory(Connection conn, String category) throws SQLException {
        List<Product> results = new ArrayList<>();
        String query = "SELECT * FROM products WHERE category = ?";
        
        try (PreparedStatement pstmt = conn.prepareStatement(query)) {
            pstmt.setString(1, category);
            ResultSet rs = pstmt.executeQuery();
            
            while (rs.next()) {
                results.add(new Product(
                    rs.getInt("id"),
                    rs.getString("name"),
                    rs.getDouble("price"),
                    rs.getString("category")
                ));
            }
        }
        
        return results;
    }
    
    public static void main(String[] args) {
        try {
            Connection conn = initializeDatabase();
            
            System.out.println("Test Case 1: Query product by ID = 1");
            List<Product> test1 = queryProductById(conn, 1);
            test1.forEach(System.out::println);
            
            System.out.println("\\nTest Case 2: Query product by name 'Mouse'");
            List<Product> test2 = queryProductByName(conn, "Mouse");
            test2.forEach(System.out::println);
            
            System.out.println("\\nTest Case 3: Query products by category 'Electronics'");
            List<Product> test3 = queryProductByCategory(conn, "Electronics");
            test3.forEach(System.out::println);
            
            System.out.println("\\nTest Case 4: Query product by partial name 'ote'");
            List<Product> test4 = queryProductByName(conn, "ote");
            test4.forEach(System.out::println);
            
            System.out.println("\\nTest Case 5: Query non-existent product ID = 999");
            List<Product> test5 = queryProductById(conn, 999);
            if (test5.isEmpty()) {
                System.out.println("No products found");
            } else {
                test5.forEach(System.out::println);
            }
            
            conn.close();
        } catch (SQLException e) {
            System.err.println("Database error: " + e.getMessage());
        }
    }
}
