import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.*;
import java.util.zip.*;

public class Task117 {

    // Problem understanding and security-aware extraction utilities for ZIP and TAR archives.

    // Secure path resolution to prevent Zip Slip / Tar Slip
    private static Path resolveSecure(Path baseDir, String entryName) throws IOException {
        // Disallow absolute paths and normalize
        Path target = baseDir.resolve(entryName).normalize();
        Path base = baseDir.toAbsolutePath().normalize();
        if (!target.startsWith(base)) {
            throw new IOException("Blocked path traversal attempt: " + entryName);
        }
        return target;
    }

    // Copy stream with a maximum limit to prevent resource exhaustion
    private static long copyWithLimit(InputStream in, OutputStream out, long remainingLimit) throws IOException {
        byte[] buffer = new byte[8192];
        long total = 0;
        int read;
        while ((read = in.read(buffer)) != -1) {
            if (remainingLimit - total < read) {
                // Write only up to the limit portion if needed, then abort
                int allowed = (int) Math.max(0, remainingLimit - total);
                if (allowed > 0) {
                    out.write(buffer, 0, allowed);
                    total += allowed;
                }
                throw new IOException("Extraction aborted: size limit exceeded");
            }
            out.write(buffer, 0, read);
            total += read;
        }
        return total;
    }

    // ZIP extraction
    public static int extractZipSecure(String zipFilePath, String destinationDir, long maxTotalBytes) throws IOException {
        Path zipPath = Paths.get(zipFilePath);
        Path destDir = Paths.get(destinationDir);
        Files.createDirectories(destDir);

        int filesExtracted = 0;
        long totalBytes = 0;

        try (ZipFile zipFile = new ZipFile(zipPath.toFile())) {
            Enumeration<? extends ZipEntry> entries = zipFile.entries();
            while (entries.hasMoreElements()) {
                ZipEntry entry = entries.nextElement();

                // Skip entries with absolute or dangerous names
                String entryName = entry.getName();
                if (entryName.contains("\0")) continue; // NUL byte guard
                if (entryName.startsWith("/") || entryName.startsWith("\\"))
                    continue;

                Path target = resolveSecure(destDir, entryName);

                if (entry.isDirectory()) {
                    Files.createDirectories(target);
                    continue;
                }

                // Ensure parent directory exists
                Files.createDirectories(target.getParent());

                try (InputStream in = new BufferedInputStream(zipFile.getInputStream(entry));
                     OutputStream out = new BufferedOutputStream(Files.newOutputStream(target, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING))) {
                    long written = copyWithLimit(in, out, maxTotalBytes - totalBytes);
                    totalBytes += written;
                    filesExtracted++;
                }
            }
        }
        return filesExtracted;
    }

    // TAR extraction utilities
    private static boolean isAllZero(byte[] buf) {
        for (byte b : buf) {
            if (b != 0) return false;
        }
        return true;
    }

    private static String readString(byte[] header, int off, int len) {
        int end = off;
        int max = off + len;
        while (end < max && header[end] != 0) end++;
        return new String(header, off, end - off, StandardCharsets.US_ASCII);
    }

    private static long readOctal(byte[] header, int off, int len) {
        long val = 0;
        int end = off + len;
        int i = off;
        // Skip leading spaces or NUL
        while (i < end && (header[i] == 0 || header[i] == 32)) i++;
        for (; i < end; i++) {
            byte b = header[i];
            if (b == 0 || b == 32) break;
            if (b < '0' || b > '7') break;
            val = (val << 3) + (b - '0');
        }
        return val;
    }

    public static int extractTarSecure(String tarFilePath, String destinationDir, long maxTotalBytes) throws IOException {
        try (InputStream in = new BufferedInputStream(Files.newInputStream(Paths.get(tarFilePath)))) {
            return extractTarStreamSecure(in, destinationDir, maxTotalBytes);
        }
    }

