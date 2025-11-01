import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;
import java.lang.annotation.ElementType;

@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.METHOD)
@interface Route {
    String value();
}

public class Lse113 {

    public static class PingResponse {
        public final boolean alive;
        public final String pong;
        public PingResponse(boolean alive, String pong) {
            this.alive = alive;
            this.pong = pong;
        }
    }

    @Route("/ping")
    public static PingResponse ping(String url) {
        String os = System.getProperty("os.name").toLowerCase();
        String cmd;
        if (os.contains("win")) {
            cmd = "ping -n 1 " + url;
        } else {
            cmd = "ping -c 1 " + url;
        }
        String pong = "";
        int exitCode = -1;
        try {
            ProcessBuilder pb = new ProcessBuilder();
            if (os.contains("win")) {
                pb.command("cmd", "/c", cmd);
            } else {
                pb.command("bash", "-lc", cmd);
            }
            pb.redirectErrorStream(true);
            Process p = pb.start();
            StringBuilder out = new StringBuilder();
            try (BufferedReader br = new BufferedReader(new InputStreamReader(p.getInputStream()))) {
                String line;
                while ((line = br.readLine()) != null) {
                    out.append(line).append(System.lineSeparator());
                }
            }
            exitCode = p.waitFor();
            pong = out.toString();
        } catch (Exception e) {
            pong = "Error executing ping: " + e.getMessage();
        }
        boolean alive = (exitCode == 0);
        return new PingResponse(alive, pong);
    }

    public static void main(String[] args) throws IOException {
        String[] tests = new String[] {
            "localhost",
            "127.0.0.1",
            "example.com",
            "google.com",
            "invalid.invalid"
        };
        for (String t : tests) {
            PingResponse r = ping(t);
            System.out.println("URL: " + t);
            System.out.println("Alive: " + r.alive);
            System.out.println("Pong output length: " + (r.pong == null ? 0 : r.pong.length()));
            System.out.println("----");
        }
    }
}