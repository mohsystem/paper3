import java.util.*;
import java.nio.charset.StandardCharsets;

public class Lse93 {
    static class User {
        String username;
        String hashedPassword;
        User(String u, String h) { this.username = u; this.hashedPassword = h; }
    }

    private static final List<User> database = new ArrayList<>();

    public static String hashPassword(String password) {
        return fnv1a64(password);
    }

    public static boolean insertUser(String username, String password) {
        String hashed = hashPassword(password);
        database.add(new User(username, hashed));
        return true;
    }

    // FNV-1a 64-bit hash
    private static String fnv1a64(String input) {
        long hash = 0xcbf29ce484222325L;
        long prime = 0x100000001b3L;
        byte[] bytes = input.getBytes(StandardCharsets.UTF_8);
        for (byte b : bytes) {
            hash ^= (b & 0xff);
            hash *= prime;
        }
        return String.format("%016x", hash);
    }

    private static void printDatabase() {
        System.out.println("Current database:");
        for (int i = 0; i < database.size(); i++) {
            User u = database.get(i);
            System.out.println((i + 1) + ") " + u.username + " | " + u.hashedPassword);
        }
    }

    public static void main(String[] args) {
        insertUser("alice", "password123");
        insertUser("bob", "s3cr3t");
        insertUser("charlie", "letmein");
        insertUser("diana", "P@ssw0rd!");
        insertUser("eve", "qwerty");

        printDatabase();
    }
}