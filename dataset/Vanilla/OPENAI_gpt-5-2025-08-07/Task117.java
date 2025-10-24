import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.Closeable;
import java.io.DataOutputStream;
import java.io.EOFException;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.InvalidPathException;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.zip.GZIPInputStream;
import java.util.zip.GZIPOutputStream;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipOutputStream;

public class Task117 {

    public static List<String> extractArchive(String archivePath, String destDir) throws IOException {
        Path archive = Paths.get(archivePath);
        Path dest = Paths.get(destDir);
        Files.createDirectories(dest);

        String name = archive.getFileName().toString().toLowerCase();
        if (name.endsWith(".zip")) {
            return extractZip(archive, dest);
        } else if (name.endsWith(".tar")) {
            try (InputStream in = new BufferedInputStream(new FileInputStream(archive.toFile()))) {
                return extractTar(in, dest);
            }
        } else if (name.endsWith(".tar.gz") || name.endsWith(".tgz")) {
            try (InputStream in = new BufferedInputStream(new GZIPInputStream(new FileInputStream(archive.toFile())))) {
                return extractTar(in, dest);
            }
        } else {
            throw new IOException("Unsupported archive type: " + archivePath);
        }
    }

    private static List<String> extractZip(Path archive, Path dest) throws IOException {
        List<String> extracted = new ArrayList<>();
        try (ZipInputStream zis = new ZipInputStream(new BufferedInputStream(new FileInputStream(archive.toFile())))) {
            ZipEntry entry;
            byte[] buffer = new byte[8192];
            while ((entry = zis.getNextEntry()) != null) {
                String entryName = entry.getName();
                Path target = safeResolve(dest, entryName);
                if (target == null) {
                    // Skip zip-slip entry
                    zis.closeEntry();
                    continue;
                }
                if (entry.isDirectory() || entryName.endsWith("/")) {
                    Files.createDirectories(target);
                } else {
                    Files.createDirectories(target.getParent());
                    try (OutputStream out = new BufferedOutputStream(Files.newOutputStream(target, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING))) {
                        int read;
                        while ((read = zis.read(buffer)) != -1) {
                            out.write(buffer, 0, read);
                        }
                    }
                }
                extracted.add(dest.relativize(target).toString().replace('\\', '/'));
                zis.closeEntry();
            }
        }
        return extracted;
    }

    private static List<String> extractTar(InputStream in, Path dest) throws IOException {
        List<String> extracted = new ArrayList<>();
        BufferedInputStream bin = new BufferedInputStream(in);
        byte[] header = new byte[512];
        String pendingLongName = null;

        while (true) {
            int n = readFully(bin, header, 0, 512);
            if (n == -1) {
                break;
            }
            if (n < 512) {
                throw new EOFException("Unexpected EOF in tar header");
            }
            if (isAllZero(header)) {
                // End of archive (tar typically has two 512-byte zero blocks)
                break;
            }

            String name = getString(header, 0, 100);
            String prefix = getString(header, 345, 155);
            if (!prefix.isEmpty()) {
                name = (prefix + "/" + name).replaceAll("/{2,}", "/");
            }
            long size = parseOctal(header, 124, 12);
            byte typeflag = header[156];
            if (pendingLongName != null) {
                name = pendingLongName;
                pendingLongName = null;
            }

            if (typeflag == 'L') {
                // GNU longname: next data block contains file name
                byte[] data = readData(bin, size);
                pendingLongName = new String(data, 0, (int)size).trim();
                // Skip padding to 512
                skipPadding(bin, size);
                continue;
            } else if (typeflag == 'x') {
                // PAX extended header: ignore content
                skip(bin, size);
                skipPadding(bin, size);
                continue;
            }

            Path target = name.isEmpty() ? null : safeResolve(dest, name);
            if (typeflag == '5') { // directory
                if (target != null) {
                    Files.createDirectories(target);
                    extracted.add(dest.relativize(target).toString().replace('\\', '/'));
                }
                // Directories have no data to read (size should be 0), but still align to block
                skip(bin, size);
                skipPadding(bin, size);
            } else if (typeflag == '0' || typeflag == 0) { // regular file
                if (target != null) {
                    Files.createDirectories(target.getParent());
                    try (OutputStream out = new BufferedOutputStream(Files.newOutputStream(target, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING))) {
                        copy(bin, out, size);
                    }
                    extracted.add(dest.relativize(target).toString().replace('\\', '/'));
                } else {
                    // Skip content if unsafe path
                    skip(bin, size);
                }
                skipPadding(bin, size);
            } else {
                // Other types (symlink '2', hardlink '1', etc.) are ignored: skip payload and padding
                skip(bin, size);
                skipPadding(bin, size);
            }
        }

        return extracted;
    }

