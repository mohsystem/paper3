import java.util.*;

public class Task126 {
    public static String redirect(String input) {
        if (input == null) return "/home";
        String key = input.trim().toLowerCase(Locale.ROOT);
        if (key.isEmpty()) return "/home";

        if (key.startsWith("profile")) return "/user/profile";

        switch (key) {
            case "home":
                return "/home";
            case "login":
                return "/auth/login";
            case "logout":
                return "/auth/logout";
            case "admin":
                return "/admin/dashboard";
            case "help":
                return "/support/help";
            case "settings":
                return "/user/settings";
            default:
                return "/error/404";
        }
    }

    public static void main(String[] args) {
        String[] tests = {
            "home",
            "LOGIN",
            "Admin ",
            "profile?id=42",
            "unknown"
        };
        for (String t : tests) {
            System.out.println(t + " -> " + redirect(t));
        }
    }
}