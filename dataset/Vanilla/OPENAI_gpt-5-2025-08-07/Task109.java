import java.util.regex.Pattern;

public class Task109 {
    private static final Pattern EMAIL = Pattern.compile("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");
    private static final Pattern PHONE = Pattern.compile("^\\+?[0-9]{1,3}?[-. ]?(\\([0-9]{3}\\)|[0-9]{3})[-. ]?[0-9]{3}[-. ]?[0-9]{4}$");
    private static final Pattern USERNAME = Pattern.compile("^[A-Za-z][A-Za-z0-9_]{2,15}$");

    public static boolean isValidEmail(String s) {
        return s != null && EMAIL.matcher(s).matches();
    }

    public static boolean isValidPhone(String s) {
        return s != null && PHONE.matcher(s).matches();
    }

    public static boolean isValidUsername(String s) {
        return s != null && USERNAME.matcher(s).matches();
    }

    public static void main(String[] args) {
        String[] emails = {
            "user@example.com",
            "foo.bar+tag@sub.domain.co",
            "bad@domain",
            "noatsymbol.com",
            "user@domain.c"
        };
        String[] phones = {
            "+1 (555) 123-4567",
            "555-123-4567",
            "5551234567",
            "123-45-6789",
            "++1 555 123 4567"
        };
        String[] usernames = {
            "Alice_123",
            "a",
            "1start",
            "Good_Name",
            "ThisUsernameIsWayTooLong123"
        };

        System.out.println("Email validations:");
        for (String e : emails) {
            System.out.println(e + " -> " + isValidEmail(e));
        }
        System.out.println("\nPhone validations:");
        for (String p : phones) {
            System.out.println(p + " -> " + isValidPhone(p));
        }
        System.out.println("\nUsername validations:");
        for (String u : usernames) {
            System.out.println(u + " -> " + isValidUsername(u));
        }
    }
}