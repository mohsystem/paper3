import java.util.*;

public class Task126 {
    private static final Map<String, String> ROUTES;
    static {
        Map<String, String> m = new HashMap<>();
        m.put("home", "/home");
        m.put("index", "/home");
        m.put("main", "/home");
        m.put("about", "/about");
        m.put("contact", "/contact");
        m.put("dashboard", "/dashboard");
        m.put("settings", "/settings");
        m.put("help", "/help");
        ROUTES = Collections.unmodifiableMap(m);
    }

    public static String redirectUser(String input) {
        final String DEFAULT_HOME = "/home";
        final String ERROR_PAGE = "/error";

        if (input == null) {
            return DEFAULT_HOME;
        }

        String s = input.trim();
        if (s.isEmpty() || s.length() > 100) {
            return ERROR_PAGE;
        }

        s = s.toLowerCase(Locale.ROOT);

        if (s.startsWith("http:") || s.startsWith("https:") || s.contains("://") || s.startsWith("//")) {
            return ERROR_PAGE;
        }

        for (int i = 0; i < s.length(); i++) {
            char ch = s.charAt(i);
            if (!((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') || ch == '-' || ch == '_')) {
                return ERROR_PAGE;
            }
        }

        return ROUTES.getOrDefault(s, ERROR_PAGE);
    }

    public static void main(String[] args) {
        String[] tests = new String[] { "home", "ABOUT", "Contact", "http://evil.com", "settings" };
        for (String t : tests) {
            System.out.println("Input: " + t + " -> " + redirectUser(t));
        }
    }
}