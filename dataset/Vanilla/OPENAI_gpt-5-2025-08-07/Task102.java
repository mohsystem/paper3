import java.util.*;
import java.nio.charset.StandardCharsets;

public class Task102 {
    private static final String SALT = "Task102_SALT";

    public static String hashPassword(String password) {
        return fnv1a64Hex(SALT + password);
    }

    public static boolean updatePassword(Map<String, String> db, String username, String newPassword) {
        if (username == null || newPassword == null || newPassword.isEmpty()) return false;
        if (!db.containsKey(username)) return false;
        db.put(username, hashPassword(newPassword));
        return true;
    }

    public static String getPasswordHash(Map<String, String> db, String username) {
        return db.get(username);
    }

    public static Map<String, String> initDb() {
        Map<String, String> db = new HashMap<>();
        db.put("alice", hashPassword("old1"));
        db.put("bob", hashPassword("old2"));
        db.put("charlie", hashPassword("old3"));
        return db;
    }

    private static String fnv1a64Hex(String input) {
        long hash = 0xcbf29ce484222325L;
        long prime = 0x100000001b3L;
        for (byte b : input.getBytes(StandardCharsets.UTF_8)) {
            hash ^= (b & 0xFFL);
            hash *= prime;
        }
        String s = Long.toHexString(hash);
        if (s.length() < 16) {
            StringBuilder sb = new StringBuilder(16);
            for (int i = s.length(); i < 16; i++) sb.append('0');
            sb.append(s);
            s = sb.toString();
        } else if (s.length() > 16) {
            s = s.substring(s.length() - 16);
        }
        return s;
    }

    public static void main(String[] args) {
        Map<String, String> db = initDb();
        System.out.println("Initial DB: " + db);

        boolean r1 = updatePassword(db, "alice", "newPass1");
        System.out.println("Test1 update alice: " + r1 + " hash=" + getPasswordHash(db, "alice"));

        boolean r2 = updatePassword(db, "bob", "P@ssw0rd!");
        System.out.println("Test2 update bob: " + r2 + " hash=" + getPasswordHash(db, "bob"));

        boolean r3 = updatePassword(db, "david", "something");
        System.out.println("Test3 update david: " + r3);

        boolean r4 = updatePassword(db, "charlie", "");
        System.out.println("Test4 update charlie with empty: " + r4 + " hash=" + getPasswordHash(db, "charlie"));

        boolean r5 = updatePassword(db, "alice", "another123");
        System.out.println("Test5 update alice again: " + r5 + " hash=" + getPasswordHash(db, "alice"));
    }
}