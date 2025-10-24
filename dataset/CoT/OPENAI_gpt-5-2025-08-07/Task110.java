import java.net.URI;
import java.net.URISyntaxException;
import java.util.Locale;
import java.util.Arrays;
import java.util.List;
import java.net.IDN;

public class Task110 {
    // Steps 1-5: Problem understanding, security requirements, secure generation, review, secure output (summarized)
    public static boolean isValidURL(String url) {
        if (url == null) return false;
        String s = url.trim();
        if (s.length() == 0 || s.length() > 2048) return false;
        if (!hasNoWhitespaceOrCtl(s)) return false;

        URI uri;
        try {
            uri = new URI(s);
        } catch (URISyntaxException e) {
            return false;
        }

        String scheme = uri.getScheme();
        if (scheme == null) return false;
        scheme = scheme.toLowerCase(Locale.ROOT);
        if (!scheme.equals("http") && !scheme.equals("https")) return false;

        if (uri.getRawUserInfo() != null) return false;

        String host = uri.getHost();
        if (host == null || host.isEmpty()) return false;

        // Validate port
        int port = uri.getPort();
        if (port != -1 && (port < 1 || port > 65535)) return false;

        // Validate host
        if (!isValidHost(host)) return false;

        return true;
    }

    private static boolean hasNoWhitespaceOrCtl(String s) {
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (Character.isISOControl(c) || Character.isWhitespace(c)) {
                return false;
            }
        }
        return true;
    }

    private static boolean isValidHost(String host) {
        if (host.equalsIgnoreCase("localhost")) return true;
        if (host.indexOf(':') >= 0) {
            return isValidIPv6(host);
        }
        if (isValidIPv4(host)) return true;
        return isValidDomain(host);
    }

    private static boolean isValidIPv4(String host) {
        String[] parts = host.split("\\.", -1);
        if (parts.length != 4) return false;
        for (String p : parts) {
            if (p.isEmpty() || p.length() > 3) return false;
            for (int i = 0; i < p.length(); i++) {
                if (!Character.isDigit(p.charAt(i))) return false;
            }
            try {
                int v = Integer.parseInt(p);
                if (v < 0 || v > 255) return false;
                if (p.length() > 1 && p.charAt(0) == '0') {
                    // leading zeros not allowed except single 0
                    return false;
                }
            } catch (NumberFormatException e) {
                return false;
            }
        }
        return true;
    }

    private static boolean isValidIPv6(String host) {
        // Basic safe check: only hex digits, colon, dot allowed; must contain at least one colon.
        if (!host.contains(":")) return false;
        for (int i = 0; i < host.length(); i++) {
            char c = host.charAt(i);
            boolean ok = (c >= '0' && c <= '9') ||
                         (c >= 'a' && c <= 'f') ||
                         (c >= 'A' && c <= 'F') ||
                         c == ':' || c == '.';
            if (!ok) return false;
        }
        // Very loose structural check: max 8 groups (excluding IPv4 tail), not starting/ending with multiple consecutive colons except ::
        // Count groups split by ':', allow empty for compression
        String[] groups = host.split(":", -1);
        if (groups.length < 2) return false;
        // Ensure no more than one compression '::'
        int compress = 0;
        for (int i = 0; i < host.length() - 1; i++) {
            if (host.charAt(i) == ':' && host.charAt(i + 1) == ':') compress++;
        }
        if (compress > 1) return false;
        return true;
    }

    private static boolean isValidDomain(String host) {
        String ascii;
        try {
            ascii = IDN.toASCII(host, IDN.USE_STD3_ASCII_RULES);
        } catch (IllegalArgumentException e) {
            return false;
        }
        if (ascii.isEmpty() || ascii.length() > 253) return false;
        String[] labels = ascii.split("\\.");
        for (String label : labels) {
            if (label.isEmpty() || label.length() > 63) return false;
            if (label.charAt(0) == '-' || label.charAt(label.length() - 1) == '-') return false;
            for (int i = 0; i < label.length(); i++) {
                char c = label.charAt(i);
                boolean ok = (c >= 'a' && c <= 'z') ||
                             (c >= 'A' && c <= 'Z') ||
                             (c >= '0' && c <= '9') ||
                             c == '-';
                if (!ok) return false;
            }
        }
        return true;
    }

    public static void main(String[] args) {
        List<String> tests = Arrays.asList(
            "https://example.com",
            "http://sub.example.co.uk:8080/path?query=1#frag",
            "ftp://example.com",
            "https://user:pass@example.com",
            "http://-bad.com"
        );
        for (String t : tests) {
            boolean v = isValidURL(t);
            System.out.println(t + " -> " + v);
        }
    }
}