import java.security.MessageDigest;
import java.security.SecureRandom;
import java.nio.charset.StandardCharsets;
import java.util.*;

public class Lse94 {
    static class UserRecord {
        final String username;
        final String saltHex;
        final String hashHex;

        UserRecord(String username, String saltHex, String hashHex) {
            this.username = username;
            this.saltHex = saltHex;
            this.hashHex = hashHex;
        }

        @Override
        public String toString() {
            return "UserRecord{username='" + username + "', salt='" + saltHex + "', hash='" + hashHex + "'}";
        }
    }

    static class Database {
        private final Map<String, UserRecord> table = new LinkedHashMap<>();

        public boolean insert(UserRecord rec) {
            if (table.containsKey(rec.username)) return false;
            table.put(rec.username, rec);
            return true;
        }

        public Collection<UserRecord> getAll() {
            return table.values();
        }
    }

    public static UserRecord registerUser(Database db, String username, String password) {
        try {
            byte[] salt = new byte[16];
            SecureRandom sr = new SecureRandom();
            sr.nextBytes(salt);
            String saltHex = bytesToHex(salt);
            String hashHex = hashPasswordWithSalt(password, salt);
            UserRecord rec = new UserRecord(username, saltHex, hashHex);
            boolean ok = db.insert(rec);
            return ok ? rec : null;
        } catch (Exception e) {
            return null;
        }
    }

    private static String hashPasswordWithSalt(String password, byte[] salt) throws Exception {
        MessageDigest md = MessageDigest.getInstance("SHA-256");
        md.update(salt);
        byte[] digest = md.digest(password.getBytes(StandardCharsets.UTF_8));
        return bytesToHex(digest);
    }

    private static String bytesToHex(byte[] bytes) {
        StringBuilder sb = new StringBuilder(bytes.length * 2);
        for (byte b : bytes) sb.append(String.format("%02x", b));
        return sb.toString();
    }

    public static void main(String[] args) {
        Database db = new Database();
        String[][] tests = {
            {"alice", "p@ssw0rd"},
            {"bob", "hunter2"},
            {"charlie", "letmein"},
            {"dana", "qwerty123"},
            {"eve", "correct horse battery staple"}
        };
        for (String[] t : tests) {
            UserRecord r = registerUser(db, t[0], t[1]);
            System.out.println(r != null ? "Inserted: " + r : "Insert failed for user " + t[0]);
        }
        System.out.println("All records in DB:");
        for (UserRecord r : db.getAll()) {
            System.out.println(r);
        }
    }
}