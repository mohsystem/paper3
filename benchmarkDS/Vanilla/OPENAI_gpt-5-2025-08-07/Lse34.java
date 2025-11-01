import java.util.HashMap;
import java.util.Map;

public class Lse34 {
    // Returns the redirect path based on authentication result: "/dashboard" or "/login"
    public static String loginRedirect(String username, String password, Map<String, String> database) {
        if (username == null || password == null || database == null) return "/login";
        String stored = database.get(username);
        return (stored != null && stored.equals(password)) ? "/dashboard" : "/login";
    }

    public static void main(String[] args) {
        Map<String, String> db = new HashMap<>();
        db.put("alice", "pass123");
        db.put("bob", "secret");
        db.put("carol", "hunter2");

        String[][] tests = new String[][]{
            {"alice", "pass123"},
            {"alice", "wrong"},
            {"unknown", "pass123"},
            {"bob", "secret"},
            {"carol", "HUNTER2"}
        };

        int i = 1;
        for (String[] t : tests) {
            String result = loginRedirect(t[0], t[1], db);
            System.out.println("Test " + i++ + " (" + t[0] + "): " + result);
        }
    }
}