import java.util.*;

public class Task73 {
    public static boolean isValidAdminPassword(String pwd) {
        if (pwd == null || pwd.length() < 12) return false;

        boolean hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;

        char prev1 = 0, prev2 = 0;
        for (int i = 0; i < pwd.length(); i++) {
            char ch = pwd.charAt(i);
            if (Character.isWhitespace(ch)) return false;

            if (Character.isUpperCase(ch)) hasUpper = true;
            else if (Character.isLowerCase(ch)) hasLower = true;
            else if (Character.isDigit(ch)) hasDigit = true;
            else hasSpecial = true;

            if (i >= 2) {
                if (ch == prev1 && ch == prev2) return false; // 3 identical in a row
            }
            prev2 = prev1;
            prev1 = ch;
        }

        if (!(hasUpper && hasLower && hasDigit && hasSpecial)) return false;

        String lowerPwd = pwd.toLowerCase();
        String[] badEquals = {"password", "admin", "admin123", "123456", "qwerty", "letmein", "welcome", "passw0rd", "iloveyou"};
        for (String s : badEquals) {
            if (lowerPwd.equals(s)) return false;
        }
        String[] badContains = {"admin", "password", "qwerty", "12345"};
        for (String s : badContains) {
            if (lowerPwd.contains(s)) return false;
        }

        return true;
    }

    public static void main(String[] args) {
        String[] tests = {
            "Admin@123",
            "Str0ng!AdminPwd",
            "CorrectHorse9!",
            "aaaaaaaaaaaa",
            "N0_Spaces-Allowed?"
        };
        for (String t : tests) {
            System.out.println(t + " -> " + isValidAdminPassword(t));
        }
    }
}