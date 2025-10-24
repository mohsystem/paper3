// Chain-of-Through Process in Code (Java):
// 1) Problem understanding: Retrieve a customer by 'customerusername' from 'customer' table, using a username from command-line args; return structured object.
// 2) Security requirements: Use parameterized queries; validate input; handle resources safely; avoid leaking sensitive info; limit outputs.
// 3) Secure coding generation: Implement input validation, prepared statements, minimal privileges, safe resource handling.
// 4) Code review: Ensure no SQL injection, proper resource cleanup, and robust error handling.
// 5) Secure code output: Final code uses prepared statements, regex validation, and careful printing.

import java.sql.*;
import java.util.*;
import java.util.regex.Pattern;

public class Task67 {

    // Data model for Customer
    public static class Customer {
        public final int id;
        public final String username;
        public final String fullName;
        public final String email;
        public final String createdAt;

        public Customer(int id, String username, String fullName, String email, String createdAt) {
            this.id = id;
            this.username = username;
            this.fullName = fullName;
            this.email = email;
            this.createdAt = createdAt;
        }

        @Override
        public String toString() {
            // Simple JSON-like serialization
            return "{"
                + "\"id\":" + id + ","
                + "\"customerusername\":\"" + escape(username) + "\","
                + "\"fullname\":\"" + escape(fullName) + "\","
                + "\"email\":\"" + escape(email) + "\","
                + "\"created_at\":\"" + escape(createdAt) + "\""
                + "}";
        }

        private String escape(String s) {
            if (s == null) return "";
            return s.replace("\\", "\\\\").replace("\"", "\\\"");
        }
    }

    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9_.-]{1,64}$");

    // Validate username input
    public static boolean isValidUsername(String username) {
        if (username == null) return false;
        return USERNAME_PATTERN.matcher(username).matches();
    }

    // Ensure DB schema and seed test data
    public static void ensureSchemaAndSeed(Connection conn) throws SQLException {
        String createTable = "CREATE TABLE IF NOT EXISTS customer ("
            + "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            + "customerusername TEXT UNIQUE NOT NULL,"
            + "fullname TEXT NOT NULL,"
            + "email TEXT NOT NULL,"
            + "created_at TEXT DEFAULT CURRENT_TIMESTAMP"
            + ")";
        try (Statement st = conn.createStatement()) {
            st.execute(createTable);
        }

        // Seed data securely with parameterized upsert
        String upsert = "INSERT INTO customer (customerusername, fullname, email) "
                      + "VALUES (?, ?, ?) "
                      + "ON CONFLICT(customerusername) DO UPDATE SET "
                      + "fullname=excluded.fullname, email=excluded.email";
        try (PreparedStatement ps = conn.prepareStatement(upsert)) {
            seedOne(ps, "alice",   "Alice Anderson",   "alice@example.com");
            seedOne(ps, "bob",     "Bob Brown",        "bob@example.com");
            seedOne(ps, "charlie", "Charlie Clark",    "charlie@example.com");
            seedOne(ps, "dora",    "Dora Dawson",      "dora@example.net");
            seedOne(ps, "eve",     "Eve Edwards",      "eve@example.org");
        }
    }

    private static void seedOne(PreparedStatement ps, String u, String f, String e) throws SQLException {
        ps.setString(1, u);
        ps.setString(2, f);
        ps.setString(3, e);
        ps.executeUpdate();
    }

    // Core function: fetch a Customer by username using a parameterized query
    public static Customer getCustomerByUsername(Connection conn, String username) throws SQLException {
        if (!isValidUsername(username)) {
            return null; // Invalid username format
        }
        String sql = "SELECT id, customerusername, fullname, email, COALESCE(created_at,'') "
                   + "FROM customer WHERE customerusername = ? LIMIT 1";
        try (PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setString(1, username);
            try (ResultSet rs = ps.executeQuery()) {
                if (rs.next()) {
                    return new Customer(
                        rs.getInt(1),
                        rs.getString(2),
                        rs.getString(3),
                        rs.getString(4),
                        rs.getString(5)
                    );
                }
            }
        }
        return null;
    }

    // Main with 5 test cases; also honors a command-line username argument
    public static void main(String[] args) {
        String url = "jdbc:sqlite:customers.db";
        List<String> tests = new ArrayList<>();
        if (args != null && args.length >= 1 && args[0] != null && !args[0].isEmpty()) {
            tests.add(args[0]); // user-provided
        }
        // Fill up to 5 test cases
        String[] defaults = new String[] {"alice", "bob", "charlie", "not_exist_user_123", "eve"};
        for (String d : defaults) {
            if (tests.size() >= 5) break;
            if (!tests.contains(d)) tests.add(d);
        }
        // If still less than 5 (e.g., user provided multiple), trim or add more samples
        while (tests.size() < 5) {
            tests.add("dora");
        }
        if (tests.size() > 5) {
            tests = tests.subList(0, 5);
        }

        try (Connection conn = DriverManager.getConnection(url)) {
            ensureSchemaAndSeed(conn);
            for (String username : tests) {
                Customer c = getCustomerByUsername(conn, username);
                if (c != null) {
                    System.out.println(c.toString());
                } else {
                    System.out.println("{\"customerusername\":\"" + username.replace("\"","\\\"") + "\",\"result\":\"not_found_or_invalid\"}");
                }
            }
        } catch (SQLException e) {
            // Minimal leak-safe error message
            System.err.println("{\"error\":\"database_error\"}");
        }
    }
}