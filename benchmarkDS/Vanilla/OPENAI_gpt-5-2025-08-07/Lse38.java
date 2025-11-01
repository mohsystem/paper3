import java.util.*;

public class Lse38 {
    private static final Map<String, String> DB = new HashMap<>();
    static {
        DB.put("alice", "password123");
        DB.put("bob", "qwerty");
        DB.put("charlie", "charlie!");
        DB.put("dana", "D@n@2025");
        DB.put("eve", "12345");
    }

    // Returns "dashboard" if credentials are correct, otherwise "login"
    public static String login(String username, String password) {
        if (username == null || password == null) return "login";
        String stored = DB.get(username);
        if (stored != null && stored.equals(password)) return "dashboard";
        return "login";
    }

    public static void main(String[] args) {
        // 5 test cases
        String[][] tests = new String[][]{
            {"alice", "password123"}, // correct
            {"alice", "wrong"},       // wrong password
            {"unknown", "whatever"},  // unknown user
            {"Bob", "qwerty"},        // case sensitivity
            {"", ""}                  // empty credentials
        };

        for (String[] t : tests) {
            String user = t[0];
            String pass = t[1];
            String result = login(user, pass);
            System.out.println("login(\"" + user + "\", \"" + pass + "\") => " + result);
        }
    }
}