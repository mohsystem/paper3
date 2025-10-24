import java.util.*;

public class Task138 {
    static class PermissionSystem {
        private final Map<String, Set<String>> userRoles = new HashMap<>();
        private final Map<String, Set<String>> rolePerms = new HashMap<>();
        private final Map<String, Set<String>> userPerms = new HashMap<>();

        private Set<String> getOrCreate(Map<String, Set<String>> map, String key) {
            return map.computeIfAbsent(key, k -> new HashSet<>());
        }

        public void addUser(String user) {
            getOrCreate(userRoles, user);
            getOrCreate(userPerms, user);
        }

        public void addRole(String role) {
            getOrCreate(rolePerms, role);
        }

        public void grantRoleToUser(String user, String role) {
            addUser(user);
            addRole(role);
            userRoles.get(user).add(role);
        }

        public void revokeRoleFromUser(String user, String role) {
            getOrCreate(userRoles, user).remove(role);
        }

        public void grantPermissionToRole(String role, String permission) {
            addRole(role);
            rolePerms.get(role).add(permission);
        }

        public void revokePermissionFromRole(String role, String permission) {
            getOrCreate(rolePerms, role).remove(permission);
        }

        public void grantPermissionToUser(String user, String permission) {
            addUser(user);
            userPerms.get(user).add(permission);
        }

        public void revokePermissionFromUser(String user, String permission) {
            getOrCreate(userPerms, user).remove(permission);
        }

        public boolean hasPermission(String user, String permission) {
            if (getOrCreate(userPerms, user).contains(permission)) return true;
            for (String role : getOrCreate(userRoles, user)) {
                if (getOrCreate(rolePerms, role).contains(permission)) return true;
            }
            return false;
        }

        public List<String> getUserPermissions(String user) {
            Set<String> perms = new HashSet<>(getOrCreate(userPerms, user));
            for (String role : getOrCreate(userRoles, user)) {
                perms.addAll(getOrCreate(rolePerms, role));
            }
            List<String> list = new ArrayList<>(perms);
            Collections.sort(list);
            return list;
        }
    }

    private static void printList(List<String> list) {
        System.out.println(list.toString());
    }

    public static void main(String[] args) {
        PermissionSystem ps = new PermissionSystem();

        // Test 1: Admin role and permissions for alice
        ps.addRole("admin");
        ps.grantPermissionToRole("admin", "read");
        ps.grantPermissionToRole("admin", "write");
        ps.grantPermissionToRole("admin", "delete");
        ps.addUser("alice");
        ps.grantRoleToUser("alice", "admin");
        System.out.println("Test1 alice read: " + ps.hasPermission("alice", "read"));
        System.out.println("Test1 alice write: " + ps.hasPermission("alice", "write"));
        System.out.println("Test1 alice delete: " + ps.hasPermission("alice", "delete"));

        // Test 2: Direct permission for bob
        ps.addUser("bob");
        ps.grantPermissionToUser("bob", "read");
        System.out.println("Test2 bob read: " + ps.hasPermission("bob", "read"));
        System.out.println("Test2 bob write: " + ps.hasPermission("bob", "write"));

        // Test 3: Multiple roles for charlie
        ps.addRole("editor");
        ps.grantPermissionToRole("editor", "read");
        ps.grantPermissionToRole("editor", "write");
        ps.addRole("viewer");
        ps.grantPermissionToRole("viewer", "read");
        ps.addUser("charlie");
        ps.grantRoleToUser("charlie", "editor");
        ps.grantRoleToUser("charlie", "viewer");
        System.out.println("Test3 charlie read: " + ps.hasPermission("charlie", "read"));
        System.out.println("Test3 charlie write: " + ps.hasPermission("charlie", "write"));

        // Test 4: Revocations for bob
        ps.grantRoleToUser("bob", "viewer");
        ps.revokePermissionFromUser("bob", "read");
        System.out.println("Test4 bob read via role: " + ps.hasPermission("bob", "read"));
        ps.revokeRoleFromUser("bob", "viewer");
        System.out.println("Test4 bob read after revoke role: " + ps.hasPermission("bob", "read"));

        // Test 5: List user permissions for dana
        ps.addUser("dana");
        ps.grantRoleToUser("dana", "editor");
        ps.grantPermissionToUser("dana", "export");
        List<String> danaPerms = ps.getUserPermissions("dana");
        System.out.print("Test5 dana perms: ");
        printList(danaPerms);
    }
}