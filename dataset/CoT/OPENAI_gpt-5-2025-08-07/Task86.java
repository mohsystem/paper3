import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.IOException;
import java.net.HttpURLConnection;
import java.net.ProtocolException;
import java.net.URL;
import java.net.MalformedURLException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Locale;

public class Task86 {

    // Downloads an EXE file from a URL (restricting to Google Drive/Googleusercontent) with HTTPS, size cap, and SHA-256 verification.
    // Returns the absolute path to the downloaded file if verification succeeds, otherwise throws IOException.
    public static String downloadExeFromGoogleDrive(String inputUrl, String expectedSha256Hex, long maxBytes) throws IOException {
        if (inputUrl == null || inputUrl.isEmpty()) {
            throw new MalformedURLException("URL must not be empty");
        }
        if (expectedSha256Hex == null || expectedSha256Hex.trim().isEmpty()) {
            throw new IOException("Expected SHA-256 must be provided");
        }
        String normalizedHash = normalizeHex(expectedSha256Hex);
        if (normalizedHash.length() != 64) {
            throw new IOException("Expected SHA-256 must be 64 hex characters");
        }
        URL currentUrl = new URL(inputUrl);
        int redirects = 0;
        HttpURLConnection conn = null;

        while (true) {
            if (!"https".equalsIgnoreCase(currentUrl.getProtocol())) {
                throw new IOException("Only HTTPS is allowed");
            }
            conn = (HttpURLConnection) currentUrl.openConnection();
            conn.setInstanceFollowRedirects(false);
            conn.setConnectTimeout(15000);
            conn.setReadTimeout(20000);
            conn.setRequestMethod("GET");
            conn.setRequestProperty("User-Agent", "Task86SecureDownloader/1.0");
            int code = conn.getResponseCode();

            if (isRedirect(code)) {
                if (++redirects > 5) {
                    conn.disconnect();
                    throw new IOException("Too many redirects");
                }
                String location = conn.getHeaderField("Location");
                conn.disconnect();
                if (location == null || location.isEmpty()) {
                    throw new IOException("Redirect without Location header");
                }
                currentUrl = new URL(currentUrl, location);
                continue;
            } else if (code == HttpURLConnection.HTTP_OK) {
                // Final URL checks
                String host = currentUrl.getHost();
                if (!isAllowedHost(host)) {
                    conn.disconnect();
                    throw new IOException("Disallowed host: " + host);
                }

                long contentLength = conn.getContentLengthLong();
                if (contentLength > 0 && contentLength > maxBytes) {
                    conn.disconnect();
                    throw new IOException("Content too large: " + contentLength + " > " + maxBytes);
                }

                File temp = File.createTempFile("task86_", ".exe");
                temp.deleteOnExit();

                MessageDigest md = getSha256Digest();
                long total = 0L;
                byte[] buf = new byte[8192];
                try (InputStream in = new BufferedInputStream(conn.getInputStream());
                     BufferedOutputStream out = new BufferedOutputStream(new FileOutputStream(temp))) {
                    int r;
                    while ((r = in.read(buf)) != -1) {
                        total += r;
                        if (total > maxBytes) {
                            throw new IOException("Downloaded size exceeded limit: " + total + " > " + maxBytes);
                        }
                        md.update(buf, 0, r);
                        out.write(buf, 0, r);
                    }
                    out.flush();
                } catch (IOException e) {
                    // Cleanup on failure
                    safeDelete(temp);
                    throw e;
                } finally {
                    conn.disconnect();
                }

                String actualHex = toHex(md.digest());
                if (!actualHex.equalsIgnoreCase(normalizedHash)) {
                    safeDelete(temp);
                    throw new IOException("SHA-256 mismatch. Expected " + normalizedHash + " but got " + actualHex);
                }

                return temp.getAbsolutePath();
            } else {
                String msg = "HTTP error: " + code;
                conn.disconnect();
                throw new IOException(msg);
            }
        }
    }

    // Intentionally disabled: Executing arbitrary binaries can be unsafe.
    // Returns false and does not execute anything.
    public static boolean executeExeSecurely(String exePath) {
        // Disabled by design for safety. If you must execute, ensure:
        // 1) Strict allowlist of trusted hashes and code-signing verification.
        // 2) User interactive consent and sandboxing.
        // 3) Principle of least privilege.
        // This method returns false to indicate no execution was performed.
        return false;
    }

    private static boolean isRedirect(int code) {
        return code == HttpURLConnection.HTTP_MOVED_PERM ||
               code == HttpURLConnection.HTTP_MOVED_TEMP ||
               code == HttpURLConnection.HTTP_SEE_OTHER ||
               code == 307 || code == 308;
    }

    private static boolean isAllowedHost(String host) {
        if (host == null) return false;
        String h = host.toLowerCase(Locale.ROOT);
        return h.equals("drive.google.com") || h.endsWith(".googleusercontent.com");
    }

    private static MessageDigest getSha256Digest() throws IOException {
        try {
            return MessageDigest.getInstance("SHA-256");
        } catch (NoSuchAlgorithmException e) {
            throw new IOException("SHA-256 not available", e);
        }
    }

    private static String toHex(byte[] bytes) {
        StringBuilder sb = new StringBuilder(bytes.length * 2);
        for (byte b : bytes) {
            sb.append(Character.forDigit((b >>> 4) & 0xF, 16));
            sb.append(Character.forDigit((b & 0xF), 16));
        }
        return sb.toString();
    }

    private static String normalizeHex(String hex) {
        StringBuilder sb = new StringBuilder(64);
        for (char c : hex.toCharArray()) {
            if (isHexChar(c)) {
                sb.append(Character.toLowerCase(c));
            }
        }
        return sb.toString();
    }

    private static boolean isHexChar(char c) {
        return (c >= '0' && c <= '9') ||
               (c >= 'a' && c <= 'f') ||
               (c >= 'A' && c <= 'F');
    }

    private static void safeDelete(File f) {
        if (f != null) {
            try { f.delete(); } catch (Throwable ignored) { }
        }
    }

    // 5 test cases
    public static void main(String[] args) {
        String[] urls = new String[] {
            "https://drive.google.com/uc?export=download&id=FILE_ID_1",
            "https://drive.google.com/uc?export=download&id=FILE_ID_2",
            "http://drive.google.com/uc?export=download&id=FILE_ID_3",
            "https://example.com/file.exe",
            "https://drive.google.com/INVALID"
        };
        String[] hashes = new String[] {
            "0000000000000000000000000000000000000000000000000000000000000000",
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
            "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
            "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc",
            "dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd"
        };

        for (int i = 0; i < urls.length; i++) {
            System.out.println("Test " + (i+1) + ": URL=" + urls[i]);
            try {
                String path = downloadExeFromGoogleDrive(urls[i], hashes[i], 50L * 1024L * 1024L);
                System.out.println("Downloaded to: " + path);
                boolean executed = executeExeSecurely(path);
                System.out.println("Executed? " + executed);
            } catch (Exception e) {
                System.out.println("Failed: " + e.getMessage());
            }
            System.out.println("----");
        }
    }
}