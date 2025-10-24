import java.util.*;
import java.util.concurrent.ConcurrentHashMap;

public class Task138 {
    // Permissions and Roles
    public enum Permission {
        READ, WRITE, DELETE, EXECUTE, ADMIN
    }

    public enum Role {
        GUEST, USER, MODERATOR, ADMINISTRATOR
    }

    private static final Map<Role, EnumSet<Permission>> ROLE_PERMISSIONS;
    static {
        Map<Role, EnumSet<Permission>> map = new EnumMap<>(Role.class);
        map.put(Role.GUEST, EnumSet.of(Permission.READ));
        map.put(Role.USER, EnumSet.of(Permission.READ, Permission.WRITE));
        map.put(Role.MODERATOR, EnumSet.of(Permission.READ, Permission.WRITE, Permission.DELETE));
        map.put(Role.ADMINISTRATOR, EnumSet.of(Permission.READ, Permission.WRITE, Permission.DELETE, Permission.EXECUTE, Permission.ADMIN));
        ROLE_PERMISSIONS = Collections.unmodifiableMap(map);
    }

    // User representation
    public static final class User {
        private final String username;
        private final EnumSet<Role> roles;
        private final EnumSet<Permission> directPermissions;

        private User(String username) {
            this.username = username;
            this.roles = EnumSet.noneOf(Role.class);
            this.directPermissions = EnumSet.noneOf(Permission.class);
        }

        public String getUsername() {
            return username;
        }

        // Defensive copies
        public Set<Role> getRoles() {
            return Collections.unmodifiableSet(EnumSet.copyOf(roles));
        }

        public Set<Permission> getDirectPermissions() {
            return Collections.unmodifiableSet(EnumSet.copyOf(directPermissions));
        }
    }

    // Permission Manager
    public static final class PermissionManager {
        private final Map<String, User> users = new ConcurrentHashMap<>();

        public static String sanitizeName(String input) {
            if (input == null) return "";
            String trimmed = input.trim();
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < trimmed.length(); i++) {
                char c = trimmed.charAt(i);
                if (Character.isLetterOrDigit(c) || c == '_' || c == '.' || c == '-') {
                    sb.append(c);
                }
                if (sb.length() >= 64) break; // limit length
            }
            return sb.toString();
        }

        public synchronized boolean createUser(String rawUsername) {
            String username = sanitizeName(rawUsername);
            if (username.isEmpty()) return false;
            if (users.containsKey(username)) return false;
            users.put(username, new User(username));
            return true;
        }

        public synchronized boolean deleteUser(String rawUsername) {
            String username = sanitizeName(rawUsername);
            return users.remove(username) != null;
        }

        public synchronized boolean assignRole(String rawUsername, Role role) {
            String username = sanitizeName(rawUsername);
            User user = users.get(username);
            if (user == null || role == null) return false;
            return user.roles.add(role);
        }

        public synchronized boolean revokeRole(String rawUsername, Role role) {
            String username = sanitizeName(rawUsername);
            User user = users.get(username);
            if (user == null || role == null) return false;
            return user.roles.remove(role);
        }

        public synchronized boolean grantPermission(String rawUsername, Permission perm) {
            String username = sanitizeName(rawUsername);
            User user = users.get(username);
            if (user == null || perm == null) return false;
            return user.directPermissions.add(perm);
        }

        public synchronized boolean revokePermission(String rawUsername, Permission perm) {
            String username = sanitizeName(rawUsername);
            User user = users.get(username);
            if (user == null || perm == null) return false;
            return user.directPermissions.remove(perm);
        }

        public synchronized boolean hasPermission(String rawUsername, Permission perm) {
            String username = sanitizeName(rawUsername);
            User user = users.get(username);
            if (user == null || perm == null) return false;
            if (user.directPermissions.contains(perm)) return true;
            for (Role r : user.roles) {
                EnumSet<Permission> pset = ROLE_PERMISSIONS.get(r);
                if (pset != null && pset.contains(perm)) return true;
            }
            return false;
        }

        public synchronized Set<Permission> listEffectivePermissions(String rawUsername) {
            String username = sanitizeName(rawUsername);
            User user = users.get(username);
            if (user == null) return Collections.emptySet();
            EnumSet<Permission> effective = EnumSet.noneOf(Permission.class);
            effective.addAll(user.directPermissions);
            for (Role r : user.roles) {
                EnumSet<Permission> pset = ROLE_PERMISSIONS.get(r);
                if (pset != null) effective.addAll(pset);
            }
            return Collections.unmodifiableSet(effective);
        }

        public synchronized Set<String> listUsers() {
            return Collections.unmodifiableSet(new HashSet<>(users.keySet()));
        }
    }

    // Demonstration with 5 test cases
    public static void main(String[] args) {
        PermissionManager pm = new PermissionManager();

        // Test 1: Create user and assign role USER; check READ true, DELETE false
        System.out.println("Test 1:");
        System.out.println(pm.createUser("alice"));
        System.out.println(pm.assignRole("alice", Role.USER));
        System.out.println(pm.hasPermission("alice", Permission.READ));
        System.out.println(pm.hasPermission("alice", Permission.DELETE));

        // Test 2: Grant direct WRITE and check
        System.out.println("Test 2:");
        System.out.println(pm.grantPermission("alice", Permission.WRITE));
        System.out.println(pm.hasPermission("alice", Permission.WRITE));
        System.out.println(pm.listEffectivePermissions("alice"));

        // Test 3: Admin role for bob
        System.out.println("Test 3:");
        System.out.println(pm.createUser("bob"));
        System.out.println(pm.assignRole("bob", Role.ADMINISTRATOR));
        System.out.println(pm.hasPermission("bob", Permission.ADMIN));
        System.out.println(pm.hasPermission("bob", Permission.DELETE));
        System.out.println(pm.listEffectivePermissions("bob"));

        // Test 4: Revoke permission
        System.out.println("Test 4:");
        System.out.println(pm.revokePermission("alice", Permission.WRITE));
        System.out.println(pm.hasPermission("alice", Permission.WRITE));
        System.out.println(pm.listEffectivePermissions("alice"));

        // Test 5: Sanitize and duplicate handling
        System.out.println("Test 5:");
        System.out.println(pm.createUser("  Eve!!  "));
        System.out.println(pm.createUser("Eve")); // duplicate after sanitize
        System.out.println(pm.listUsers());
    }
}