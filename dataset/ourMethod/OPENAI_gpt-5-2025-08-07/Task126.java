import java.util.HashMap;
import java.util.Locale;
import java.util.Map;

public class Task126 {
    private static final Map<String, String> ROUTES = new HashMap<>();
    static {
        ROUTES.put("home", "/home");
        ROUTES.put("about", "/about");
        ROUTES.put("contact", "/contact");
        ROUTES.put("help", "/help");
        ROUTES.put("dashboard", "/user/dashboard");
    }

    private static boolean isSafeUsername(String username) {
        if (username == null) return false;
        if (username.length() < 1 || username.length() > 30) return false;
        for (int i = 0; i < username.length(); i++) {
            char c = username.charAt(i);
            boolean ok = (c >= 'A' && c <= 'Z') ||
                         (c >= 'a' && c <= 'z') ||
                         (c >= '0' && c <= '9') ||
                         (c == '_');
            if (!ok) {
                return false;
            }
        }
        return true;
    }

    public static String resolveRedirect(String input) {
        if (input == null) return "/error?code=bad-request";
        String s = input.trim();
        if (s.isEmpty()) return "/error?code=bad-request";
        if (s.length() > 100) return "/error?code=too-long";

        int idx = s.indexOf(':');
        if (idx >= 0) {
            String head = s.substring(0, idx).trim().toLowerCase(Locale.ROOT);
            String tail = s.substring(idx + 1).trim();
            if ("profile".equals(head)) {
                if (!isSafeUsername(tail)) {
                    return "/error?code=invalid-username";
                }
                return "/user/profile/" + tail;
            }
            return "/error?code=unknown-page";
        } else {
            String key = s.toLowerCase(Locale.ROOT);
            String path = ROUTES.get(key);
            return path != null ? path : "/error?code=unknown-page";
        }
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "home",
            "Profile:Alice_123",
            "profile:bad/../hacker",
            "unknown",
            "dashboard"
        };
        for (String t : tests) {
            String out = resolveRedirect(t);
            System.out.println("Input: " + t + " -> Redirect: " + out);
        }
    }
}