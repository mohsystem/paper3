import java.util.HashMap;
import java.util.Map;

public class Lse98 {
    private static final Map<String, String> DB = new HashMap<>();
    private static final String SALT = "Lse98$Salt";

    public static boolean register(String username, String password) {
        if (username == null || password == null || username.isEmpty() || password.isEmpty()) return false;
        if (DB.containsKey(username)) return false;
        String hashed = hashPassword(password);
        DB.put(username, hashed);
        return true;
    }

    public static String getStoredHash(String username) {
        return DB.get(username);
    }

    private static String hashPassword(String password) {
        String input = SALT + ":" + password;
        long hash = 5381L;
        for (int i = 0; i < input.length(); i++) {
            hash = ((hash << 5) + hash) ^ input.charAt(i); // djb2 variant
        }
        return Long.toHexString(hash);
    }

    public static void main(String[] args) {
        System.out.println("Test1 (alice): " + register("alice", "password123"));
        System.out.println("Test2 (bob): " + register("bob", "secret"));
        System.out.println("Test3 (duplicate alice): " + register("alice", "another"));
        System.out.println("Test4 (empty username): " + register("", "nopass"));
        System.out.println("Test5 (empty password): " + register("charlie", ""));
        // Show stored hashes for successful inserts
        System.out.println("alice hash: " + getStoredHash("alice"));
        System.out.println("bob hash: " + getStoredHash("bob"));
        System.out.println("charlie hash: " + getStoredHash("charlie"));
    }
}