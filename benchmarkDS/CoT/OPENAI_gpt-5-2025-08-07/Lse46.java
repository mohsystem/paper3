import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.core.io.InputStreamResource;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.io.IOException;
import java.io.InputStream;
import java.nio.file.*;
import java.util.HashSet;
import java.util.Locale;
import java.util.Set;

@SpringBootApplication
@RestController
public class Lse46 {

    private final Path imageDir = Paths.get("images").toAbsolutePath().normalize();
    private static final Set<String> ALLOWED_EXT = new HashSet<>();

    static {
        ALLOWED_EXT.add("png");
        ALLOWED_EXT.add("jpg");
        ALLOWED_EXT.add("jpeg");
        ALLOWED_EXT.add("gif");
        ALLOWED_EXT.add("webp");
        ALLOWED_EXT.add("bmp");
        ALLOWED_EXT.add("svg");
    }

    private boolean isValidName(String name) {
        if (name == null || name.isEmpty()) return false;
        String base = Paths.get(name).getFileName().toString();
        if (!name.equals(base)) return false; // prevent path traversal attempts
        if (base.contains("/") || base.contains("\\") || base.contains("..")) return false;
        int dot = base.lastIndexOf('.');
        if (dot <= 0 || dot == base.length() - 1) return false;
        String ext = base.substring(dot + 1).toLowerCase(Locale.ROOT);
        return ALLOWED_EXT.contains(ext);
    }

    @GetMapping("/img")
    public ResponseEntity<InputStreamResource> img(@RequestParam("name") String name) throws IOException {
        if (!isValidName(name)) {
            return ResponseEntity.badRequest().build();
        }
        String base = Paths.get(name).getFileName().toString();
        Path p = imageDir.resolve(base).normalize();
        if (!p.startsWith(imageDir)) {
            return ResponseEntity.badRequest().build();
        }
        if (!Files.exists(p) || !Files.isRegularFile(p)) {
            return ResponseEntity.notFound().build();
        }
        InputStream is = Files.newInputStream(p, StandardOpenOption.READ);
        String ct = Files.probeContentType(p);
        if (ct == null || ct.isBlank()) {
            // simple mapping fallback
            String lower = base.toLowerCase(Locale.ROOT);
            if (lower.endsWith(".png")) ct = "image/png";
            else if (lower.endsWith(".jpg") || lower.endsWith(".jpeg")) ct = "image/jpeg";
            else if (lower.endsWith(".gif")) ct = "image/gif";
            else if (lower.endsWith(".webp")) ct = "image/webp";
            else if (lower.endsWith(".bmp")) ct = "image/bmp";
            else if (lower.endsWith(".svg")) ct = "image/svg+xml";
            else ct = "application/octet-stream";
        }
        return ResponseEntity.ok()
                .contentType(MediaType.parseMediaType(ct))
                .body(new InputStreamResource(is));
    }

    // Helper to create sample files for tests
    private static void createSampleFiles(Path dir) throws IOException {
        if (!Files.exists(dir)) {
            Files.createDirectories(dir);
        }
        Path p1 = dir.resolve("sample.png");
        Path p2 = dir.resolve("logo.jpg");
        if (!Files.exists(p1)) {
            // minimal fake PNG bytes
            byte[] png = new byte[] {(byte)0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00};
            Files.write(p1, png, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);
        }
        if (!Files.exists(p2)) {
            byte[] jpg = new byte[] {(byte)0xFF, (byte)0xD8, (byte)0xFF, 0x00, 0x11, 0x22};
            Files.write(p2, jpg, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);
        }
    }

    public static void main(String[] args) throws Exception {
        Lse46 controller = new Lse46();
        createSampleFiles(controller.imageDir);

        String[] tests = new String[] {
                "sample.png",
                "logo.jpg",
                "../secret.png",       // invalid traversal
                "notfound.png",        // missing
                "subdir/evil.png"      // invalid path
        };

        for (String t : tests) {
            try {
                ResponseEntity<InputStreamResource> resp = controller.img(t);
                int code = resp.getStatusCode().value();
                long size = -1;
                if (resp.getBody() != null && resp.getStatusCode().is2xxSuccessful()) {
                    try (InputStream in = resp.getBody().getInputStream()) {
                        long n = 0;
                        byte[] buf = new byte[4096];
                        int r;
                        while ((r = in.read(buf)) != -1) n += r;
                        size = n;
                    }
                }
                System.out.println("Test name=" + t + " -> HTTP " + code + (size >= 0 ? (", bytes=" + size) : ""));
            } catch (Exception ex) {
                System.out.println("Test name=" + t + " -> exception: " + ex.getMessage());
            }
        }
    }
}