import java.net.URI;
import java.net.URISyntaxException;

public class Lse31 {

    public static String redirect_url(String url) {
        try {
            URI uri = new URI(url);
            String host = uri.getHost();
            if (host == null) {
                try {
                    uri = new URI("http://" + url);
                    host = uri.getHost();
                } catch (URISyntaxException e2) {
                    return "404 Not Found";
                }
            }
            if (host != null && host.equalsIgnoreCase("example.com")) {
                return "Redirecting to " + url;
            }
        } catch (URISyntaxException e) {
            // fall through
        }
        return "404 Not Found";
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "http://example.com",
            "https://example.com/path",
            "http://example.com:8080/abc",
            "https://sub.example.com/home",
            "https://notexample.com"
        };
        for (String t : tests) {
            System.out.println(redirect_url(t));
        }
    }
}