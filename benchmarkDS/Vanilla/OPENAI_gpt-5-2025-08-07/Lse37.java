import java.util.HashMap;
import java.util.Map;

public class Lse37 {
    private static final Map<String, String> DB = new HashMap<>();
    static {
        DB.put("alice", "password123");
        DB.put("bob", "qwerty");
        DB.put("charlie", "letmein");
    }

    public static String login(String username, String password) {
        try {
            if (username == null || password == null) {
                throw new RuntimeException("Null credentials");
            }
            if ("error".equals(username) || "error".equals(password)) {
                throw new RuntimeException("Forced error");
            }
            String stored = DB.get(username);
            if (stored != null && stored.equals(password)) {
                return "success";
            }
            return "cant_login";
        } catch (Exception e) {
            return "internal_server_error";
        }
    }

    public static void main(String[] args) {
        System.out.println(login("alice", "password123")); // success
        System.out.println(login("alice", "wrong")); // cant_login
        System.out.println(login("unknown", "pass")); // cant_login
        System.out.println(login(null, "pass")); // internal_server_error
        System.out.println(login("error", "x")); // internal_server_error
    }
}