    // Utilities

    private static Path safeResolve(Path base, String entryName) {
        try {
            // Normalize separators and remove leading slashes
            String cleaned = entryName.replace('\\', '/');
            while (cleaned.startsWith("/")) cleaned = cleaned.substring(1);
            Path target = base.resolve(cleaned).normalize();
            if (!target.startsWith(base.normalize())) {
                return null; // zip-slip attempt
            }
            return target;
        } catch (InvalidPathException e) {
            return null;
        }
    }

    private static boolean isAllZero(byte[] block) {
        for (byte b : block) {
            if (b != 0) return false;
        }
        return true;
    }

    private static String getString(byte[] buf, int off, int len) {
        int end = off + len;
        int i = off;
        while (i < end && buf[i] != 0) i++;
        String s = new String(buf, off, i - off, StandardCharsets.US_ASCII);
        return s.trim();
    }

    private static long parseOctal(byte[] buf, int off, int len) {
        long val = 0;
        int end = off + len;
        int i = off;
        // Skip leading spaces or nulls
        while (i < end && (buf[i] == 0 || buf[i] == ' ')) i++;
        for (; i < end; i++) {
            byte b = buf[i];
            if (b == 0 || b == ' ') break;
            if (b < '0' || b > '7') break;
            val = (val << 3) + (b - '0');
        }
        return val;
    }

    private static int readFully(InputStream in, byte[] buf, int off, int len) throws IOException {
        int total = 0;
        while (total < len) {
            int n = in.read(buf, off + total, len - total);
            if (n == -1) {
                return total == 0 ? -1 : total;
            }
            total += n;
        }
        return total;
    }

    private static byte[] readData(InputStream in, long size) throws IOException {
        if (size > Integer.MAX_VALUE) throw new IOException("Entry too large");
        byte[] data = new byte[(int) size];
        int read = readFully(in, data, 0, (int) size);
        if (read < size) throw new EOFException("Unexpected EOF in tar entry");
        return data;
    }

    private static void copy(InputStream in, OutputStream out, long size) throws IOException {
        byte[] buf = new byte[8192];
        long remaining = size;
        while (remaining > 0) {
            int toRead = (int) Math.min(buf.length, remaining);
            int n = in.read(buf, 0, toRead);
            if (n == -1) throw new EOFException("Unexpected EOF in tar entry");
            out.write(buf, 0, n);
            remaining -= n;
        }
    }

    private static void skip(InputStream in, long size) throws IOException {
        long remaining = size;
        while (remaining > 0) {
            long skipped = in.skip(remaining);
            if (skipped <= 0) {
                // Fall back to reading a byte
                if (in.read() == -1) throw new EOFException("Unexpected EOF in tar skip");
                skipped = 1;
            }
            remaining -= skipped;
        }
    }

    private static void skipPadding(InputStream in, long size) throws IOException {
        long padding = (512 - (size % 512)) % 512;
        if (padding > 0) skip(in, padding);
    }

