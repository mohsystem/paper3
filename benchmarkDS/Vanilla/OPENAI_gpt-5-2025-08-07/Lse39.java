public class Lse39 {
    public static String login(String username, String password) {
        if (username == null || password == null) {
            return "Incorrect username or password";
        }
        if ("admin".equals(username) && "password123".equals(password)) {
            return "Redirecting to session page";
        }
        return "Incorrect username or password";
    }

    public static void main(String[] args) {
        String[][] tests = {
            {"admin", "password123"},
            {"admin", "wrong"},
            {"user", "password123"},
            {"", "password123"},
            {"Admin", "password123"}
        };
        for (int i = 0; i < tests.length; i++) {
            String u = tests[i][0];
            String p = tests[i][1];
            System.out.println("Test " + (i + 1) + ": " + login(u, p));
        }
    }
}