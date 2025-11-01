import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;
import java.lang.annotation.ElementType;
import java.lang.reflect.Method;
import java.nio.charset.StandardCharsets;
import java.util.LinkedHashMap;
import java.util.Map;

public class Lse134 {

    @Retention(RetentionPolicy.RUNTIME)
    @Target(ElementType.METHOD)
    @interface Route {
        String value();
    }

    // Simple router that maps patterns to methods.
    static final class Router {
        private static final int MAX_PATH_LEN = 2048;
        private static final int MAX_USERNAME_LEN = 256;
        private static final Map<String, Method> routes = new LinkedHashMap<>();

        static void registerAnnotatedRoutes() {
            Method[] methods = Lse134.class.getDeclaredMethods();
            for (Method m : methods) {
                if (m.isAnnotationPresent(Route.class)) {
                    Route r = m.getAnnotation(Route.class);
                    routes.put(r.value(), m);
                }
            }
        }

        static String handleRequest(String path) {
            if (path == null) {
                return "400 Bad Request";
            }
            // Validate path encoding length
            byte[] bytes = path.getBytes(StandardCharsets.UTF_8);
            if (bytes.length == 0 || bytes.length > MAX_PATH_LEN) {
                return "400 Bad Request";
            }

            for (Map.Entry<String, Method> e : routes.entrySet()) {
                String pattern = e.getKey();
                Method method = e.getValue();

                // We support a single path variable: {username} at end.
                if ("/hello/{username}".equals(pattern)) {
                    final String prefix = "/hello/";
                    if (!path.startsWith(prefix)) {
                        continue;
                    }
                    String username = path.substring(prefix.length());
                    if (username.isEmpty() || username.contains("/")) {
                        // Not matching or traversal-like
                        continue;
                    }
                    if (username.getBytes(StandardCharsets.UTF_8).length > MAX_USERNAME_LEN) {
                        return "400 Bad Request";
                    }
                    try {
                        // Call the handler
                        return (String) method.invoke(null, username);
                    } catch (Exception ex) {
                        return "500 Internal Server Error";
                    }
                }
            }
            return "404 Not Found";
        }
    }

    // HTML escape for &, <, >, ", ' characters
    static String escapeHtml(String input) {
        if (input == null) return "";
        StringBuilder sb = new StringBuilder(input.length() + 16);
        for (int i = 0; i < input.length(); i++) {
            char c = input.charAt(i);
            switch (c) {
                case '&': sb.append("&amp;"); break;
                case '<': sb.append("&lt;"); break;
                case '>': sb.append("&gt;"); break;
                case '"': sb.append("&quot;"); break;
                case '\'': sb.append("&#x27;"); break;
                default: sb.append(c);
            }
        }
        return sb.toString();
    }

    @Route("/hello/{username}")
    public static String hello(String username) {
        // Sanitize output
        return "Hello " + escapeHtml(username);
    }

    // Exposed function: takes a URL path and returns response
    public static String handle(String urlPath) {
        return Router.handleRequest(urlPath);
    }

    public static void main(String[] args) {
        Router.registerAnnotatedRoutes();

        // 5 test cases
        String[] tests = new String[] {
            "/hello/Alice",
            "/hello/Bob<Admin>",
            "/hello/",
            "/hi/Alice",
            "/hello/" + "A".repeat(300)
        };
        for (String t : tests) {
            System.out.println("URL: " + t + " -> " + handle(t));
        }
    }
}