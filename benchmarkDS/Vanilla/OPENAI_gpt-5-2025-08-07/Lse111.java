import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.TimeUnit;

public class Lse111 {
    public static String ping(String input) throws IOException, InterruptedException {
        String url = input; // store input in variable called url
        boolean isWindows = System.getProperty("os.name").toLowerCase().contains("win");
        List<String> cmd;
        if (isWindows) {
            cmd = Arrays.asList("ping", "-n", "1", "-w", "3000", url);
        } else {
            cmd = Arrays.asList("ping", "-c", "1", "-W", "3", url);
        }
        ProcessBuilder pb = new ProcessBuilder(cmd);
        pb.redirectErrorStream(true);
        Process p = pb.start();
        StringBuilder output = new StringBuilder();
        try (BufferedReader reader = new BufferedReader(new InputStreamReader(p.getInputStream(), StandardCharsets.UTF_8))) {
            String line;
            while ((line = reader.readLine()) != null) {
                output.append(line).append(System.lineSeparator());
            }
        }
        p.waitFor(10, TimeUnit.SECONDS);
        return output.toString();
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "localhost",
            "127.0.0.1",
            "8.8.8.8",
            "example.com",
            "invalid.host"
        };
        for (String t : tests) {
            try {
                System.out.println("=== Pinging: " + t + " ===");
                String result = ping(t);
                System.out.println(result);
            } catch (Exception e) {
                System.out.println("Error: " + e.getMessage());
            }
        }
    }
}