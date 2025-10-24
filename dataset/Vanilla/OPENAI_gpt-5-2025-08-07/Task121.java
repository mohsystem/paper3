import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;

public class Task121 {

    public static Path uploadFile(Path source, Path destDir, String newName) throws IOException {
        if (!Files.exists(source) || !Files.isRegularFile(source)) {
            throw new IOException("Source file does not exist: " + source);
        }
        if (destDir == null) {
            throw new IOException("Destination directory is null");
        }
        Files.createDirectories(destDir);
        String finalName = (newName == null || newName.isBlank())
                ? source.getFileName().toString()
                : newName;
        Path dest = destDir.resolve(finalName);
        Files.copy(source, dest, StandardCopyOption.REPLACE_EXISTING);
        return dest.toAbsolutePath();
    }

    private static void writeFile(Path path, String content) throws IOException {
        Files.createDirectories(path.getParent());
        Files.write(path, content.getBytes(StandardCharsets.UTF_8), StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);
    }

    public static void main(String[] args) throws Exception {
        Path fixtures = Paths.get("fixtures_java");
        Path uploads = Paths.get("uploads_java");
        Files.createDirectories(fixtures);
        Files.createDirectories(uploads);

        // Create 5 sample files
        Path f1 = fixtures.resolve("file1.txt");
        Path f2 = fixtures.resolve("file2.txt");
        Path f3 = fixtures.resolve("file3.txt");
        Path f4 = fixtures.resolve("data.bin");
        Path f5 = fixtures.resolve("report.md");
        writeFile(f1, "Hello from Java file1");
        writeFile(f2, "Hello from Java file2");
        writeFile(f3, "Hello from Java file3");
        writeFile(f4, "Binary-like content \u0000 \u0001 \u0002");
        writeFile(f5, "# Java Report\nDetails...");

        // 5 test uploads
        System.out.println(uploadFile(f1, uploads, null));
        System.out.println(uploadFile(f2, uploads, "renamed_file2.txt"));
        System.out.println(uploadFile(f3, uploads.resolve("nested1/nested2"), null));
        System.out.println(uploadFile(f4, uploads, "data_copy.bin"));
        System.out.println(uploadFile(f5, uploads.resolve("docs"), "report_copy.md"));
    }
}