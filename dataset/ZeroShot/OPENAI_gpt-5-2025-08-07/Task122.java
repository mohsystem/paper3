import java.security.SecureRandom;
import java.security.spec.KeySpec;
import java.util.Base64;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.regex.Pattern;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Task122 {
    public static class UserStore {
        private final Map<String, String> store = new ConcurrentHashMap<>();
        private static final int SALT_LEN = 16;
        private static final int ITERATIONS = 200_000;
        private static final int KEY_LEN = 32; // bytes
        private static final SecureRandom RNG = new SecureRandom();
        private static final Pattern USER_RE = Pattern.compile("^[A-Za-z0-9_]{3,32}$");

        public String signup(String username, char[] password) {
            if (username == null || password == null) return "ERROR: Null input.";
            if (!USER_RE.matcher(username).matches()) {
                secureWipe(password);
                return "ERROR: Invalid username.";
            }
            String pwdCheck = validatePassword(password);
            if (pwdCheck != null) {
                secureWipe(password);
                return "ERROR: " + pwdCheck;
            }
            if (store.containsKey(username)) {
                secureWipe(password);
                return "ERROR: Username already exists.";
            }
            byte[] salt = new byte[SALT_LEN];
            RNG.nextBytes(salt);
            byte[] dk;
            try {
                KeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, KEY_LEN * 8);
                SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
                dk = skf.generateSecret(spec).getEncoded();
            } catch (Exception e) {
                secureWipe(password);
                return "ERROR: Hashing failure.";
            }
            String rec = "pbkdf2_sha256$" + ITERATIONS + "$" +
                    Base64.getEncoder().encodeToString(salt) + "$" +
                    Base64.getEncoder().encodeToString(dk);
            store.put(username, rec);
            secureWipe(password);
            return "SUCCESS";
        }

        private static String validatePassword(char[] pwd) {
            if (pwd.length < 12) return "Password too short (min 12).";
            boolean hasU = false, hasL = false, hasD = false, hasS = false;
            for (char c : pwd) {
                if (Character.isUpperCase(c)) hasU = true;
                else if (Character.isLowerCase(c)) hasL = true;
                else if (Character.isDigit(c)) hasD = true;
                else if (!Character.isWhitespace(c)) hasS = true;
            }
            if (!(hasU && hasL && hasD && hasS)) return "Password must include upper, lower, digit, and symbol.";
            return null;
        }

        private static void secureWipe(char[] arr) {
            if (arr != null) {
                for (int i = 0; i < arr.length; i++) arr[i] = 0;
            }
        }

        public String getRecord(String username) {
            return store.get(username);
        }
    }

    public static void main(String[] args) {
        UserStore us = new UserStore();
        System.out.println(us.signup("alice", "StrongPassw0rd!".toCharArray()));
        System.out.println(us.getRecord("alice"));
        System.out.println(us.signup("bob", "short".toCharArray()));
        System.out.println(us.signup("invalid user", "AnotherStrong1!".toCharArray()));
        System.out.println(us.signup("alice", "AnotherStrong1!".toCharArray()));
        System.out.println(us.signup("carol_123", "C0mplEx#Passw0rd".toCharArray()));
        System.out.println(us.getRecord("carol_123"));
    }
}