    public static int extractTarGzSecure(String tarGzFilePath, String destinationDir, long maxTotalBytes) throws IOException {
        try (InputStream fileIn = new BufferedInputStream(Files.newInputStream(Paths.get(tarGzFilePath)));
             InputStream gin = new GZIPInputStream(fileIn)) {
            return extractTarStreamSecure(gin, destinationDir, maxTotalBytes);
        }
    }

    public static int extractTarStreamSecure(InputStream in, String destinationDir, long maxTotalBytes) throws IOException {
        Path destDir = Paths.get(destinationDir);
        Files.createDirectories(destDir);

        int filesExtracted = 0;
        long totalBytes = 0;

        byte[] header = new byte[512];
        byte[] skipBuf = new byte[8192];

        while (true) {
            int read = readFully(in, header, 0, 512);
            if (read == 0) break; // end of stream
            if (read < 512) throw new EOFException("Unexpected EOF in tar header");
            if (isAllZero(header)) {
                // optional second zero block - read and ignore if present
                in.read(header, 0, 512);
                break;
            }

            String name = readString(header, 0, 100);
            String prefix = readString(header, 345, 155);
            char typeflag = (char) header[156];
            long size = readOctal(header, 124, 12);

            String fullName = (prefix != null && !prefix.isEmpty()) ? (prefix + "/" + name) : name;
            if (fullName.startsWith("/") || fullName.startsWith("\\")) {
                // skip absolute entries
                skipFully(in, size + pad512(size), skipBuf);
                continue;
            }

            Path target = null;
            try {
                target = resolveSecure(destDir, fullName);
            } catch (IOException ex) {
                // skip dangerous entry and its data
                skipFully(in, size + pad512(size), skipBuf);
                continue;
            }

            if (typeflag == '5') {
                Files.createDirectories(target);
                // Directory has no data, but size could be 0; still skip padding
                skipFully(in, pad512(size), skipBuf);
            } else if (typeflag == '0' || typeflag == 0) {
                Files.createDirectories(target.getParent());
                try (OutputStream out = new BufferedOutputStream(Files.newOutputStream(target, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING))) {
                    long remaining = size;
                    while (remaining > 0) {
                        int chunk = (int) Math.min(skipBuf.length, remaining);
                        int r = in.read(skipBuf, 0, chunk);
                        if (r < 0) throw new EOFException("Unexpected EOF in tar file content");
                        // enforce max limit
                        if (totalBytes + r > maxTotalBytes) {
                            int allowed = (int) Math.max(0, maxTotalBytes - totalBytes);
                            if (allowed > 0) out.write(skipBuf, 0, allowed);
                            throw new IOException("Extraction aborted: size limit exceeded");
                        }
                        out.write(skipBuf, 0, r);
                        totalBytes += r;
                        remaining -= r;
                    }
                }
                // Skip padding to 512 boundary
                long pad = pad512(size);
                if (pad > 0) skipFully(in, pad, skipBuf);
                filesExtracted++;
            } else {
                // For security, skip symlinks and other types
                skipFully(in, size + pad512(size), skipBuf);
            }
        }

        return filesExtracted;
    }

    private static long pad512(long size) {
        long rem = size % 512;
        return rem == 0 ? 0 : (512 - rem);
    }

    private static int readFully(InputStream in, byte[] buf, int off, int len) throws IOException {
        int total = 0;
        while (total < len) {
            int r = in.read(buf, off + total, len - total);
            if (r < 0) break;
            total += r;
        }
        return total;
    }

    private static void skipFully(InputStream in, long n, byte[] buf) throws IOException {
        long remaining = n;
        while (remaining > 0) {
            int chunk = (int) Math.min(buf.length, remaining);
            int r = in.read(buf, 0, chunk);
            if (r < 0) throw new EOFException("Unexpected EOF while skipping");
            remaining -= r;
        }
    }

