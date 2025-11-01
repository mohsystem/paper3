import java.util.*;

public class Lse54 {
    public static String login() {
        return "<!DOCTYPE html>\n"
             + "<html>\n"
             + "<head><title>Login</title></head>\n"
             + "<body>\n"
             + "  <h1>Login</h1>\n"
             + "  <form method=\"POST\" action=\"/do_login\">\n"
             + "    <label>Username: <input type=\"text\" name=\"username\"/></label><br/>\n"
             + "    <label>Password: <input type=\"password\" name=\"password\"/></label><br/>\n"
             + "    <button type=\"submit\">Login</button>\n"
             + "  </form>\n"
             + "</body>\n"
             + "</html>";
    }

    public static String do_login(String username, String password) {
        if ("admin".equals(username)) {
            return "REDIRECT:/admin_page";
        }
        return login();
    }

    public static String admin_page() {
        return "<!DOCTYPE html>\n"
             + "<html>\n"
             + "<head><title>Admin</title></head>\n"
             + "<body><h1>Welcome, Admin!</h1></body>\n"
             + "</html>";
    }

    public static void main(String[] args) {
        // Test 1: GET /login
        System.out.println("Test 1 - GET /login:");
        System.out.println(login());
        System.out.println("-----");

        // Test 2: POST /do_login with admin user
        System.out.println("Test 2 - POST /do_login (admin, 1234):");
        String res2 = do_login("admin", "1234");
        System.out.println(res2);
        if (res2.startsWith("REDIRECT:/admin_page")) {
            System.out.println(admin_page());
        }
        System.out.println("-----");

        // Test 3: POST /do_login with non-admin user
        System.out.println("Test 3 - POST /do_login (user, pass):");
        String res3 = do_login("user", "pass");
        System.out.println(res3);
        System.out.println("-----");

        // Test 4: POST /do_login with Admin (case-sensitive fail)
        System.out.println("Test 4 - POST /do_login (Admin, 1234):");
        String res4 = do_login("Admin", "1234");
        System.out.println(res4);
        System.out.println("-----");

        // Test 5: POST /do_login with admin and empty password
        System.out.println("Test 5 - POST /do_login (admin, ''):");
        String res5 = do_login("admin", "");
        System.out.println(res5);
        if (res5.startsWith("REDIRECT:/admin_page")) {
            System.out.println(admin_page());
        }
        System.out.println("-----");
    }
}