import java.net.IDN;
import java.net.URI;
import java.net.URISyntaxException;

public class Lse31 {
    public static String redirect_url(String url) {
        if (url == null) return "404 Not Found";
        String u = url.trim();
        if (u.isEmpty() || containsCtl(u)) return "404 Not Found";
        if (!u.contains("://")) {
            u = "http://" + u;
        }
        URI uri;
        try {
            uri = new URI(u).normalize();
        } catch (URISyntaxException e) {
            return "404 Not Found";
        }
        String scheme = uri.getScheme();
        if (scheme == null) return "404 Not Found";
        String s = scheme.toLowerCase();
        if (!s.equals("http") && !s.equals("https")) return "404 Not Found";
        String host = uri.getHost();
        if (host == null || host.isEmpty()) return "404 Not Found";
        String asciiHost;
        try {
            asciiHost = IDN.toASCII(host);
        } catch (IllegalArgumentException iae) {
            return "404 Not Found";
        }
        if ("example.com".equalsIgnoreCase(asciiHost)) {
            return "Redirected to: " + u;
        }
        return "404 Not Found";
    }

    private static boolean containsCtl(String s) {
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if ((c >= 0 && c < 0x20) || c == 0x7F) return true;
        }
        return false;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "http://example.com/path",
            "https://EXAMPLE.com",
            "http://sub.example.com",
            "http://example.com@evil.com",
            "example.com"
        };
        for (String t : tests) {
            System.out.println(redirect_url(t));
        }
    }
}