import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;

public class Lse52 {

    private static final String EXPECTED_USER = "admin";
    private static final String EXPECTED_PASS = "S3cureP@ss";

    public static String indexPage(Map<String, String> session) {
        return "<html><body><h1>Index</h1><a href=\"/login\">Login</a></body></html>";
    }

    public static String loginPage() {
        return "<html><body><h1>Login</h1><form method=\"POST\" action=\"/do_login\">" +
               "<input type=\"text\" name=\"username\" maxlength=\"64\"/>" +
               "<input type=\"password\" name=\"password\" maxlength=\"64\"/>" +
               "<input type=\"submit\" value=\"Login\"/>" +
               "</form></body></html>";
    }

    public static String doLogin(String username, String password, Map<String, String> session) {
        String u = sanitize(username);
        String p = sanitize(password);
        if (u.isEmpty() || p.isEmpty()) {
            return "REDIRECT:/login?error=1";
        }
        boolean okUser = constantTimeEquals(u, EXPECTED_USER);
        boolean okPass = constantTimeEquals(p, EXPECTED_PASS);
        if (okUser && okPass) {
            session.put("username", u);
            return "REDIRECT:/secret";
        }
        return "REDIRECT:/login?error=1";
    }

    public static String secretPage(Map<String, String> session) {
        if (session != null && session.get("username") != null && !session.get("username").isEmpty()) {
            return "<html><body>Secret settings</body></html>";
        }
        return "REDIRECT:/login";
    }

    private static String sanitize(String s) {
        if (s == null) return "";
        StringBuilder sb = new StringBuilder();
        int max = Math.min(64, s.length());
        for (int i = 0, count = 0; i < s.length() && count < max; i++) {
            char c = s.charAt(i);
            if (c >= 32 && c <= 126) {
                sb.append(c);
                count++;
            }
        }
        return sb.toString().trim();
    }

    private static boolean constantTimeEquals(String a, String b) {
        byte[] x = a == null ? new byte[0] : a.getBytes(StandardCharsets.UTF_8);
        byte[] y = b == null ? new byte[0] : b.getBytes(StandardCharsets.UTF_8);
        int max = Math.max(x.length, y.length);
        int diff = x.length ^ y.length;
        for (int i = 0; i < max; i++) {
            byte xb = i < x.length ? x[i] : 0;
            byte yb = i < y.length ? y[i] : 0;
            diff |= (xb ^ yb);
        }
        return diff == 0;
    }

    public static void main(String[] args) {
        // Test case 1: Access index page
        Map<String, String> session1 = new HashMap<>();
        System.out.println("TC1 INDEX: " + indexPage(session1));

        // Test case 2: Access secret without login
        Map<String, String> session2 = new HashMap<>();
        System.out.println("TC2 SECRET (no login): " + secretPage(session2));

        // Test case 3: Wrong password
        Map<String, String> session3 = new HashMap<>();
        System.out.println("TC3 LOGIN wrong: " + doLogin("admin", "wrong", session3));
        System.out.println("TC3 SECRET after wrong: " + secretPage(session3));

        // Test case 4: Correct login then secret
        Map<String, String> session4 = new HashMap<>();
        System.out.println("TC4 LOGIN correct: " + doLogin("admin", "S3cureP@ss", session4));
        System.out.println("TC4 SECRET after correct: " + secretPage(session4));

        // Test case 5: Empty credentials
        Map<String, String> session5 = new HashMap<>();
        System.out.println("TC5 LOGIN empty: " + doLogin("", "", session5));
        System.out.println("TC5 SECRET after empty: " + secretPage(session5));
    }
}