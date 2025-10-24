// Chain-of-Through process:
// 1) Problem understanding: This Java code defines a function to connect to an FTP server and download a file to the current directory using command line args if provided or sample test cases otherwise.
// 2) Security requirements: Avoid logging credentials, sanitize filenames, avoid overwriting files, set timeouts, handle exceptions safely.
// 3) Secure coding generation: Implement URLConnection with timeouts, safe file handling with atomic move, no sensitive logging.
// 4) Code review: Checked for input validation, exceptions handling, proper resource closing, prevention of path traversal, and safe writes.
// 5) Secure code output: Finalized code mitigates common risks inherent to FTP where possible in standard Java (note: FTP is insecure; FTPS is recommended but not implemented here due to JDK limitations without external libs).
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URI;
import java.net.URL;
import java.net.URLConnection;
import java.nio.file.*;
import java.util.Arrays;

public class Task77 {

    // Download file from FTP to current directory
    // Returns true on success, false otherwise
    public static boolean downloadSensitiveFile(String host, int port, String username, String password, String remotePath) {
        if (host == null || host.isEmpty() || username == null || password == null || remotePath == null || remotePath.isEmpty()) {
            return false;
        }
        // Sanitize and derive local file name
        String baseName = Paths.get(remotePath.replace("\\", "/")).getFileName() != null
                ? Paths.get(remotePath.replace("\\", "/")).getFileName().toString()
                : "downloaded_file";
        if (baseName.equals("") || baseName.equals(".") || baseName.equals("..")) {
            baseName = "downloaded_file";
        }
        // Replace unsafe characters
        baseName = baseName.replaceAll("[\\r\\n\\t\\\\/]", "_");

        Path finalPath = Paths.get(baseName).toAbsolutePath().normalize();
        Path tmpPath = finalPath.resolveSibling(baseName + ".tmp");

        // Prevent overwriting existing files
        if (Files.exists(finalPath)) {
            // Do not overwrite sensitive local files
            return false;
        }

        InputStream in = null;
        OutputStream out = null;
        try {
            String normalizedPath = remotePath.startsWith("/") ? remotePath : "/" + remotePath;
            // Note: This uses plain FTP which is not encrypted. Prefer FTPS where possible.
            URI uri = new URI("ftp", username + ":" + password, host, port, normalizedPath, null, null);
            URL url = uri.toURL();
            URLConnection conn = url.openConnection();
            conn.setConnectTimeout(15000);
            conn.setReadTimeout(60000);

            in = conn.getInputStream();

            // Create tmp file exclusively
            out = Files.newOutputStream(tmpPath, StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE);
            byte[] buffer = new byte[8192];
            int r;
            while ((r = in.read(buffer)) != -1) {
                out.write(buffer, 0, r);
            }
            out.flush();

            // Try to harden permissions when POSIX is supported
            try {
                Files.setPosixFilePermissions(tmpPath, java.util.EnumSet.of(
                        java.nio.file.attribute.PosixFilePermission.OWNER_READ,
                        java.nio.file.attribute.PosixFilePermission.OWNER_WRITE
                ));
            } catch (Exception ignored) {
                // Not POSIX or not supported
            }

            // Atomic move to final path
            Files.move(tmpPath, finalPath, StandardCopyOption.ATOMIC_MOVE);

            return true;
        } catch (Exception e) {
            // Avoid leaking sensitive info
            try {
                if (Files.exists(tmpPath)) {
                    Files.deleteIfExists(tmpPath);
                }
            } catch (Exception ignored) {}
            return false;
        } finally {
            try {
                if (in != null) in.close();
            } catch (Exception ignored) {}
            try {
                if (out != null) out.close();
            } catch (Exception ignored) {}
        }
    }

    // If args provided: host username password remotePath [port]
    // Otherwise run 5 test cases.
    public static void main(String[] args) {
        if (args != null && (args.length == 4 || args.length == 5)) {
            String host = args[0];
            String user = args[1];
            String pass = args[2];
            String remotePath = args[3];
            int port = 21;
            if (args.length == 5) {
                try {
                    port = Integer.parseInt(args[4]);
                } catch (Exception ignored) {}
            }
            boolean ok = downloadSensitiveFile(host, port, user, pass, remotePath);
            System.out.println(ok ? "SUCCESS" : "FAILURE");
            return;
        }

        // 5 test cases (placeholders; may fail depending on environment)
        String[][] tests = new String[][]{
                {"localhost", "user", "pass", "/sensitive.txt", "21"},
                {"127.0.0.1", "anonymous", "anonymous@", "/pub/example.bin", "21"},
                {"ftp.example.com", "user1", "pass1", "/data/file1.dat", "21"},
                {"ftp.testserver.com", "user2", "pass2", "docs/readme.txt", "21"},
                {"192.0.2.1", "user3", "pass3", "/secure/secret.cfg", "21"}
        };
        for (int i = 0; i < tests.length; i++) {
            String[] t = tests[i];
            boolean ok = downloadSensitiveFile(t[0], Integer.parseInt(t[4]), t[1], t[2], t[3]);
            System.out.println("Test " + (i + 1) + ": " + (ok ? "SUCCESS" : "FAILURE"));
        }
    }
}