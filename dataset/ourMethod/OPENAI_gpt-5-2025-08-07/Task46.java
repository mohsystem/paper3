import java.nio.charset.StandardCharsets;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;
import java.util.regex.Pattern;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Task46 {

    // Simple immutable record for stored user
    static final class UserRecord {
        private final String name;
        private final String emailLower;
        private final String passwordHashB64;
        private final String saltB64;
        private final int iterations;

        UserRecord(String name, String emailLower, String passwordHashB64, String saltB64, int iterations) {
            this.name = name;
            this.emailLower = emailLower;
            this.passwordHashB64 = passwordHashB64;
            this.saltB64 = saltB64;
            this.iterations = iterations;
        }

        public String getName() { return name; }
        public String getEmailLower() { return emailLower; }
        public String getPasswordHashB64() { return passwordHashB64; }
        public String getSaltB64() { return saltB64; }
        public int getIterations() { return iterations; }
    }

    static final class InMemoryDatabase {
        // email (lowercased) -> user
        private final Map<String, UserRecord> users = new HashMap<>();

        public synchronized boolean emailExists(String emailLower) {
            return users.containsKey(emailLower);
        }

        public synchronized boolean addUser(UserRecord user) {
            if (users.containsKey(user.getEmailLower())) {
                return false;
            }
            users.put(user.getEmailLower(), user);
            return true;
        }

        public synchronized int size() {
            return users.size();
        }
    }

    static final class PasswordHasher {
        private static final int SALT_LEN = 16; // bytes
        private static final int ITERATIONS = 210_000;
        private static final int KEY_LEN_BITS = 256; // bits
        private static final String ALGO = "PBKDF2WithHmacSHA256";

        public static byte[] generateSalt() {
            byte[] salt = new byte[SALT_LEN];
            SecureRandom sr = new SecureRandom();
            sr.nextBytes(salt);
            return salt;
        }

        public static byte[] hash(char[] password, byte[] salt, int iterations) throws NoSuchAlgorithmException, InvalidKeySpecException {
            PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, KEY_LEN_BITS);
            SecretKeyFactory skf = SecretKeyFactory.getInstance(ALGO);
            try {
                return skf.generateSecret(spec).getEncoded();
            } finally {
                spec.clearPassword();
            }
        }

        public static HashedPassword hashPassword(String password) throws NoSuchAlgorithmException, InvalidKeySpecException {
            Objects.requireNonNull(password, "password");
            byte[] salt = generateSalt();
            byte[] hash = hash(password.toCharArray(), salt, ITERATIONS);
            String saltB64 = Base64.getEncoder().encodeToString(salt);
            String hashB64 = Base64.getEncoder().encodeToString(hash);
            return new HashedPassword(hashB64, saltB64, ITERATIONS);
        }

        static final class HashedPassword {
            final String hashB64;
            final String saltB64;
            final int iterations;
            HashedPassword(String hashB64, String saltB64, int iterations) {
                this.hashB64 = hashB64;
                this.saltB64 = saltB64;
                this.iterations = iterations;
            }
        }
    }

    static final class Validator {
        private static final Pattern EMAIL_RE = Pattern.compile("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,63}$");
        private static final Pattern NAME_RE = Pattern.compile("^[A-Za-z][A-Za-z '\\-]{0,99}$");
        private static final Pattern UPPER = Pattern.compile(".*[A-Z].*");
        private static final Pattern LOWER = Pattern.compile(".*[a-z].*");
        private static final Pattern DIGIT = Pattern.compile(".*[0-9].*");
        private static final Pattern SPECIAL = Pattern.compile(".*[^A-Za-z0-9].*");

        public static String validateName(String name) {
            if (name == null) return "Name is required.";
            String trimmed = name.trim();
            if (trimmed.isEmpty()) return "Name cannot be empty.";
            if (trimmed.length() > 100) return "Name is too long.";
            if (!NAME_RE.matcher(trimmed).matches()) return "Name contains invalid characters.";
            return null;
        }

        public static String validateEmail(String email) {
            if (email == null) return "Email is required.";
            String trimmed = email.trim();
            if (trimmed.length() > 254) return "Email is too long.";
            if (!EMAIL_RE.matcher(trimmed).matches()) return "Email is invalid.";
            return null;
        }

        public static String validatePassword(String password) {
            if (password == null) return "Password is required.";
            if (password.length() < 12) return "Password must be at least 12 characters.";
            if (password.length() > 128) return "Password is too long.";
            if (!UPPER.matcher(password).matches()) return "Password must include an uppercase letter.";
            if (!LOWER.matcher(password).matches()) return "Password must include a lowercase letter.";
            if (!DIGIT.matcher(password).matches()) return "Password must include a digit.";
            if (!SPECIAL.matcher(password).matches()) return "Password must include a special character.";
            if (password.contains(" ")) return "Password must not contain spaces.";
            return null;
        }
    }

    static final class RegistrationService {
        private final InMemoryDatabase db;

        RegistrationService(InMemoryDatabase db) {
            this.db = db;
        }

        public Result register(String name, String email, String password) {
            // Validate inputs
            String nameErr = Validator.validateName(name);
            if (nameErr != null) return Result.fail(nameErr);

            String emailErr = Validator.validateEmail(email);
            if (emailErr != null) return Result.fail(emailErr);

            String passwordErr = Validator.validatePassword(password);
            if (passwordErr != null) return Result.fail(passwordErr);

            String emailLower = email.trim().toLowerCase();
            if (db.emailExists(emailLower)) {
                return Result.fail("An account with this email already exists.");
            }

            try {
                PasswordHasher.HashedPassword hp = PasswordHasher.hashPassword(password);
                UserRecord user = new UserRecord(
                        name.trim(),
                        emailLower,
                        hp.hashB64,
                        hp.saltB64,
                        hp.iterations
                );
                boolean ok = db.addUser(user);
                if (!ok) {
                    return Result.fail("Registration failed due to a concurrent duplicate email.");
                }
                return Result.ok("Registration successful.");
            } catch (Exception e) {
                return Result.fail("Internal error during registration.");
            }
        }
    }

    static final class Result {
        final boolean success;
        final String message;
        private Result(boolean success, String message) {
            this.success = success;
            this.message = message;
        }
        public static Result ok(String msg) { return new Result(true, msg); }
        public static Result fail(String msg) { return new Result(false, msg); }
    }

    public static void main(String[] args) {
        InMemoryDatabase db = new InMemoryDatabase();
        RegistrationService service = new RegistrationService(db);

        // Test case 1: valid user
        printResult("TC1", service.register("Alice Johnson", "alice@example.com", "Str0ng!Password#1"));

        // Test case 2: another valid user
        printResult("TC2", service.register("Bob O'Hara", "bob.o'hara@example.org".replace("'", ""), "An0ther$ecurePass!"));

        // Test case 3: duplicate email
        printResult("TC3", service.register("Alice J.", "alice@example.com", "NewStrong!Passw0rd"));

        // Test case 4: invalid email
        printResult("TC4", service.register("Charlie Brown", "invalid-email@", "ValidPassw0rd!@#"));

        // Test case 5: weak password
        printResult("TC5", service.register("Dana White", "dana.white@example.net", "weakpass"));
    }

    private static void printResult(String label, Result r) {
        System.out.println(label + ": " + (r.success ? "SUCCESS" : "ERROR") + " - " + r.message);
    }
}