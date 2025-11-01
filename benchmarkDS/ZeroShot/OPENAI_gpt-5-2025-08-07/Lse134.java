import java.util.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Lse134 {

    // HTML escape utility
    public static String escapeHtml(String input) {
        if (input == null) return "";
        StringBuilder sb = new StringBuilder(input.length());
        for (int i = 0; i < input.length(); i++) {
            char c = input.charAt(i);
            switch (c) {
                case '&': sb.append("&amp;"); break;
                case '<': sb.append("&lt;"); break;
                case '>': sb.append("&gt;"); break;
                case '"': sb.append("&quot;"); break;
                case '\'': sb.append("&#39;"); break;
                default: sb.append(c);
            }
        }
        return sb.toString();
    }

    // The target function
    public static String hello(String username) {
        return "Hello " + escapeHtml(username);
    }

    // Simple handler interface
    interface Handler {
        String handle(List<String> params);
    }

    // Router that supports a single variable placeholder like {username}
    static class Router {
        private static class Route {
            final Pattern pattern;
            final Handler handler;

            Route(Pattern pattern, Handler handler) {
                this.pattern = pattern;
                this.handler = handler;
            }
        }

        private final List<Route> routes = new ArrayList<>();

        public void addRoute(String template, Handler handler) {
            // Convert "/hello/{username}" -> "^/hello/([^/]+)$"
            String regex = Pattern.quote(template);
            // Replace quoted "{username}" occurrences with a capture group
            regex = regex.replace("\\{username\\}", "([^/]+)");
            Pattern p = Pattern.compile("^" + regex + "$");
            routes.add(new Route(p, handler));
        }

        public String navigate(String url) {
            if (url == null) return null;
            for (Route r : routes) {
                Matcher m = r.pattern.matcher(url);
                if (m.matches()) {
                    List<String> params = new ArrayList<>();
                    for (int i = 1; i <= m.groupCount(); i++) {
                        params.add(m.group(i));
                    }
                    return r.handler.handle(params);
                }
            }
            return null;
        }
    }

    // Register routes similar to a decorator mapping
    public static Router initRouter() {
        Router router = new Router();
        router.addRoute("/hello/{username}", params -> hello(params.get(0)));
        return router;
    }

    public static void main(String[] args) {
        Router router = initRouter();
        List<String> tests = Arrays.asList(
                "/hello/World",
                "/hello/%3Cscript%3Ealert(1)%3C%2Fscript%3E",
                "/hello/O'Connor%20%26%20Sons",
                "/hello/Tom%20%22T%22",
                "/hello/..%2Fpath%3Fx%3D1%26y%3D2"
        );

        for (String url : tests) {
            // Decode minimal percent-encodings for demonstration; avoid exceptions
            String safeUrl = url;
            try {
                safeUrl = java.net.URLDecoder.decode(url, java.nio.charset.StandardCharsets.UTF_8.name());
            } catch (IllegalArgumentException e) {
                // keep original if bad encoding
            }
            String result = router.navigate(safeUrl);
            System.out.println((result != null) ? result : "404 Not Found");
        }
    }
}