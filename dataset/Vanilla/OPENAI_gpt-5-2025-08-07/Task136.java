import java.util.HashMap;
import java.util.Map;

public class Task136 {
    public static boolean authenticate(Map<String, String> users, String username, String password) {
        if (users == null || username == null || password == null) return false;
        String stored = users.get(username);
        return stored != null && stored.equals(password);
    }

    public static void main(String[] args) {
        Map<String, String> users = new HashMap<>();
        users.put("alice", "pass123");
        users.put("bob", "qwerty");
        users.put("charlie", "letmein");

        System.out.println("Test 1 (alice, correct): " + authenticate(users, "alice", "pass123"));
        System.out.println("Test 2 (bob, wrong): " + authenticate(users, "bob", "wrong"));
        System.out.println("Test 3 (unknown user): " + authenticate(users, "eve", "anything"));
        System.out.println("Test 4 (charlie, empty password): " + authenticate(users, "charlie", ""));
        System.out.println("Test 5 (case sensitivity): " + authenticate(users, "Alice", "pass123"));
    }
}