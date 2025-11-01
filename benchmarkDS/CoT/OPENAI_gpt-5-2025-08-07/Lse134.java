import java.util.*;
import java.lang.annotation.*;
import java.lang.reflect.*;

@Retention(RetentionPolicy.RUNTIME)
@interface Route {
    String value();
}

public class Lse134 {
    private static final Map<String, Method> routes = new HashMap<>();

    public static String escape(String s) {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
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

    @Route("/hello")
    public static String hello(String username) {
        return "Hello " + escape(username);
    }

    public static void registerRoutes() {
        for (Method m : Lse134.class.getDeclaredMethods()) {
            if (m.isAnnotationPresent(Route.class)) {
                Route r = m.getAnnotation(Route.class);
                routes.put(r.value(), m);
            }
        }
    }

    public static String handleRequest(String url) {
        String prefix = "/hello/";
        if (url.startsWith(prefix)) {
            String username = url.substring(prefix.length());
            Method m = routes.get("/hello");
            if (m != null) {
                try {
                    return (String) m.invoke(null, username);
                } catch (Exception e) {
                    return "500 Internal Server Error";
                }
            }
        }
        return "404 Not Found";
    }

    public static void main(String[] args) {
        registerRoutes();
        String[] tests = new String[] {
            "/hello/world",
            "/hello/Alice&Bob",
            "/hello/Eve<Admin>",
            "/hello/O'Reilly",
            "/hello/Tom \"The Tiger\""
        };
        for (String t : tests) {
            System.out.println(handleRequest(t));
        }
    }
}