    // Helper to create sample archives for testing
    private static void createSampleZip(Path zipPath) throws IOException {
        Files.createDirectories(zipPath.getParent());
        try (ZipOutputStream zos = new ZipOutputStream(new BufferedOutputStream(new FileOutputStream(zipPath.toFile())))) {
            addZipEntry(zos, "dirA/", null);
            addZipEntry(zos, "dirA/file1.txt", "Hello from ZIP file1");
            addZipEntry(zos, "file2.txt", "Hello from ZIP file2");
        }
    }

    private static void createZipWithZipSlip(Path zipPath) throws IOException {
        Files.createDirectories(zipPath.getParent());
        try (ZipOutputStream zos = new ZipOutputStream(new BufferedOutputStream(new FileOutputStream(zipPath.toFile())))) {
            addZipEntry(zos, "../evil.txt", "You should not see me outside the dest dir");
            addZipEntry(zos, "safe.txt", "Safe file");
        }
    }

    private static void addZipEntry(ZipOutputStream zos, String name, String content) throws IOException {
        ZipEntry ze = new ZipEntry(name);
        zos.putNextEntry(ze);
        if (content != null && !name.endsWith("/")) {
            byte[] data = content.getBytes(StandardCharsets.UTF_8);
            zos.write(data);
        }
        zos.closeEntry();
    }

    private static class TarWriter implements Closeable {
        private final OutputStream out;
        private boolean closed = false;

        TarWriter(OutputStream out) {
            this.out = new BufferedOutputStream(out);
        }

        void addDirectory(String name, long mtime) throws IOException {
            if (!name.endsWith("/")) name = name + "/";
            writeHeader(name, 0, '5', mtime);
            padTo512(0);
        }

        void addFile(String name, byte[] data, long mtime) throws IOException {
            writeHeader(name, data.length, '0', mtime);
            out.write(data);
            padTo512(data.length);
        }

        private void writeHeader(String name, long size, char typeflag, long mtime) throws IOException {
            byte[] header = new byte[512];
            // name field (or split into prefix/name if long)
            byte[] nameBytes = name.getBytes(StandardCharsets.US_ASCII);
            String useName = name;
            String prefix = "";
            if (nameBytes.length > 100) {
                // Try to split at last '/'
                int idx = name.lastIndexOf('/');
                if (idx > 0) {
                    prefix = name.substring(0, idx);
                    useName = name.substring(idx + 1);
                }
            }
            putString(header, 0, 100, useName);
            putOctal(header, 100, 8, 0644);
            putOctal(header, 108, 8, 0);
            putOctal(header, 116, 8, 0);
            putOctal(header, 124, 12, size);
            putOctal(header, 136, 12, mtime <= 0 ? (System.currentTimeMillis() / 1000L) : mtime);
            // chksum initially spaces
            for (int i = 148; i < 156; i++) header[i] = (byte) ' ';
            header[156] = (byte) typeflag;
            // magic and version
            putStringRaw(header, 257, "ustar\u0000");
            putStringRaw(header, 263, "00");
            // uname/gname optional
            putString(header, 265, 32, "user");
            putString(header, 297, 32, "group");
            // prefix
            if (!prefix.isEmpty()) {
                putString(header, 345, 155, prefix);
            }
            long chksum = 0;
            for (byte b : header) chksum += (b & 0xFF);
            putOctal(header, 148, 8, chksum);
            out.write(header);
        }

        private void putString(byte[] buf, int off, int len, String s) {
            byte[] bytes = s.getBytes(StandardCharsets.US_ASCII);
            int n = Math.min(len, bytes.length);
            System.arraycopy(bytes, 0, buf, off, n);
        }

        private void putStringRaw(byte[] buf, int off, String s) {
            byte[] bytes = s.getBytes(StandardCharsets.US_ASCII);
            System.arraycopy(bytes, 0, buf, off, Math.min(bytes.length, buf.length - off));
        }

        private void putOctal(byte[] buf, int off, int len, long value) {
            String s = Long.toOctalString(value);
            // leave space for null
            int n = len - 1;
            int start = off + n - s.length();
            Arrays.fill(buf, off, off + n, (byte) '0');
            if (start < off) start = off;
            int idx = 0;
            for (int i = start; i < off + n && idx < s.length(); i++) {
                buf[i] = (byte) s.charAt(idx++);
            }
            buf[off + len - 1] = 0;
        }

