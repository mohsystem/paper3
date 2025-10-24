import java.nio.charset.StandardCharsets;
import java.security.GeneralSecurityException;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.time.Instant;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Task53 {

    // Data models
    static final class User {
        final String username;
        final byte[] salt; // 16 bytes
        final byte[] passwordHash; // derived key
        final int iterations;
        final long passwordChangedAtEpochSeconds;

        User(String username, byte[] salt, byte[] passwordHash, int iterations, long passwordChangedAtEpochSeconds) {
            this.username = username;
            this.salt = Arrays.copyOf(salt, salt.length);
            this.passwordHash = Arrays.copyOf(passwordHash, passwordHash.length);
            this.iterations = iterations;
            this.passwordChangedAtEpochSeconds = passwordChangedAtEpochSeconds;
        }
    }

    static final class Session {
        final String username;
        final long expiresAtEpochMillis;

        Session(String username, long expiresAtEpochMillis) {
            this.username = username;
            this.expiresAtEpochMillis = expiresAtEpochMillis;
        }
    }

    // Utilities
    static final class Crypto {
        private static final String PBKDF2_ALG = "PBKDF2WithHmacSHA256";
        private static final int SALT_LEN = 16;
        private static final int ITERATIONS = 120_000;
        private static final int KEY_LEN_BITS = 256;
        private static final SecureRandom RNG = new SecureRandom();

        static byte[] newSalt() {
            byte[] s = new byte[SALT_LEN];
            RNG.nextBytes(s);
            return s;
        }

        static byte[] pbkdf2(char[] password, byte[] salt, int iterations) {
            try {
                PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, KEY_LEN_BITS);
                SecretKeyFactory skf = SecretKeyFactory.getInstance(PBKDF2_ALG);
                return skf.generateSecret(spec).getEncoded();
            } catch (GeneralSecurityException e) {
                throw new IllegalStateException("PBKDF2 failure", e);
            }
        }

        static String newToken() {
            byte[] token = new byte[32];
            RNG.nextBytes(token);
            return Base64.getUrlEncoder().withoutPadding().encodeToString(token);
        }

        static boolean constantTimeEquals(byte[] a, byte[] b) {
            return MessageDigest.isEqual(a, b);
        }

        static boolean constantTimeEquals(String a, String b) {
            byte[] ab = a == null ? new byte[0] : a.getBytes(StandardCharsets.UTF_8);
            byte[] bb = b == null ? new byte[0] : b.getBytes(StandardCharsets.UTF_8);
            return constantTimeEquals(ab, bb);
        }
    }

    static final class Validators {
        static boolean isUsernameValid(String username) {
            if (username == null) return false;
            if (username.length() < 3 || username.length() > 32) return false;
            for (int i = 0; i < username.length(); i++) {
                char c = username.charAt(i);
                if (!(Character.isLetterOrDigit(c) || c == '_' || c == '-')) return false;
            }
            return true;
        }

        static boolean isPasswordStrong(String password) {
            if (password == null || password.length() < 12 || password.length() > 128) return false;
            boolean hasLower = false, hasUpper = false, hasDigit = false, hasSpecial = false;
            String specials = "!@#$%^&*()_+-=[]{};':\",.<>/?\\|`~";
            for (int i = 0; i < password.length(); i++) {
                char c = password.charAt(i);
                if (Character.isLowerCase(c)) hasLower = true;
                else if (Character.isUpperCase(c)) hasUpper = true;
                else if (Character.isDigit(c)) hasDigit = true;
                else if (specials.indexOf(c) >= 0) hasSpecial = true;
            }
            return hasLower && hasUpper && hasDigit && hasSpecial;
        }
    }

    // Core services
    static final class UserStore {
        private final Map<String, User> users = new HashMap<>();

        synchronized boolean addUser(User u) {
            if (users.containsKey(u.username)) return false;
            users.put(u.username, u);
            return true;
        }

        synchronized User getUser(String username) {
            return users.get(username);
        }

        synchronized boolean updateUser(User updated) {
            if (!users.containsKey(updated.username)) return false;
            users.put(updated.username, updated);
            return true;
        }
    }

    static final class SessionManager {
        private final Map<String, Session> sessions = new HashMap<>();
        private final long sessionTtlMillis;

        SessionManager(long sessionTtlMillis) {
            this.sessionTtlMillis = sessionTtlMillis;
        }

        synchronized String createSession(String username) {
            String token = Crypto.newToken();
            long exp = System.currentTimeMillis() + sessionTtlMillis;
            sessions.put(token, new Session(username, exp));
            return token;
        }

        synchronized boolean validate(String token) {
            if (token == null || token.length() < 10) return false;
            Session s = sessions.get(token);
            if (s == null) return false;
            if (System.currentTimeMillis() > s.expiresAtEpochMillis) {
                sessions.remove(token);
                return false;
            }
            return true;
        }

        synchronized String getUsername(String token) {
            if (!validate(token)) return null;
            return sessions.get(token).username;
        }

        synchronized boolean logout(String token) {
            if (token == null) return false;
            return sessions.remove(token) != null;
        }

        synchronized String rotate(String token) {
            if (!validate(token)) return null;
            Session old = sessions.remove(token);
            String newToken = Crypto.newToken();
            long exp = System.currentTimeMillis() + sessionTtlMillis;
            sessions.put(newToken, new Session(old.username, exp));
            return newToken;
        }
    }

    static final class AuthService {
        private final UserStore userStore;
        private final SessionManager sessionManager;
        private final long passwordExpiryDays;

        AuthService(UserStore userStore, SessionManager sessionManager, long passwordExpiryDays) {
            this.userStore = userStore;
            this.sessionManager = sessionManager;
            this.passwordExpiryDays = passwordExpiryDays;
        }

        boolean register(String username, String password) {
            if (!Validators.isUsernameValid(username)) return false;
            if (!Validators.isPasswordStrong(password)) return false;
            byte[] salt = Crypto.newSalt();
            byte[] hash = Crypto.pbkdf2(password.toCharArray(), salt, Crypto.ITERATIONS);
            long now = Instant.now().getEpochSecond();
            User u = new User(username, salt, hash, Crypto.ITERATIONS, now);
            return userStore.addUser(u);
        }

        private boolean isPasswordExpired(User u) {
            long maxAgeSeconds = passwordExpiryDays * 24L * 3600L;
            return (Instant.now().getEpochSecond() - u.passwordChangedAtEpochSeconds) > maxAgeSeconds;
        }

        String login(String username, String password) {
            if (!Validators.isUsernameValid(username)) return null;
            if (password == null || password.length() < 1) return null;
            User u = userStore.getUser(username);
            if (u == null) return null;
            byte[] calc = Crypto.pbkdf2(password.toCharArray(), u.salt, u.iterations);
            if (!Crypto.constantTimeEquals(calc, u.passwordHash)) return null;
            if (isPasswordExpired(u)) {
                return null;
            }
            return sessionManager.createSession(username);
        }

        boolean validateSession(String token) {
            return sessionManager.validate(token);
        }

        String rotateSession(String token) {
            return sessionManager.rotate(token);
        }

        boolean logout(String token) {
            return sessionManager.logout(token);
        }

        boolean changePassword(String username, String oldPassword, String newPassword) {
            if (!Validators.isUsernameValid(username)) return false;
            if (!Validators.isPasswordStrong(newPassword)) return false;
            User u = userStore.getUser(username);
            if (u == null) return false;
            byte[] calc = Crypto.pbkdf2(oldPassword.toCharArray(), u.salt, u.iterations);
            if (!Crypto.constantTimeEquals(calc, u.passwordHash)) return false;
            byte[] newSalt = Crypto.newSalt();
            byte[] newHash = Crypto.pbkdf2(newPassword.toCharArray(), newSalt, Crypto.ITERATIONS);
            long now = Instant.now().getEpochSecond();
            User updated = new User(username, newSalt, newHash, Crypto.ITERATIONS, now);
            return userStore.updateUser(updated);
        }

        String getUsernameFromSession(String token) {
            return sessionManager.getUsername(token);
        }
    }

    // Demo with 5 test cases (no secrets printed)
    public static void main(String[] args) {
        UserStore store = new UserStore();
        SessionManager sessions = new SessionManager(30 * 60 * 1000); // 30 minutes
        AuthService auth = new AuthService(store, sessions, 90); // 90 days password expiry

        // Test 1: Register user with strong password
        boolean t1 = auth.register("alice_user", "Str0ngP@ssw0rd!");
        System.out.println("Test1 register strong password: " + t1);

        // Test 2: Duplicate registration should fail
        boolean t2 = auth.register("alice_user", "An0ther$tr0ngPass!");
        System.out.println("Test2 duplicate register: " + t2);

        // Test 3: Login with wrong password should fail
        String badLogin = auth.login("alice_user", "WrongP@ssword123");
        System.out.println("Test3 login wrong password success? " + (badLogin != null));

        // Test 4: Login with correct password, validate, rotate session
        String token = auth.login("alice_user", "Str0ngP@ssw0rd!");
        boolean validBeforeRotate = auth.validateSession(token);
        String newToken = auth.rotateSession(token);
        boolean oldValidAfterRotate = auth.validateSession(token);
        boolean newValidAfterRotate = auth.validateSession(newToken);
        System.out.println("Test4 login+validate before rotate: " + validBeforeRotate
                + ", old token valid after rotate: " + oldValidAfterRotate
                + ", new token valid after rotate: " + newValidAfterRotate);

        // Test 5: Change password, old login fails, new login succeeds, logout invalidates session
        boolean changed = auth.changePassword("alice_user", "Str0ngP@ssw0rd!", "N3w$uperStr0ngP@ss!");
        String oldTry = auth.login("alice_user", "Str0ngP@ssw0rd!");
        String newTry = auth.login("alice_user", "N3w$uperStr0ngP@ss!");
        boolean logoutOk = auth.logout(newTry); // logout using session token
        boolean validAfterLogout = auth.validateSession(newTry);
        System.out.println("Test5 change password: " + changed
                + ", old login success? " + (oldTry != null)
                + ", new login success? " + (newTry != null)
                + ", logout ok: " + logoutOk
                + ", session valid after logout? " + validAfterLogout);
    }
}