import java.util.*;
import java.util.regex.*;

public class Lse144 {

    private static final Pattern EMAIL_PATTERN = Pattern.compile("^[A-Z0-9._%+-]{1,64}@[A-Z0-9.-]+\\.[A-Z]{2,63}$", Pattern.CASE_INSENSITIVE);

    public static int processUnsubscribe(String email, Set<String> database) {
        if (!isValidEmail(email) || database == null) {
            return 0;
        }
        if (!database.contains(email)) {
            return 0;
        }
        int exitCode = runUnsubscribeScript(email);
        return (exitCode == 0) ? 1 : 0;
    }

    private static boolean isValidEmail(String email) {
        if (email == null || email.length() > 254) {
            return false;
        }
        return EMAIL_PATTERN.matcher(email).matches();
    }

    private static int runUnsubscribeScript(String email) {
        // Secure stub simulating a script execution.
        // In real use, prefer ProcessBuilder with explicit arguments (no shell) and proper path whitelisting.
        // Example (commented):
        // try {
        //     ProcessBuilder pb = new ProcessBuilder("/usr/local/bin/unsubscribe_script", email);
        //     pb.redirectErrorStream(true);
        //     Process proc = pb.start();
        //     int code = proc.waitFor();
        //     return code;
        // } catch (Exception e) {
        //     return 1; // Non-zero indicates failure
        // }
        String lower = email.toLowerCase(Locale.ROOT);
        if (lower.startsWith("error") || lower.contains("fail")) {
            return 2; // Simulate script failure
        }
        return 0; // Simulate success
    }

    public static void main(String[] args) {
        Set<String> db = new HashSet<>();
        db.add("alice@example.com");
        db.add("bob@sample.org");
        db.add("charlie@test.net");
        db.add("dora@domain.com");
        db.add("error@test.net");

        String[] tests = new String[] {
            "alice@example.com",     // exists, script OK -> 1
            "unknown@x.com",         // not exists -> 0
            "error@test.net",        // exists, script fails -> 0
            "invalid",               // invalid email -> 0
            "dora@domain.com"        // exists, script OK -> 1
        };

        for (String t : tests) {
            int result = processUnsubscribe(t, db);
            System.out.println("Email: " + t + " => " + result);
        }
    }
}