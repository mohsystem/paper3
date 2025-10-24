// Task39 - Java
// Chain-of-Through process in code comments:
// 1) Problem understanding: Take a filename, invoke a system command to display its contents, return output.
// 2) Security requirements: prevent command injection and unsafe usage by validating the filename, avoid shell where possible, and ensure the file is a regular file.
// 3) Secure coding generation: use ProcessBuilder without shell on Unix (/bin/cat -- file), on Windows use cmd with separate args; validate input; handle errors.
// 4) Code review: validate length/charset, use "--" for cat, avoid shell concatenation on Unix, restrict dangerous characters on Windows.
// 5) Secure code output: final sanitized implementation below.

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.Locale;
import java.util.regex.Pattern;

public class Task39 {

    private static final int MAX_FILENAME_LEN = 4096;
    private static final Pattern SAFE_PATTERN = Pattern.compile("^[A-Za-z0-9_./\\\\ \\-:]+$");

    public static String displayFileSecure(String filename) {
        // Validate filename input
        if (!isSafeFilename(filename)) {
            return "Error: Invalid filename.";
        }

        Path p = Paths.get(filename);
        try {
            if (!Files.exists(p) || !Files.isRegularFile(p)) {
                return "Error: File not found or not a regular file.";
            }
        } catch (SecurityException se) {
            return "Error: Access denied.";
        }

        boolean isWindows = System.getProperty("os.name").toLowerCase(Locale.ROOT).contains("win");
        ProcessBuilder pb;
        if (isWindows) {
            // Using cmd with separate arguments to allow ProcessBuilder to quote safely.
            pb = new ProcessBuilder("cmd.exe", "/c", "type", filename);
        } else {
            // Use cat without a shell and with "--" to prevent option parsing of filenames starting with '-'.
            pb = new ProcessBuilder("/bin/cat", "--", filename);
        }
        // Merge stderr into stdout to simplify handling
        pb.redirectErrorStream(true);

        try {
            Process proc = pb.start();
            String output = readAll(proc.getInputStream());
            int code = proc.waitFor();
            if (code == 0) {
                return output;
            } else {
                return "Error: Command failed with exit code " + code + ". Output:\n" + output;
            }
        } catch (IOException e) {
            return "Error: IO failure executing command.";
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            return "Error: Execution interrupted.";
        }
    }

    private static boolean isSafeFilename(String filename) {
        if (filename == null) return false;
        if (filename.length() == 0 || filename.length() > MAX_FILENAME_LEN) return false;
        // Reject any null character
        if (filename.indexOf('\0') != -1) return false;
        // Whitelist-based validation
        if (!SAFE_PATTERN.matcher(filename).matches()) return false;

        // Additional Windows-specific restrictions to avoid shell metacharacters when cmd is used.
        boolean isWindows = System.getProperty("os.name").toLowerCase(Locale.ROOT).contains("win");
        if (isWindows) {
            String bad = "&|><^%!\r\n\"'";
            for (int i = 0; i < bad.length(); i++) {
                if (filename.indexOf(bad.charAt(i)) >= 0) return false;
            }
        }
        return true;
    }

    private static String readAll(InputStream is) throws IOException {
        StringBuilder sb = new StringBuilder();
        try (BufferedReader br = new BufferedReader(new InputStreamReader(is, StandardCharsets.UTF_8))) {
            char[] buf = new char[4096];
            int n;
            while ((n = br.read(buf)) != -1) {
                sb.append(buf, 0, n);
            }
        }
        return sb.toString();
    }

    // Main with 5 test cases
    public static void main(String[] args) throws Exception {
        // Prepare test files
        Path tempRoot = Files.createTempDirectory("task39-java-");
        Path f1 = tempRoot.resolve("file1.txt");
        Files.writeString(f1, "Hello from file1\nLine2\n", StandardCharsets.UTF_8);

        Path f2 = tempRoot.resolve("file with spaces.txt");
        Files.writeString(f2, "Content with spaces in filename\n", StandardCharsets.UTF_8);

        Path f3 = tempRoot.resolve("-leadingDash.txt");
        Files.writeString(f3, "Filename starts with dash\n", StandardCharsets.UTF_8);

        Path f4 = tempRoot.resolve("normal-_.txt");
        Files.writeString(f4, "Normal file name content\n", StandardCharsets.UTF_8);

        String f5 = tempRoot.resolve("does_not_exist.txt").toString();

        System.out.println("Test1:\n" + displayFileSecure(f1.toString()));
        System.out.println("Test2:\n" + displayFileSecure(f2.toString()));
        System.out.println("Test3:\n" + displayFileSecure(f3.toString()));
        System.out.println("Test4:\n" + displayFileSecure(f4.toString()));
        System.out.println("Test5:\n" + displayFileSecure(f5));
    }
}