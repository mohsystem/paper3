import java.util.*;

public class Lse36 {
    private static final Map<String, String> DB = new HashMap<>();
    static {
        DB.put("alice", "wonderland");
        DB.put("bob", "builder");
        DB.put("admin", "admin123");
    }

    public static String login(String username, String password) {
        if (username == null || password == null) return "return";
        String stored = DB.get(username);
        if (stored == null) return "return";
        if (stored.equals(password)) return "redirect:/home";
        return "return";
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println(login("alice", "wonderland")); // success -> redirect
        System.out.println(login("alice", "wrong"));      // wrong password -> return
        System.out.println(login("bob", "builder"));      // success -> redirect
        System.out.println(login("charlie", "pass"));     // user not found -> return
        System.out.println(login("", ""));                // empty credentials -> return
    }
}