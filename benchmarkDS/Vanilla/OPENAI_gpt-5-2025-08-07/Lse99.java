import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.nio.charset.StandardCharsets;
import java.util.LinkedHashMap;
import java.util.Map;

public class Lse99 {
    private static final Map<String, String> DB = new LinkedHashMap<>();

    public static String hashPassword(String password) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            byte[] hashed = md.digest(password.getBytes(StandardCharsets.UTF_8));
            StringBuilder sb = new StringBuilder(hashed.length * 2);
            for (byte b : hashed) {
                sb.append(String.format("%02x", b));
            }
            return sb.toString();
        } catch (NoSuchAlgorithmException e) {
            // Fallback (should not happen on standard JVMs)
            throw new RuntimeException("SHA-256 not available", e);
        }
    }

    public static boolean insertUser(String username, String password) {
        if (username == null || password == null) return false;
        if (DB.containsKey(username)) return false;
        String hashed = hashPassword(password);
        DB.put(username, hashed);
        return true;
    }

    public static String getHashedPassword(String username) {
        return DB.get(username);
    }

    public static void clear() {
        DB.clear();
    }

    public static int size() {
        return DB.size();
    }

    public static void main(String[] args) {
        String[][] tests = new String[][] {
            {"alice", "password123"},
            {"bob", "s3cr3t!"},
            {"charlie", "Pa$$w0rd"},
            {"alice", "newpass"},
            {"eve", ""}
        };

        for (String[] t : tests) {
            String user = t[0];
            String pass = t[1];
            boolean ok = insertUser(user, pass);
            System.out.println("Insert(" + user + "): " + ok + " | hash=" + getHashedPassword(user));
        }

        System.out.println("Total users: " + size());
    }
}