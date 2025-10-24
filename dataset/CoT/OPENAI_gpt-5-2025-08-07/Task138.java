import java.util.*;
import java.util.regex.Pattern;

public class Task138 {
    // Minimal high-level comments to reflect secure development lifecycle
    // Steps applied: problem definition, security requirements, secure implementation, review, finalized output.

    private static final class SecurityUtil {
        private static final Pattern NAME_PATTERN = Pattern.compile("^[A-Za-z0-9_.:-]{1,64}$");
        static String normalize(String s) {
            if (s == null) throw new IllegalArgumentException("null not allowed");
            String v = s.trim().toLowerCase(Locale.ROOT);
            if (!NAME_PATTERN.matcher(v).matches()) {
                throw new IllegalArgumentException("Invalid name: " + s);
            }
            return v;
        }
        static Set<String> copySet(Set<String> src) {
            return Collections.unmodifiableSet(new HashSet<>(src));
        }
    }

    private static final class Role {
        private final String name;
        private final Set<String> permissions = new HashSet<>();
        Role(String name) { this.name = name; }
        boolean addPermission(String perm) { return permissions.add(perm); }
        boolean removePermission(String perm) { return permissions.remove(perm); }
        Set<String> getPermissions() { return SecurityUtil.copySet(permissions); }
        String getName() { return name; }
    }

    private static final class User {
        private final String username;
        private final Set<String> directPermissions = new HashSet<>();
        private final Set<String> roleNames = new HashSet<>();
        User(String username) { this.username = username; }
        boolean grantPermission(String perm) { return directPermissions.add(perm); }
        boolean revokePermission(String perm) { return directPermissions.remove(perm); }
        boolean assignRole(String roleName) { return roleNames.add(roleName); }
        boolean unassignRole(String roleName) { return roleNames.remove(roleName); }
        Set<String> getDirectPermissions() { return SecurityUtil.copySet(directPermissions); }
        Set<String> getRoleNames() { return SecurityUtil.copySet(roleNames); }
        String getUsername() { return username; }
    }

    public static final class PermissionManager {
        private final Map<String, Role> roles = new HashMap<>();
        private final Map<String, User> users = new HashMap<>();

        public synchronized boolean addRole(String roleNameRaw) {
            String roleName = SecurityUtil.normalize(roleNameRaw);
            if (roles.containsKey(roleName)) return false;
            roles.put(roleName, new Role(roleName));
            return true;
        }

        public synchronized boolean addUser(String usernameRaw) {
            String username = SecurityUtil.normalize(usernameRaw);
            if (users.containsKey(username)) return false;
            users.put(username, new User(username));
            return true;
        }

        public synchronized boolean addPermissionToRole(String roleNameRaw, String permissionRaw) {
            String roleName = SecurityUtil.normalize(roleNameRaw);
            String perm = SecurityUtil.normalize(permissionRaw);
            Role role = roles.get(roleName);
            if (role == null) return false;
            return role.addPermission(perm);
        }

        public synchronized boolean removePermissionFromRole(String roleNameRaw, String permissionRaw) {
            String roleName = SecurityUtil.normalize(roleNameRaw);
            String perm = SecurityUtil.normalize(permissionRaw);
            Role role = roles.get(roleName);
            if (role == null) return false;
            return role.removePermission(perm);
        }

        public synchronized boolean grantPermissionToUser(String usernameRaw, String permissionRaw) {
            String username = SecurityUtil.normalize(usernameRaw);
            String perm = SecurityUtil.normalize(permissionRaw);
            User user = users.get(username);
            if (user == null) return false;
            return user.grantPermission(perm);
        }

        public synchronized boolean revokePermissionFromUser(String usernameRaw, String permissionRaw) {
            String username = SecurityUtil.normalize(usernameRaw);
            String perm = SecurityUtil.normalize(permissionRaw);
            User user = users.get(username);
            if (user == null) return false;
            return user.revokePermission(perm);
        }

        public synchronized boolean assignRoleToUser(String usernameRaw, String roleNameRaw) {
            String username = SecurityUtil.normalize(usernameRaw);
            String roleName = SecurityUtil.normalize(roleNameRaw);
            User user = users.get(username);
            Role role = roles.get(roleName);
            if (user == null || role == null) return false;
            return user.assignRole(roleName);
        }

        public synchronized boolean unassignRoleFromUser(String usernameRaw, String roleNameRaw) {
            String username = SecurityUtil.normalize(usernameRaw);
            String roleName = SecurityUtil.normalize(roleNameRaw);
            User user = users.get(username);
            if (user == null) return false;
            return user.unassignRole(roleName);
        }

        public synchronized boolean userHasPermission(String usernameRaw, String permissionRaw) {
            String username = SecurityUtil.normalize(usernameRaw);
            String perm = SecurityUtil.normalize(permissionRaw);
            User user = users.get(username);
            if (user == null) return false;
            if (user.getDirectPermissions().contains(perm)) return true;
            for (String roleName : user.getRoleNames()) {
                Role role = roles.get(roleName);
                if (role != null && role.getPermissions().contains(perm)) return true;
            }
            return false;
        }

        public synchronized Set<String> getEffectivePermissions(String usernameRaw) {
            String username = SecurityUtil.normalize(usernameRaw);
            User user = users.get(username);
            if (user == null) return Collections.emptySet();
            Set<String> effective = new HashSet<>(user.getDirectPermissions());
            for (String roleName : user.getRoleNames()) {
                Role role = roles.get(roleName);
                if (role != null) effective.addAll(role.getPermissions());
            }
            return SecurityUtil.copySet(effective);
        }
    }

    public static void main(String[] args) {
        PermissionManager pm = new PermissionManager();

        // Test Case 1: Admin role and Alice
        pm.addRole("admin");
        pm.addPermissionToRole("admin", "read");
        pm.addPermissionToRole("admin", "write");
        pm.addPermissionToRole("admin", "delete");
        pm.addUser("alice");
        pm.assignRoleToUser("alice", "admin");
        System.out.println("TC1 alice has delete: " + pm.userHasPermission("alice", "delete"));

        // Test Case 2: Editor role, Bob with direct read
        pm.addRole("editor");
        pm.addPermissionToRole("editor", "write");
        pm.addUser("bob");
        pm.grantPermissionToUser("bob", "read");
        System.out.println("TC2 bob has write before role: " + pm.userHasPermission("bob", "write"));
        pm.assignRoleToUser("bob", "editor");
        System.out.println("TC2 bob has write after role: " + pm.userHasPermission("bob", "write"));

        // Test Case 3: Remove delete from admin, impacts alice
        pm.removePermissionFromRole("admin", "delete");
        System.out.println("TC3 alice has delete after removal from role: " + pm.userHasPermission("alice", "delete"));

        // Test Case 4: Grant direct delete to alice
        pm.grantPermissionToUser("alice", "delete");
        System.out.println("TC4 alice has delete after direct grant: " + pm.userHasPermission("alice", "delete"));

        // Test Case 5: Edge cases and effective permissions
        System.out.println("TC5 unknown user has read: " + pm.userHasPermission("charlie", "read"));
        System.out.println("TC5 adding duplicate role 'admin': " + pm.addRole("admin"));
        Set<String> alicePerms = pm.getEffectivePermissions("alice");
        List<String> sorted = new ArrayList<>(alicePerms);
        Collections.sort(sorted);
        System.out.println("TC5 alice effective perms: " + sorted);
    }
}