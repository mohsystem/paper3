import java.io.BufferedReader;
import java.io.InputStreamReader;

public class Lse115 {
    public static String printFmt(String fmt, Object... args) {
        return String.format(fmt, args);
    }

    public static String execCommand(String command) {
        StringBuilder out = new StringBuilder();
        try {
            ProcessBuilder pb = new ProcessBuilder("/bin/sh", "-c", command);
            pb.redirectErrorStream(true);
            Process p = pb.start();
            try (BufferedReader br = new BufferedReader(new InputStreamReader(p.getInputStream()))) {
                String line;
                while ((line = br.readLine()) != null) {
                    out.append(line).append(System.lineSeparator());
                }
            }
            p.waitFor();
        } catch (Exception e) {
            out.append(e.toString());
        }
        return out.toString();
    }

    public static void main(String[] args) {
        String[] cmds = new String[] {
            printFmt("ls"),
            printFmt("ls %s", "-l"),
            printFmt("ls %s %s", "-a", "."),
            printFmt("ls %s %s", "-1", ".."),
            printFmt("ls %s %s %s", "-l", "-a", ".")
        };
        for (int i = 0; i < cmds.length; i++) {
            System.out.println("Command #" + (i + 1) + ": " + cmds[i]);
            String output = execCommand(cmds[i]);
            System.out.println("Output:\n" + output);
        }
    }
}