import java.util.*;
import java.nio.charset.StandardCharsets;

final class Customer {
    private final String username;
    private final String fullName;
    private final String email;
    private final String phone;
    private final String createdAtIso;

    public Customer(String username, String fullName, String email, String phone, String createdAtIso) {
        this.username = username;
        this.fullName = fullName;
        this.email = email;
        this.phone = phone;
        this.createdAtIso = createdAtIso;
    }

    public String username() { return username; }
    public String fullName() { return fullName; }
    public String email() { return email; }
    public String phone() { return phone; }
    public String createdAtIso() { return createdAtIso; }

    private static String escapeJson(String s) {
        if (s == null) return "null";
        StringBuilder sb = new StringBuilder(s.length() + 16);
        sb.append('"');
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            switch (c) {
                case '\\': sb.append("\\\\"); break;
                case '"':  sb.append("\\\""); break;
                case '\b': sb.append("\\b"); break;
                case '\f': sb.append("\\f"); break;
                case '\n': sb.append("\\n"); break;
                case '\r': sb.append("\\r"); break;
                case '\t': sb.append("\\t"); break;
                default:
                    if (c < 0x20) {
                        sb.append(String.format(Locale.ROOT, "\\u%04x", (int)c));
                    } else {
                        sb.append(c);
                    }
            }
        }
        sb.append('"');
        return sb.toString();
    }

    public String toJson() {
        return "{"
                + "\"username\":" + escapeJson(username) + ","
                + "\"fullName\":" + escapeJson(fullName) + ","
                + "\"email\":" + escapeJson(email) + ","
                + "\"phone\":" + escapeJson(phone) + ","
                + "\"createdAtIso\":" + escapeJson(createdAtIso)
                + "}";
    }
}

public class Task67 {
    // Security: validate and sanitize username input
    public static boolean isValidUsername(String username) {
        if (username == null) return false;
        byte[] bytes = username.getBytes(StandardCharsets.UTF_8);
        if (bytes.length < 1 || bytes.length > 32) return false; // length in bytes to be strict
        for (int i = 0; i < username.length(); i++) {
            char c = username.charAt(i);
            boolean ok = (c >= 'A' && c <= 'Z')
                    || (c >= 'a' && c <= 'z')
                    || (c >= '0' && c <= '9')
                    || c == '_' || c == '-' || c == '.';
            if (!ok) return false;
        }
        return true;
    }

    // Simulated secure DB query using a Map as the "customer" table
    public static Optional<Customer> getCustomerByUsername(Map<String, Customer> customerTable, String username) {
        if (!isValidUsername(username)) return Optional.empty();
        return Optional.ofNullable(customerTable.get(username));
    }

    private static Map<String, Customer> seedInMemoryDb() {
        Map<String, Customer> db = new LinkedHashMap<>();
        db.put("alice.smith", new Customer("alice.smith", "Alice Smith", "alice@example.com", "+1-202-555-0101", "2024-07-01T10:00:00Z"));
        db.put("bob_1984", new Customer("bob_1984", "Bob Stone", "bob1984@example.com", "+1-202-555-0102", "2023-12-11T08:45:30Z"));
        db.put("charlie", new Customer("charlie", "Charlie Kim", "charlie@example.org", "+44-20-7946-0958", "2022-05-20T15:20:10Z"));
        return db;
    }

    private static void runSingleQuery(Map<String, Customer> db, String uname) {
        Optional<Customer> res = getCustomerByUsername(db, uname);
        if (res.isPresent()) {
            System.out.println(res.get().toJson());
        } else {
            System.out.println("{\"error\":\"Customer not found or invalid username\",\"username\":\"" + uname.replace("\"","\\\"") + "\"}");
        }
    }

    public static void main(String[] args) {
        Map<String, Customer> db = seedInMemoryDb();

        // If a username is provided as command line argument, query it
        if (args != null && args.length > 0) {
            runSingleQuery(db, args[0]);
        }

        // 5 test cases
        String[] tests = new String[] {
                "alice.smith",        // existing
                "bob_1984",           // existing
                "invalid user",       // invalid (space)
                "unknown_user",       // not existing but valid
                "averylongusername_exceeding_32_chars" // invalid (too long)
        };
        for (String t : tests) {
            runSingleQuery(db, t);
        }
    }
}