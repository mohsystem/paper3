public class Lse135 {
    public static String check_mod(String username, String password) {
        if (username == null || password == null) return "false";
        if (username.equals("admin") && password.equals("secret")) return "true";
        return "false";
    }

    public static void main(String[] args) {
        String[][] tests = {
            {"admin", "secret"},
            {"Admin", "secret"},
            {"admin", "Secret"},
            {"user", "secret"},
            {"admin", "secret123"}
        };
        for (int i = 0; i < tests.length; i++) {
            System.out.println("Test " + (i + 1) + ": " + check_mod(tests[i][0], tests[i][1]));
        }
    }
}