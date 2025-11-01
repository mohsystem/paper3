import java.util.*;
import java.util.regex.Pattern;
import java.nio.charset.StandardCharsets;

public class Lse142 {
    // Simple in-memory "database" simulation
    static final class Database implements AutoCloseable {
        private static final Database INSTANCE = new Database();
        private final Set<String> emails;

        private Database() {
            this.emails = new HashSet<>();
            // Preload with some emails (stored normalized/lowercase)
            Collections.addAll(this.emails,
                "user@example.com",
                "alice@test.com",
                "bob@example.org",
                "newsletter@company.com",
                "sales@shop.co"
            );
        }

        public static Database connect() {
            return INSTANCE; // Simulate connecting to a DB
        }

        public boolean contains(String emailLower) {
            return emails.contains(emailLower);
        }

        public boolean remove(String emailLower) {
            return emails.remove(emailLower);
        }

        @Override
        public void close() {
            // No-op for in-memory demo
        }
    }

    private static final int MAX_EMAIL_LEN = 254;
    private static final Pattern EMAIL_PATTERN = Pattern.compile(
        "^[A-Za-z0-9._%+-]{1,64}@[A-Za-z0-9.-]+\\.[A-Za-z]{2,63}$"
    );

    // Validate and normalize email: trim, length check, regex, lowercase
    private static Optional<String> sanitizeEmail(String input) {
        if (input == null) return Optional.empty();
        String trimmed = input.trim();
        // Enforce simple ASCII-only emails for this demo
        byte[] bytes = trimmed.getBytes(StandardCharsets.US_ASCII);
        String ascii = new String(bytes, StandardCharsets.US_ASCII);
        if (!trimmed.equals(ascii)) return Optional.empty(); // Non-ASCII rejected
        if (trimmed.isEmpty() || trimmed.length() > MAX_EMAIL_LEN) return Optional.empty();
        if (!EMAIL_PATTERN.matcher(trimmed).matches()) return Optional.empty();
        String lower = trimmed.toLowerCase(Locale.ROOT);
        return Optional.of(lower);
    }

    // Unsubscribe function: validates, checks/removes from DB, returns a minimal message
    public static String unsubscribe(String email) {
        Optional<String> sanitized = sanitizeEmail(email);
        if (sanitized.isEmpty()) {
            return "Invalid email address.";
        }
        String normalized = sanitized.get();
        try (Database db = Database.connect()) {
            boolean removed = db.remove(normalized);
            return removed ? "Unsubscribed successfully." : "Email address not found in database.";
        } catch (Exception e) {
            // Fail closed with minimal info
            return "An error occurred.";
        }
    }

    public static void main(String[] args) {
        // 5 test cases
        String[] tests = new String[] {
            "user@example.com",         // should unsubscribe
            "ALICE@test.com",           // should unsubscribe (case-insensitive)
            "nobody@nope.com",          // not found
            "bad@@domain..com",         // invalid
            "user@example.com"          // already unsubscribed -> not found
        };
        for (String t : tests) {
            System.out.println("Input: " + t + " -> " + unsubscribe(t));
        }
    }
}