import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

public class Lse116 {
    public static String print(String fmt, Object... args) {
        return (args == null || args.length == 0) ? fmt : String.format(fmt, args);
    }

    public static int system(String command) throws IOException, InterruptedException {
        ProcessBuilder pb = new ProcessBuilder("sh", "-c", command);
        pb.redirectErrorStream(true);
        Process p = pb.start();
        try (BufferedReader r = new BufferedReader(new InputStreamReader(p.getInputStream()))) {
            String line;
            while ((line = r.readLine()) != null) {
                System.out.println(line);
            }
        }
        return p.waitFor();
    }

    public static int runCommand(String fmt, Object... args) {
        String cmd = print(fmt, args);
        try {
            return system(cmd);
        } catch (Exception e) {
            return -1;
        }
    }

    public static void main(String[] args) {
        runCommand("ls");
        runCommand("ls %s", "-l");
        runCommand("ls %s", "-a");
        runCommand("ls %s %s", "-l", ".");
        runCommand("ls %s", "-1");
    }
}