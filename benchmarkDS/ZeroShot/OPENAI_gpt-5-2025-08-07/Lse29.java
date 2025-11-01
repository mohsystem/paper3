import java.net.IDN;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.regex.Pattern;

public class Lse29 {

    private static final int MAX_URL_LENGTH = 2048;
    private static final Pattern HOST_PATTERN = Pattern.compile("^([^.]+\\.)*example\\.com$", Pattern.CASE_INSENSITIVE);

    public static String safeRedirect(String target) {
        if (target == null) {
            return "/";
        }

        String t = target.trim();
        if (t.isEmpty() || t.length() > MAX_URL_LENGTH) {
            return "/";
        }

        // Prevent CRLF injection
        if (t.indexOf('\r') >= 0 || t.indexOf('\n') >= 0) {
            return "/";
        }

        URI uri;
        try {
            uri = new URI(t);
        } catch (URISyntaxException e) {
            return "/";
        }

        String scheme = uri.getScheme();
        if (scheme == null) {
            return "/";
        }
        String lowerScheme = scheme.toLowerCase();
        if (!("http".equals(lowerScheme) || "https".equals(lowerScheme))) {
            return "/";
        }

        String host = uri.getHost();
        if (host == null || host.isEmpty()) {
            return "/";
        }

        String asciiHost;
        try {
            asciiHost = IDN.toASCII(host);
        } catch (IllegalArgumentException e) {
            return "/";
        }

        if (!HOST_PATTERN.matcher(asciiHost).matches()) {
            return "/";
        }

        return t;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
                "https://example.com/abc",
                "http://sub.example.com:8080/path",
                "https://evil.com",
                "/relative/path",
                "https://notexample.com"
        };
        for (String test : tests) {
            String result = safeRedirect(test);
            System.out.println("Input: " + test + " -> Redirect: " + result);
        }
    }
}