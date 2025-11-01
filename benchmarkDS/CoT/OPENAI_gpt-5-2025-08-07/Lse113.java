import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.reflect.Method;
import java.util.HashMap;
import java.util.Map;

@Retention(RetentionPolicy.RUNTIME)
@interface Route {
    String value();
}

class PingResult {
    public final boolean alive;
    public final String pong;

    public PingResult(boolean alive, String pong) {
        this.alive = alive;
        this.pong = pong;
    }

    @Override
    public String toString() {
        return "alive=" + alive + ", pong_len=" + (pong == null ? 0 : pong.length());
    }
}

public class Lse113 {
    private static final Map<String, Method> routes = new HashMap<>();

    static {
        registerRoutes();
    }

    private static void registerRoutes() {
        Method[] methods = Lse113.class.getDeclaredMethods();
        for (Method m : methods) {
            if (m.isAnnotationPresent(Route.class)) {
                Route r = m.getAnnotation(Route.class);
                routes.put(r.value(), m);
            }
        }
    }

    public static PingResult handleRequest(String requestPath) {
        for (Map.Entry<String, Method> e : routes.entrySet()) {
            String pattern = e.getKey();
            if ("<url> ping".equals(pattern) && requestPath.endsWith(" ping")) {
                String url = requestPath.substring(0, requestPath.length() - " ping".length()).trim();
                try {
                    return (PingResult) e.getValue().invoke(null, url);
                } catch (Exception ex) {
                    return new PingResult(false, ex.toString());
                }
            }
        }
        return new PingResult(false, "No route matched");
    }

    private static String runPingCommand(String target) throws Exception {
        boolean isWindows = System.getProperty("os.name").toLowerCase().contains("win");
        String[] cmd = isWindows ? new String[]{"ping", "-n", "1", target}
                                 : new String[]{"ping", "-c", "1", target};
        ProcessBuilder pb = new ProcessBuilder(cmd);
        pb.redirectErrorStream(true);
        Process proc = pb.start();
        StringBuilder out = new StringBuilder();
        try (BufferedReader br = new BufferedReader(new InputStreamReader(proc.getInputStream()))) {
            String line;
            while ((line = br.readLine()) != null) {
                out.append(line).append(System.lineSeparator());
            }
        }
        int code = proc.waitFor();
        // store full output in pong
        boolean alive = (code == 0);
        return (alive ? "ALIVE\n" : "DEAD\n") + out.toString();
    }

    @Route("<url> ping")
    public static PingResult ping(String url) {
        try {
            String pong = runPingCommand(url);
            boolean alive = pong.startsWith("ALIVE");
            return new PingResult(alive, pong);
        } catch (Exception e) {
            return new PingResult(false, e.toString());
        }
    }

    public static void main(String[] args) {
        String[] tests = new String[]{
            "localhost ping",
            "127.0.0.1 ping",
            "google.com ping",
            "example.com ping",
            "no.such.domain.invalid ping"
        };
        for (String t : tests) {
            PingResult r = handleRequest(t);
            System.out.println(t + " -> " + r.toString());
        }
    }
}