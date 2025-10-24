import java.util.*;

public class Task125 {
    private static final Map<String, Integer> ROLE_LEVEL = new HashMap<>();
    private static final Map<String, Integer> RESOURCE_REQUIRED_LEVEL = new HashMap<>();
    private static final Map<String, String> RESOURCE_CONTENT = new HashMap<>();

    static {
        // Role hierarchy
        ROLE_LEVEL.put("guest", 0);
        ROLE_LEVEL.put("user", 1);
        ROLE_LEVEL.put("editor", 2);
        ROLE_LEVEL.put("admin", 3);

        // Resources and required role levels
        RESOURCE_REQUIRED_LEVEL.put("public:news", 0);
        RESOURCE_REQUIRED_LEVEL.put("content:view", 1);
        RESOURCE_REQUIRED_LEVEL.put("user:profile", 1);
        RESOURCE_REQUIRED_LEVEL.put("content:edit", 2);
        RESOURCE_REQUIRED_LEVEL.put("admin:dashboard", 3);
        RESOURCE_REQUIRED_LEVEL.put("reports:financial", 3);

        // Resource contents
        RESOURCE_CONTENT.put("public:news", "Top headlines for everyone");
        RESOURCE_CONTENT.put("content:view", "Catalog of articles");
        RESOURCE_CONTENT.put("user:profile", "Personal profile details");
        RESOURCE_CONTENT.put("content:edit", "Editor panel for articles");
        RESOURCE_CONTENT.put("admin:dashboard", "System metrics and controls");
        RESOURCE_CONTENT.put("reports:financial", "Quarterly financial report");
    }

    public static String accessResource(String user, String role, String resourceId) {
        if (role == null || resourceId == null) {
            return "ACCESS DENIED: Invalid input";
        }
        Integer userLevel = ROLE_LEVEL.get(role.toLowerCase());
        if (userLevel == null) {
            return "ACCESS DENIED: Unknown role";
        }
        Integer requiredLevel = RESOURCE_REQUIRED_LEVEL.get(resourceId);
        if (requiredLevel == null) {
            return "ACCESS DENIED: Unknown resource";
        }
        if (userLevel >= requiredLevel) {
            String content = RESOURCE_CONTENT.getOrDefault(resourceId, "(no content)");
            return "ACCESS GRANTED: " + resourceId + " -> " + content;
        } else {
            return "ACCESS DENIED: Insufficient permissions";
        }
    }

    public static void main(String[] args) {
        String[][] tests = new String[][]{
            {"alice", "guest", "public:news"},
            {"bob", "user", "content:view"},
            {"cindy", "user", "admin:dashboard"},
            {"dave", "editor", "content:edit"},
            {"erica", "guest", "user:profile"}
        };
        for (String[] t : tests) {
            System.out.println(accessResource(t[0], t[1], t[2]));
        }
    }
}