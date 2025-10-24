import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.nio.charset.StandardCharsets;

public class Task122 {
    public static class UserStore {
        private final Map<String, String> store = new HashMap<>();
        private final SecureRandom rng = new SecureRandom();

        public boolean signup(String username, String password) {
            if (username == null || password == null) return false;
            if (store.containsKey(username)) return false;
            byte[] salt = new byte[16];
            rng.nextBytes(salt);
            String saltHex = toHex(salt);
            String hashHex = hashPassword(password, salt);
            store.put(username, saltHex + ":" + hashHex);
            return true;
        }

        public boolean verify(String username, String password) {
            String stored = store.get(username);
            if (stored == null || password == null) return false;
            String[] parts = stored.split(":", 2);
            if (parts.length != 2) return false;
            byte[] salt = fromHex(parts[0]);
            String expectedHash = parts[1];
            String actualHash = hashPassword(password, salt);
            return constantTimeEquals(expectedHash, actualHash);
        }

        public String getStored(String username) {
            return store.get(username);
        }

        private String hashPassword(String password, byte[] salt) {
            try {
                MessageDigest md = MessageDigest.getInstance("SHA-256");
                md.update(salt);
                md.update(password.getBytes(StandardCharsets.UTF_8));
                byte[] digest = md.digest();
                return toHex(digest);
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
        }

        private static String toHex(byte[] data) {
            StringBuilder sb = new StringBuilder(data.length * 2);
            for (byte b : data) {
                sb.append(String.format("%02x", b));
            }
            return sb.toString();
        }

        private static byte[] fromHex(String hex) {
            int len = hex.length();
            byte[] out = new byte[len / 2];
            for (int i = 0; i < out.length; i++) {
                int idx = i * 2;
                out[i] = (byte) Integer.parseInt(hex.substring(idx, idx + 2), 16);
            }
            return out;
        }

        private static boolean constantTimeEquals(String a, String b) {
            if (a == null || b == null) return false;
            byte[] x = a.getBytes(StandardCharsets.UTF_8);
            byte[] y = b.getBytes(StandardCharsets.UTF_8);
            if (x.length != y.length) return false;
            int res = 0;
            for (int i = 0; i < x.length; i++) {
                res |= x[i] ^ y[i];
            }
            return res == 0;
        }
    }

    public static void main(String[] args) {
        UserStore us = new UserStore();
        System.out.println("Test1 signup alice: " + us.signup("alice", "Password123!"));
        System.out.println("Test2 signup bob: " + us.signup("bob", "Secr3t!"));
        System.out.println("Test3 duplicate alice: " + us.signup("alice", "another"));
        System.out.println("Test4 verify bob correct: " + us.verify("bob", "Secr3t!"));
        System.out.println("Test5 verify bob wrong: " + us.verify("bob", "wrong"));
        System.out.println("Stored alice: " + us.getStored("alice"));
    }
}