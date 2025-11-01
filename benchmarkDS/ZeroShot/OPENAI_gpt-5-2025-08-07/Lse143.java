import java.util.*;
import java.util.regex.Pattern;

public class Lse143 {

    private static final Pattern EMAIL_PATTERN = Pattern.compile("^[A-Za-z0-9._%+-]{1,64}@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");

    private static String normalizeEmail(String email) {
        if (email == null) return null;
        return email.trim().toLowerCase(Locale.ROOT);
    }

    private static boolean isValidEmail(String email) {
        if (email == null) return false;
        if (email.length() > 254) return false;
        return EMAIL_PATTERN.matcher(email).matches();
    }

    // Unsubscribe function: accepts input as parameters and returns output
    public static String unsubscribe(String email, Set<String> database, Set<String> mailingList) {
        String normalized = normalizeEmail(email);
        if (!isValidEmail(normalized)) {
            return "Invalid email address.";
        }
        if (database.contains(normalized)) {
            database.remove(normalized);
            // Unsubscribe from mailing list (idempotent remove)
            mailingList.remove(normalized);
            return "Successfully unsubscribed.";
        } else {
            return "Email not subscribed.";
        }
    }

    public static void main(String[] args) {
        Set<String> database = new HashSet<>();
        Set<String> mailingList = new HashSet<>();

        // Seed data (store normalized)
        String[] initialDb = {
            "alice@example.com",
            "bob@example.org",
            "upper@example.com",
            "carol+news@sub.example.co",
            "dave@example.net"
        };
        for (String e : initialDb) {
            database.add(normalizeEmail(e));
        }

        // Mailing list contains all except dave to simulate user not on mailing list
        String[] initialMl = {
            "alice@example.com",
            "bob@example.org",
            "upper@example.com",
            "carol+news@sub.example.co"
        };
        for (String e : initialMl) {
            mailingList.add(normalizeEmail(e));
        }

        // 5 test cases
        System.out.println(unsubscribe("Alice@Example.com", database, mailingList));      // expected: success
        System.out.println(unsubscribe("unknown@example.com", database, mailingList));    // expected: not subscribed
        System.out.println(unsubscribe("invalid-email", database, mailingList));          // expected: invalid
        System.out.println(unsubscribe("BOB@EXAMPLE.ORG", database, mailingList));        // expected: success
        System.out.println(unsubscribe("dave@example.net", database, mailingList));       // expected: success (even if not on mailing list)
    }
}