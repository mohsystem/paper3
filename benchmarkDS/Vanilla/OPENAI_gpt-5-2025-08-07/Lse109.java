import java.io.*;
import java.nio.charset.StandardCharsets;

public class Lse109 {
    public static String execCommand(String command) throws IOException, InterruptedException {
        Process proc = new ProcessBuilder("/bin/sh", "-c", command)
                .redirectErrorStream(true)
                .start();
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        try (InputStream is = proc.getInputStream()) {
            byte[] buf = new byte[4096];
            int n;
            while ((n = is.read(buf)) != -1) {
                baos.write(buf, 0, n);
            }
        }
        proc.waitFor();
        return baos.toString(StandardCharsets.UTF_8);
    }

    public static String getUserInfo(String username) throws IOException, InterruptedException {
        String cmd = "cat /etc/passwd | grep " + username;
        return execCommand(cmd);
    }

    public static void main(String[] args) throws Exception {
        if (args.length > 0) {
            System.out.print(getUserInfo(args[0]));
        } else {
            String[] tests = {"root", "daemon", "bin", "sys", "nobody"};
            for (String u : tests) {
                System.out.print(getUserInfo(u));
            }
        }
    }
}