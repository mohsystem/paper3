import java.util.regex.Pattern;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;

public class Lse30 {
    public static String getRedirect(String query) {
        String target = getQueryParam(query, "target");
        if (target == null) return "/";
        String decoded = urlDecode(target);
        Pattern pattern = Pattern.compile("^[A-Za-z0-9-]+\\..*");
        if (pattern.matcher(decoded).matches()) {
            return decoded;
        }
        return "/";
    }

    private static String getQueryParam(String query, String key) {
        if (query == null) return null;
        String[] parts = query.split("&");
        for (String part : parts) {
            int idx = part.indexOf('=');
            String k = idx >= 0 ? part.substring(0, idx) : part;
            if (k.equals(key)) {
                return idx >= 0 ? part.substring(idx + 1) : "";
            }
        }
        return null;
    }

    private static String urlDecode(String s) {
        try {
            return URLDecoder.decode(s, StandardCharsets.UTF_8.name());
        } catch (Exception e) {
            return s;
        }
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "target=sub.example.com",
            "target=http%3A%2F%2Fevil.com",
            "target=foo.bar/baz",
            "page=home",
            "target=foo."
        };
        for (String q : tests) {
            System.out.println(getRedirect(q));
        }
    }
}