    // Helpers to create test archives safely
    private static void createSampleZip(Path zipPath) throws IOException {
        try (ZipOutputStream zos = new ZipOutputStream(new BufferedOutputStream(Files.newOutputStream(zipPath)))) {
            // dir/
            zos.putNextEntry(new ZipEntry("dir/"));
            zos.closeEntry();

            // dir/a.txt
            zos.putNextEntry(new ZipEntry("dir/a.txt"));
            zos.write("Hello from a.txt".getBytes(StandardCharsets.UTF_8));
            zos.closeEntry();

            // b.txt
            zos.putNextEntry(new ZipEntry("b.txt"));
            zos.write("Hello from b.txt".getBytes(StandardCharsets.UTF_8));
            zos.closeEntry();
        }
    }

    private static void createTraversalZip(Path zipPath) throws IOException {
        try (ZipOutputStream zos = new ZipOutputStream(new BufferedOutputStream(Files.newOutputStream(zipPath)))) {
            // Safe file
            zos.putNextEntry(new ZipEntry("safe.txt"));
            zos.write("safe".getBytes(StandardCharsets.UTF_8));
            zos.closeEntry();

            // Malicious traversal
            zos.putNextEntry(new ZipEntry("../evil.txt"));
            zos.write("evil".getBytes(StandardCharsets.UTF_8));
            zos.closeEntry();
        }
    }

    // TAR creation for tests
    private static void writeTarHeader(OutputStream out, String name, long size, char typeflag) throws IOException {
        byte[] hdr = new byte[512];
        // name
        byte[] nameBytes = name.getBytes(StandardCharsets.US_ASCII);
        System.arraycopy(nameBytes, 0, hdr, 0, Math.min(nameBytes.length, 100));
        // mode
        writeOctal(hdr, 100, 8, 0644);
        // uid/gid
        writeOctal(hdr, 108, 8, 0);
        writeOctal(hdr, 116, 8, 0);
        // size
        writeOctal(hdr, 124, 12, size);
        // mtime
        writeOctal(hdr, 136, 12, System.currentTimeMillis() / 1000);
        // chksum: initially spaces
        for (int i = 148; i < 156; i++) hdr[i] = 32;
        // typeflag
        hdr[156] = (byte) typeflag;
        // magic and version
        byte[] magic = "ustar".getBytes(StandardCharsets.US_ASCII);
        System.arraycopy(magic, 0, hdr, 257, magic.length);
        hdr[262] = '0';
        hdr[263] = '0';
        // compute checksum
        long sum = 0;
        for (byte b : hdr) sum += (b & 0xFF);
        writeOctal(hdr, 148, 8, sum);
        out.write(hdr);
    }

    private static void writeOctal(byte[] hdr, int off, int len, long value) {
        String s = Long.toOctalString(value);
        int idx = len - 1;
        // NUL
        hdr[off + idx] = 0;
        idx--;
        // trailing space
        if (idx >= 0) hdr[off + idx] = 32;
        idx--;
        for (int i = s.length() - 1; i >= 0 && idx >= 0; i--, idx--) {
            hdr[off + idx] = (byte) s.charAt(i);
        }
        while (idx >= 0) {
            hdr[off + idx] = '0';
            idx--;
        }
    }

    private static void padTo512(OutputStream out, long size) throws IOException {
        long pad = pad512(size);
        for (long i = 0; i < pad; i++) out.write(0);
    }

    private static void createSampleTar(Path tarPath) throws IOException {
        try (OutputStream out = new BufferedOutputStream(Files.newOutputStream(tarPath))) {
            // dir/
            writeTarHeader(out, "dir/", 0, '5');
            // no data, but padded to block boundary
            // regular file dir/a.txt
            byte[] a = "Hello from tar a.txt".getBytes(StandardCharsets.UTF_8);
            writeTarHeader(out, "dir/a.txt", a.length, '0');
            out.write(a);
            padTo512(out, a.length);

            // b.txt
            byte[] b = "Hello from tar b.txt".getBytes(StandardCharsets.UTF_8);
            writeTarHeader(out, "b.txt", b.length, '0');
            out.write(b);
            padTo512(out, b.length);

            // two 512-byte zero blocks to mark end
            out.write(new byte[512]);
            out.write(new byte[512]);
        }
    }

