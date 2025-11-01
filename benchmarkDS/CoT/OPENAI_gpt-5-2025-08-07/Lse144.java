import java.util.*;

public class Lse144 {
    private static final Set<String> DATABASE = new HashSet<>(Arrays.asList(
        "alice@example.com",
        "bob@fail.com",
        "carol@example.com",
        "dave@example.com",
        "eve@example.org"
    ));

    private static int unsubscribeScript(String email) {
        int at = email.lastIndexOf('@');
        String domain = (at >= 0) ? email.substring(at + 1) : "";
        if ("fail.com".equalsIgnoreCase(domain)) {
            return 2; // simulate failure
        }
        return 0; // success
    }

    public static int checkAndUnsubscribe(String email) {
        if (email == null || !DATABASE.contains(email)) {
            return 0;
        }
        int rc = unsubscribeScript(email);
        return rc == 0 ? 1 : 0;
    }

    public static void main(String[] args) {
        String[] tests = {
            "alice@example.com",   // exists, script success -> 1
            "bob@fail.com",        // exists, script failure -> 0
            "notfound@example.com",// not in DB -> 0
            "carol@example.com",   // exists, script success -> 1
            "unknown@fail.com"     // not in DB -> 0
        };
        for (String email : tests) {
            int result = checkAndUnsubscribe(email);
            System.out.println(email + " -> " + result);
        }
    }
}