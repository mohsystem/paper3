import java.nio.charset.StandardCharsets;
import java.util.regex.Pattern;

public class Task109 {
    // Precompiled regex patterns
    private static final Pattern EMAIL_PATTERN = Pattern.compile("^[A-Za-z0-9._%+-]{1,64}@[A-Za-z0-9.-]{1,253}\\.[A-Za-z]{2,63}$");
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z][A-Za-z0-9_]{2,19}$");
    private static final Pattern PHONE_ALLOWED_PATTERN = Pattern.compile("^[+0-9() .-]{7,20}$");
    private static final Pattern DATE_ISO_PATTERN = Pattern.compile("^\\d{4}-(0[1-9]|1[0-2])-(0[1-9]|[12]\\d|3[01])$");
    private static final Pattern PASSWORD_ALLOWED_PATTERN = Pattern.compile("^[\\S]{8,64}$"); // no whitespace
    private static final Pattern PW_LOWER = Pattern.compile(".*[a-z].*");
    private static final Pattern PW_UPPER = Pattern.compile(".*[A-Z].*");
    private static final Pattern PW_DIGIT = Pattern.compile(".*\\d.*");
    private static final Pattern PW_SPECIAL = Pattern.compile(".*[^A-Za-z0-9].*");

    private static boolean withinLength(String s, int min, int max) {
        if (s == null) return false;
        int len = s.codePointCount(0, s.length());
        return len >= min && len <= max;
    }

    public static boolean isValidEmail(String s) {
        // RFC says max total length 254, local 64, domain 255; we apply a safe cap here.
        if (!withinLength(s, 3, 254)) return false;
        return EMAIL_PATTERN.matcher(s).matches();
    }

    public static boolean isValidUsername(String s) {
        if (!withinLength(s, 3, 20)) return false;
        return USERNAME_PATTERN.matcher(s).matches();
    }

    public static boolean isValidPhone(String s) {
        if (!withinLength(s, 7, 20)) return false;
        if (!PHONE_ALLOWED_PATTERN.matcher(s).matches()) return false;
        // Count digits and enforce 7..15 digits
        int digits = 0;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c >= '0' && c <= '9') digits++;
        }
        return digits >= 7 && digits <= 15;
    }

    public static boolean isValidISODate(String s) {
        if (!withinLength(s, 10, 10)) return false;
        return DATE_ISO_PATTERN.matcher(s).matches();
    }

    public static boolean isValidPassword(String s) {
        if (!withinLength(s, 8, 64)) return false;
        if (!PASSWORD_ALLOWED_PATTERN.matcher(s).matches()) return false;
        if (!PW_LOWER.matcher(s).matches()) return false;
        if (!PW_UPPER.matcher(s).matches()) return false;
        if (!PW_DIGIT.matcher(s).matches()) return false;
        if (!PW_SPECIAL.matcher(s).matches()) return false;
        return true;
    }

    public static void main(String[] args) {
        String[] emails = {
            "alice@example.com",
            "alice@@example.com",
            "user.name+tag@sub.domain.co",
            "no-at-symbol.domain",
            "this-is-long_but.valid-123@domain-info.org"
        };
        String[] usernames = {
            "Alice_01",
            "1bad",
            "ab",
            "Valid_User_20",
            "UserNameWithMaxLen20"
        };
        String[] phones = {
            "+1-202-555-0123",
            "12345",
            "(020) 7946 0958",
            "+44 20 7946 0958 12345",
            "202.555.0180"
        };
        String[] dates = {
            "2025-10-14",
            "2025-13-01",
            "2025-02-00",
            "1999-12-31",
            "abcd-ef-gh"
        };
        String[] passwords = {
            "Str0ng!Pwd",
            "weak",
            "NoSpecial123",
            "alllower!!1",
            "GoodPass#2025"
        };

        for (int i = 0; i < 5; i++) {
            System.out.println("Test case #" + (i + 1));
            System.out.println("  Email: " + emails[i] + " -> " + isValidEmail(emails[i]));
            System.out.println("  Username: " + usernames[i] + " -> " + isValidUsername(usernames[i]));
            System.out.println("  Phone: " + phones[i] + " -> " + isValidPhone(phones[i]));
            System.out.println("  Date: " + dates[i] + " -> " + isValidISODate(dates[i]));
            // Do not print password values for security
            System.out.println("  Password valid? -> " + isValidPassword(passwords[i]));
        }
    }
}