    private static void createTraversalTar(Path tarPath) throws IOException {
        try (OutputStream out = new BufferedOutputStream(Files.newOutputStream(tarPath))) {
            byte[] safe = "safe".getBytes(StandardCharsets.UTF_8);
            writeTarHeader(out, "safe.txt", safe.length, '0');
            out.write(safe);
            padTo512(out, safe.length);

            byte[] evil = "evil".getBytes(StandardCharsets.UTF_8);
            writeTarHeader(out, "../evil.txt", evil.length, '0');
            out.write(evil);
            padTo512(out, evil.length);

            out.write(new byte[512]);
            out.write(new byte[512]);
        }
    }

    private static void createSampleTarGz(Path tgzPath) throws IOException {
        Path tmpTar = Files.createTempFile("sample", ".tar");
        try {
            createSampleTar(tmpTar);
            try (InputStream in = Files.newInputStream(tmpTar);
                 OutputStream fout = new BufferedOutputStream(Files.newOutputStream(tgzPath));
                 GZIPOutputStream gout = new GZIPOutputStream(fout)) {
                in.transferTo(gout);
            }
        } finally {
            Files.deleteIfExists(tmpTar);
        }
    }

    private static void createLargeZip(Path zipPath, int sizeBytes) throws IOException {
        try (ZipOutputStream zos = new ZipOutputStream(new BufferedOutputStream(Files.newOutputStream(zipPath)))) {
            zos.putNextEntry(new ZipEntry("large.bin"));
            byte[] buf = new byte[8192];
            int written = 0;
            Arrays.fill(buf, (byte) 1);
            while (written < sizeBytes) {
                int chunk = Math.min(buf.length, sizeBytes - written);
                zos.write(buf, 0, chunk);
                written += chunk;
            }
            zos.closeEntry();
        }
    }

    public static void main(String[] args) {
        try {
            Path base = Files.createTempDirectory("task117_java_tests");
            System.out.println("Java tests working dir: " + base);

            // Test 1: extract sample ZIP
            Path zip1 = base.resolve("sample.zip");
            Path out1 = base.resolve("out_zip1");
            createSampleZip(zip1);
            int count1 = extractZipSecure(zip1.toString(), out1.toString(), 10_000_000);
            System.out.println("Test1 ZIP extracted files: " + count1);

            // Test 2: traversal ZIP should skip bad entry
            Path zip2 = base.resolve("traversal.zip");
            Path out2 = base.resolve("out_zip2");
            createTraversalZip(zip2);
            int count2 = extractZipSecure(zip2.toString(), out2.toString(), 10_000_000);
            System.out.println("Test2 ZIP extracted files (expect 1): " + count2);

            // Test 3: extract sample TAR
            Path tar1 = base.resolve("sample.tar");
            Path out3 = base.resolve("out_tar1");
            createSampleTar(tar1);
            int count3 = extractTarSecure(tar1.toString(), out3.toString(), 10_000_000);
            System.out.println("Test3 TAR extracted files: " + count3);

            // Test 4: extract sample TAR.GZ
            Path tgz1 = base.resolve("sample.tgz");
            Path out4 = base.resolve("out_tgz1");
            createSampleTarGz(tgz1);
            int count4 = extractTarGzSecure(tgz1.toString(), out4.toString(), 10_000_000);
            System.out.println("Test4 TAR.GZ extracted files: " + count4);

            // Test 5: enforce size limit on ZIP
            Path zipLarge = base.resolve("large.zip");
            Path out5 = base.resolve("out_zip_large");
            createLargeZip(zipLarge, 2_000_000);
            try {
                extractZipSecure(zipLarge.toString(), out5.toString(), 100_000);
                System.out.println("Test5 ZIP size limit: FAILED (limit not enforced)");
            } else {
                System.out.println();
            }
            catch (IOException ex) {
                System.out.println("Test5 ZIP size limit: PASSED (" + ex.getMessage() + ")");
            }
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }
}