import java.util.*;
import java.util.function.Function;

public class Lse134 {

    interface Handler { String handle(Map<String,String> params); }

    static class Route {
        String pattern;
        Handler handler;
        Route(String p, Handler h) { pattern = p; handler = h; }
    }

    static List<Route> routes = new ArrayList<>();

    public static void route(String pattern, Handler handler) {
        routes.add(new Route(pattern, handler));
    }

    public static String escape(String s) {
        if (s == null) return "";
        StringBuilder b = new StringBuilder();
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            switch (c) {
                case '&': b.append("&amp;"); break;
                case '<': b.append("&lt;"); break;
                case '>': b.append("&gt;"); break;
                case '"': b.append("&quot;"); break;
                case '\'': b.append("&#x27;"); break;
                default: b.append(c);
            }
        }
        return b.toString();
    }

    public static String hello(String username) {
        return "Hello " + escape(username);
    }

    private static String trimSlashes(String s) {
        String t = s;
        while (t.startsWith("/")) t = t.substring(1);
        while (t.endsWith("/")) t = t.substring(0, t.length() - 1);
        return t;
    }

    public static Map<String,String> match(String pattern, String url) {
        String[] p = trimSlashes(pattern).split("/");
        String[] u = trimSlashes(url).split("/");
        if (pattern.equals("/")) p = new String[]{""};
        if (url.equals("/")) u = new String[]{""};
        if (p.length != u.length) return null;

        Map<String,String> params = new HashMap<>();
        for (int i = 0; i < p.length; i++) {
            String part = p[i];
            String seg = u[i];
            if (part.startsWith("<") && part.endsWith(">")) {
                String name = part.substring(1, part.length() - 1);
                params.put(name, seg);
            } else {
                if (!part.equals(seg)) return null;
            }
        }
        return params;
    }

    public static String handleRequest(String url) {
        for (Route r : routes) {
            Map<String,String> params = match(r.pattern, url);
            if (params != null) {
                return r.handler.handle(params);
            }
        }
        return "404 Not Found";
    }

    public static void main(String[] args) {
        route("/hello/<username>", params -> hello(params.get("username")));

        String[] testUrls = new String[] {
            "/hello/Alice",
            "/hello/Bob & Co",
            "/hello/User<Name>",
            "/hello/Quote\"User'",
            "/hello/Greater>Less<"
        };
        for (String url : testUrls) {
            System.out.println(handleRequest(url));
        }
    }
}