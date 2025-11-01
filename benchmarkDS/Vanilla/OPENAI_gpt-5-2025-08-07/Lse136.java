public class Lse136 {
    public static String check_mod(String username, String password) {
        if (username == null || password == null) return "false";
        if (username.equals("moderator") && password.equals("mod123")) return "true";
        return "false";
    }

    public static void main(String[] args) {
        System.out.println(check_mod("moderator", "mod123"));  // true
        System.out.println(check_mod("moderator", "wrong"));   // false
        System.out.println(check_mod("user", "mod123"));       // false
        System.out.println(check_mod("moderator", ""));        // false
        System.out.println(check_mod("MODERATOR", "mod123"));  // false
    }
}