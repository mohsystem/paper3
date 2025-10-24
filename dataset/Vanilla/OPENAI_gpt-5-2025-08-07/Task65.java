import java.nio.file.*;
import java.nio.charset.StandardCharsets;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class Task65 {
    public static String readFile(String filename) {
        // Validate input
        if (filename == null || filename.trim().isEmpty()) {
            return "ERROR: Filename is empty.";
        }
        if (!isValidFilename(filename)) {
            return "ERROR: Filename contains invalid characters.";
        }

        try {
            Path path = Paths.get(filename);
            if (Files.notExists(path)) {
                return "ERROR: File does not exist.";
            }
            if (Files.isDirectory(path)) {
                return "ERROR: Path is a directory, not a file.";
            }
            if (!Files.isReadable(path)) {
                return "ERROR: File is not readable.";
            }
            return Files.readString(path, StandardCharsets.UTF_8);
        } catch (InvalidPathException ipe) {
            return "ERROR: Invalid path: " + ipe.getMessage();
        } catch (IOException ioe) {
            return "ERROR: I/O error: " + ioe.getMessage();
        } catch (SecurityException se) {
            return "ERROR: Access denied: " + se.getMessage();
        }
    }

    private static boolean isValidFilename(String name) {
        // Disallow control characters and common invalid filename characters
        String invalid = "<>\"|?*";
        for (int i = 0; i < name.length(); i++) {
            char c = name.charAt(i);
            if (c < 32) return false;
            if (invalid.indexOf(c) >= 0) return false;
        }
        // Limit length of individual name component (basic check)
        String[] parts = name.split("[/\\\\]");
        for (String part : parts) {
            if (part.length() > 255) return false;
        }
        return true;
    }

    public static void main(String[] args) {
        // If a filename is provided via command line, read it
        if (args.length > 0) {
            String res = readFile(args[0]);
            System.out.println("CLI arg result:");
            System.out.println(res);
        }

        // Five test cases
        List<String> testFiles = new ArrayList<>();
        try {
            // 1) Valid temp file with simple content
            Path f1 = Files.createTempFile("task65_", ".txt");
            Files.writeString(f1, "Hello from Task65 test case 1.\nLine 2.", StandardCharsets.UTF_8);
            f1.toFile().deleteOnExit();
            testFiles.add(f1.toString());

            // 2) Valid temp file with larger content
            Path f2 = Files.createTempFile("task65_", ".log");
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < 100; i++) sb.append("Line ").append(i + 1).append(": Sample content\n");
            Files.writeString(f2, sb.toString(), StandardCharsets.UTF_8);
            f2.toFile().deleteOnExit();
            testFiles.add(f2.toString());

            // 3) Non-existent file
            testFiles.add("this_file_should_not_exist_1234567890.txt");

            // 4) Empty filename
            testFiles.add("");

            // 5) Invalid filename
            testFiles.add("bad|name.txt");

        } catch (IOException e) {
            System.out.println("Setup error: " + e.getMessage());
        }

        for (int i = 0; i < testFiles.size(); i++) {
            String fn = testFiles.get(i);
            String res = readFile(fn);
            System.out.println("Test " + (i + 1) + " (" + fn + "):");
            System.out.println(res);
        }
    }
}