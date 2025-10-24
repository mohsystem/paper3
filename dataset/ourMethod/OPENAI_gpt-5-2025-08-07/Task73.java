import java.util.*;

public class Task73 {

    // Validate if a given password is valid for an admin user
    public static boolean isValidAdminPassword(String password) {
        if (password == null) {
            return false;
        }
        final int minLen = 12;
        final int maxLen = 128;

        int length = password.length();
        if (length < minLen || length > maxLen) {
            return false;
        }

        // Ensure only printable ASCII (no spaces, no control chars)
        for (int i = 0; i < length; i++) {
            int code = password.charAt(i);
            if (code < 33 || code > 126) {
                return false;
            }
        }

        // Category checks
        boolean hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
        int repeatRun = 1;
        for (int i = 0; i < length; i++) {
            char ch = password.charAt(i);
            if (Character.isUpperCase(ch)) hasUpper = true;
            else if (Character.isLowerCase(ch)) hasLower = true;
            else if (Character.isDigit(ch)) hasDigit = true;
            else hasSpecial = true;

            if (i > 0) {
                if (password.charAt(i) == password.charAt(i - 1)) {
                    repeatRun++;
                    if (repeatRun >= 3) return false; // Disallow 3+ identical in a row
                } else {
                    repeatRun = 1;
                }
            }
        }
        if (!(hasUpper && hasLower && hasDigit && hasSpecial)) {
            return false;
        }

        // Disallow obvious/banned substrings and common passwords
        String lower = password.toLowerCase(Locale.ROOT);
        if (lower.contains("admin")) return false;
        if (lower.contains("password") || lower.contains("qwerty")) return false;

        Set<String> banned = new HashSet<>(Arrays.asList(
                "admin", "admin123", "admin@123", "administrator",
                "password", "passw0rd", "p@ssw0rd", "p4ssw0rd",
                "qwerty", "qwerty123", "letmein", "welcome",
                "123456", "123456789", "iloveyou", "12345678"
        ));
        if (banned.contains(lower)) return false;

        // Disallow sequential runs like abc, 123 or cba, 987
        if (hasSequentialRun(password)) return false;

        return true;
    }

    private static boolean hasSequentialRun(String s) {
        for (int i = 2; i < s.length(); i++) {
            char a = s.charAt(i - 2), b = s.charAt(i - 1), c = s.charAt(i);
            if (Character.isLetter(a) && Character.isLetter(b) && Character.isLetter(c)) {
                int x = Character.toLowerCase(a), y = Character.toLowerCase(b), z = Character.toLowerCase(c);
                if ((y == x + 1 && z == y + 1) || (y == x - 1 && z == y - 1)) return true;
            } else if (Character.isDigit(a) && Character.isDigit(b) && Character.isDigit(c)) {
                int x = a, y = b, z = c;
                if ((y == x + 1 && z == y + 1) || (y == x - 1 && z == y - 1)) return true;
            }
        }
        return false;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "Admin@123",       // invalid (too short, banned, contains admin)
            "S3cure#Admin!",   // invalid (contains admin)
            "G00d#Pa55word!",  // valid (complex, no banned exact match)
            "aaaaaaaaaaaa",    // invalid (repetition, lacks categories)
            "N0t$0Obv10us!"    // valid (complex)
        };
        for (int i = 0; i < tests.length; i++) {
            boolean res = isValidAdminPassword(tests[i]);
            System.out.println("Test " + (i + 1) + ": " + res);
        }
    }
}