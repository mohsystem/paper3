import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.regex.Pattern;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse97 {

    // In-memory "database"
    static final class UserRecord {
        final String username;
        final String firstName;
        final String lastName;
        final String email;
        final byte[] salt;
        final byte[] hash;
        final int iterations;

        UserRecord(String username, String firstName, String lastName, String email, byte[] salt, byte[] hash, int iterations) {
            this.username = username;
            this.firstName = firstName;
            this.lastName = lastName;
            this.email = email;
            this.salt = salt;
            this.hash = hash;
            this.iterations = iterations;
        }
    }

    static final class InMemoryDatabase {
        private final Map<String, UserRecord> users = new ConcurrentHashMap<>();

        synchronized boolean usernameExists(String username) {
            return users.containsKey(username);
        }

        synchronized void insert(UserRecord record) {
            users.put(record.username, record);
        }
    }

    // Validation rules
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9_.-]{3,32}$");
    private static final Pattern NAME_PATTERN = Pattern.compile("^[A-Za-z][A-Za-z '\\-]{0,49}$");
    private static final Pattern EMAIL_PATTERN = Pattern.compile("^[A-Za-z0-9._%+-]{1,64}@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");

    private static boolean isValidUsername(String s) {
        if (s == null) return false;
        String v = s.trim();
        return USERNAME_PATTERN.matcher(v).matches();
    }

    private static boolean isValidName(String s) {
        if (s == null) return false;
        String v = s.trim();
        return v.length() >= 1 && v.length() <= 50 && NAME_PATTERN.matcher(v).matches();
    }

    private static boolean isValidEmail(String s) {
        if (s == null) return false;
        String v = s.trim();
        return v.length() <= 254 && EMAIL_PATTERN.matcher(v).matches();
    }

    private static boolean isStrongPassword(String s) {
        if (s == null) return false;
        if (s.length() < 12 || s.length() > 1024) return false;
        boolean hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (Character.isUpperCase(c)) hasUpper = true;
            else if (Character.isLowerCase(c)) hasLower = true;
            else if (Character.isDigit(c)) hasDigit = true;
            else hasSpecial = true;
        }
        return hasUpper && hasLower && hasDigit && hasSpecial;
    }

    private static byte[] pbkdf2(char[] password, byte[] salt, int iterations, int keyLenBytes) throws Exception {
        PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, keyLenBytes * 8);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        return skf.generateSecret(spec).getEncoded();
    }

    private static byte[] randomBytes(int len) {
        byte[] b = new byte[len];
        new SecureRandom().nextBytes(b);
        return b;
    }

    public static String registerUser(InMemoryDatabase db, String username, String password, String firstName, String lastName, String email) {
        // Validate inputs
        if (!isValidUsername(username)) return "Invalid username";
        if (!isStrongPassword(password)) return "Invalid password";
        if (!isValidName(firstName)) return "Invalid first name";
        if (!isValidName(lastName)) return "Invalid last name";
        if (!isValidEmail(email)) return "Invalid email";

        String uname = username.trim();
        String fname = firstName.trim();
        String lname = lastName.trim();
        String mail = email.trim();

        synchronized (db) {
            if (db.usernameExists(uname)) {
                return "Username already exists";
            }
            int iterations = 210_000;
            byte[] salt = randomBytes(16);
            char[] pwChars = password.toCharArray();
            byte[] hash;
            try {
                hash = pbkdf2(pwChars, salt, iterations, 32);
            } catch (Exception e) {
                return "Internal error";
            } finally {
                // Zeroize password chars
                for (int i = 0; i < pwChars.length; i++) pwChars[i] = 0;
            }
            UserRecord rec = new UserRecord(uname, fname, lname, mail, salt, hash, iterations);
            db.insert(rec);
        }
        return "Registration succeeded";
    }

    public static void main(String[] args) {
        InMemoryDatabase db = new InMemoryDatabase();

        String[] results = new String[5];
        results[0] = registerUser(db, "alice.user", "Str0ng!Password", "Alice", "Smith", "alice@example.com");
        results[1] = registerUser(db, "alice.user", "An0ther!StrongPass", "Alice", "Smith", "alice+dup@example.com");
        results[2] = registerUser(db, "a", "Str0ng!Password", "A", "User", "a@ex.com");
        results[3] = registerUser(db, "bob-user", "weakpass", "Bob", "Jones", "bob@example.com");
        results[4] = registerUser(db, "charlie99", "G00d!EnoughPass", "Charlie", "Brown", "charlie.brown@example.org");

        for (int i = 0; i < results.length; i++) {
            System.out.println("Test " + (i + 1) + ": " + results[i]);
        }
    }
}