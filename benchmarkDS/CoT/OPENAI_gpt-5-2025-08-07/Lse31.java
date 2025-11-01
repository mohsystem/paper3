import java.net.URI;
import java.net.URISyntaxException;
import java.util.Locale;

public class Lse31 {

    public static int redirect_url(String url) {
        if (url == null) {
            return 404;
        }
        String u = url.trim();
        if (u.isEmpty()) {
            return 404;
        }
        if (u.startsWith("//")) {
            u = "http:" + u;
        }
        URI uri;
        try {
            uri = new URI(u);
        } catch (URISyntaxException e) {
            return 404;
        }
        String host = uri.getHost();
        if (host == null) {
            return 404;
        }
        host = host.toLowerCase(Locale.ROOT);
        if ("example.com".equals(host)) {
            return 302;
        }
        return 404;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "https://example.com/page",
            "http://Example.com",
            "https://sub.example.com/home",
            "//example.com/resource",
            "mailto:user@example.com"
        };
        for (String t : tests) {
            int code = redirect_url(t);
            System.out.println(t + " -> " + code);
        }
    }
}