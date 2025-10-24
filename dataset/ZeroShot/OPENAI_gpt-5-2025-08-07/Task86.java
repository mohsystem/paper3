import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.RandomAccessFile;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Locale;
import java.util.concurrent.TimeUnit;
import javax.net.ssl.HttpsURLConnection;

public class Task86 {

    private static final long MAX_DOWNLOAD_SIZE = 50L * 1024L * 1024L; // 50 MB
    private static final int CONNECT_TIMEOUT_MS = 10_000;
    private static final int READ_TIMEOUT_MS = 30_000;
    private static final int EXEC_TIMEOUT_SECONDS = 30;

    public static int downloadAndExecuteExe(String urlStr) {
        URL initialUrl;
        try {
            initialUrl = new URL(urlStr);
        } catch (Exception e) {
            return -10; // invalid URL
        }

        // Require HTTPS
        if (!"https".equalsIgnoreCase(initialUrl.getProtocol())) {
            return -10; // invalid scheme
        }

        // Validate allowed host (Google domains only)
        if (!isAllowedHost(initialUrl.getHost())) {
            return -11; // disallowed host
        }

        HttpURLConnection conn = null;
        Path tempFile = null;
        try {
            conn = openConnectionFollowingRedirects(initialUrl, 5);
            int code = conn.getResponseCode();
            if (code != HttpURLConnection.HTTP_OK) {
                return -15; // network or HTTP error
            }

            long contentLength = getContentLength(conn);
            if (contentLength > 0 && contentLength > MAX_DOWNLOAD_SIZE) {
                return -13; // payload too large
            }

            tempFile = Files.createTempFile("Task86_", ".exe");

            try (InputStream rawIn = new BufferedInputStream(conn.getInputStream());
                 BufferedOutputStream out = new BufferedOutputStream(Files.newOutputStream(tempFile))) {
                byte[] buf = new byte[8192];
                long total = 0;
                int r;
                while ((r = rawIn.read(buf)) != -1) {
                    total += r;
                    if (total > MAX_DOWNLOAD_SIZE) {
                        return -13; // payload too large
                    }
                    out.write(buf, 0, r);
                }
            }

            // Verify PE signature (MZ and PE\0\0)
            if (!isValidPEFile(tempFile)) {
                return -14; // invalid EXE signature
            }

            // Only attempt to execute on Windows
            String os = System.getProperty("os.name", "").toLowerCase(Locale.ROOT);
            boolean isWindows = os.contains("win");
            if (!isWindows) {
                return -12; // not supported OS for EXE execution
            }

            ProcessBuilder pb = new ProcessBuilder(tempFile.toString());
            pb.redirectErrorStream(true);
            pb.directory(tempFile.getParent().toFile());
            Process p = pb.start();
            boolean finished = p.waitFor(EXEC_TIMEOUT_SECONDS, TimeUnit.SECONDS);
            if (!finished) {
                p.destroyForcibly();
                return -17; // execution timeout
            }
            return p.exitValue();

        } catch (SecurityException se) {
            return -16; // execution not permitted
        } catch (IOException ioe) {
            return -15; // network or IO error
        } catch (InterruptedException ie) {
            Thread.currentThread().interrupt();
            return -16; // interrupted
        } finally {
            if (conn != null) {
                conn.disconnect();
            }
            if (tempFile != null) {
                try {
                    Files.deleteIfExists(tempFile);
                } catch (IOException ignore) {
                }
            }
        }
    }

    private static boolean isAllowedHost(String host) {
        if (host == null) return false;
        String h = host.toLowerCase(Locale.ROOT);
        if (h.equals("google.com") || h.equals("googleusercontent.com")) return true;
        if (h.endsWith(".google.com")) return true;
        if (h.endsWith(".googleusercontent.com")) return true;
        // Typical Google Drive hosts
        if (h.equals("drive.google.com") || h.equals("docs.google.com")) return true;
        return false;
    }

    private static long getContentLength(HttpURLConnection conn) {
        long len = -1L;
        try {
            String cl = conn.getHeaderField("Content-Length");
            if (cl != null) {
                len = Long.parseLong(cl.trim());
            }
        } catch (Exception ignore) {
        }
        return len;
    }

    private static HttpURLConnection openConnectionFollowingRedirects(URL url, int maxRedirects) throws IOException {
        URL current = url;
        int redirects = 0;
        while (true) {
            HttpURLConnection c = (HttpURLConnection) current.openConnection();
            if (!(c instanceof HttpsURLConnection)) {
                c.disconnect();
                throw new IOException("Only HTTPS is allowed.");
            }
            c.setInstanceFollowRedirects(false);
            c.setConnectTimeout(CONNECT_TIMEOUT_MS);
            c.setReadTimeout(READ_TIMEOUT_MS);
            c.setRequestProperty("User-Agent", "Task86/1.0 (+https://example.com)");
            c.setRequestProperty("Accept", "*/*");
            int code = c.getResponseCode();
            if (code >= 300 && code < 400) {
                String loc = c.getHeaderField("Location");
                c.disconnect();
                if (loc == null) throw new IOException("Redirect without Location");
                URL next = new URL(current, loc);
                if (!"https".equalsIgnoreCase(next.getProtocol())) {
                    throw new IOException("Redirected to non-HTTPS");
                }
                if (!isAllowedHost(next.getHost())) {
                    throw new IOException("Redirected to disallowed host");
                }
                redirects++;
                if (redirects > maxRedirects) {
                    throw new IOException("Too many redirects");
                }
                current = next;
                continue;
            }
            return c;
        }
    }

    private static boolean isValidPEFile(Path file) throws IOException {
        try (RandomAccessFile raf = new RandomAccessFile(file.toFile(), "r")) {
            if (raf.length() < 64) return false;
            raf.seek(0);
            int b0 = raf.read();
            int b1 = raf.read();
            if (b0 != 'M' || b1 != 'Z') return false;
            raf.seek(0x3C);
            long e_lfanew = Integer.toUnsignedLong(Integer.reverseBytes(raf.readInt()));
            if (e_lfanew < 0 || e_lfanew > raf.length() - 4) return false;
            raf.seek(e_lfanew);
            int sig = raf.readInt();
            // 'P' 'E' 0 0 == 0x00004550 little-endian
            if (sig != 0x00004550) return false;
            return true;
        }
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "https://drive.google.com/uc?export=download&id=FILE_ID_1",
            "https://docs.google.com/uc?export=download&id=FILE_ID_2",
            "http://drive.google.com/uc?export=download&id=FILE_ID_3",
            "https://example.com/file.exe",
            "https://googleusercontent.com/path/to/file.exe"
        };
        for (int i = 0; i < tests.length; i++) {
            int code = downloadAndExecuteExe(tests[i]);
            System.out.println("Test " + (i + 1) + " exit code: " + code);
        }
    }
}