public class Lse135 {
    public static String check_mod(String username, String password) {
        if (username == null || password == null) return "false";
        return ("admin".equals(username) && "secret".equals(password)) ? "true" : "false";
    }

    public static void main(String[] args) {
        String[][] tests = {
            {"admin", "secret"},
            {"admin", "wrong"},
            {"user", "secret"},
            {"admin", ""},
            {"Admin", "secret"}
        };
        for (String[] t : tests) {
            System.out.println(check_mod(t[0], t[1]));
        }
    }
}