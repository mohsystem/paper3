import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.reflect.Method;
import java.nio.charset.StandardCharsets;
import java.time.Duration;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.TimeUnit;
import java.net.URI;
import java.net.URISyntaxException;

@Retention(RetentionPolicy.RUNTIME)
@interface Route {
    String value();
}

class PingResult {
    public final String url;
    public final String pong;
    public final boolean alive;
    public final int exitCode;

    PingResult(String url, String pong, boolean alive, int exitCode) {
        this.url = url;
        this.pong = pong;
        this.alive = alive;
        this.exitCode = exitCode;
    }

    @Override
    public String toString() {
        return "url=" + url + ", alive=" + alive + ", exitCode=" + exitCode + ", pong_len=" + (pong == null ? 0 : pong.length());
    }
}

public class Lse113 {
    private static final int MAX_OUTPUT = 8192;
    private static final Map<String, Method> ROUTES = new HashMap<>();

    private static void initRoutes() {
        for (Method m : Lse113.class.getDeclaredMethods()) {
            if (m.isAnnotationPresent(Route.class)) {
                Route r = m.getAnnotation(Route.class);
                ROUTES.put(r.value(), m);
            }
        }
    }

    private static String extractAndValidateHost(String url) throws IllegalArgumentException {
        if (url == null) throw new IllegalArgumentException("invalid url");
        String s = url.trim();
        if (s.length() == 0 || s.length() > 2048) throw new IllegalArgumentException("invalid url");
        String host = s;
        try {
            if (s.contains("://")) {
                URI u = new URI(s);
                if (u.getHost() != null) {
                    host = u.getHost();
                } else {
                    // Handle IPv6 literal in URI without brackets or raw host after scheme
                    String auth = u.getRawAuthority();
                    if (auth != null) {
                        int at = auth.lastIndexOf('@');
                        host = (at >= 0 ? auth.substring(at + 1) : auth);
                        int c = host.indexOf(':');
                        if (c >= 0) host = host.substring(0, c);
                    } else {
                        throw new IllegalArgumentException("invalid url");
                    }
                }
            }
        } catch (URISyntaxException e) {
            // treat as raw host if not a valid URI
            host = s;
        }

        host = host.trim();
        if (host.startsWith("[") && host.endsWith("]")) {
            host = host.substring(1, host.length() - 1);
        }
        if (host.length() == 0 || host.length() > 253) throw new IllegalArgumentException("invalid host length");

        // allow only safe chars
        for (int i = 0; i < host.length(); i++) {
            char ch = host.charAt(i);
            boolean ok = (ch >= 'a' && ch <= 'z') ||
                         (ch >= 'A' && ch <= 'Z') ||
                         (ch >= '0' && ch <= '9') ||
                         ch == '.' || ch == '-' || ch == ':'; // ':' for IPv6
            if (!ok) throw new IllegalArgumentException("invalid host char");
        }
        if (host.contains("..") || host.contains("--") || host.startsWith("-") || host.startsWith(".") || host.endsWith(".") || host.endsWith("-")) {
            // basic sanity checks
        }
        return host;
    }

    public static PingResult ping(String url) {
        String pong = "";
        int exitCode = -1;
        boolean alive = false;
        String host;
        try {
            host = extractAndValidateHost(url);
        } catch (IllegalArgumentException e) {
            return new PingResult(url, "invalid url/host", false, -1);
        }

        List<String> cmd = new ArrayList<>();
        String os = System.getProperty("os.name").toLowerCase();
        boolean isWindows = os.contains("win");
        cmd.add("ping");
        if (isWindows) {
            cmd.add("-n");
            cmd.add("1");
            cmd.add("-w");
            cmd.add("2000");
        } else {
            cmd.add("-c");
            cmd.add("1");
            cmd.add("-W");
            cmd.add("2");
        }
        cmd.add(host);

        ProcessBuilder pb = new ProcessBuilder(cmd);
        pb.redirectErrorStream(true);
        Process proc = null;
        StringBuilder out = new StringBuilder();
        try {
            proc = pb.start();
            try (BufferedReader br = new BufferedReader(new InputStreamReader(proc.getInputStream(), StandardCharsets.UTF_8))) {
                char[] buf = new char[512];
                int r;
                while ((r = br.read(buf)) != -1) {
                    if (out.length() + r > MAX_OUTPUT) {
                        out.append(buf, 0, Math.max(0, MAX_OUTPUT - out.length()));
                        break;
                    } else {
                        out.append(buf, 0, r);
                    }
                }
            }
            boolean finished = proc.waitFor(5, TimeUnit.SECONDS);
            if (!finished) {
                proc.destroyForcibly();
                exitCode = -1;
            } else {
                exitCode = proc.exitValue();
            }
        } catch (IOException | InterruptedException e) {
            exitCode = -1;
        } finally {
            if (proc != null) proc.destroy();
        }
        pong = out.toString();
        alive = (exitCode == 0);
        return new PingResult(url, pong, alive, exitCode);
    }

    @Route("/ping")
    public static PingResult pingHandler(String url) {
        return ping(url);
    }

    public static PingResult handleRequest(String path, String urlParam) {
        Method m = ROUTES.get(path);
        if (m == null) {
            return new PingResult(urlParam, "route not found", false, -1);
        }
        try {
            Object res = m.invoke(null, urlParam);
            if (res instanceof PingResult) return (PingResult) res;
            return new PingResult(urlParam, "invalid handler result", false, -1);
        } catch (Exception e) {
            return new PingResult(urlParam, "handler error", false, -1);
        }
    }

    public static void main(String[] args) {
        initRoutes();
        String[] tests = new String[] {
            "127.0.0.1",
            "localhost",
            "https://example.com",
            "invalid host!!",
            "8.8.8.8"
        };
        for (String url : tests) {
            PingResult r = handleRequest("/ping", url);
            System.out.println(r.toString());
            System.out.println("Sample of pong: " + (r.pong.length() > 120 ? r.pong.substring(0, 120) + "..." : r.pong));
            System.out.println("-----");
        }
    }
}