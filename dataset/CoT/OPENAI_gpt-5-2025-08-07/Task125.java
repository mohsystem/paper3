import java.util.*;
import java.util.regex.Pattern;

public class Task125 {
    // Chain-of-Through Step 1: Problem understanding
    // Purpose: Securely gate access to in-memory resources based on user inputs (userId, role, resourceId).

    // Chain-of-Through Step 2: Security requirements
    // - Validate all inputs (length and allowed characters).
    // - Normalize role case and whitelist roles.
    // - Do not touch filesystem or external resources.
    // - Avoid mutable shared state exposure.

    private static final Pattern TOKEN_PATTERN = Pattern.compile("^[A-Za-z0-9_-]{1,32}$");
    private static final Set<String> ROLES = Collections.unmodifiableSet(new HashSet<>(Arrays.asList("ADMIN", "USER", "GUEST")));

    private static final class Resource {
        final String id;
        final String content;
        final Set<String> allowedRoles;
        Resource(String id, String content, Set<String> allowedRoles) {
            this.id = id;
            this.content = content;
            this.allowedRoles = Collections.unmodifiableSet(new HashSet<>(allowedRoles));
        }
    }

    private static final Map<String, Resource> RESOURCES;
    static {
        Map<String, Resource> m = new HashMap<>();
        m.put("doc_public", new Resource("doc_public", "Public Document", setOf("ADMIN","USER","GUEST")));
        m.put("doc_user",   new Resource("doc_user",   "User Document",   setOf("ADMIN","USER")));
        m.put("doc_admin",  new Resource("doc_admin",  "Admin Secrets",   setOf("ADMIN")));
        m.put("img_banner", new Resource("img_banner", "Image Banner",    setOf("ADMIN","USER","GUEST")));
        m.put("report_q3",  new Resource("report_q3",  "Q3 Report",       setOf("ADMIN","USER")));
        RESOURCES = Collections.unmodifiableMap(m);
    }

    private static Set<String> setOf(String... vals) {
        return new HashSet<>(Arrays.asList(vals));
    }

    private static boolean validToken(String s) {
        return s != null && TOKEN_PATTERN.matcher(s).matches();
    }

    // Chain-of-Through Step 3: Secure coding generation
    // Main API: validate inputs, authorize, and return safe, normalized outputs.
    public static String accessResource(String userId, String role, String resourceId) {
        if (userId == null || role == null || resourceId == null) return "ERROR:INVALID_INPUT";
        if (!validToken(userId) || !validToken(resourceId)) return "ERROR:INVALID_INPUT";
        String normRole = role.trim().toUpperCase(Locale.ROOT);
        if (!ROLES.contains(normRole)) return "ERROR:INVALID_INPUT";

        Resource r = RESOURCES.get(resourceId);
        if (r == null) return "ERROR:NOT_FOUND";

        if (!r.allowedRoles.contains(normRole)) return "ERROR:ACCESS_DENIED";

        return "OK:" + r.id + ":" + r.content;
    }

    // Chain-of-Through Step 4: Code review
    // Reviewed input validation, constants immutability, and constrained outputs.

    // Chain-of-Through Step 5: Secure code output

    public static void main(String[] args) {
        // 5 test cases
        String[] users =  {"alice","bob","carol","",      "dave"};
        String[] roles =  {"user", "guest","ADMIN","user","user"};
        String[] resIds = {"doc_user","doc_admin","report_q3","doc_public","unknown"};

        for (int i = 0; i < users.length; i++) {
            String result = accessResource(users[i], roles[i], resIds[i]);
            System.out.println(result);
        }
    }
}