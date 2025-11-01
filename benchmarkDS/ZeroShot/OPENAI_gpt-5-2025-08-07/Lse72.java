import java.io.*;
import java.net.URI;
import java.nio.file.*;
import java.util.*;
import java.util.regex.Pattern;

public class Lse72 {
    private static final Set<String> WINDOWS_RESERVED = new HashSet<>(Arrays.asList(
            "CON","PRN","AUX","NUL",
            "COM1","COM2","COM3","COM4","COM5","COM6","COM7","COM8","COM9",
            "LPT1","LPT2","LPT3","LPT4","LPT5","LPT6","LPT7","LPT8","LPT9"
    ));
    private static final Pattern ALLOWED = Pattern.compile("[A-Za-z0-9._-]");

    private static String sanitizeFilename(String filename) {
        if (filename == null) filename = "";
        filename = filename.replace("\u0000", "");
        String base = new File(filename).getName(); // strips directories
        if (base.isEmpty()) base = "upload.bin";

        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < base.length(); i++) {
            String ch = String.valueOf(base.charAt(i));
            if (ALLOWED.matcher(ch).matches()) {
                sb.append(ch);
            } else {
                sb.append('_');
            }
        }
        String cleaned = sb.toString();
        if (cleaned.isEmpty()) cleaned = "upload.bin";

        // prevent leading dot which could create hidden files on Unix
        if (cleaned.startsWith(".")) cleaned = "_" + cleaned.substring(1);

        // Split base and ext
        int lastDot = cleaned.lastIndexOf('.');
        String namePart = cleaned;
        String extPart = "";
        if (lastDot > 0 && lastDot < cleaned.length() - 1) {
            namePart = cleaned.substring(0, lastDot);
            extPart = cleaned.substring(lastDot); // includes dot
        }

        // Windows reserved names check
        String upper = namePart.toUpperCase(Locale.ROOT);
        if (WINDOWS_RESERVED.contains(upper)) {
            namePart = "_" + namePart;
        }

        // limit total length to 255
        int maxTotal = 255;
        int availableForName = Math.max(1, maxTotal - extPart.length());
        if (namePart.length() > availableForName) {
            namePart = namePart.substring(0, availableForName);
        }

        if (namePart.isEmpty()) namePart = "upload";
        // If no extension at all, consider default
        if (extPart.isEmpty()) extPart = ".bin";

        return namePart + extPart;
    }

    private static Path resolveProgramDir() {
        try {
            URI uri = Lse72.class.getProtectionDomain().getCodeSource().getLocation().toURI();
            Path p = Paths.get(uri).toAbsolutePath();
            Path parent = p.getParent();
            if (parent != null) return parent;
        } catch (Exception ignored) {}
        return Paths.get(System.getProperty("user.dir")).toAbsolutePath();
    }

    public static String uploadFile(byte[] data, String filename) throws IOException {
        if (data == null) throw new IllegalArgumentException("data cannot be null");
        String safeName = sanitizeFilename(filename);
        Path baseDir = resolveProgramDir();
        Path uploadsDir = baseDir.resolve("uploads");
        Files.createDirectories(uploadsDir);

        int counter = 0;
        String namePart;
        String extPart;
        int dot = safeName.lastIndexOf('.');
        if (dot > 0 && dot < safeName.length() - 1) {
            namePart = safeName.substring(0, dot);
            extPart = safeName.substring(dot);
        } else {
            namePart = safeName;
            extPart = "";
        }

        while (true) {
            String candidateName = (counter == 0) ? (namePart + extPart) : (namePart + "-" + counter + extPart);
            Path candidate = uploadsDir.resolve(candidateName);
            try (OutputStream os = Files.newOutputStream(candidate, StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE)) {
                os.write(data);
                return candidate.toAbsolutePath().toString();
            } catch (FileAlreadyExistsException ex) {
                counter++;
            }
        }
    }

    public static void main(String[] args) {
        try {
            byte[] data1 = "Hello, world!".getBytes();
            byte[] data2 = "Another content".getBytes();
            byte[] data3 = "Malicious path attempt".getBytes();
            byte[] data4 = "Absolute path attempt".getBytes();
            byte[] data5 = "Special chars & spaces".getBytes();

            String r1 = uploadFile(data1, "greeting.txt");
            String r2 = uploadFile(data2, "../evil.txt");
            String r3 = uploadFile(data3, "/etc/passwd");
            String r4 = uploadFile(data4, "COM1"); // Windows reserved
            String r5 = uploadFile(data5, "my file (final)!.pdf");

            System.out.println(r1);
            System.out.println(r2);
            System.out.println(r3);
            System.out.println(r4);
            System.out.println(r5);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}