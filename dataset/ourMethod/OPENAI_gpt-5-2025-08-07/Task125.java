import java.util.*;

public class Task125 {

    private static final String GENERIC_ERROR = "ERROR: Access denied or resource not found.";
    private static final Set<String> ALLOWED_ROLES;
    private static final Map<String, Resource> RESOURCES;

    static {
        Set<String> roles = new HashSet<>(Arrays.asList("guest", "user", "admin"));
        ALLOWED_ROLES = Collections.unmodifiableSet(roles);

        Map<String, Resource> res = new HashMap<>();
        res.put("public", new Resource("Welcome to the public area.", new HashSet<>(Arrays.asList("guest", "user", "admin"))));
        res.put("dashboard", new Resource("Admin control panel.", new HashSet<>(Arrays.asList("admin"))));
        res.put("profile", new Resource("User profile page.", new HashSet<>(Arrays.asList("user", "admin"))));
        res.put("report2025", new Resource("Confidential report 2025.", new HashSet<>(Arrays.asList("user", "admin"))));
        res.put("help", new Resource("Help and FAQs.", new HashSet<>(Arrays.asList("guest", "user", "admin"))));
        RESOURCES = Collections.unmodifiableMap(res);
    }

    private static final class Resource {
        private final String content;
        private final Set<String> allowedRoles;

        private Resource(String content, Set<String> allowedRoles) {
            this.content = content;
            this.allowedRoles = Collections.unmodifiableSet(new HashSet<>(allowedRoles));
        }
    }

    public static String accessResource(String role, String resourceId) {
        if (!isValidRole(role) || !isValidResourceId(resourceId)) {
            return GENERIC_ERROR;
        }
        Resource r = RESOURCES.get(resourceId);
        if (r == null) {
            return GENERIC_ERROR;
        }
        if (!r.allowedRoles.contains(role)) {
            return GENERIC_ERROR;
        }
        return r.content;
    }

    private static boolean isValidRole(String role) {
        if (role == null) return false;
        return ALLOWED_ROLES.contains(role);
    }

    private static boolean isValidResourceId(String id) {
        if (id == null) return false;
        int len = id.length();
        if (len < 1 || len > 20) return false;
        for (int i = 0; i < len; i++) {
            char c = id.charAt(i);
            boolean ok = (c >= 'A' && c <= 'Z') ||
                         (c >= 'a' && c <= 'z') ||
                         (c >= '0' && c <= '9') ||
                         c == '_' || c == '-';
            if (!ok) return false;
        }
        return true;
    }

    public static void main(String[] args) {
        // 5 test cases
        String[] roles = {"admin", "guest", "user", "guest", "user"};
        String[] resources = {"dashboard", "public", "report2025", "report2025", "bad$id"};
        for (int i = 0; i < roles.length; i++) {
            String result = accessResource(roles[i], resources[i]);
            System.out.println("Test " + (i + 1) + " (" + roles[i] + ", " + resources[i] + "): " + result);
        }
    }
}