import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.util.Arrays;

public class Lse54 {

    private static final String ADMIN_USERNAME = "admin";
    private static final String ADMIN_PASSWORD = "AdminPassword!234";

    public static String login() {
        // Return a minimal secure HTML form (no user-controlled data embedded)
        return "<!DOCTYPE html>"
                + "<html><head><meta charset=\"utf-8\"><title>Login</title></head>"
                + "<body>"
                + "<form action=\"/do_login\" method=\"post\" autocomplete=\"off\">"
                + "<label for=\"username\">Username:</label>"
                + "<input type=\"text\" id=\"username\" name=\"username\" required minlength=\"1\" maxlength=\"64\" />"
                + "<label for=\"password\">Password:</label>"
                + "<input type=\"password\" id=\"password\" name=\"password\" required minlength=\"1\" maxlength=\"128\" />"
                + "<button type=\"submit\">Login</button>"
                + "</form>"
                + "</body></html>";
    }

    private static boolean isValidUsername(String username) {
        if (username == null) return false;
        if (username.length() < 1 || username.length() > 64) return false;
        for (int i = 0; i < username.length(); i++) {
            char c = username.charAt(i);
            boolean ok = (c >= 'A' && c <= 'Z')
                    || (c >= 'a' && c <= 'z')
                    || (c >= '0' && c <= '9')
                    || c == '.' || c == '_' || c == '-';
            if (!ok) return false;
        }
        return true;
    }

    private static boolean isValidPassword(String password) {
        if (password == null) return false;
        int len = password.length();
        return len >= 1 && len <= 128;
    }

    private static boolean constantTimeEquals(String a, String b) {
        byte[] x = a == null ? new byte[0] : a.getBytes(StandardCharsets.UTF_8);
        byte[] y = b == null ? new byte[0] : b.getBytes(StandardCharsets.UTF_8);
        return MessageDigest.isEqual(x, y);
    }

    public static String do_login(String username, String password) {
        if (!isValidUsername(username) || !isValidPassword(password)) {
            return "400 Bad Request";
        }
        boolean isAdminUser = constantTimeEquals(username, ADMIN_USERNAME);
        boolean isAdminPass = constantTimeEquals(password, ADMIN_PASSWORD);
        if (isAdminUser && isAdminPass) {
            return "REDIRECT:/admin_page";
        }
        return "401 Unauthorized";
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println("TEST1 FORM:");
        System.out.println(login());

        System.out.println("TEST2 ADMIN CORRECT:");
        System.out.println(do_login("admin", "AdminPassword!234"));

        System.out.println("TEST3 ADMIN WRONG PASS:");
        System.out.println(do_login("admin", "wrong"));

        System.out.println("TEST4 NON-ADMIN:");
        System.out.println(do_login("user", "AdminPassword!234"));

        System.out.println("TEST5 BAD INPUT:");
        System.out.println(do_login("", ""));
    }
}