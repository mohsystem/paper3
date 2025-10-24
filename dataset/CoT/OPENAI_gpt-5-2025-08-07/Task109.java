// Chain-of-Through process in code generation:
// 1) Problem understanding: validate inputs (email, phone, username, password, URL) using regular expressions.
// 2) Security requirements: avoid ReDoS by anchoring patterns, limiting input lengths, and using simple, bounded regexes.
// 3) Secure coding generation: implement null/length checks, compile and reuse regex patterns, and perform supplemental checks where regex engines differ.
// 4) Code review: patterns anchored, lengths bounded, no unsafe operations, clear returns, safe output.
// 5) Secure code output: final code with mitigations applied and 5 test cases in main.
import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Task109 {

    // Length limits to mitigate potential ReDoS or excessive processing
    private static final int MAX_EMAIL_LEN = 320;
    private static final int MAX_PHONE_LEN = 16;
    private static final int MIN_USERNAME_LEN = 2;
    private static final int MAX_USERNAME_LEN = 30;
    private static final int MIN_PASSWORD_LEN = 8;
    private static final int MAX_PASSWORD_LEN = 64;
    private static final int MAX_URL_LEN = 2083;

    // Compiled regex patterns (anchored, with bounded constructs)
    private static final Pattern EMAIL_PATTERN = Pattern.compile(
        "^[A-Za-z0-9](?:[A-Za-z0-9._%+-]{0,62}[A-Za-z0-9])?@(?:[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?\\.)+[A-Za-z]{2,63}$"
    );

    private static final Pattern PHONE_PATTERN = Pattern.compile(
        "^\\+?[1-9]\\d{1,14}$"
    );

    private static final Pattern USERNAME_PATTERN = Pattern.compile(
        "^[A-Za-z0-9](?:[A-Za-z0-9._-]{0,28}[A-Za-z0-9])?$"
    );

    private static final Pattern PWD_HAS_LOWER = Pattern.compile("[a-z]");
    private static final Pattern PWD_HAS_UPPER = Pattern.compile("[A-Z]");
    private static final Pattern PWD_HAS_DIGIT = Pattern.compile("\\d");
    private static final Pattern PWD_HAS_SPECIAL = Pattern.compile("[^A-Za-z0-9\\s]");
    private static final Pattern PWD_HAS_SPACE = Pattern.compile("\\s");

    private static final Pattern URL_PATTERN = Pattern.compile(
        "^https?://(?:[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?\\.)+[A-Za-z]{2,63}(?::\\d{2,5})?(?:/[A-Za-z0-9._~!$&'()*+,;=:@%/\\-?]*)?$"
    );

    private static boolean within(String s, int min, int max) {
        return s != null && s.length() >= min && s.length() <= max;
    }

    public static boolean validateEmail(String input) {
        if (!within(input, 3, MAX_EMAIL_LEN)) return false;
        if (!EMAIL_PATTERN.matcher(input).matches()) return false;
        int at = input.indexOf('@');
        if (at <= 0 || at == input.length() - 1) return false;
        String local = input.substring(0, at);
        String domain = input.substring(at + 1);
        if (local.contains("..")) return false;      // avoid consecutive dots in local-part
        if (domain.contains("..")) return false;     // avoid empty labels
        return true;
    }

    public static boolean validatePhoneE164(String input) {
        if (!within(input, 2, MAX_PHONE_LEN)) return false;
        return PHONE_PATTERN.matcher(input).matches();
    }

    public static boolean validateUsername(String input) {
        if (!within(input, MIN_USERNAME_LEN, MAX_USERNAME_LEN)) return false;
        if (!USERNAME_PATTERN.matcher(input).matches()) return false;
        // Disallow consecutive special characters among . _ -
        for (int i = 1; i < input.length(); i++) {
            char c1 = input.charAt(i - 1);
            char c2 = input.charAt(i);
            if ((c1 == '.' || c1 == '_' || c1 == '-') && (c2 == '.' || c2 == '_' || c2 == '-')) {
                return false;
            }
        }
        return true;
    }

    public static boolean validateStrongPassword(String input) {
        if (!within(input, MIN_PASSWORD_LEN, MAX_PASSWORD_LEN)) return false;
        if (PWD_HAS_SPACE.matcher(input).find()) return false;
        if (!PWD_HAS_LOWER.matcher(input).find()) return false;
        if (!PWD_HAS_UPPER.matcher(input).find()) return false;
        if (!PWD_HAS_DIGIT.matcher(input).find()) return false;
        if (!PWD_HAS_SPECIAL.matcher(input).find()) return false;
        return true;
    }

    public static boolean validateURL(String input) {
        if (!within(input, 8, MAX_URL_LEN)) return false; // minimal http://
        return URL_PATTERN.matcher(input).matches();
    }

    private static void runEmailTests() {
        String[] tests = {
            "user@example.com",
            "first.last+alias@sub.domain.co",
            ".user@domain.com",
            "user..name@domain.com",
            "user@-domain.com"
        };
        for (String t : tests) {
            System.out.println("Email: " + t + " -> " + validateEmail(t));
        }
    }

    private static void runPhoneTests() {
        String[] tests = {
            "+14155552671",
            "4155552671",
            "+001234",
            "++123",
            "+1234567890123456"
        };
        for (String t : tests) {
            System.out.println("Phone: " + t + " -> " + validatePhoneE164(t));
        }
    }

    private static void runUsernameTests() {
        String[] tests = {
            "alice_01",
            "a",
            "user--name",
            "_user",
            "valid.user-name"
        };
        for (String t : tests) {
            System.out.println("Username: " + t + " -> " + validateUsername(t));
        }
    }

    private static void runPasswordTests() {
        String[] tests = {
            "Abcdef1!",
            "short1!",
            "NoDigits!",
            "noupper1!",
            "With Space1!"
        };
        for (String t : tests) {
            System.out.println("Password: " + t + " -> " + validateStrongPassword(t));
        }
    }

    private static void runUrlTests() {
        String[] tests = {
            "https://example.com",
            "http://sub.domain.co:8080/path/to/resource?x=1",
            "ftp://example.com",
            "http://-bad.com",
            "http://example"
        };
        for (String t : tests) {
            System.out.println("URL: " + t + " -> " + validateURL(t));
        }
    }

    public static void main(String[] args) {
        runEmailTests();
        runPhoneTests();
        runUsernameTests();
        runPasswordTests();
        runUrlTests();
    }
}