        private void padTo512(long size) throws IOException {
            int pad = (int) ((512 - (size % 512)) % 512);
            if (pad > 0) {
                byte[] zeros = new byte[pad];
                out.write(zeros);
            }
        }

        @Override
        public void close() throws IOException {
            if (closed) return;
            closed = true;
            // Two 512-byte zero blocks
            byte[] zeros = new byte[1024];
            out.write(zeros);
            out.flush();
            out.close();
        }
    }

    private static void createSampleTar(Path tarPath) throws IOException {
        Files.createDirectories(tarPath.getParent());
        try (TarWriter tw = new TarWriter(new FileOutputStream(tarPath.toFile()))) {
            tw.addDirectory("dirB", System.currentTimeMillis() / 1000L);
            tw.addFile("dirB/file3.txt", "Hello from TAR file3".getBytes(StandardCharsets.UTF_8), System.currentTimeMillis() / 1000L);
            tw.addFile("file4.txt", "Hello from TAR file4".getBytes(StandardCharsets.UTF_8), System.currentTimeMillis() / 1000L);
        }
    }

    private static void createSampleTarGz(Path tarGzPath) throws IOException {
        Files.createDirectories(tarGzPath.getParent());
        try (TarWriter tw = new TarWriter(new GZIPOutputStream(new FileOutputStream(tarGzPath.toFile())))) {
            tw.addDirectory("dirC", System.currentTimeMillis() / 1000L);
            tw.addFile("dirC/file5.txt", "Hello from TAR.GZ file5".getBytes(StandardCharsets.UTF_8), System.currentTimeMillis() / 1000L);
            tw.addFile("file6.txt", "Hello from TAR.GZ file6".getBytes(StandardCharsets.UTF_8), System.currentTimeMillis() / 1000L);
        }
    }

    // Main with 5 test cases
    public static void main(String[] args) throws Exception {
        Path base = Files.createTempDirectory("task117");
        Path archives = base.resolve("archives");
        Path outputs = base.resolve("outputs");
        Files.createDirectories(archives);
        Files.createDirectories(outputs);

        Path zip1 = archives.resolve("sample1.zip");
        Path tar1 = archives.resolve("sample2.tar");
        Path tgz1 = archives.resolve("sample3.tar.gz");
        Path zipslip = archives.resolve("zipslip.zip");
        Path missing = archives.resolve("missing.zip");

        createSampleZip(zip1);
        createSampleTar(tar1);
        createSampleTarGz(tgz1);
        createZipWithZipSlip(zipslip);

        // Test case 1: ZIP
        List<String> out1 = extractArchive(zip1.toString(), outputs.resolve("zip1").toString());
        System.out.println("Test1 ZIP extracted: " + out1);

        // Test case 2: TAR
        List<String> out2 = extractArchive(tar1.toString(), outputs.resolve("tar1").toString());
        System.out.println("Test2 TAR extracted: " + out2);

        // Test case 3: TAR.GZ
        List<String> out3 = extractArchive(tgz1.toString(), outputs.resolve("tgz1").toString());
        System.out.println("Test3 TGZ extracted: " + out3);

        // Test case 4: ZIP with zip-slip entry (should skip unsafe paths)
        List<String> out4 = extractArchive(zipslip.toString(), outputs.resolve("zipslip").toString());
        System.out.println("Test4 ZipSlip extracted (unsafe entries skipped): " + out4);

        // Test case 5: Non-existent archive (expect error)
        try {
            List<String> out5 = extractArchive(missing.toString(), outputs.resolve("missing").toString());
            System.out.println("Test5 Missing extracted: " + out5);
        } catch (IOException e) {
            System.out.println("Test5 Missing archive error: " + e.getMessage());
        }

        System.out.println("Artifacts in: " + base.toAbsolutePath());
    }
}