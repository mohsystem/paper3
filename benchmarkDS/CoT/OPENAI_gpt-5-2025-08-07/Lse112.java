import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpServer;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;

public class Lse112 {

    @Retention(RetentionPolicy.RUNTIME)
    @Target(ElementType.METHOD)
    public @interface Route {
        String value();
    }

    public static String runPing(String url) throws IOException, InterruptedException {
        String os = System.getProperty("os.name").toLowerCase();
        String countFlag = os.contains("win") ? "-n" : "-c";
        ProcessBuilder pb = new ProcessBuilder("ping", countFlag, "1", url);
        pb.redirectErrorStream(true);
        Process p = pb.start();
        try (BufferedReader br = new BufferedReader(new InputStreamReader(p.getInputStream()))) {
            while (br.readLine() != null) {
                // Drain output
            }
        }
        p.waitFor();
        return url;
    }

    @Route("/ping")
    public static void pingHandler(HttpExchange exchange) throws IOException {
        String url = "example.com";
        try {
            runPing(url);
        } catch (Exception ignored) {}
        byte[] resp = url.getBytes(StandardCharsets.UTF_8);
        exchange.getResponseHeaders().add("Content-Type", "text/plain; charset=utf-8");
        exchange.sendResponseHeaders(200, resp.length);
        try (OutputStream os = exchange.getResponseBody()) {
            os.write(resp);
        }
    }

    public static void main(String[] args) throws Exception {
        // Start a simple HTTP server on port 8080
        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        for (Method m : Lse112.class.getDeclaredMethods()) {
            if (m.isAnnotationPresent(Lse112.Route.class)) {
                Route r = m.getAnnotation(Lse112.Route.class);
                server.createContext(r.value(), exchange -> {
                    try {
                        m.invoke(null, exchange);
                    } catch (InvocationTargetException | IllegalAccessException e) {
                        String msg = "Internal Server Error";
                        exchange.sendResponseHeaders(500, msg.length());
                        try (OutputStream os = exchange.getResponseBody()) {
                            os.write(msg.getBytes(StandardCharsets.UTF_8));
                        }
                    }
                });
            }
        }
        server.setExecutor(null);
        server.start();

        // 5 test cases
        String[] tests = {"example.com", "localhost", "127.0.0.1", "google.com", "invalid.invalid"};
        for (String t : tests) {
            try {
                String result = runPing(t);
                System.out.println("Pinged and returned URL: " + result);
            } catch (Exception e) {
                System.out.println("Ping failed for: " + t + " (returned URL anyway: " + t + ")");
            }
        }
    }
}