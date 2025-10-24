import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.*;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

public class Task117 {

    public static List<String> extractArchive(String archivePath, String destDir) throws IOException {
        Objects.requireNonNull(archivePath, "archivePath");
        Objects.requireNonNull(destDir, "destDir");
        String lower = archivePath.toLowerCase(Locale.ROOT);
        if (lower.endsWith(".zip")) {
            return extractZip(archivePath, destDir);
        } else if (lower.endsWith(".tar")) {
            return extractTar(archivePath, destDir);
        } else {
            throw new IOException("Unsupported archive type: " + archivePath);
        }
    }

    public static List<String> extractZip(String zipPath, String destDir) throws IOException {
        Path dest = Paths.get(destDir).toAbsolutePath().normalize();
        Files.createDirectories(dest);

        List<String> extracted = new ArrayList<>();
        try (InputStream fis = new BufferedInputStream(Files.newInputStream(Paths.get(zipPath)));
             ZipInputStream zis = new ZipInputStream(fis, StandardCharsets.UTF_8)) {

            ZipEntry entry;
            byte[] buffer = new byte[8192];
            while ((entry = zis.getNextEntry()) != null) {
                String name = entry.getName();
                if (name == null || name.isEmpty()) {
                    zis.closeEntry();
                    continue;
                }
                // Disallow absolute paths and backslashes normalization
                name = name.replace('\\', '/');
                if (name.startsWith("/")) {
                    zis.closeEntry();
                    continue;
                }
                // Normalize path to prevent traversal
                Path outPath = dest.resolve(name).normalize();
                if (!outPath.startsWith(dest)) {
                    zis.closeEntry();
                    continue;
                }
                if (entry.isDirectory()) {
                    Files.createDirectories(outPath);
                } else {
                    // Ensure parent directories exist
                    Path parent = outPath.getParent();
                    if (parent != null) {
                        Files.createDirectories(parent);
                    }
                    try (OutputStream os = new BufferedOutputStream(Files.newOutputStream(outPath, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING))) {
                        int len;
                        while ((len = zis.read(buffer)) > 0) {
                            os.write(buffer, 0, len);
                        }
                    }
                    extracted.add(outPath.toString());
                }
                zis.closeEntry();
            }
        }
        return extracted;
    }

    // Minimal TAR extraction (uncompressed TAR only)
    public static List<String> extractTar(String tarPath, String destDir) throws IOException {
        Path dest = Paths.get(destDir).toAbsolutePath().normalize();
        Files.createDirectories(dest);
        List<String> extracted = new ArrayList<>();

        try (InputStream is = new BufferedInputStream(Files.newInputStream(Paths.get(tarPath)))) {
            byte[] header = new byte[512];
            while (true) {
                int read = readFully(is, header, 0, 512);
                if (read == -1) {
                    break; // EOF
                }
                if (read < 512) {
                    throw new IOException("Truncated TAR header");
                }
                if (isAllZero(header)) {
                    // Two consecutive zero blocks signal end of archive; consume next zero block if exists
                    int next = readFully(is, header, 0, 512);
                    break;
                }
                String name = getString(header, 0, 100);
                String prefix = getString(header, 345, 155);
                char typeflag = (char) header[156];
                long size = parseOctal(header, 124, 12);

                if (!prefix.isEmpty()) {
                    name = prefix + "/" + name;
                }
                name = name.replace('\\', '/');
                if (name.startsWith("/")) {
                    skipFully(is, roundUp512(size));
                    continue;
                }
                // Normalize path
                Path outPath = dest.resolve(name).normalize();
                if (!outPath.startsWith(dest)) {
                    skipFully(is, roundUp512(size));
                    continue;
                }

                if (typeflag == '5' || name.endsWith("/")) { // directory
                    Files.createDirectories(outPath);
                } else if (typeflag == '0' || typeflag == 0) { // regular file
                    Path parent = outPath.getParent();
                    if (parent != null) {
                        Files.createDirectories(parent);
                    }
                    try (OutputStream os = new BufferedOutputStream(Files.newOutputStream(outPath, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING))) {
                        copyN(is, os, size);
                    }
                    extracted.add(outPath.toString());
                    // Skip padding
                    long padding = roundUp512(size) - size;
                    if (padding > 0) skipFully(is, padding);
                } else {
                    // Skip other entry types (symlink, hardlink, etc.)
                    skipFully(is, roundUp512(size));
                }
            }
        }
        return extracted;
    }

