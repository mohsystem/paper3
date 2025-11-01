import java.io.IOException;

public class Lse116 {
    // Java does not have a C-style system() function. Using ProcessBuilder to execute shell commands.
    public static String print(String fmt, Object... args) {
        return String.format(fmt, args);
    }

    public static int runLs(String options, String directory) throws IOException, InterruptedException {
        if (options == null) options = "";
        if (directory == null || directory.isEmpty()) directory = ".";
        String cmd = print("ls %s %s", options, directory);
        Process p = new ProcessBuilder("sh", "-c", cmd).inheritIO().start();
        return p.waitFor();
    }

    public static void main(String[] args) throws Exception {
        runLs("", ".");
        runLs("-l", ".");
        runLs("-a", ".");
        runLs("-la", ".");
        runLs("", "..");
    }
}