import java.util.*;

public class Task133 {
    public static String resetPassword(Map<String, String> users, String username, String oldPass, String newPass, String confirmPass) {
        if (username == null || oldPass == null || newPass == null || confirmPass == null) {
            return "ERROR: Invalid input.";
        }
        if (!users.containsKey(username)) {
            return "ERROR: User not found.";
        }
        String current = users.get(username);
        if (!Objects.equals(current, oldPass)) {
            return "ERROR: Old password is incorrect.";
        }
        if (!newPass.equals(confirmPass)) {
            return "ERROR: New password and confirmation do not match.";
        }
        if (newPass.equals(oldPass)) {
            return "ERROR: New password must be different from old password.";
        }
        if (newPass.length() < 8) {
            return "ERROR: Password must be at least 8 characters long.";
        }
        if (!hasUpper(newPass) || !hasLower(newPass) || !hasDigit(newPass) || !hasSpecial(newPass)) {
            return "ERROR: Password must include uppercase, lowercase, digit, and special character.";
        }
        if (containsIgnoreCase(newPass, username)) {
            return "ERROR: Password must not contain the username.";
        }
        users.put(username, newPass);
        return "SUCCESS: Password updated.";
    }

    private static boolean hasUpper(String s) {
        for (char c : s.toCharArray()) if (Character.isUpperCase(c)) return true;
        return false;
    }
    private static boolean hasLower(String s) {
        for (char c : s.toCharArray()) if (Character.isLowerCase(c)) return true;
        return false;
    }
    private static boolean hasDigit(String s) {
        for (char c : s.toCharArray()) if (Character.isDigit(c)) return true;
        return false;
    }
    private static boolean hasSpecial(String s) {
        for (char c : s.toCharArray()) if (!Character.isLetterOrDigit(c)) return true;
        return false;
    }
    private static boolean containsIgnoreCase(String hay, String needle) {
        return hay.toLowerCase().contains(needle.toLowerCase());
        }

    public static void main(String[] args) {
        Map<String, String> users = new HashMap<>();
        users.put("alice", "OldPass1!");
        users.put("bob", "Bob#2020");
        users.put("carol", "Car0l@123");

        // Test 1: Successful reset for alice
        System.out.println(resetPassword(users, "alice", "OldPass1!", "NewStrong1$", "NewStrong1$"));

        // Test 2: Wrong old password for bob
        System.out.println(resetPassword(users, "bob", "wrong", "BobNew#2024", "BobNew#2024"));

        // Test 3: Confirm mismatch for carol
        System.out.println(resetPassword(users, "carol", "Car0l@123", "Another1!", "Mismatch1!"));

        // Test 4: Unknown user dave
        System.out.println(resetPassword(users, "dave", "anything", "SomePass1!", "SomePass1!"));

        // Test 5: Weak password for alice after successful change (note old is updated)
        System.out.println(resetPassword(users, "alice", "NewStrong1$", "short", "short"));
    }
}