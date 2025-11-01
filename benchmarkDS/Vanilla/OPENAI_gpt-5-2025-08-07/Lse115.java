import java.io.BufferedReader;
import java.io.InputStreamReader;

public class Lse115 {
    public static String print(String format, Object... args) {
        return String.format(format, args);
    }

    public static String execCommand(String command) {
        StringBuilder sb = new StringBuilder();
        try {
            ProcessBuilder pb = new ProcessBuilder("/bin/sh", "-c", command);
            pb.redirectErrorStream(true);
            Process p = pb.start();
            try (BufferedReader r = new BufferedReader(new InputStreamReader(p.getInputStream()))) {
                String line;
                while ((line = r.readLine()) != null) {
                    sb.append(line).append(System.lineSeparator());
                }
            }
            p.waitFor();
        } catch (Exception e) {
            sb.append(e.toString()).append(System.lineSeparator());
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        String[] cmds = new String[] {
            print("ls %s", "-1"),
            print("ls"),
            print("ls %s %s", "-a", "-1"),
            print("ls %s %s", "-l", "."),
            print("ls %s", "this_path_does_not_exist")
        };
        for (String cmd : cmds) {
            System.out.println("Command: " + cmd);
            String out = execCommand(cmd);
            System.out.println(out);
        }
    }
}