    private static int readFully(InputStream is, byte[] buf, int off, int len) throws IOException {
        int total = 0;
        while (total < len) {
            int r = is.read(buf, off + total, len - total);
            if (r < 0) {
                return total == 0 ? -1 : total;
            }
            total += r;
        }
        return total;
    }

    private static boolean isAllZero(byte[] buf) {
        for (byte b : buf) {
            if (b != 0) return false;
        }
        return true;
    }

    private static String getString(byte[] buf, int off, int len) {
        int end = off;
        int max = off + len;
        while (end < max && buf[end] != 0) end++;
        return new String(buf, off, end - off, StandardCharsets.US_ASCII).trim();
    }

    private static long parseOctal(byte[] buf, int off, int len) {
        long result = 0;
        int end = off + len;
        int i = off;
        // skip leading spaces and nulls
        while (i < end && (buf[i] == 0 || buf[i] == ' ')) i++;
        for (; i < end; i++) {
            byte b = buf[i];
            if (b == 0 || b == ' ') break;
            if (b < '0' || b > '7') break;
            result = (result << 3) + (b - '0');
        }
        return result;
    }

    private static long roundUp512(long size) {
        long rem = size % 512;
        return rem == 0 ? size : size + (512 - rem);
    }

    private static void copyN(InputStream is, OutputStream os, long n) throws IOException {
        byte[] buffer = new byte[8192];
        long remaining = n;
        while (remaining > 0) {
            int toRead = (int) Math.min(buffer.length, remaining);
            int r = is.read(buffer, 0, toRead);
            if (r < 0) throw new EOFException("Unexpected EOF while reading file data");
            os.write(buffer, 0, r);
            remaining -= r;
        }
    }

    public static void main(String[] args) {
        // 5 test cases demonstrating usage; adjust paths as needed.
        List<Runnable> tests = new ArrayList<>();
        try {
            Path base = Files.createTempDirectory("task117_java_tests");

            // Prepare a small ZIP file
            Path zip1 = base.resolve("t1.zip");
            try (OutputStream fos = Files.newOutputStream(zip1);
                 java.util.zip.ZipOutputStream zos = new java.util.zip.ZipOutputStream(fos, StandardCharsets.UTF_8)) {
                zos.putNextEntry(new ZipEntry("a.txt"));
                zos.write("hello zip\n".getBytes(StandardCharsets.UTF_8));
                zos.closeEntry();
                zos.putNextEntry(new ZipEntry("sub/b.txt"));
                zos.write("world\n".getBytes(StandardCharsets.UTF_8));
                zos.closeEntry();
                // traversal attempt - should be skipped on extract
                zos.putNextEntry(new ZipEntry("../evil.txt"));
                zos.write("should not extract".getBytes(StandardCharsets.UTF_8));
                zos.closeEntry();
            }

            // Prepare a small TAR file (uncompressed)
            Path tar1 = base.resolve("t1.tar");
            createSimpleTar(tar1, Map.of(
                "x.txt", "hello tar\n",
                "dir/y.txt", "nested\n"
            ), List.of("../escape.txt")); // traversal attempt will be skipped

            // Destinations
            Path out1 = base.resolve("out_zip1");
            Path out2 = base.resolve("out_tar1");
            Path out3 = base.resolve("out_zip2");
            Path out4 = base.resolve("out_tar2");
            Path out5 = base.resolve("out_unknown");

            tests.add(() -> {
                try {
                    System.out.println("Test1 ZIP safe: " + extractArchive(zip1.toString(), out1.toString()));
                } catch (Exception e) {
                    System.out.println("Test1 failed: " + e.getMessage());
                }
            });
            tests.add(() -> {
                try {
                    System.out.println("Test2 ZIP traversal filtered: " + extractZip(zip1.toString(), out3.toString()));
                } catch (Exception e) {
                    System.out.println("Test2 failed: " + e.getMessage());
                }
            });
            tests.add(() -> {
                try {
                    System.out.println("Test3 TAR safe: " + extractArchive(tar1.toString(), out2.toString()));
                } catch (Exception e) {
                    System.out.println("Test3 failed: " + e.getMessage());
                }
            });
            tests.add(() -> {
                try {
                    System.out.println("Test4 TAR traversal filtered: " + extractTar(tar1.toString(), out4.toString()));
                } catch (Exception e) {
                    System.out.println("Test4 failed: " + e.getMessage());
                }
            });
            tests.add(() -> {
                try {
                    System.out.println("Test5 unsupported: ");
                    System.out.println(extractArchive(base.resolve("file.rar").toString(), out5.toString()));
                } catch (Exception e) {
                    System.out.println("Test5 expected failure: " + e.getMessage());
                }
            });
        } catch (IOException e) {
            System.out.println("Setup failed: " + e.getMessage());
        }

        for (Runnable r : tests) r.run();
    }

