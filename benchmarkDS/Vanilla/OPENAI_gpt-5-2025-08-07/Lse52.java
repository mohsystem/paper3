import java.util.*;

public class Lse52 {
    public static String indexPage() {
        return "<html><body><a href='/login'>Login</a></body></html>";
    }

    public static String loginPage() {
        return "<html><body><form method='post' action='/do_login'>"
                + "Username: <input type='text' name='username'/> "
                + "Password: <input type='password' name='password'/> "
                + "<button type='submit'>Login</button>"
                + "</form></body></html>";
    }

    public static boolean credentialsValid(String u, String p) {
        return ("admin".equals(u) && "password".equals(p)) ||
               ("alice".equals(u) && "wonderland".equals(p));
    }

    public static String doLogin(String username, String password, Map<String, String> session) {
        if (credentialsValid(username, password)) {
            session.put("username", username);
            return "REDIRECT:/secret";
        } else {
            return "REDIRECT:/login?error=1";
        }
    }

    public static String secretPage(Map<String, String> session) {
        if (session != null && session.containsKey("username")) {
            return "Secret settings";
        } else {
            return "REDIRECT:/login";
        }
    }

    public static void main(String[] args) {
        // Test 1: index page
        System.out.println("TEST1 index: " + indexPage());
        // Test 2: login page
        System.out.println("TEST2 login page: " + loginPage());
        // Test 3: failed login attempt
        Map<String, String> session1 = new HashMap<>();
        System.out.println("TEST3 do_login fail: " + doLogin("admin", "wrong", session1));
        System.out.println("TEST3 secret after fail: " + secretPage(session1));
        // Test 4: successful login (admin)
        Map<String, String> session2 = new HashMap<>();
        System.out.println("TEST4 do_login success: " + doLogin("admin", "password", session2));
        System.out.println("TEST4 secret after success: " + secretPage(session2));
        // Test 5: successful login (alice)
        Map<String, String> session3 = new HashMap<>();
        System.out.println("TEST5 do_login success: " + doLogin("alice", "wonderland", session3));
        System.out.println("TEST5 secret after success: " + secretPage(session3));
    }
}