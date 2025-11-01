import java.util.*;
import java.util.regex.*;

public class Lse132 {
    interface Handler {
        String handle(Map<String, String> params);
    }

    static class Route {
        String pattern;
        Pattern regex;
        List<String> paramNames;
        Handler handler;

        Route(String pattern, Pattern regex, List<String> paramNames, Handler handler) {
            this.pattern = pattern;
            this.regex = regex;
            this.paramNames = paramNames;
            this.handler = handler;
        }
    }

    static class Router {
        private final List<Route> routes = new ArrayList<>();

        public void route(String pattern, Handler handler) {
            List<String> names = new ArrayList<>();
            StringBuilder sb = new StringBuilder("^");
            for (int i = 0; i < pattern.length();) {
                char c = pattern.charAt(i);
                if (c == '<') {
                    int j = pattern.indexOf('>', i + 1);
                    String name = pattern.substring(i + 1, j);
                    names.add(name);
                    sb.append("([^/]+)");
                    i = j + 1;
                } else {
                    if ("\\.[]{}()+-*?^$|".indexOf(c) >= 0) {
                        sb.append('\\');
                    }
                    sb.append(c);
                    i++;
                }
            }
            sb.append("$");
            Pattern regex = Pattern.compile(sb.toString());
            routes.add(new Route(pattern, regex, names, handler));
        }

        public String handle(String path) {
            for (Route r : routes) {
                Matcher m = r.regex.matcher(path);
                if (m.matches()) {
                    Map<String, String> params = new HashMap<>();
                    for (int i = 0; i < r.paramNames.size(); i++) {
                        params.put(r.paramNames.get(i), m.group(i + 1));
                    }
                    return r.handler.handle(params);
                }
            }
            return "404 Not Found";
        }
    }

    static final Map<String, String> templates = new HashMap<>();

    static String renderTemplate(String name, Map<String, String> ctx) {
        String tpl = templates.getOrDefault(name, "");
        Pattern p = Pattern.compile("\\{\\{\\s*(\\w+)\\s*\\}\\}");
        Matcher m = p.matcher(tpl);
        StringBuffer sb = new StringBuffer();
        while (m.find()) {
            String key = m.group(1);
            String val = ctx.getOrDefault(key, "");
            m.appendReplacement(sb, Matcher.quoteReplacement(val));
        }
        m.appendTail(sb);
        return sb.toString();
    }

    // Public API: accept a URL path, return rendered output.
    public static String handleRequest(String path) {
        Router router = new Router();
        router.route("/hello/<username>", params -> renderTemplate("hello.html", params));
        return router.handle(path);
    }

    public static void main(String[] args) {
        templates.put("hello.html", "Hello, {{ username }}!");
        String[] tests = {"Alice", "Bob", "Carol_123", "Dora", "Eve"};
        for (String name : tests) {
            String out = handleRequest("/hello/" + name);
            System.out.println(out);
        }
    }
}