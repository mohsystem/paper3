import java.io.BufferedReader;
import java.io.InputStreamReader;

public class Lse112 {
    public static String ping(String url) {
        try {
            boolean isWin = System.getProperty("os.name").toLowerCase().contains("win");
            ProcessBuilder pb = new ProcessBuilder(
                isWin ? new String[] {"ping", "-n", "1", url} : new String[] {"ping", "-c", "1", url}
            );
            pb.redirectErrorStream(true);
            Process p = pb.start();
            try (BufferedReader br = new BufferedReader(new InputStreamReader(p.getInputStream()))) {
                while (br.readLine() != null) { /* consume output */ }
            }
            p.waitFor();
        } catch (Exception e) {
            // ignore for simple example
        }
        return url;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "localhost",
            "example.com",
            "127.0.0.1",
            "8.8.8.8",
            "github.com"
        };
        for (String t : tests) {
            System.out.println("Returned URL: " + ping(t));
        }
    }
}