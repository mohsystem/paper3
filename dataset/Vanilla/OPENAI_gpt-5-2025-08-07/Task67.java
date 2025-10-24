import java.sql.*;

public class Task67 {

    public static class Customer {
        public final int id;
        public final String customerusername;
        public final String name;
        public final String email;
        public final String phone;
        public final String address;

        public Customer(int id, String customerusername, String name, String email, String phone, String address) {
            this.id = id;
            this.customerusername = customerusername;
            this.name = name;
            this.email = email;
            this.phone = phone;
            this.address = address;
        }

        public String toJson() {
            return "{"
                + "\"id\":" + id + ","
                + "\"customerusername\":\"" + escape(customerusername) + "\","
                + "\"name\":\"" + escape(name) + "\","
                + "\"email\":\"" + escape(email) + "\","
                + "\"phone\":\"" + escape(phone) + "\","
                + "\"address\":\"" + escape(address) + "\""
                + "}";
        }

        private String escape(String s) {
            return s == null ? "" : s.replace("\\", "\\\\").replace("\"", "\\\"");
        }
    }

    public static Connection initDatabase() throws SQLException {
        Connection conn = DriverManager.getConnection("jdbc:sqlite::memory:");
        try (Statement st = conn.createStatement()) {
            st.executeUpdate("CREATE TABLE customer ("
                    + "id INTEGER PRIMARY KEY,"
                    + "customerusername TEXT UNIQUE,"
                    + "name TEXT,"
                    + "email TEXT,"
                    + "phone TEXT,"
                    + "address TEXT"
                    + ")");
        }
        try (PreparedStatement ps = conn.prepareStatement(
                "INSERT INTO customer (id, customerusername, name, email, phone, address) VALUES (?, ?, ?, ?, ?, ?)")) {
            insertCustomer(ps, 1, "alice", "Alice Johnson", "alice@example.com", "111-222-3333", "123 Maple St");
            insertCustomer(ps, 2, "bob", "Bob Smith", "bob@example.com", "222-333-4444", "456 Oak Ave");
            insertCustomer(ps, 3, "charlie", "Charlie Lee", "charlie@example.com", "333-444-5555", "789 Pine Rd");
            insertCustomer(ps, 4, "diana", "Diana Prince", "diana@example.com", "444-555-6666", "101 Cedar Blvd");
            insertCustomer(ps, 5, "eric", "Eric Yang", "eric@example.com", "555-666-7777", "202 Birch Ln");
        }
        return conn;
    }

    private static void insertCustomer(PreparedStatement ps, int id, String username, String name, String email, String phone, String address) throws SQLException {
        ps.setInt(1, id);
        ps.setString(2, username);
        ps.setString(3, name);
        ps.setString(4, email);
        ps.setString(5, phone);
        ps.setString(6, address);
        ps.executeUpdate();
    }

    public static Customer getCustomerByUsername(Connection conn, String customerusername) throws SQLException {
        String sql = "SELECT id, customerusername, name, email, phone, address FROM customer WHERE customerusername = ?";
        try (PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setString(1, customerusername);
            try (ResultSet rs = ps.executeQuery()) {
                if (rs.next()) {
                    return new Customer(
                        rs.getInt("id"),
                        rs.getString("customerusername"),
                        rs.getString("name"),
                        rs.getString("email"),
                        rs.getString("phone"),
                        rs.getString("address")
                    );
                }
            }
        }
        return null;
    }

    public static void main(String[] args) throws Exception {
        Connection conn = initDatabase();

        if (args.length > 0) {
            Customer c = getCustomerByUsername(conn, args[0]);
            System.out.println(c != null ? c.toJson() : "null");
            return;
        }

        String[] tests = { "alice", "bob", "charlie", "diana", "unknown" };
        for (String u : tests) {
            Customer c = getCustomerByUsername(conn, u);
            System.out.println(c != null ? c.toJson() : "null");
        }
    }
}