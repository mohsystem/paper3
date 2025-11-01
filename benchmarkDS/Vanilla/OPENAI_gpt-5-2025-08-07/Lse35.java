import java.util.HashMap;
import java.util.Map;

public class Lse35 {
    private static final Map<String, String> DB = new HashMap<>();
    static {
        DB.put("alice", "secret");
        DB.put("bob", "hunter2");
        DB.put("eve", "password123");
    }

    // Returns redirect path based on login success: "/home" or "/login"
    public static String login(String username, String password) {
        if (username == null || password == null) return "/login";
        String stored = DB.get(username);
        return (stored != null && stored.equals(password)) ? "/home" : "/login";
    }

    public static void main(String[] args) {
        String[][] tests = new String[][]{
            {"alice", "secret"},
            {"bob", "hunter2"},
            {"alice", "wrong"},
            {"unknown", "nopass"},
            {"eve", "password123"}
        };
        for (String[] t : tests) {
            String user = t[0], pass = t[1];
            String redirect = login(user, pass);
            System.out.println("login(" + user + ", " + pass + ") -> " + redirect);
        }
    }
}