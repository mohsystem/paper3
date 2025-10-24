import java.util.*;
import java.nio.charset.StandardCharsets;

public class Task138 {
    // Manager for users, roles, and permissions
    public static final class AccessManager {
        private static final int MAX_NAME_LEN = 64;
        private static final int MAX_USERS = 10000;
        private static final int MAX_ROLES = 10000;
        private static final int MAX_PERMS_PER_ROLE = 1000;
        private static final int MAX_ROLES_PER_USER = 1000;
        private static final String NAME_PATTERN = "^[A-Za-z0-9_-]{1,64}$";

        private final Map<String, Set<String>> roleToPerms = new HashMap<>();
        private final Map<String, Set<String>> userToRoles = new HashMap<>();

        private boolean isValidName(String s) {
            if (s == null) return false;
            if (s.length() < 1 || s.length() > MAX_NAME_LEN) return false;
            return s.matches(NAME_PATTERN);
        }

        public boolean createRole(String roleName) {
            if (!isValidName(roleName)) return false;
            if (roleToPerms.size() >= MAX_ROLES) return false;
            return roleToPerms.putIfAbsent(roleName, new HashSet<>()) == null;
        }

        public boolean removeRole(String roleName) {
            if (!isValidName(roleName)) return false;
            if (!roleToPerms.containsKey(roleName)) return false;
            roleToPerms.remove(roleName);
            // remove from all users
            for (Set<String> roles : userToRoles.values()) {
                roles.remove(roleName);
            }
            return true;
        }

        public boolean addPermissionToRole(String roleName, String permission) {
            if (!isValidName(roleName) || !isValidName(permission)) return false;
            Set<String> perms = roleToPerms.get(roleName);
            if (perms == null) return false;
            if (perms.size() >= MAX_PERMS_PER_ROLE) return false;
            return perms.add(permission);
        }

        public boolean removePermissionFromRole(String roleName, String permission) {
            if (!isValidName(roleName) || !isValidName(permission)) return false;
            Set<String> perms = roleToPerms.get(roleName);
            if (perms == null) return false;
            return perms.remove(permission);
        }

        public boolean createUser(String username) {
            if (!isValidName(username)) return false;
            if (userToRoles.size() >= MAX_USERS) return false;
            return userToRoles.putIfAbsent(username, new HashSet<>()) == null;
        }

        public boolean deleteUser(String username) {
            if (!isValidName(username)) return false;
            return userToRoles.remove(username) != null;
        }

        public boolean assignRoleToUser(String username, String roleName) {
            if (!isValidName(username) || !isValidName(roleName)) return false;
            if (!roleToPerms.containsKey(roleName)) return false;
            Set<String> roles = userToRoles.get(username);
            if (roles == null) return false;
            if (roles.size() >= MAX_ROLES_PER_USER) return false;
            return roles.add(roleName);
        }

        public boolean revokeRoleFromUser(String username, String roleName) {
            if (!isValidName(username) || !isValidName(roleName)) return false;
            Set<String> roles = userToRoles.get(username);
            if (roles == null) return false;
            return roles.remove(roleName);
        }

        public boolean userHasPermission(String username, String permission) {
            if (!isValidName(username) || !isValidName(permission)) return false;
            Set<String> roles = userToRoles.get(username);
            if (roles == null) return false;
            for (String role : roles) {
                Set<String> perms = roleToPerms.get(role);
                if (perms != null && perms.contains(permission)) {
                    return true;
                }
            }
            return false;
        }

        public List<String> listUserPermissions(String username) {
            if (!isValidName(username)) return Collections.emptyList();
            Set<String> roles = userToRoles.get(username);
            if (roles == null) return Collections.emptyList();
            Set<String> result = new HashSet<>();
            for (String role : roles) {
                Set<String> perms = roleToPerms.get(role);
                if (perms != null) {
                    result.addAll(perms);
                }
            }
            List<String> out = new ArrayList<>(result);
            Collections.sort(out);
            return out;
        }

        public List<String> listUserRoles(String username) {
            if (!isValidName(username)) return Collections.emptyList();
            Set<String> roles = userToRoles.get(username);
            if (roles == null) return Collections.emptyList();
            List<String> out = new ArrayList<>(roles);
            Collections.sort(out);
            return out;
        }
    }

    public static void main(String[] args) {
        AccessManager mgr = new AccessManager();

        // Test 1: Create roles, permissions, users, assignments, checks
        System.out.println("Test1-CreateRoles: " + (mgr.createRole("admin") && mgr.createRole("editor")));
        System.out.println("Test1-AddPermsAdmin: " + (mgr.addPermissionToRole("admin", "read_all")
                && mgr.addPermissionToRole("admin", "write_all")
                && mgr.addPermissionToRole("admin", "manage_users")));
        System.out.println("Test1-AddPermsEditor: " + (mgr.addPermissionToRole("editor", "read_articles")
                && mgr.addPermissionToRole("editor", "edit_articles")));
        System.out.println("Test1-CreateUsers: " + (mgr.createUser("alice") && mgr.createUser("bob")));
        System.out.println("Test1-AssignRoles: " + (mgr.assignRoleToUser("alice", "admin")
                && mgr.assignRoleToUser("bob", "editor")));
        System.out.println("Test1-CheckPerms: " + (mgr.userHasPermission("alice", "manage_users") && !mgr.userHasPermission("bob", "manage_users")));

        // Test 2: Invalid names should fail
        System.out.println("Test2-InvalidRoleName: " + (!mgr.createRole("invalid role")));
        System.out.println("Test2-InvalidUserName: " + (!mgr.createUser("")));
        System.out.println("Test2-InvalidPermToRole: " + (!mgr.addPermissionToRole("admin", "bad perm")));

        // Test 3: Duplicates should fail gracefully
        System.out.println("Test3-DuplicateRole: " + (!mgr.createRole("admin")));
        System.out.println("Test3-DuplicateUser: " + (!mgr.createUser("alice")));
        System.out.println("Test3-DuplicatePermission: " + (!mgr.addPermissionToRole("admin", "read_all")));

        // Test 4: Revoke role and verify permissions update
        System.out.println("Test4-RevokeRole: " + (mgr.revokeRoleFromUser("alice", "admin")));
        System.out.println("Test4-CheckPermAfterRevoke: " + (!mgr.userHasPermission("alice", "read_all")));

        // Test 5: Remove permission from role and verify
        System.out.println("Test5-AddRoleBack: " + (mgr.assignRoleToUser("alice", "admin")));
        System.out.println("Test5-RemovePermFromRole: " + (mgr.removePermissionFromRole("admin", "write_all")));
        System.out.println("Test5-CheckRemovedPerm: " + (!mgr.userHasPermission("alice", "write_all")));

        // Display permissions for users
        System.out.println("AlicePerms: " + mgr.listUserPermissions("alice"));
        System.out.println("BobPerms: " + mgr.listUserPermissions("bob"));
    }
}