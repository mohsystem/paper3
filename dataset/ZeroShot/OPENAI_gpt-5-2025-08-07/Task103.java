import java.util.*;

public class Task103 {
    public static boolean isStrongPassword(String password) {
        if (password == null) return false;
        if (password.length() < 8) return false;

        boolean hasUpper = false;
        boolean hasLower = false;
        boolean hasDigit = false;

        for (int i = 0; i < password.length(); i++) {
            char ch = password.charAt(i);
            if (Character.isUpperCase(ch)) {
                hasUpper = true;
            } else if (Character.isLowerCase(ch)) {
                hasLower = true;
            } else if (Character.isDigit(ch)) {
                hasDigit = true;
            }
            if (hasUpper && hasLower && hasDigit) {
                return true;
            }
        }
        return hasUpper && hasLower && hasDigit;
    }

    public static void main(String[] args) {
        String[] tests = {"Password1", "password1", "PASSWORD1", "Passw1", "Secure123"};
        for (String t : tests) {
            System.out.println(t + " -> " + isStrongPassword(t));
        }
    }
}