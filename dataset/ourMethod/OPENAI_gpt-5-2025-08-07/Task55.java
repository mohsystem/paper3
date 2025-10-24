import java.nio.charset.StandardCharsets;

public class Task55 {

    public static boolean isValidEmail(String email) {
        if (email == null) return false;

        // Length checks based on common standards
        int len = email.length();
        if (len < 3 || len > 254) return false;

        // No leading/trailing whitespace and must be ASCII
        if (!email.equals(email.trim())) return false;
        for (int i = 0; i < len; i++) {
            char c = email.charAt(i);
            if (c <= 0x1F || c >= 0x7F) return false; // control or non-ASCII
        }

        int at = email.indexOf('@');
        if (at <= 0 || at != email.lastIndexOf('@') || at == len - 1) return false;

        String local = email.substring(0, at);
        String domain = email.substring(at + 1);

        // Local part length
        if (local.length() < 1 || local.length() > 64) return false;

        // Validate local part characters and dot rules
        if (local.charAt(0) == '.' || local.charAt(local.length() - 1) == '.') return false;
        boolean prevDot = false;
        for (int i = 0; i < local.length(); i++) {
            char c = local.charAt(i);
            if (!isAllowedLocalChar(c)) return false;
            if (c == '.') {
                if (prevDot) return false; // consecutive dots
                prevDot = true;
            } else {
                prevDot = false;
            }
        }

        // Validate domain
        if (domain.length() < 1 || domain.length() > 253) return false;
        if (domain.charAt(0) == '.' || domain.charAt(domain.length() - 1) == '.') return false;

        boolean hasDot = false;
        int labelLen = 0;
        boolean lastLabelAlphaOnly = false; // will set when a label completes
        boolean currentLabelAlphaOnly = true;
        char prev = 0;

        for (int i = 0; i < domain.length(); i++) {
            char c = domain.charAt(i);

            if (c == '.') {
                // label boundary
                if (labelLen == 0) return false; // empty label or consecutive dots
                if (prev == '-') return false; // label cannot end with hyphen
                hasDot = true;
                lastLabelAlphaOnly = currentLabelAlphaOnly;
                // reset for next label
                labelLen = 0;
                currentLabelAlphaOnly = true;
                prev = c;
                continue;
            }

            if (!isAllowedDomainChar(c)) return false;

            if (c == '-') {
                if (labelLen == 0) return false; // label cannot start with hyphen
                currentLabelAlphaOnly = false;
            } else if (isAsciiDigit(c)) {
                currentLabelAlphaOnly = false;
            } else if (isAsciiLetter(c)) {
                // remains as is
            } else {
                return false;
            }

            labelLen++;
            if (labelLen > 63) return false; // label length limit
            prev = c;
        }

        // Final label checks
        if (labelLen == 0) return false; // domain cannot end with dot
        if (prev == '-') return false; // label cannot end with hyphen
        lastLabelAlphaOnly = currentLabelAlphaOnly;

        // Must have at least one dot and TLD must be all letters and length >= 2
        if (!hasDot) return false;
        if (!lastLabelAlphaOnly || labelLen < 2) return false;

        return true;
    }

    private static boolean isAsciiLetter(char c) {
        return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
    }

    private static boolean isAsciiDigit(char c) {
        return (c >= '0' && c <= '9');
    }

    private static boolean isAllowedLocalChar(char c) {
        // Allowed in local-part: A-Z a-z 0-9 and these specials: ! # $ % & ' * + - / = ? ^ _ ` { | } ~ .
        if (isAsciiLetter(c) || isAsciiDigit(c)) return true;
        switch (c) {
            case '!': case '#': case '$': case '%': case '&': case '\'': case '*':
            case '+': case '-': case '/': case '=': case '?': case '^': case '_':
            case '`': case '{': case '|': case '}': case '~': case '.':
                return true;
            default:
                return false;
        }
    }

    private static boolean isAllowedDomainChar(char c) {
        return isAsciiLetter(c) || isAsciiDigit(c) || c == '-' || c == '.';
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "alice@example.com",
            "john.doe+tag@sub.example.co.uk",
            ".startsWithDot@domain.com",
            "no_at_symbol.domain.com",
            "bad-domain@exa_mple.com"
        };
        for (String t : tests) {
            System.out.println(t + " => " + isValidEmail(t));
        }
    }
}