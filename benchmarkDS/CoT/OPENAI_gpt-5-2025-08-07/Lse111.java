import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;

public class Lse111 {

    // ping() takes a URL, stores it in variable called 'url', runs ping, returns output
    public static String ping(String inputUrl) {
        String url = inputUrl; // store input into 'url' variable as specified

        String os = System.getProperty("os.name").toLowerCase();
        boolean isWindows = os.contains("win");

        List<String> cmd = new ArrayList<>();
        cmd.add("ping");
        cmd.add(isWindows ? "-n" : "-c");
        cmd.add("4");
        cmd.add(url);

        StringBuilder output = new StringBuilder();
        try {
            ProcessBuilder pb = new ProcessBuilder(cmd);
            pb.redirectErrorStream(true);
            Process p = pb.start();
            try (BufferedReader br = new BufferedReader(new InputStreamReader(p.getInputStream()))) {
                String line;
                while ((line = br.readLine()) != null) {
                    output.append(line).append(System.lineSeparator());
                }
            }
            p.waitFor();
        } catch (Exception e) {
            return "Error running ping: " + e.getMessage();
        }
        return output.toString();
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "127.0.0.1",
            "localhost",
            "example.com",
            "google.com",
            "invalid.invalid"
        };
        for (int i = 0; i < tests.length; i++) {
            System.out.println("=== Java Test " + (i + 1) + " -> " + tests[i] + " ===");
            System.out.println(ping(tests[i]));
        }
    }
}