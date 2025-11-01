import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.nio.charset.StandardCharsets;

public class Lse92 {

    public static class User {
        public final String username;
        public final String passwordHash;

        public User(String username, String passwordHash) {
            this.username = username;
            this.passwordHash = passwordHash;
        }

        @Override
        public String toString() {
            return "User{username='" + username + "', passwordHash='" + passwordHash + "'}";
        }
    }

    public static String hashPassword(String password) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            byte[] digest = md.digest(password.getBytes(StandardCharsets.UTF_8));
            return bytesToHex(digest);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("SHA-256 not available", e);
        }
    }

    public static User registerUser(String username, String password) {
        String hash = hashPassword(password);
        return new User(username, hash);
    }

    private static String bytesToHex(byte[] bytes) {
        StringBuilder sb = new StringBuilder(bytes.length * 2);
        for (byte b : bytes) {
            sb.append(Character.forDigit((b >>> 4) & 0xF, 16));
            sb.append(Character.forDigit((b & 0xF), 16));
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        String[] usernames = {"alice", "bob", "charlie", "diana", "eve"};
        String[] passwords = {"p@ssw0rd", "letmein", "123456", "correcthorsebatterystaple", "Tr0ub4dor&3"};

        for (int i = 0; i < usernames.length; i++) {
            User u = registerUser(usernames[i], passwords[i]);
            System.out.println(u);
        }
    }
}