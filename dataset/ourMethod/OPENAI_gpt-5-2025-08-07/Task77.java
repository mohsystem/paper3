import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.IOException;
import java.net.URL;
import java.net.URLConnection;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFileAttributeView;
import java.nio.file.attribute.PosixFilePermissions;
import java.nio.file.*;
import java.util.EnumSet;
import java.util.Set;

public class Task77 {

    public static String downloadFtpFile(String host, String username, String password, String remotePath) throws IOException {
        validateInputs(host, username, password, remotePath);

        String sanitizedFilename = sanitizeFilename(remotePath);
        if (sanitizedFilename.isEmpty()) {
            sanitizedFilename = "downloaded.bin";
        }

        String pathPart = encodePath(remotePath.startsWith("/") ? remotePath : "/" + remotePath);
        String userInfo = encodeUserInfo(username) + ":" + encodeUserInfo(password);
        String urlStr = "ftp://" + host + pathPart + ";type=i";
        System.setProperty("sun.net.ftp.passiveMode", "true");

        URL url = new URL(urlStr);
        URLConnection conn = url.openConnection();
        // Set login via URL userInfo as per java.net FTP handler
        // But avoid logging or printing credentials
        // Java's FTP handler uses the URL's userInfo automatically, so construct URL with auth:
        // Rebuild with userInfo embedded safely
        URL authUrl = new URL(url.getProtocol(), url.getHost(), url.getPort(), url.getFile(), null);
        String authSpec = "ftp://" + userInfo + "@" + host + pathPart + ";type=i";
        url = new URL(authSpec);

        conn = url.openConnection();
        conn.setConnectTimeout(15000);
        conn.setReadTimeout(30000);

        Path tempFile = Files.createTempFile(Paths.get("."), "dl_", ".part");
        securePermissions(tempFile);

        try (InputStream in = conn.getInputStream();
             FileOutputStream fos = new FileOutputStream(tempFile.toFile());
             FileChannel channel = fos.getChannel()) {

            byte[] buffer = new byte[8192];
            int read;
            long total = 0L;
            while ((read = in.read(buffer)) != -1) {
                if (read == 0) continue;
                fos.write(buffer, 0, read);
                total += read;
                if (total > (long)1024 * 1024 * 1024 * 2) { // 2GB safety limit
                    throw new IOException("File too large");
                }
            }
            fos.flush();
            channel.force(true);
        } catch (IOException e) {
            try { Files.deleteIfExists(tempFile); } catch (IOException ignore) {}
            throw new IOException("Download failed", e);
        }

        Path target = Paths.get(sanitizedFilename);
        // Atomic move if possible
        try {
            Files.move(tempFile, target, StandardCopyOption.ATOMIC_MOVE, StandardCopyOption.REPLACE_EXISTING);
        } catch (AtomicMoveNotSupportedException e) {
            Files.move(tempFile, target, StandardCopyOption.REPLACE_EXISTING);
        }
        securePermissions(target);
        return target.toAbsolutePath().toString();
    }

    private static void validateInputs(String host, String username, String password, String remotePath) {
        if (host == null || host.length() < 1 || host.length() > 255 || !host.matches("^[A-Za-z0-9._-]+$")) {
            throw new IllegalArgumentException("Invalid host");
        }
        if (username == null || username.length() < 1 || username.length() > 128) {
            throw new IllegalArgumentException("Invalid username");
        }
        if (password == null || password.length() < 1 || password.length() > 256) {
            throw new IllegalArgumentException("Invalid password");
        }
        if (remotePath == null || remotePath.length() < 1 || remotePath.length() > 1024) {
            throw new IllegalArgumentException("Invalid remote path");
        }
        if (remotePath.endsWith("/")) {
            throw new IllegalArgumentException("Remote path must be a file, not a directory");
        }
    }

    private static String encodeUserInfo(String s) {
        StringBuilder sb = new StringBuilder();
        for (byte b : s.getBytes(StandardCharsets.UTF_8)) {
            int c = b & 0xFF;
            if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '.' || c == '_' || c == '~') {
                sb.append((char) c);
            } else {
                sb.append('%');
                String hex = Integer.toHexString(c).toUpperCase();
                if (hex.length() == 1) sb.append('0');
                sb.append(hex);
            }
        }
        return sb.toString();
    }

    private static String encodePath(String s) {
        StringBuilder sb = new StringBuilder();
        for (byte b : s.getBytes(StandardCharsets.UTF_8)) {
            int c = b & 0xFF;
            if (c == '/') {
                sb.append('/');
            } else if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '.' || c == '_' || c == '~') {
                sb.append((char) c);
            } else {
                sb.append('%');
                String hex = Integer.toHexString(c).toUpperCase();
                if (hex.length() == 1) sb.append('0');
                sb.append(hex);
            }
        }
        return sb.toString();
    }

    private static String sanitizeFilename(String remotePath) {
        String rp = remotePath.replace('\\', '/');
        int idx = rp.lastIndexOf('/');
        String base = (idx >= 0) ? rp.substring(idx + 1) : rp;
        if (base.length() > 255) base = base.substring(0, 255);
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < base.length(); i++) {
            char c = base.charAt(i);
            if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '.' || c == '-' || c == '_') {
                sb.append(c);
            } else {
                sb.append('_');
            }
        }
        String result = sb.toString();
        if (result.equals(".") || result.equals("..") || result.isEmpty()) {
            return "downloaded.bin";
        }
        return result;
    }

    private static void securePermissions(Path p) {
        try {
            PosixFileAttributeView view = Files.getFileAttributeView(p, PosixFileAttributeView.class);
            if (view != null) {
                Set<PosixFilePermission> perms = EnumSet.of(PosixFilePermission.OWNER_READ, PosixFilePermission.OWNER_WRITE);
                Files.setPosixFilePermissions(p, perms);
            }
        } catch (Exception ignored) {
        }
    }

    public static void main(String[] args) {
        // Usage: java Task77 <host> <username> <password> <remotePath>
        if (args.length == 4) {
            try {
                String saved = downloadFtpFile(args[0], args[1], args[2], args[3]);
                System.out.println("Downloaded to: " + saved);
            } catch (Exception e) {
                System.err.println("Operation failed: " + e.getMessage());
                System.exit(1);
            }
            return;
        }

        System.out.println("Usage: java Task77 <host> <username> <password> <remotePath>");
        String[][] tests = new String[][]{
                {"invalid.example", "user", "pass", "/readme.txt"},
                {"example.com", "anonymous", "guest@example.com", "/pub/example.dat"},
                {"localhost", "ftpuser", "p@ssw0rd!", "folder/file.txt"},
                {"ftp.example.org", "user_name", "secret", "/path/with space/file name.bin"},
                {"192.0.2.10", "u", "p", "/sensitive/secret.bin"}
        };
        for (int i = 0; i < tests.length; i++) {
            try {
                String saved = downloadFtpFile(tests[i][0], tests[i][1], tests[i][2], tests[i][3]);
                System.out.println("Test " + (i + 1) + " downloaded to: " + saved);
            } catch (Exception e) {
                System.out.println("Test " + (i + 1) + " failed: " + e.getMessage());
            }
        }
    }
}