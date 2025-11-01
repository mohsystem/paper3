import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.List;
import java.util.regex.Pattern;

public class Lse109 {

    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z_][A-Za-z0-9_-]{0,31}$");

    public static String getUserInfo(String username) throws IOException, InterruptedException {
        if (username == null || !USERNAME_PATTERN.matcher(username).matches()) {
            throw new IllegalArgumentException("Invalid username");
        }

        String regexPattern = "^" + username + ":";

        Process catProc = new ProcessBuilder("/bin/cat", "/etc/passwd")
                .redirectErrorStream(true)
                .start();

        byte[] passwdData = readAllBytes(catProc.getInputStream(), 2 * 1024 * 1024); // cap at 2MB
        int catExit = catProc.waitFor();
        if (catExit != 0) {
            throw new IOException("Failed to read /etc/passwd");
        }

        Process grepProc = new ProcessBuilder("/bin/grep", "-E", regexPattern)
                .redirectErrorStream(true)
                .start();

        try (OutputStream grepStdin = grepProc.getOutputStream()) {
            grepStdin.write(passwdData);
            grepStdin.flush();
        }

        byte[] grepOut = readAllBytes(grepProc.getInputStream(), 2 * 1024 * 1024);
        grepProc.waitFor(); // exit code 1 means no match; still fine

        return new String(grepOut, StandardCharsets.UTF_8);
    }

    private static byte[] readAllBytes(InputStream in, int maxBytes) throws IOException {
        ByteArrayOutputStream bos = new ByteArrayOutputStream();
        byte[] buf = new byte[8192];
        int total = 0;
        int r;
        while ((r = in.read(buf)) != -1) {
            total += r;
            if (total > maxBytes) {
                throw new IOException("Input too large");
            }
            bos.write(buf, 0, r);
        }
        return bos.toByteArray();
    }

    public static void main(String[] args) {
        List<String> tests;
        if (args.length >= 1) {
            tests = Arrays.asList(args[0]);
        } else {
            tests = Arrays.asList("root", "daemon", "bin", "nobody", "no_such_user_zzz");
        }

        for (String u : tests) {
            System.out.println("=== Query: " + u + " ===");
            try {
                String out = getUserInfo(u);
                if (out.isEmpty()) {
                    System.out.println("(no match)");
                } else {
                    System.out.print(out);
                }
            } catch (Exception e) {
                System.out.println("(error)");
            }
        }
    }
}