import java.security.SecureRandom;
import java.util.HashMap;
import java.util.Map;

public class Task54 {
    static class OTPEntry {
        String code;
        long expiryMillis;
        OTPEntry(String code, long expiryMillis) {
            this.code = code;
            this.expiryMillis = expiryMillis;
        }
    }

    static class OTPManager {
        private final Map<String, OTPEntry> store = new HashMap<>();
        private final SecureRandom random = new SecureRandom();

        public String generateOTP(String username, int length, long ttlMillis) {
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < length; i++) {
                sb.append(random.nextInt(10));
            }
            String code = sb.toString();
            long expiry = System.currentTimeMillis() + ttlMillis;
            store.put(username, new OTPEntry(code, expiry));
            return code;
        }

        public boolean verifyOTP(String username, String code) {
            OTPEntry entry = store.get(username);
            if (entry == null) return false;
            long now = System.currentTimeMillis();
            if (now > entry.expiryMillis) {
                store.remove(username);
                return false;
            }
            boolean ok = entry.code.equals(code);
            if (ok) {
                store.remove(username); // one-time use
            }
            return ok;
        }
    }

    static class AuthService {
        private final Map<String, String> users = new HashMap<>();
        private final OTPManager otpManager;

        AuthService(OTPManager otpManager) {
            this.otpManager = otpManager;
        }

        public void addUser(String username, String password) {
            users.put(username, password);
        }

        public String startLogin(String username, String password, int otpLength, long ttlMillis) {
            String stored = users.get(username);
            if (stored != null && stored.equals(password)) {
                return otpManager.generateOTP(username, otpLength, ttlMillis);
            }
            return null;
        }

        public boolean finishLogin(String username, String otp) {
            if (otp == null) return false;
            return otpManager.verifyOTP(username, otp);
        }
    }

    public static void main(String[] args) throws Exception {
        OTPManager otpManager = new OTPManager();
        AuthService auth = new AuthService(otpManager);
        auth.addUser("alice", "password123");
        auth.addUser("bob", "qwerty");
        auth.addUser("charlie", "letmein");
        auth.addUser("dave", "passw0rd");
        auth.addUser("eve", "123456");

        // Test 1: Successful login
        String otp1 = auth.startLogin("alice", "password123", 6, 120_000);
        boolean t1 = auth.finishLogin("alice", otp1);
        System.out.println("Test1:" + t1);

        // Test 2: Wrong password
        String otp2 = auth.startLogin("bob", "wrong", 6, 120_000);
        boolean t2 = auth.finishLogin("bob", "000000");
        System.out.println("Test2:" + t2);

        // Test 3: Correct password, wrong OTP
        String otp3 = auth.startLogin("charlie", "letmein", 6, 120_000);
        boolean t3 = auth.finishLogin("charlie", "111111");
        System.out.println("Test3:" + t3);

        // Test 4: Expired OTP
        String otp4 = auth.startLogin("dave", "passw0rd", 6, 1_000);
        Thread.sleep(1500);
        boolean t4 = auth.finishLogin("dave", otp4);
        System.out.println("Test4:" + t4);

        // Test 5: Reuse OTP attempt
        String otp5 = auth.startLogin("eve", "123456", 6, 120_000);
        boolean t5a = auth.finishLogin("eve", otp5);
        boolean t5b = auth.finishLogin("eve", otp5);
        System.out.println("Test5:" + (t5a && !t5b));
    }
}