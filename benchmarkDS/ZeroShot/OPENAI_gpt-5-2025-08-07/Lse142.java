import java.util.*;
import java.util.regex.Pattern;
import java.util.regex.Matcher;
import java.nio.charset.StandardCharsets;

public class Lse142 {
    // Simulated database of subscribed emails (normalized to lowercase, trimmed)
    private static final Set<String> DB = Collections.synchronizedSet(new HashSet<>());
    private static final int MAX_EMAIL_LENGTH = 254;
    private static final int MAX_LOCAL_LENGTH = 64;

    // A conservative email regex to avoid catastrophic backtracking
    private static final Pattern EMAIL_REGEX = Pattern.compile(
        "^[A-Z0-9._%+-]{1,64}@[A-Z0-9.-]{1,253}\\.[A-Z]{2,63}$",
        Pattern.CASE_INSENSITIVE
    );

    private static String normalize(String email) {
        if (email == null) return null;
        String trimmed = email.trim();
        // Avoid extremely long inputs
        if (trimmed.length() == 0 || trimmed.length() > MAX_EMAIL_LENGTH) return null;
        return trimmed.toLowerCase(Locale.ROOT);
    }

    private static boolean isValidEmail(String email) {
        if (email == null) return false;
        if (email.length() > MAX_EMAIL_LENGTH) return false;

        Matcher m = EMAIL_REGEX.matcher(email);
        if (!m.matches()) return false;

        int at = email.indexOf('@');
        if (at <= 0) return false;
        String local = email.substring(at + 1 > 0 ? 0 : 0, at);
        if (local.length() == 0 || local.length() > MAX_LOCAL_LENGTH) return false;

        // Additional simple domain checks
        String domain = email.substring(at + 1);
        if (domain.startsWith("-") || domain.endsWith("-")) return false;
        if (domain.contains("..")) return false;
        String[] labels = domain.split("\\.");
        for (String label : labels) {
            if (label.isEmpty()) return false;
            if (label.startsWith("-") || label.endsWith("-")) return false;
        }
        return true;
    }

    public static String unsubscribe(String emailInput) {
        try {
            String normalized = normalize(emailInput);
            if (normalized == null || !isValidEmail(normalized)) {
                return "Invalid email address.";
            }
            synchronized (DB) {
                if (DB.remove(normalized)) {
                    return "You have been unsubscribed.";
                } else {
                    return "Email not found in database.";
                }
            }
        } catch (Exception e) {
            // Avoid leaking internal errors
            return "An error occurred processing your request.";
        }
    }

    private static void seedDb() {
        // seed with normalized values
        String[] initial = new String[] {
            "a@example.com",
            "b@example.org",
            "c+news@domain.co",
            "user@sub.domain.com",
            "test@example.com"
        };
        DB.clear();
        for (String s : initial) {
            DB.add(s);
        }
    }

    public static void main(String[] args) {
        seedDb();
        String[] tests = new String[] {
            "a@example.com",
            "  TEST@example.com  ",
            "notfound@nowhere.net",
            "invalid-email",
            "C+News@Domain.Co"
        };
        for (String t : tests) {
            System.out.println("Input: " + t + " -> " + unsubscribe(t));
        }
    }
}