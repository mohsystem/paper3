import java.util.*;

public class Task103 {

    public static boolean isStrongPassword(String password) {
        if (password == null) return false;
        if (password.length() < 8) return false;

        boolean hasLower = false;
        boolean hasUpper = false;
        boolean hasDigit = false;

        for (int i = 0; i < password.length(); i++) {
            char ch = password.charAt(i);
            if (!hasLower && Character.isLowerCase(ch)) hasLower = true;
            else if (!hasUpper && Character.isUpperCase(ch)) hasUpper = true;
            else if (!hasDigit && Character.isDigit(ch)) hasDigit = true;

            if (hasLower && hasUpper && hasDigit) break;
        }

        return hasLower && hasUpper && hasDigit;
    }

    public static void main(String[] args) {
        String[] tests = {
            "Password1",
            "password",
            "PASSWORD1",
            "Pass1",
            "StrongPass123"
        };

        for (int i = 0; i < tests.length; i++) {
            boolean result = isStrongPassword(tests[i]);
            System.out.println("Test case " + (i + 1) + ": " + result);
        }
    }
}