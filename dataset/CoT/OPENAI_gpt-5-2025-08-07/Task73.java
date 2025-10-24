import java.util.*;

public class Task73 {
    // Step 1-5 combined into secure implementation

    // Core validation function
    public static boolean isValidAdminPassword(String pwd) {
        if (pwd == null) return false;

        int len = pwd.length();
        if (len < 12 || len > 128) return false;

        boolean hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;

        // Check requirements and weaknesses
        int repeatCount = 1;
        for (int i = 0; i < len; i++) {
            char c = pwd.charAt(i);

            if (Character.isWhitespace(c)) return false;

            if (Character.isUpperCase(c)) hasUpper = true;
            else if (Character.isLowerCase(c)) hasLower = true;
            else if (Character.isDigit(c)) hasDigit = true;

            // consider any non-alphanumeric and non-whitespace as special
            if (!Character.isLetterOrDigit(c) && !Character.isWhitespace(c)) hasSpecial = true;

            if (i > 0) {
                if (pwd.charAt(i) == pwd.charAt(i - 1)) {
                    repeatCount++;
                    if (repeatCount >= 3) return false; // no 3 identical chars in a row
                } else {
                    repeatCount = 1;
                }
            }
        }

        if (!(hasUpper && hasLower && hasDigit && hasSpecial)) return false;

        String lower = pwd.toLowerCase(Locale.ROOT);
        String[] banned = { "password", "admin", "qwerty", "letmein", "123456" };
        for (String b : banned) {
            if (lower.contains(b)) return false;
        }

        if (hasConsecutiveSequenceLetters(pwd) || hasConsecutiveSequenceDigits(pwd)) return false;

        return true;
    }

    private static boolean hasConsecutiveSequenceDigits(String s) {
        int run = 1;
        char prev = 0;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (!Character.isDigit(c)) {
                run = 1;
                prev = 0;
                continue;
            }
            if (prev != 0) {
                if (c == prev + 1 || c == prev - 1) {
                    run++;
                    if (run >= 4) return true;
                } else {
                    run = 1;
                }
            }
            prev = c;
        }
        return false;
    }

    private static boolean hasConsecutiveSequenceLetters(String s) {
        int run = 1;
        char prev = 0;
        for (int i = 0; i < s.length(); i++) {
            char raw = s.charAt(i);
            if (!Character.isLetter(raw)) {
                run = 1;
                prev = 0;
                continue;
            }
            char c = Character.toLowerCase(raw);
            if (prev != 0) {
                if (c == prev + 1 || c == prev - 1) {
                    run++;
                    if (run >= 4) return true;
                } else {
                    run = 1;
                }
            }
            prev = c;
        }
        return false;
    }

    // 5 test cases in main
    public static void main(String[] args) {
        String[] tests = {
            "password123!",
            "Admin123!",
            "ValidStrong#2024Xx",
            "NoSpecialChar1234",
            "AAAaaa111!!!"
        };
        for (int i = 0; i < tests.length; i++) {
            boolean res = isValidAdminPassword(tests[i]);
            System.out.println("Test " + (i + 1) + ": " + res);
        }
    }
}