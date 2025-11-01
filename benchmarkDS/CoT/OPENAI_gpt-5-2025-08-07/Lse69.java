// Steps (Chain-of-Through):
// 1) Problem understanding: Provide upload_image() that reads an image, base64-encodes it, and writes the encoded data to a file.
// 2) Security requirements: Validate file existence, size limit, allowed extension, basic signature check, and safe output path.
// 3) Secure coding generation: Implement careful I/O, path normalization, and error handling.
// 4) Code review: Ensure no obvious vulnerabilities (path traversal, oversized input, invalid type).
// 5) Secure code output: Final hardened implementation with tests.

import java.nio.file.*;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.io.*;

public class Lse69 {
    private static final Set<String> ALLOWED_EXTS = new HashSet<>(Arrays.asList(
        ".png", ".jpg", ".jpeg", ".gif", ".bmp", ".webp"
    ));
    private static final long MAX_SIZE = 10L * 1024L * 1024L; // 10 MB

    private static boolean hasAllowedExtension(String name) {
        String lower = name.toLowerCase(Locale.ROOT);
        for (String ext : ALLOWED_EXTS) {
            if (lower.endsWith(ext)) return true;
        }
        return false;
    }

    private static boolean isAllowedSignature(byte[] head) {
        if (head == null || head.length < 12) return false;
        // PNG
        byte[] png = new byte[]{(byte)0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
        boolean isPng = true;
        for (int i = 0; i < png.length; i++) if (head[i] != png[i]) { isPng = false; break; }
        if (isPng) return true;

        // JPEG
        if ((head[0] & 0xFF) == 0xFF && (head[1] & 0xFF) == 0xD8 && (head[2] & 0xFF) == 0xFF) return true;

        // GIF
        String sig6 = new String(Arrays.copyOf(head, 6), StandardCharsets.US_ASCII);
        if ("GIF87a".equals(sig6) || "GIF89a".equals(sig6)) return true;

        // BMP
        if (head[0] == 'B' && head[1] == 'M') return true;

        // WEBP: "RIFF" .... "WEBP"
        if (head[0]=='R' && head[1]=='I' && head[2]=='F' && head[3]=='F' &&
            head[8]=='W' && head[9]=='E' && head[10]=='B' && head[11]=='P') return true;

        return false;
    }

    public static String upload_image(String inputPathStr, String outputPathStr) throws IOException {
        if (inputPathStr == null || outputPathStr == null) throw new IllegalArgumentException("Paths must not be null");

        Path inPath = Paths.get(inputPathStr);
        if (!Files.isRegularFile(inPath)) throw new FileNotFoundException("Input is not a regular file");
        if (!Files.isReadable(inPath)) throw new IOException("Input file is not readable");

        long size = Files.size(inPath);
        if (size <= 0 || size > MAX_SIZE) throw new IOException("Invalid file size");

        String fileName = inPath.getFileName().toString();
        if (!hasAllowedExtension(fileName)) throw new IOException("Disallowed file extension");

        byte[] header = new byte[12];
        try (InputStream is = Files.newInputStream(inPath)) {
            int read = is.read(header);
            if (read < header.length) throw new IOException("File too small or truncated");
        }
        if (!isAllowedSignature(header)) throw new IOException("Invalid or unsupported image signature");

        // Read full file into memory (size already limited)
        byte[] data = Files.readAllBytes(inPath);
        String encoded = Base64.getEncoder().encodeToString(data);

        // Secure output path handling
        Path cwd = Paths.get("").toAbsolutePath().normalize();
        Path outPath = Paths.get(outputPathStr).toAbsolutePath().normalize();
        if (!outPath.startsWith(cwd)) throw new IOException("Output path escapes working directory");

        Path parent = outPath.getParent();
        if (parent != null) Files.createDirectories(parent);

        // Write encoded data atomically when possible
        Path tmp = outPath.resolveSibling(outPath.getFileName().toString() + ".tmp");
        Files.write(tmp, encoded.getBytes(StandardCharsets.US_ASCII), StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE);
        Files.move(tmp, outPath, StandardCopyOption.REPLACE_EXISTING, StandardCopyOption.ATOMIC_MOVE);

        return encoded;
    }

    private static byte[] decodeBase64(String s) {
        return Base64.getDecoder().decode(s);
    }

    private static void writeBytes(Path p, byte[] b) throws IOException {
        Path parent = p.getParent();
        if (parent != null) Files.createDirectories(parent);
        Files.write(p, b, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE);
    }

    public static void main(String[] args) throws Exception {
        // Prepare test data: 1x1 PNG
        String pngB64 = "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mP8/x8AAwMBAe0BqZcAAAAASUVORK5CYII=";
        byte[] pngBytes = decodeBase64(pngB64);

        Path inDir = Paths.get("test_inputs_java").toAbsolutePath().normalize();
        Path outDir = Paths.get("encoded_outputs_java").toAbsolutePath().normalize();
        Files.createDirectories(inDir);
        Files.createDirectories(outDir);

        String[] inNames = {"img1.png", "img2.PNG", "img3.PnG", "img4.jpeg".replace("jpeg","png"), "img5.png"};
        for (String name : inNames) {
            writeBytes(inDir.resolve(name), pngBytes);
        }

        String[] outNames = {"img1.b64", "img2.b64", "img3.b64", "img4.b64", "img5.b64"};

        for (int i = 0; i < 5; i++) {
            Path in = inDir.resolve(inNames[i]);
            Path out = outDir.resolve(outNames[i]);
            String enc = upload_image(in.toString(), out.toString());
            System.out.println("Test " + (i+1) + " -> output: " + out + " (encoded length=" + enc.length() + ")");
        }
    }
}