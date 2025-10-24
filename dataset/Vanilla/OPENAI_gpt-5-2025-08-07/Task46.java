import java.io.BufferedWriter;
import java.io.IOException;
import java.nio.file.*;
import java.util.*;
import java.util.regex.Pattern;

public class Task46 {
    private static final String DB_FILE = "users_java.csv";
    private static final String HEADER = "name,email,password";
    private static final Pattern EMAIL_PATTERN = Pattern.compile("^[^@\\s]+@[^@\\s]+\\.[^@\\s]+$");

    static class User {
        String name;
        String email;
        String password;
        User(String n, String e, String p) { name = n; email = e; password = p; }
        @Override public String toString() { return "User{name='" + name + "', email='" + email + "', password='" + password + "'}"; }
    }

    public static void initDB() {
        Path path = Paths.get(DB_FILE);
        if (!Files.exists(path)) {
            try {
                Files.createFile(path);
                try (BufferedWriter bw = Files.newBufferedWriter(path, StandardOpenOption.APPEND)) {
                    bw.write(HEADER);
                    bw.newLine();
                }
            } catch (IOException e) {
                throw new RuntimeException("Failed to initialize DB: " + e.getMessage(), e);
            }
        }
    }

    private static String sanitize(String s) {
        if (s == null) return "";
        return s.replace("\n", " ").replace("\r", " ").replace(",", " ").trim();
    }

    private static boolean isValidEmail(String email) {
        return email != null && EMAIL_PATTERN.matcher(email).matches();
    }

    private static boolean emailExists(String email) {
        Path path = Paths.get(DB_FILE);
        try {
            List<String> lines = Files.readAllLines(path);
            for (int i = 1; i < lines.size(); i++) {
                String line = lines.get(i);
                String[] parts = line.split(",", -1);
                if (parts.length >= 3) {
                    if (parts[1].equalsIgnoreCase(email)) return true;
                }
            }
        } catch (IOException e) {
            throw new RuntimeException("Failed to read DB: " + e.getMessage(), e);
        }
        return false;
    }

    public static boolean registerUser(String name, String email, String password) {
        initDB();
        name = sanitize(name);
        email = sanitize(email);
        password = sanitize(password);

        if (name.isEmpty() || email.isEmpty() || password.isEmpty()) return false;
        if (!isValidEmail(email)) return false;
        if (password.length() < 6) return false;
        if (emailExists(email)) return false;

        Path path = Paths.get(DB_FILE);
        try (BufferedWriter bw = Files.newBufferedWriter(path, StandardOpenOption.APPEND)) {
            bw.write(String.join(",", name, email, password));
            bw.newLine();
        } catch (IOException e) {
            return false;
        }
        return true;
    }

    public static List<User> getAllUsers() {
        initDB();
        List<User> users = new ArrayList<>();
        Path path = Paths.get(DB_FILE);
        try {
            List<String> lines = Files.readAllLines(path);
            for (int i = 1; i < lines.size(); i++) {
                String[] parts = lines.get(i).split(",", -1);
                if (parts.length >= 3) {
                    users.add(new User(parts[0], parts[1], parts[2]));
                }
            }
        } catch (IOException e) {
            throw new RuntimeException("Failed to read DB: " + e.getMessage(), e);
        }
        return users;
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println("Test 1 (valid): " + registerUser("Alice", "alice@example.com", "pass123"));
        System.out.println("Test 2 (duplicate email): " + registerUser("Alicia", "alice@example.com", "differentPass"));
        System.out.println("Test 3 (invalid email): " + registerUser("Bob", "bob_at_example.com", "secret12"));
        System.out.println("Test 4 (short password): " + registerUser("Charlie", "charlie@example.com", "123"));
        System.out.println("Test 5 (valid): " + registerUser("Dana", "dana@example.org", "StrongPass9"));

        System.out.println("All users:");
        for (User u : getAllUsers()) {
            System.out.println(u);
        }
    }
}