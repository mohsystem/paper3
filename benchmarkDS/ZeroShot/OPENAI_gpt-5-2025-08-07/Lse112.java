import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.Locale;
import java.util.concurrent.TimeUnit;
import java.util.regex.Pattern;

public class Lse112 {

    private static final Pattern IPV4 = Pattern.compile(
            "^(?:(?:25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)\\.){3}(?:25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)$"
    );
    private static final Pattern HOSTNAME = Pattern.compile(
            "^(?=.{1,253}$)(?:[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?\\.)+[A-Za-z]{2,63}$"
    );

    public static boolean isValidHost(String url) {
        if (url == null) return false;
        String u = url.trim();
        if (u.equalsIgnoreCase("localhost")) return true;
        if (IPV4.matcher(u).matches()) return true;
        return HOSTNAME.matcher(u).matches();
    }

    public static String securePing(String url) {
        if (!isValidHost(url)) {
            return null;
        }
        boolean isWindows = System.getProperty("os.name", "").toLowerCase(Locale.ROOT).contains("win");
        String[] cmd = isWindows
                ? new String[]{"ping", "-n", "1", "-w", "2000", url}
                : new String[]{"ping", "-c", "1", "-W", "2", url};

        ProcessBuilder pb = new ProcessBuilder(cmd);
        pb.redirectErrorStream(true);
        Process p = null;
        try {
            p = pb.start();
            boolean finished = p.waitFor(3, TimeUnit.SECONDS);
            if (!finished) {
                p.destroyForcibly();
                return null;
            }
            // Read a small portion of output to avoid blocking; ignore content, just demonstrate execution
            try (BufferedReader br = new BufferedReader(new InputStreamReader(p.getInputStream(), StandardCharsets.UTF_8))) {
                char[] buf = new char[512];
                br.read(buf, 0, 512);
            } catch (Exception ignored) {
            }
            return url;
        } catch (Exception e) {
            if (p != null) p.destroyForcibly();
            return null;
        }
    }

    public static void main(String[] args) {
        String[] tests = new String[]{
                "example.com",
                "127.0.0.1",
                "8.8.8.8",
                "invalid;;host",
                "localhost"
        };
        for (String t : tests) {
            String result = securePing(t);
            System.out.println("Input: " + t + " -> Returned: " + (result == null ? "null" : result));
        }
    }
}