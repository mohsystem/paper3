import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.util.regex.Pattern;

public class Lse108 {
    // Note: Java does not expose C's system(). We safely execute the command via ProcessBuilder.
    private static final Pattern USER_RE = Pattern.compile("^[A-Za-z0-9._-]{1,32}$");

    public static int getentToOutput(String username, OutputStream output) throws IOException, InterruptedException {
        if (username == null || !USER_RE.matcher(username).matches()) {
            return -1;
        }
        ProcessBuilder pb = new ProcessBuilder("getent", "passwd", username);
        pb.redirectError(ProcessBuilder.Redirect.DISCARD);
        Process p = pb.start();
        try (var in = p.getInputStream()) {
            byte[] buf = new byte[4096];
            int r;
            while ((r = in.read(buf)) != -1) {
                output.write(buf, 0, r);
            }
            output.flush();
        }
        int code = p.waitFor();
        return code;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "root",
            "nobody",
            "daemon",
            "thisuserdoesnotexist",
            "bad;injection" // invalid by validation
        };
        for (String u : tests) {
            try {
                ByteArrayOutputStream baos = new ByteArrayOutputStream();
                int rc = getentToOutput(u, baos);
                System.out.println("User: " + u + " -> rc=" + rc);
                String out = baos.toString(StandardCharsets.UTF_8);
                System.out.print(out.isEmpty() ? "(no output)\n" : out);
                System.out.println("----");
            } catch (Exception e) {
                System.out.println("User: " + u + " -> error");
                System.out.println("----");
            }
        }
    }
}