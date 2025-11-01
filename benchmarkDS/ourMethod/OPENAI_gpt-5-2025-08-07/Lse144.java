import java.util.*;
import java.nio.charset.StandardCharsets;

public class Lse144 {

    public interface UnsubscribeScript {
        int run(String email);
    }

    // Validate email format securely
    public static boolean isValidEmail(String email) {
        if (email == null) return false;
        String trimmed = email.trim();
        int len = trimmed.length();
        if (len < 3 || len > 254) return false;

        int at = trimmed.indexOf('@');
        if (at <= 0 || at != trimmed.lastIndexOf('@') || at == len - 1) return false;

        String local = trimmed.substring(0, at);
        String domain = trimmed.substring(at + 1);

        if (local.length() == 0 || local.length() > 64) return false;
        // local part: allowed A-Za-z0-9._+-
        if (local.charAt(0) == '.' || local.charAt(local.length() - 1) == '.') return false;
        boolean prevDot = false;
        for (int i = 0; i < local.length(); i++) {
            char c = local.charAt(i);
            boolean ok = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
                         (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '+' || c == '-';
            if (!ok) return false;
            if (c == '.') {
                if (prevDot) return false;
                prevDot = true;
            } else {
                prevDot = false;
            }
        }

        // domain part must contain at least one dot
        if (!domain.contains(".")) return false;
        String[] labels = domain.split("\\.");
        if (labels.length < 2) return false;

        for (String label : labels) {
            if (label.isEmpty() || label.length() > 63) return false;
            if (label.charAt(0) == '-' || label.charAt(label.length() - 1) == '-') return false;
            for (int i = 0; i < label.length(); i++) {
                char c = label.charAt(i);
                boolean ok = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
                             (c >= '0' && c <= '9') || c == '-';
                if (!ok) return false;
            }
        }
        // TLD length at least 2
        String tld = labels[labels.length - 1];
        if (tld.length() < 2) return false;

        return true;
    }

    private static String normalizeEmail(String email) {
        return email.trim().toLowerCase(Locale.ROOT);
    }

    // Returns 1 if email exists and unsubscribe script returns 0, else 0
    public static int processUnsubscribe(String email, Set<String> database, UnsubscribeScript script) {
        if (email == null || database == null || script == null) return 0;
        if (!isValidEmail(email)) return 0;
        String norm = normalizeEmail(email);

        // Use a case-insensitive check by storing/looking up normalized emails
        if (!database.contains(norm)) {
            return 0;
        }
        int rc;
        try {
            rc = script.run(norm);
        } catch (Exception e) {
            // Fail closed on any exception
            return 0;
        }
        return rc == 0 ? 1 : 0;
    }

    // Example script: succeeds (returns 0) only for @example.com addresses
    public static class ExampleScript implements UnsubscribeScript {
        @Override
        public int run(String email) {
            return email.endsWith("@example.com") ? 0 : 2;
        }
    }

    public static void main(String[] args) {
        // Build normalized database
        Set<String> db = new HashSet<>();
        List<String> initial = Arrays.asList(
                "alice@example.com",
                "bob@example.com",
                "carol@block.example",
                "dave@example.org",
                "eve@example.net"
        );
        for (String e : initial) {
            if (isValidEmail(e)) {
                db.add(normalizeEmail(e));
            }
        }

        UnsubscribeScript script = new ExampleScript();

        String[] tests = new String[] {
                "alice@example.com",       // exists, script=0 -> 1
                "carol@block.example",     // exists, script!=0 -> 0
                "frank@example.com",       // not exists -> 0
                "bad@@example.com",        // invalid -> 0
                "dave@example.org"         // exists, script!=0 -> 0
        };

        for (int i = 0; i < tests.length; i++) {
            int result = processUnsubscribe(tests[i], db, script);
            System.out.println("Test " + (i + 1) + ": " + result);
        }
    }
}