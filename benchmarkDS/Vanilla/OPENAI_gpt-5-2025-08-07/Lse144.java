import java.util.*;

public class Lse144 {
    private static final Set<String> DATABASE = new HashSet<>(Arrays.asList(
        "alice@example.com",
        "bob_fail@example.com",
        "charlie@example.com"
    ));

    // Simulated external script: returns 0 on success, non-zero on failure
    private static int callUnsubscribeScript(String email) {
        if (email == null || email.isEmpty()) return 1; // failure
        if (email.contains("fail")) return 2; // simulate failure for certain emails
        return 0; // success
    }

    // Returns 1 if email exists and unsubscribe script succeeds (returns 0), else 0
    public static int unsubscribeIfExists(String email) {
        if (email == null) return 0;
        if (!DATABASE.contains(email)) return 0;
        int scriptResult = callUnsubscribeScript(email);
        return scriptResult == 0 ? 1 : 0;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "alice@example.com",      // exists, script success -> 1
            "missing@example.com",    // not exists -> 0
            "bob_fail@example.com",   // exists, script failure -> 0
            "",                       // empty -> 0
            "charlie@example.com"     // exists, script success -> 1
        };
        for (String t : tests) {
            System.out.println(unsubscribeIfExists(t));
        }
    }
}