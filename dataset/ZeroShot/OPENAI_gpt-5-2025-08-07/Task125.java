import java.util.*;
import java.util.regex.Pattern;

public final class Task125 {
    private static final Pattern SAFE = Pattern.compile("^[a-z0-9_-]{1,32}$");

    private static final Map<String, String> USER_ROLES = Map.of(
            "alice", "admin",
            "bob", "editor",
            "carol", "viewer",
            "dave", "viewer",
            "eve", "editor"
    );

    private static final Set<String> ADMIN_RES = Set.of("config", "reports", "dashboard", "users", "logs");
    private static final Set<String> EDITOR_RES = Set.of("reports", "dashboard");
    private static final Set<String> VIEWER_RES = Set.of("dashboard");

    private static final Map<String, Set<String>> ROLE_RESOURCES = Map.of(
            "admin", ADMIN_RES,
            "editor", EDITOR_RES,
            "viewer", VIEWER_RES
    );

    private static String canonicalize(String s) {
        if (s == null) return null;
        String v = s.toLowerCase(Locale.ROOT).trim();
        if (!SAFE.matcher(v).matches()) return null;
        return v;
    }

    public static List<String> getAccessibleResources(String username) {
        String u = canonicalize(username);
        if (u == null) return Collections.emptyList();
        String role = USER_ROLES.get(u);
        if (role == null) return Collections.emptyList();
        Set<String> res = ROLE_RESOURCES.get(role);
        if (res == null) return Collections.emptyList();
        return new ArrayList<>(res);
    }

    public static boolean canAccess(String username, String resource) {
        String u = canonicalize(username);
        String r = canonicalize(resource);
        if (u == null || r == null) return false;
        String role = USER_ROLES.get(u);
        if (role == null) return false;
        Set<String> res = ROLE_RESOURCES.get(role);
        return res != null && res.contains(r);
    }

    public static String requestAccess(String username, String resource) {
        if (canAccess(username, resource)) {
            return "GRANTED: " + username + " -> " + resource;
        }
        return "DENIED: " + username + " -> " + resource;
    }

    public static void main(String[] args) {
        // 5 test cases
        String[][] tests = new String[][]{
                {"alice", "logs"},
                {"bob", "config"},
                {"carol", "dashboard"},
                {"unknown", "reports"},
                {"eve", "reports"}
        };

        for (String[] t : tests) {
            String user = t[0];
            String res = t[1];
            System.out.println(requestAccess(user, res));
        }

        // Show accessible resources for sample users
        String[] sampleUsers = {"alice", "bob", "carol", "unknown", "eve"};
        for (String user : sampleUsers) {
            List<String> accessible = getAccessibleResources(user);
            System.out.println("Resources for " + user + ": " + accessible);
        }
    }
}