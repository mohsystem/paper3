import java.net.URI;
import java.net.URISyntaxException;

public class Lse31 {

    private static final int MAX_URL_LEN = 2048;

    public static String redirectUrl(String url) {
        if (url == null) {
            return "404 Not Found";
        }
        String trimmed = url.trim();
        if (trimmed.length() == 0 || trimmed.length() > MAX_URL_LEN) {
            return "404 Not Found";
        }

        final URI uri;
        try {
            uri = new URI(trimmed);
        } catch (URISyntaxException e) {
            return "404 Not Found";
        }

        String scheme = uri.getScheme();
        if (scheme == null || !(scheme.equalsIgnoreCase("http") || scheme.equalsIgnoreCase("https"))) {
            return "404 Not Found";
        }

        String host = uri.getHost();
        if (host == null) {
            // Fallback: try to parse authority if host is null (rare malformed or IPv6 cases)
            String authority = uri.getRawAuthority();
            if (authority == null || authority.isEmpty()) {
                return "404 Not Found";
            }
            // Strip userinfo if present
            int at = authority.lastIndexOf('@');
            String hostPort = (at >= 0) ? authority.substring(at + 1) : authority;
            if (hostPort.startsWith("[")) {
                int end = hostPort.indexOf(']');
                if (end <= 0) {
                    return "404 Not Found";
                }
                host = hostPort.substring(1, end);
            } else {
                int colon = hostPort.indexOf(':');
                host = (colon >= 0) ? hostPort.substring(0, colon) : hostPort;
            }
        }
        if (host == null || host.isEmpty()) {
            return "404 Not Found";
        }

        if ("example.com".equalsIgnoreCase(host)) {
            return "REDIRECT " + trimmed;
        }
        return "404 Not Found";
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "https://example.com/path",
            "http://example.com:8080?q=1",
            "http://sub.example.com/index.html",
            "ftp://example.com/file",
            "https://evil.com/"
        };
        for (String t : tests) {
            System.out.println(redirectUrl(t));
        }
    }
}