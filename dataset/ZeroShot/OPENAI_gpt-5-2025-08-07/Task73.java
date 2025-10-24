import java.util.*;

public class Task73 {
    private static final int MIN_LENGTH = 12;
    private static final int MAX_LENGTH = 128;
    private static final Set<String> COMMON_PASSWORDS = new HashSet<>(Arrays.asList(
        "password","123456","qwerty","letmein","welcome","admin","administrator","passw0rd",
        "iloveyou","monkey","dragon","football","abc123","123456789","12345678","111111",
        "123123","login","princess","admin123","adminadmin","root","root123"
    ));

    public static boolean isValidAdminPassword(String password) {
        if (password == null) return false;
        int len = password.length();
        if (len < MIN_LENGTH || len > MAX_LENGTH) return false;

        String lower = password.toLowerCase(Locale.ROOT);
        if (COMMON_PASSWORDS.contains(lower)) return false;
        if (lower.contains("admin") || lower.contains("administrator") || lower.contains("root")) return false;

        boolean hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
        for (int i = 0; i < len; i++) {
            char ch = password.charAt(i);
            if (Character.isWhitespace(ch)) return false;
            if (Character.isUpperCase(ch)) hasUpper = true;
            else if (Character.isLowerCase(ch)) hasLower = true;
            else if (Character.isDigit(ch)) hasDigit = true;
            else hasSpecial = true;
        }
        if (!(hasUpper && hasLower && hasDigit && hasSpecial)) return false;

        if (hasTripleRepeat(password)) return false;
        if (hasSequentialPattern(password)) return false;

        return true;
    }

    private static boolean hasTripleRepeat(String s) {
        int count = 1;
        for (int i = 1; i < s.length(); i++) {
            if (s.charAt(i) == s.charAt(i - 1)) {
                count++;
                if (count >= 3) return false; // triple repeat not allowed
            } else {
                count = 1;
            }
        }
        return true;
    }

    private static boolean hasSequentialPattern(String s) {
        String lower = s.toLowerCase(Locale.ROOT);
        int n = lower.length();
        for (int i = 0; i <= n - 4; i++) {
            // letters ascending/descending
            if (isAlpha(lower.charAt(i)) && isAlpha(lower.charAt(i+1)) && isAlpha(lower.charAt(i+2)) && isAlpha(lower.charAt(i+3))) {
                boolean asc = true, desc = true;
                for (int k = i; k < i + 3; k++) {
                    if (lower.charAt(k + 1) != (char)(lower.charAt(k) + 1)) asc = false;
                    if (lower.charAt(k + 1) != (char)(lower.charAt(k) - 1)) desc = false;
                }
                if (asc || desc) return true;
            }
            // digits ascending/descending
            if (Character.isDigit(lower.charAt(i)) && Character.isDigit(lower.charAt(i+1)) &&
                Character.isDigit(lower.charAt(i+2)) && Character.isDigit(lower.charAt(i+3))) {
                boolean asc = true, desc = true;
                for (int k = i; k < i + 3; k++) {
                    if (lower.charAt(k + 1) != (char)(lower.charAt(k) + 1)) asc = false;
                    if (lower.charAt(k + 1) != (char)(lower.charAt(k) - 1)) desc = false;
                }
                if (asc || desc) return true;
            }
        }
        return false;
    }

    private static boolean isAlpha(char c) {
        return (c >= 'a' && c <= 'z');
    }

    // For demonstration purposes: 5 test cases
    public static void main(String[] args) {
        String[] tests = new String[] {
            "password",          // false: common, too short
            "Admin123!",         // false: contains 'admin' and too short
            "StrongPassw0rd!",   // true: strong
            "Adm!nStrong#2024",  // true: strong (obfuscated admin not detected)
            "Abcd1234!xyZ"       // false: sequential letters/digits
        };
        for (String t : tests) {
            boolean ok = isValidAdminPassword(t);
            System.out.println(ok ? "true" : "false");
        }
    }
}