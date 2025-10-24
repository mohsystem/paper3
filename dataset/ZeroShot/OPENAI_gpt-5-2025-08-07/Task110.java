import java.net.*;
import java.util.*;
import java.nio.charset.StandardCharsets;

public class Task110 {
    public static boolean isValidURL(String url) {
        if (url == null) return false;
        if (url.length() == 0 || url.length() > 2048) return false;
        for (int i = 0; i < url.length(); i++) {
            char ch = url.charAt(i);
            if (Character.isWhitespace(ch) || ch < 0x20 || ch == 0x7F) return false;
        }
        final Set<String> allowedSchemes = new HashSet<>(Arrays.asList("http", "https"));
        URI uri;
        try {
            uri = new URI(url);
        } catch (URISyntaxException e) {
            return false;
        }
        String scheme = uri.getScheme();
        if (scheme == null || !allowedSchemes.contains(scheme.toLowerCase(Locale.ROOT))) return false;
        if (uri.getUserInfo() != null) return false; // disallow credentials in URLs
        String host = uri.getHost();
        if (host == null || host.length() == 0) return false;
        if (!isValidHost(host)) return false;
        int port = uri.getPort();
        if (port != -1 && (port < 1 || port > 65535)) return false;
        return true;
    }

    private static boolean isValidHost(String host) {
        // IPv6 literal
        if (host.indexOf(':') >= 0) {
            try {
                InetAddress addr = InetAddress.getByName(host);
                return addr instanceof Inet6Address;
            } catch (Exception e) {
                return false;
            }
        }
        // IPv4
        if (isLikelyIPv4(host)) {
            return isValidIPv4(host);
        }
        // Domain name (IDN allowed)
        try {
            String normalized = host.endsWith(".") ? host.substring(0, host.length() - 1) : host;
            String ascii = IDN.toASCII(normalized, IDN.USE_STD3_ASCII_RULES);
            if (ascii.length() == 0 || ascii.length() > 253) return false;
            String[] labels = ascii.split("\\.");
            if (labels.length == 0) return false;
            for (String label : labels) {
                if (label.length() < 1 || label.length() > 63) return false;
                if (!label.matches("[A-Za-z0-9-]+")) return false;
                if (label.startsWith("-") || label.endsWith("-")) return false;
            }
            return true;
        } catch (Exception e) {
            return false;
        }
    }

    private static boolean isLikelyIPv4(String host) {
        for (int i = 0; i < host.length(); i++) {
            char c = host.charAt(i);
            if (!(c == '.' || (c >= '0' && c <= '9'))) return false;
        }
        return true;
    }

    private static boolean isValidIPv4(String host) {
        String[] parts = host.split("\\.", -1);
        if (parts.length != 4) return false;
        for (String p : parts) {
            if (p.length() == 0 || p.length() > 3) return false;
            for (int i = 0; i < p.length(); i++) {
                char c = p.charAt(i);
                if (c < '0' || c > '9') return false;
            }
            if (p.length() > 1 && p.charAt(0) == '0') return false; // no leading zeros
            try {
                int val = Integer.parseInt(p);
                if (val < 0 || val > 255) return false;
            } catch (NumberFormatException e) {
                return false;
            }
        }
        return true;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "https://example.com",
            "http://sub.example.co.uk/path?query=1#frag",
            "https://user:pass@example.com",
            "ftp://example.com",
            "http://256.256.256.256"
        };
        for (String t : tests) {
            System.out.println(t + " -> " + isValidURL(t));
        }
    }
}