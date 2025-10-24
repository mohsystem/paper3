import java.sql.*;
import java.util.*;
import java.util.regex.Pattern;

public class Task67 {

    public static final class Customer {
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
            return "Customer{id=" + id + ", username='" + username + "', fullName='" + fullName +
                    "', email='" + email + "', createdAt='" + createdAt + "'}";
        }
    }

    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9._-]{1,64}$");

    public static boolean isValidUsername(String username) {
        if (username == null) return false;
        return USERNAME_PATTERN.matcher(username).matches();
    }

    // Returns a connection. If DB_URL env var is set, uses that. Otherwise, uses in-memory SQLite and initializes sample data.
    public static Connection getConnectionOrInitSample() throws SQLException {
        String url = System.getenv("DB_URL");
        String user = System.getenv("DB_USER");
        String pass = System.getenv("DB_PASSWORD");
        Connection conn;

        if (url != null && !url.isEmpty()) {
            conn = DriverManager.getConnection(url, user, pass);
        } else {
            conn = DriverManager.getConnection("jdbc:sqlite::memory:");
            initSchemaAndSampleData(conn);
        }
        setSafeConnectionOptions(conn);
        return conn;
    }

    private static void setSafeConnectionOptions(Connection conn) {
        try {
            conn.setAutoCommit(true);
            conn.setReadOnly(false);
        } catch (SQLException ignored) {
        }
    }

    private static void initSchemaAndSampleData(Connection conn) throws SQLException {
        String createTable = "CREATE TABLE IF NOT EXISTS customer (" +
                "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                "username TEXT UNIQUE NOT NULL," +
                "full_name TEXT NOT NULL," +
                "email TEXT NOT NULL," +
                "created_at TEXT NOT NULL" +
                ")";
        try (Statement st = conn.createStatement()) {
            st.execute(createTable);
        }
        String insert = "INSERT INTO customer(username, full_name, email, created_at) VALUES(?,?,?,?)";
        try (PreparedStatement ps = conn.prepareStatement(insert)) {
            insertCustomer(ps, "alice", "Alice Johnson", "alice@example.com", "2023-01-10T09:15:00Z");
            insertCustomer(ps, "bob", "Bob Smith", "bob@example.com", "2023-02-12T10:20:00Z");
            insertCustomer(ps, "carol", "Carol White", "carol@example.com", "2023-03-15T11:25:00Z");
            insertCustomer(ps, "dave", "Dave Brown", "dave@example.com", "2023-04-18T12:30:00Z");
            insertCustomer(ps, "eve", "Eve Black", "eve@example.com", "2023-05-20T13:35:00Z");
        }
    }

    private static void insertCustomer(PreparedStatement ps, String u, String f, String e, String c) throws SQLException {
        ps.setString(1, u);
        ps.setString(2, f);
        ps.setString(3, e);
        ps.setString(4, c);
        ps.executeUpdate();
    }

    // Secure retrieval using PreparedStatement. Returns Customer or null if not found.
    public static Customer getCustomerByUsername(Connection conn, String customerUsername) throws SQLException {
        if (!isValidUsername(customerUsername)) {
            return null;
        }
        String sql = "SELECT id, username, full_name, email, created_at FROM customer WHERE username = ?";
        try (PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setString(1, customerUsername);
            try (ResultSet rs = ps.executeQuery()) {
                if (rs.next()) {
                    int id = rs.getInt("id");
                    String username = rs.getString("username");
                    String fullName = rs.getString("full_name");
                    String email = rs.getString("email");
                    String createdAt = rs.getString("created_at");
                    return new Customer(id, username, fullName, email, createdAt);
                }
            }
        }
        return null;
    }

    public static void main(String[] args) {
        // If a username is provided via CLI, perform single lookup; else run 5 test cases.
        try (Connection conn = getConnectionOrInitSample()) {
            if (args != null && args.length >= 1) {
                String username = args[0];
                Customer c = getCustomerByUsername(conn, username);
                if (c != null) {
                    System.out.println(c.toString());
                } else {
                    System.out.println("No customer found for username: " + username);
                }
            } else {
                List<String> tests = Arrays.asList("alice", "bob", "carol", "dave", "mallory");
                for (String t : tests) {
                    Customer c = getCustomerByUsername(conn, t);
                    if (c != null) {
                        System.out.println(c.toString());
                    } else {
                        System.out.println("No customer found for username: " + t);
                    }
                }
            }
        } catch (SQLException e) {
            System.err.println("Database operation failed.");
        }
    }
}