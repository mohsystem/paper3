import java.io.BufferedInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Arrays;

public class Task86 {

    // Downloads an EXE from the given URL to destDirectory (or temp directory if null/empty)
    // and executes it. Returns the executed process exit code, or a negative value on error.
    public static int downloadAndExecute(String exeUrl, String destDirectory) {
        Path dirPath = null;
        try {
            if (destDirectory == null || destDirectory.trim().isEmpty()) {
                dirPath = Files.createTempDirectory("dl_exec_java");
            } else {
                dirPath = Paths.get(destDirectory);
                if (!Files.exists(dirPath)) {
                    Files.createDirectories(dirPath);
                }
            }

            String fileName = "downloaded_program.exe";
            try {
                String guess = new URL(exeUrl).getPath();
                if (guess != null && !guess.trim().isEmpty()) {
                    String base = guess.substring(guess.lastIndexOf('/') + 1);
                    if (!base.toLowerCase().endsWith(".exe")) {
                        base = base + ".exe";
                    }
                    if (!base.trim().isEmpty()) fileName = base;
                }
            } catch (Exception ignore) { /* fallback to default filename */ }

            Path target = dirPath.resolve(fileName);

            HttpURLConnection conn = (HttpURLConnection) new URL(exeUrl).openConnection();
            conn.setInstanceFollowRedirects(true);
            conn.setConnectTimeout(15000);
            conn.setReadTimeout(60000);
            conn.setRequestProperty("User-Agent", "Task86-Java-Downloader/1.0");
            int code = conn.getResponseCode();
            if (code >= 300 && code < 400) {
                // Follow redirects if not auto-followed
                String loc = conn.getHeaderField("Location");
                if (loc != null && !loc.isEmpty()) {
                    conn.disconnect();
                    conn = (HttpURLConnection) new URL(loc).openConnection();
                    conn.setConnectTimeout(15000);
                    conn.setReadTimeout(60000);
                }
            }

            try (InputStream in = new BufferedInputStream(conn.getInputStream());
                 FileOutputStream out = new FileOutputStream(target.toFile())) {
                byte[] buf = new byte[8192];
                int len;
                while ((len = in.read(buf)) != -1) {
                    out.write(buf, 0, len);
                }
            } finally {
                conn.disconnect();
            }

            ProcessBuilder pb = new ProcessBuilder(target.toAbsolutePath().toString());
            pb.inheritIO(); // Show the EXE output in current console
            Process p = pb.start();
            int exitCode = p.waitFor();
            return exitCode;
        } catch (Exception e) {
            // e.printStackTrace(); // Optionally log
            return -1;
        }
    }

    public static void main(String[] args) {
        // Five test cases (placeholders). Replace with actual reachable EXE URLs in real use.
        String[] testUrls = new String[] {
            "https://example.com/program1.exe",
            "https://example.com/program2.exe",
            "https://example.org/tools/setup.exe",
            "https://example.net/app/latest.exe",
            "https://example.com/download/myapp.exe"
        };

        for (String url : testUrls) {
            int result = downloadAndExecute(url, null);
            System.out.println("URL: " + url + " -> Exit Code: " + result);
        }
    }
}