    private static void writeTarHeader(OutputStream os, String name, long size, char typeflag) throws IOException {
        byte[] header = new byte[512];
        // name
        byte[] nameBytes = name.getBytes(StandardCharsets.US_ASCII);
        if (nameBytes.length > 100) {
            // try to split into prefix/name
            int idx = name.lastIndexOf('/');
            String prefix = idx > 0 ? name.substring(0, idx) : "";
            String base = idx > 0 ? name.substring(idx + 1) : name;
            byte[] prefixBytes = prefix.getBytes(StandardCharsets.US_ASCII);
            byte[] baseBytes = base.getBytes(StandardCharsets.US_ASCII);
            if (prefixBytes.length <= 155 && baseBytes.length <= 100) {
                System.arraycopy(baseBytes, 0, header, 0, baseBytes.length);
                System.arraycopy(prefixBytes, 0, header, 345, prefixBytes.length);
            } else {
                throw new IOException("TAR name too long: " + name);
            }
        } else {
            System.arraycopy(nameBytes, 0, header, 0, nameBytes.length);
        }
        // mode
        writeOctal(header, 100, 8, 0644);
        // uid/gid
        writeOctal(header, 108, 8, 0);
        writeOctal(header, 116, 8, 0);
        // size
        writeOctal(header, 124, 12, size);
        // mtime
        writeOctal(header, 136, 12, System.currentTimeMillis() / 1000);
        // checksum placeholder
        for (int i = 148; i < 156; i++) header[i] = ' ';
        // typeflag
        header[156] = (byte) typeflag;
        // magic + version
        byte[] magic = "ustar".getBytes(StandardCharsets.US_ASCII);
        System.arraycopy(magic, 0, header, 257, magic.length);
        header[262] = '0';
        header[263] = '0';
        // uname/gname
        byte[] uname = "user".getBytes(StandardCharsets.US_ASCII);
        System.arraycopy(uname, 0, header, 265, Math.min(32, uname.length));
        byte[] gname = "group".getBytes(StandardCharsets.US_ASCII);
        System.arraycopy(gname, 0, header, 297, Math.min(32, gname.length));

        long sum = 0;
        for (byte b : header) sum += (b & 0xFF);
        writeOctal(header, 148, 8, sum);

        os.write(header);
    }

    private static void writeOctal(byte[] buf, int off, int len, long value) {
        String s = Long.toOctalString(value);
        int pad = len - 1 - s.length();
        for (int i = 0; i < pad; i++) buf[off + i] = (byte) '0';
        for (int i = 0; i < s.length(); i++) buf[off + pad + i] = (byte) s.charAt(i);
        buf[off + len - 1] = 0;
    }

    private static void createSimpleTar(Path tarPath, Map<String, String> files, List<String> badNames) throws IOException {
        try (OutputStream os = new BufferedOutputStream(Files.newOutputStream(tarPath))) {
            for (Map.Entry<String, String> e : files.entrySet()) {
                String name = e.getKey().replace('\\', '/');
                byte[] data = e.getValue().getBytes(StandardCharsets.UTF_8);
                writeTarHeader(os, name, data.length, '0');
                os.write(data);
                long pad = roundUp512(data.length) - data.length;
                if (pad > 0) os.write(new byte[(int) pad]);
            }
            for (String bad : badNames) {
                byte[] data = "bad".getBytes(StandardCharsets.UTF_8);
                writeTarHeader(os, bad, data.length, '0');
                os.write(data);
                long pad = roundUp512(data.length) - data.length;
                if (pad > 0) os.write(new byte[(int) pad]);
            }
            // two zero blocks
            os.write(new byte[1024]);
        }
    }
}