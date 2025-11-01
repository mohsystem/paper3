import java.nio.charset.StandardCharsets;
import java.util.*;

public class Lse143 {

    // Simulated in-memory "database" and "mailing list"
    private static final Set<String> USER_DB = Collections.synchronizedSet(new HashSet<>());
    private static final Set<String> MAILING_LIST = Collections.synchronizedSet(new HashSet<>());

    // Allowed characters in email (ASCII only)
    private static boolean isAllowedEmailChar(char c) {
        return (c >= 'a' && c <= 'z') ||
               (c >= '0' && c <= '9') ||
               c == '.' || c == '_' || c == '%' || c == '+' || c == '-' || c == '@';
    }

    private static boolean isAlpha(char c) {
        return (c >= 'a' && c <= 'z');
    }

    private static boolean isAlnum(char c) {
        return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
    }

    private static String sanitizeEmail(String input) {
        if (input == null) return null;
        String s = input.trim().toLowerCase(Locale.ROOT);
        int len = s.length();
        if (len < 3 || len > 254) return null;

        for (int i = 0; i < len; i++) {
            char c = s.charAt(i);
            if (!isAllowedEmailChar(c)) {
                return null;
            }
        }

        int at = s.indexOf('@');
        if (at <= 0 || at >= len - 1) return null;
        if (s.indexOf('@', at + 1) != -1) return null; // multiple '@'

        if (s.contains("..")) return null;

        String local = s.substring(0, at);
        String domain = s.substring(at + 1);

        if (local.isEmpty() || domain.isEmpty()) return null;

        int lastDot = domain.lastIndexOf('.');
        if (lastDot <= 0 || lastDot >= domain.length() - 2) return null; // ensure at least one label and TLD len>=2

        String tld = domain.substring(lastDot + 1);
        if (tld.length() < 2 || tld.length() > 63) return null;
        for (int i = 0; i < tld.length(); i++) {
            if (!isAlpha(tld.charAt(i))) return null;
        }

        // Validate domain labels
        String[] labels = domain.split("\\.");
        for (String label : labels) {
            if (label.isEmpty()) return null;
            if (label.charAt(0) == '-' || label.charAt(label.length() - 1) == '-') return null;
            for (int i = 0; i < label.length(); i++) {
                char c = label.charAt(i);
                if (!(isAlnum(c) || c == '-')) return null;
            }
        }

        return s;
    }

    public static String unsubscribeGet(String emailParam) {
        String email = sanitizeEmail(emailParam);
        if (email == null) {
            return "Invalid email parameter.";
        }
        synchronized (USER_DB) {
            if (USER_DB.contains(email)) {
                USER_DB.remove(email);
                MAILING_LIST.remove(email);
                return "You have been unsubscribed.";
            } else {
                return "The provided email is not subscribed.";
            }
        }
    }

    private static void seed() {
        USER_DB.clear();
        MAILING_LIST.clear();
        String[] seeds = {
            "alice@example.com",
            "bob@example.org",
            "carol.test+news@sub.domain.com"
        };
        for (String e : seeds) {
            String se = sanitizeEmail(e);
            if (se != null) {
                USER_DB.add(se);
                MAILING_LIST.add(se);
            }
        }
    }

    public static void main(String[] args) {
        seed();
        // 5 test cases
        String[] tests = new String[] {
            "alice@example.com",             // existing -> unsubscribed
            " ALICE@EXAMPLE.COM ",           // now removed -> not subscribed
            "carol.test+news@sub.domain.com",// existing complex -> unsubscribed
            "dave@example.com",              // not subscribed
            "bad@@example..com"              // invalid
        };

        for (String t : tests) {
            String result = unsubscribeGet(t);
            System.out.println("Input: " + t + " -> " + result);
        }
    }
}