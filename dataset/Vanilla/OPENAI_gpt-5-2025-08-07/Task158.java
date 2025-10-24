import java.io.*;
import java.nio.charset.StandardCharsets;
import java.util.regex.Pattern;

public class Task158 {
    private static final Pattern SAFE_PATTERN = Pattern.compile("^[A-Za-z0-9 _.,:@%+/=\\-/]*$");
    private static final int MAX_LEN = 200;

    public static boolean isSafe(String s) {
        return s != null && s.length() <= MAX_LEN && SAFE_PATTERN.matcher(s).matches();
    }

    public static String runEcho(String input) throws IOException, InterruptedException {
        if (!isSafe(input)) {
            throw new IllegalArgumentException("Input contains disallowed characters or is too long.");
        }
        ProcessBuilder pb;
        String os = System.getProperty("os.name").toLowerCase();
        if (os.contains("win")) {
            pb = new ProcessBuilder("cmd.exe", "/c", "echo", input);
        } else {
            pb = new ProcessBuilder("/bin/echo", input);
        }
        pb.redirectErrorStream(true);
        Process p = pb.start();
        try (BufferedReader br = new BufferedReader(new InputStreamReader(p.getInputStream(), StandardCharsets.UTF_8))) {
            StringBuilder out = new StringBuilder();
            String line;
            while ((line = br.readLine()) != null) {
                if (out.length() > 0) out.append(System.lineSeparator());
                out.append(line);
            }
            int code = p.waitFor();
            if (code != 0) {
                throw new IOException("Process exited with code " + code);
            }
            return out.toString();
        }
    }

    public static void main(String[] args) {
        try {
            // 5 test cases
            System.out.println(runEcho("Hello World"));
            System.out.println(runEcho("Sample_123"));
            System.out.println(runEcho("Path/with/slash"));
            System.out.println(runEcho("Value+Plus=Equals"));
            System.out.println(runEcho("Email: user@example.com"));

            // Interactive input
            BufferedReader reader = new BufferedReader(new InputStreamReader(System.in, StandardCharsets.UTF_8));
            String userInput = reader.readLine();
            if (userInput == null) userInput = "";
            String result = runEcho(userInput);
            System.out.println(result);
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
        }
    }
}