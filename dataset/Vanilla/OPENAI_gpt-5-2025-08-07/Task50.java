import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.Random;
import java.util.UUID;

public class Task50 {
    private static final String UPLOAD_DIR = "uploads";

    public static String uploadFile(String filename, byte[] data) {
        try {
            Files.createDirectories(Paths.get(UPLOAD_DIR));
            String safe = sanitizeFilename(filename);
            String uniqueName = System.currentTimeMillis() + "_" + UUID.randomUUID().toString().replace("-", "") + "_" + safe;
            File target = new File(UPLOAD_DIR, uniqueName);
            try (FileOutputStream fos = new FileOutputStream(target)) {
                fos.write(data);
            }
            return "OK: stored as " + target.getPath() + " (name=" + filename + ", size=" + data.length + " bytes)";
        } catch (IOException e) {
            return "ERROR: " + e.getMessage();
        }
    }

    private static String sanitizeFilename(String name) {
        if (name == null) name = "file";
        String safe = name.replaceAll("[^A-Za-z0-9._-]", "_");
        if (safe.isEmpty()) safe = "file";
        if (safe.length() > 200) {
            safe = safe.substring(0, 200);
        }
        return safe;
    }

    public static void main(String[] args) {
        // 5 test cases
        String r1 = uploadFile("hello.txt", "Hello, world!".getBytes(StandardCharsets.UTF_8));
        System.out.println(r1);

        byte[] imgMock = new byte[256];
        new Random().nextBytes(imgMock);
        String r2 = uploadFile("image.png", imgMock);
        System.out.println(r2);

        String r3 = uploadFile("../report.pdf", "This is a mock PDF content.".getBytes(StandardCharsets.UTF_8));
        System.out.println(r3);

        byte[] dataBin = new byte[1024];
        new Random().nextBytes(dataBin);
        String r4 = uploadFile("data.bin", dataBin);
        System.out.println(r4);

        String longName = "resume_with_a_very_long_name_that_should_be_sanitized_and_truncated_if_needed_because_it_might_be_too_long_for_some_file_systems.docx";
        String r5 = uploadFile(longName, "Resume content bytes".getBytes(StandardCharsets.UTF_8));
        System.out.println(r5);
    }
}