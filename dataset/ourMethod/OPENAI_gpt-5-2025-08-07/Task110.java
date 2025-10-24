import java.util.Locale;

public class Task110 {

    private static final int MAX_URL_LENGTH = 2048;

    public static boolean isValidURL(String s) {
        if (s == null) return false;
        if (s.length() == 0 || s.length() > MAX_URL_LENGTH) return false;

        // Disallow control and whitespace; allow visible ASCII only
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c < 0x21 || c > 0x7E) return false; // exclude space and control chars
        }

        int schemeSep = s.indexOf("://");
        if (schemeSep <= 0) return false;
        String scheme = s.substring(0, schemeSep).toLowerCase(Locale.ROOT);
        if (!scheme.equals("http") && !scheme.equals("https") && !scheme.equals("ftp")) return false;

        int pos = schemeSep + 3; // after ://
        int authorityEnd = indexOfFirst(s, pos, '/', '?', '#');
        if (authorityEnd == -1) authorityEnd = s.length();

        if (authorityEnd <= pos) return false;
        String authority = s.substring(pos, authorityEnd);
        if (authority.indexOf('@') >= 0) return false; // disallow userinfo

        int port = -1;
        String host;

        if (authority.startsWith("[")) {
            int close = authority.indexOf(']');
            if (close <= 1) return false;
            host = authority.substring(1, close);
            if (!isValidIPv6(host)) return false;
            String rest = authority.substring(close + 1);
            if (rest.length() > 0) {
                if (!rest.startsWith(":")) return false;
                String portStr = rest.substring(1);
                if (portStr.length() == 0 || portStr.length() > 5) return false;
                if (!isAllDigits(portStr)) return false;
                try {
                    port = Integer.parseInt(portStr);
                } catch (NumberFormatException e) {
                    return false;
                }
                if (port < 1 || port > 65535) return false;
            }
        } else {
            int colon = authority.lastIndexOf(':');
            if (colon >= 0) {
                host = authority.substring(0, colon);
                String portStr = authority.substring(colon + 1);
                if (portStr.length() == 0 || portStr.length() > 5) return false;
                if (!isAllDigits(portStr)) return false;
                try {
                    port = Integer.parseInt(portStr);
                } catch (NumberFormatException e) {
                    return false;
                }
                if (port < 1 || port > 65535) return false;
            } else {
                host = authority;
            }
            if (!isValidHost(host)) return false;
        }

        // Validate path, query, fragment
        int idx = authorityEnd;
        // Path
        int qPos = s.indexOf('?', idx);
        int fPos = s.indexOf('#', idx);
        int pathEnd = s.length();
        if (qPos != -1) pathEnd = Math.min(pathEnd, qPos);
        if (fPos != -1) pathEnd = Math.min(pathEnd, fPos);
        String path = s.substring(idx, pathEnd);
        if (!isValidPath(path)) return false;

        // Query
        if (qPos != -1) {
            int queryEnd = (fPos != -1) ? fPos : s.length();
            String query = s.substring(qPos + 1, queryEnd);
            if (!isValidQueryOrFragment(query)) return false;
        }

        // Fragment
        if (fPos != -1) {
            String frag = s.substring(fPos + 1);
            if (!isValidQueryOrFragment(frag)) return false;
        }

        return true;
    }

    private static int indexOfFirst(String s, int from, char a, char b, char c) {
        int ia = s.indexOf(a, from);
        int ib = s.indexOf(b, from);
        int ic = s.indexOf(c, from);
        int res = -1;
        if (ia != -1) res = ia;
        if (ib != -1) res = (res == -1) ? ib : Math.min(res, ib);
        if (ic != -1) res = (res == -1) ? ic : Math.min(res, ic);
        return res;
    }

    private static boolean isAllDigits(String s) {
        if (s.isEmpty()) return false;
        for (int i = 0; i < s.length(); i++) {
            if (!Character.isDigit(s.charAt(i))) return false;
        }
        return true;
    }

    private static boolean isValidHost(String host) {
        if (host == null || host.isEmpty()) return false;
        if (host.equalsIgnoreCase("localhost")) return true;
        if (isValidIPv4(host)) return true;

        // Domain name validation
        if (host.length() > 253) return false;
        String[] labels = host.split("\\.");
        if (labels.length < 2) return false;
        for (String label : labels) {
            if (!isValidDomainLabel(label)) return false;
        }
        String tld = labels[labels.length - 1];
        if (tld.length() < 2) return false;
        // TLD should be alphabetic or punycode xn--
        boolean alphaTld = true;
        for (int i = 0; i < tld.length(); i++) {
            char ch = tld.charAt(i);
            if (!(ch >= 'a' && ch <= 'z') && !(ch >= 'A' && ch <= 'Z') && !(ch == '-')) {
                alphaTld = false;
                break;
            }
        }
        if (!alphaTld && !tld.toLowerCase(Locale.ROOT).startsWith("xn--")) return false;
        return true;
    }

    private static boolean isValidDomainLabel(String label) {
        if (label == null || label.isEmpty()) return false;
        if (label.length() > 63) return false;
        char first = label.charAt(0);
        char last = label.charAt(label.length() - 1);
        if (!isAlphaNum(first) || !isAlphaNum(last)) return false;
        for (int i = 0; i < label.length(); i++) {
            char ch = label.charAt(i);
            if (isAlphaNum(ch) || ch == '-') continue;
            return false;
        }
        return true;
    }

    private static boolean isAlphaNum(char c) {
        return (c >= 'A' && c <= 'Z') ||
               (c >= 'a' && c <= 'z') ||
               (c >= '0' && c <= '9');
    }

    private static boolean isValidIPv4(String s) {
        String[] parts = s.split("\\.", -1);
        if (parts.length != 4) return false;
        for (String p : parts) {
            if (p.isEmpty() || p.length() > 3) return false;
            if (!isAllDigits(p)) return false;
            int val;
            try {
                val = Integer.parseInt(p);
            } catch (NumberFormatException e) {
                return false;
            }
            if (val < 0 || val > 255) return false;
        }
        return true;
    }

    // Simplified but robust IPv6 validation without IPv4-mapped forms
    private static boolean isValidIPv6(String s) {
        if (s == null || s.isEmpty()) return false;
        if (s.chars().filter(ch -> ch == ':').count() < 2) return false;

        int ddc = s.indexOf("::");
        boolean hasDD = ddc != -1;
        if (hasDD && s.indexOf("::", ddc + 2) != -1) return false; // only one '::'

        String left = hasDD ? s.substring(0, ddc) : s;
        String right = hasDD ? s.substring(ddc + 2) : "";

        String[] leftParts = left.isEmpty() ? new String[0] : left.split(":", -1);
        String[] rightParts = right.isEmpty() ? new String[0] : right.split(":", -1);

        if (!validateIPv6Parts(leftParts)) return false;
        if (!validateIPv6Parts(rightParts)) return false;

        int partsCount = leftParts.length + rightParts.length;
        if (hasDD) {
            return partsCount < 8; // '::' compresses at least one group
        } else {
            return partsCount == 8;
        }
    }

    private static boolean validateIPv6Parts(String[] parts) {
        for (String part : parts) {
            if (part.isEmpty()) return false; // empty only allowed around '::'
            if (part.length() < 1 || part.length() > 4) return false;
            for (int i = 0; i < part.length(); i++) {
                char ch = part.charAt(i);
                if (!isHex(ch)) return false;
            }
        }
        return true;
    }

    private static boolean isHex(char c) {
        return (c >= '0' && c <= '9') ||
               (c >= 'A' && c <= 'F') ||
               (c >= 'a' && c <= 'f');
    }

    private static boolean isUnreserved(char c) {
        return (c >= 'A' && c <= 'Z') ||
               (c >= 'a' && c <= 'z') ||
               (c >= '0' && c <= '9') ||
               c == '-' || c == '.' || c == '_' || c == '~';
    }

    private static boolean isSubDelim(char c) {
        return c == '!' || c == '$' || c == '&' || c == '\'' ||
               c == '(' || c == ')' || c == '*' || c == '+' ||
               c == ',' || c == ';' || c == '=';
    }

    private static boolean isPchar(char c) {
        return isUnreserved(c) || isSubDelim(c) || c == ':' || c == '@';
    }

    private static boolean validatePercentEncoding(String s, int i) {
        // s.charAt(i) == '%'
        if (i + 2 >= s.length()) return false;
        return isHex(s.charAt(i + 1)) && isHex(s.charAt(i + 2));
    }

    private static boolean isValidPath(String path) {
        for (int i = 0; i < path.length(); i++) {
            char ch = path.charAt(i);
            if (ch == '%') {
                if (!validatePercentEncoding(path, i)) return false;
                i += 2;
            } else if (ch == '/') {
                continue;
            } else if (isPchar(ch)) {
                continue;
            } else {
                return false;
            }
        }
        return true;
    }

    private static boolean isValidQueryOrFragment(String part) {
        for (int i = 0; i < part.length(); i++) {
            char ch = part.charAt(i);
            if (ch == '%') {
                if (!validatePercentEncoding(part, i)) return false;
                i += 2;
            } else if (isPchar(ch) || ch == '/' || ch == '?') {
                continue;
            } else {
                return false;
            }
        }
        return true;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "https://example.com",
            "http://sub.example.co.uk:8080/path?query=ok#frag",
            "ftp://[2001:db8::1]/",
            "http://256.256.256.256",
            "https://example.com:70000"
        };
        for (String t : tests) {
            System.out.println(t + " -> " + isValidURL(t));
        }
    }
}