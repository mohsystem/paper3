import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public final class Lse54 {

    // Simulate a GET /login endpoint: returns a simple HTML form
    public static String login() {
        // Basic HTML form that posts to /do_login
        return "<!DOCTYPE html><html><head><meta charset=\"utf-8\">" +
               "<title>Login</title></head><body>" +
               "<form method=\"POST\" action=\"/do_login\">" +
               "<label>Username: <input type=\"text\" name=\"username\" required></label><br>" +
               "<label>Password: <input type=\"password\" name=\"password\" required></label><br>" +
               "<button type=\"submit\">Login</button>" +
               "</form></body></html>";
    }

    // Simulate a POST /do_login endpoint
    // Returns "REDIRECT:/admin_page" if username is admin, otherwise empty string.
    public static String do_login(String username, char[] password) {
        try {
            if (username == null) username = "";
            // Constant-time compare to avoid timing leaks
            boolean isAdmin = constantTimeEquals(username.getBytes(StandardCharsets.UTF_8),
                                                 "admin".getBytes(StandardCharsets.UTF_8));
            return isAdmin ? "REDIRECT:/admin_page" : "";
        } finally {
            if (password != null) {
                Arrays.fill(password, '\0'); // clear sensitive data
            }
        }
    }

    private static boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a == null || b == null) return false;
        int len = Math.max(a.length, b.length);
        int result = 0;
        for (int i = 0; i < len; i++) {
            byte ba = (i < a.length) ? a[i] : 0;
            byte bb = (i < b.length) ? b[i] : 0;
            result |= (ba ^ bb);
        }
        return result == 0 && a.length == b.length;
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println("GET /login ->");
        System.out.println(login());
        System.out.println();

        System.out.println("POST /do_login admin/adminpass -> " + do_login("admin", "adminpass".toCharArray()));
        System.out.println("POST /do_login admin/wrong -> " + do_login("admin", "wrong".toCharArray()));
        System.out.println("POST /do_login user/adminpass -> " + do_login("user", "adminpass".toCharArray()));
        System.out.println("POST /do_login empty/empty -> " + do_login("", new char[0]));
        System.out.println("POST /do_login Admin(case)/pass -> " + do_login("Admin", "pass".toCharArray()));
    }
}