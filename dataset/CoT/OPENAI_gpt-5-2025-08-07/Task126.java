import java.util.HashMap;
import java.util.Locale;
import java.util.Map;

public class Task126 {
    // Step 1: Problem understanding and purpose defined via method signature and comments
    // Step 2: Security requirements: Use whitelist routing; validate and sanitize input
    // Step 3: Secure coding: implement sanitization and strict map lookup
    // Step 4: Code review: comments inline; avoiding insecure redirects by not using user-provided URLs
    // Step 5: Final secure output: only whitelisted routes are returned

    private static final Map<String, String> ROUTES = new HashMap<>();
    static {
        ROUTES.put("home", "/home");
        ROUTES.put("profile", "/user/profile");
        ROUTES.put("settings", "/settings");
        ROUTES.put("help", "/help");
        ROUTES.put("logout", "/logout");
    }

    private static String sanitize(String input) {
        if (input == null) return "";
        String s = input.trim().toLowerCase(Locale.ROOT);
        if (!s.matches("[a-z0-9_-]+")) {
            return "";
        }
        return s;
    }

    public static String getRedirectTarget(String userInput) {
        String key = sanitize(userInput);
        String target = ROUTES.get(key);
        return target != null ? target : "/error";
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
                "home",
                "HoMe",
                " profile ",
                "../admin",
                "unknown"
        };
        for (String t : tests) {
            System.out.println("Input: \"" + t + "\" -> Redirect: " + getRedirectTarget(t));
        }
    }
}