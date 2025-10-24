import java.util.*;

public class Task103 {
    public static boolean isStrongPassword(final String pwd) {
        if (pwd == null) {
            return false;
        }
        if (pwd.length() < 8) {
            return false;
        }
        boolean hasUpper = false;
        boolean hasLower = false;
        boolean hasDigit = false;

        for (int i = 0; i < pwd.length(); i++) {
            char c = pwd.charAt(i);
            if (Character.isUpperCase(c)) {
                hasUpper = true;
            } else if (Character.isLowerCase(c)) {
                hasLower = true;
            } else if (Character.isDigit(c)) {
                hasDigit = true;
            }
            if (hasUpper && hasLower && hasDigit) {
                break;
            }
        }
        return hasUpper && hasLower && hasDigit;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "Short7",
            "alllowercase1",
            "ALLUPPERCASE1",
            "NoDigitsHere",
            "Str0ngPass"
        };
        boolean[] expected = new boolean[] { false, false, false, false, true };

        for (int i = 0; i < tests.length; i++) {
            boolean result = isStrongPassword(tests[i]);
            System.out.println("Test " + (i + 1) + ": expected=" + expected[i] + ", got=" + result + ", verdict=" + (result == expected[i] ? "PASS" : "FAIL"));
        }
    }
}