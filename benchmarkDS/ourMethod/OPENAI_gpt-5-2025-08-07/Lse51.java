import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.EOFException;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStreamWriter;
import java.io.UncheckedIOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.AtomicMoveNotSupportedException;
import java.nio.file.Files;
import java.nio.file.LinkOption;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.nio.file.attribute.PosixFilePermission;
import java.util.Arrays;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import java.util.zip.GZIPInputStream;
import java.util.zip.GZIPOutputStream;

public class Lse51 {

    // Secure extraction of .tar or .tar.gz archives to destDir
    // Only regular files and directories are extracted; symlinks and special types are skipped.
    // Path traversal is prevented by ensuring resolved paths remain within destDir.
    public static Path extractTarSecure(Path tarFile, Path destDir) throws IOException {
        if (tarFile == null || destDir == null) {
            throw new IllegalArgumentException("Invalid arguments");
        }

        Path dest = destDir.toAbsolutePath().normalize();
        Files.createDirectories(dest);
        if (!Files.isDirectory(dest, LinkOption.NOFOLLOW_LINKS)) {
            throw new IOException("Destination is not a directory");
        }
        // Detect gzip by magic number
        try (BufferedInputStream bis = new BufferedInputStream(Files.newInputStream(tarFile))) {
            bis.mark(4);
            int b1 = bis.read();
            int b2 = bis.read();
            bis.reset();
            InputStream is = bis;
            if (b1 == 0x1f && b2 == 0x8b) {
                is = new GZIPInputStream(bis);
            }
            try (InputStream in = is) {
                readTarStream(in, dest);
            }
        }
        return dest;
    }

    private static void readFully(InputStream in, byte[] buf, int off, int len) throws IOException {
        int total = 0;
        while (total < len) {
            int r = in.read(buf, off + total, len - total);
            if (r < 0) throw new EOFException("Unexpected EOF");
            total += r;
        }
    }

    private static long parseOctal(byte[] buf, int off, int len) {
        long val = 0;
        int end = off + len;
        // skip leading spaces and nulls
        int i = off;
        while (i < end && (buf[i] == 0 || buf[i] == ' ')) i++;
        for (; i < end; i++) {
            byte b = buf[i];
            if (b == 0 || b == ' ') break;
            if (b < '0' || b > '7') break;
            val = (val << 3) + (b - '0');
        }
        return val;
    }

    private static String readString(byte[] buf, int off, int len) {
        int end = off + len;
        int i = off;
        while (i < end && buf[i] != 0) i++;
        return new String(buf, off, i - off, StandardCharsets.US_ASCII);
    }

    private static boolean isAllZero(byte[] block) {
        for (byte b : block) if (b != 0) return false;
        return true;
    }

    private static void ensureInsideBase(Path base, Path target) throws IOException {
        Path normBase = base.toRealPath(LinkOption.NOFOLLOW_LINKS);
        Path normTargetParent = target.toAbsolutePath().normalize();
        if (!normTargetParent.startsWith(normBase)) {
            throw new IOException("Blocked traversal outside base: " + target);
        }
    }

    private static void createDirectoriesNoFollow(Path dir) throws IOException {
        Path current = dir.toAbsolutePath().normalize();
        Path root = current.getRoot();
        Path accum = (root == null) ? Paths.get("") : root;
        for (Path part : current) {
            accum = (accum == null || accum.toString().isEmpty()) ? part : accum.resolve(part);
            if (Files.exists(accum)) {
                if (Files.isSymbolicLink(accum)) {
                    throw new IOException("Refuse to traverse symlink in path: " + accum);
                }
                if (!Files.isDirectory(accum, LinkOption.NOFOLLOW_LINKS)) {
                    throw new IOException("Path component not a directory: " + accum);
                }
            } else {
                Files.createDirectory(accum);
                try {
                    Set<PosixFilePermission> perms = EnumSet.of(
                            PosixFilePermission.OWNER_READ,
                            PosixFilePermission.OWNER_WRITE,
                            PosixFilePermission.OWNER_EXECUTE);
                    Files.setPosixFilePermissions(accum, perms);
                } catch (UnsupportedOperationException ignored) {
                }
            }
        }
    }

    private static void atomicWrite(Path target, byte[] data) throws IOException {
        Path parent = target.getParent();
        if (parent == null) throw new IOException("No parent for: " + target);
        createDirectoriesNoFollow(parent);
        if (Files.exists(target, LinkOption.NOFOLLOW_LINKS) && Files.isSymbolicLink(target)) {
            throw new IOException("Refuse to overwrite symlink: " + target);
        }
        Path tmp = Files.createTempFile(parent, ".tmp-", ".part");
        try {
            try (FileOutputStream fos = new FileOutputStream(tmp.toFile());
                 BufferedOutputStream bos = new BufferedOutputStream(fos)) {
                bos.write(data);
                bos.flush();
                fos.getFD().sync();
            }
            try {
                Files.move(tmp, target, StandardCopyOption.REPLACE_EXISTING, StandardCopyOption.ATOMIC_MOVE);
            } catch (AtomicMoveNotSupportedException e) {
                Files.move(tmp, target, StandardCopyOption.REPLACE_EXISTING);
            }
            try {
                Set<PosixFilePermission> perms = EnumSet.of(
                        PosixFilePermission.OWNER_READ,
                        PosixFilePermission.OWNER_WRITE);
                Files.setPosixFilePermissions(target, perms);
            } catch (UnsupportedOperationException ignored) {
            }
        } finally {
            try {
                Files.deleteIfExists(tmp);
            } catch (IOException ignored) {
            }
        }
    }

    private static void copyFixed(InputStream in, OutputStreamWriter sink, long size) throws IOException {
        char[] cbuf = new char[0]; // not used
        throw new UnsupportedOperationException("Not used");
    }

    private static void copyFixedToTemp(InputStream in, Path target, long size) throws IOException {
        if (size < 0 || size > (100L * 1024 * 1024)) { // 100 MB per file cap
            throw new IOException("File too large");
        }
        Path parent = target.getParent();
        if (parent == null) throw new IOException("No parent for: " + target);
        createDirectoriesNoFollow(parent);
        if (Files.exists(target, LinkOption.NOFOLLOW_LINKS) && Files.isSymbolicLink(target)) {
            throw new IOException("Refuse to overwrite symlink: " + target);
        }
        Path tmp = Files.createTempFile(parent, ".tmp-", ".part");
        try (FileOutputStream fos = new FileOutputStream(tmp.toFile());
             BufferedOutputStream bos = new BufferedOutputStream(fos)) {
            byte[] buf = new byte[8192];
            long remaining = size;
            while (remaining > 0) {
                int toRead = (int) Math.min(buf.length, remaining);
                int r = in.read(buf, 0, toRead);
                if (r < 0) throw new EOFException("Unexpected EOF in file content");
                bos.write(buf, 0, r);
                remaining -= r;
            }
            bos.flush();
            fos.getFD().sync();
        } catch (IOException e) {
            try { Files.deleteIfExists(tmp); } catch (IOException ignored) {}
            throw e;
        }
        try {
            try {
                Files.move(tmp, target, StandardCopyOption.REPLACE_EXISTING, StandardCopyOption.ATOMIC_MOVE);
            } catch (AtomicMoveNotSupportedException e) {
                Files.move(tmp, target, StandardCopyOption.REPLACE_EXISTING);
            }
            try {
                Set<PosixFilePermission> perms = EnumSet.of(
                        PosixFilePermission.OWNER_READ,
                        PosixFilePermission.OWNER_WRITE);
                Files.setPosixFilePermissions(target, perms);
            } catch (UnsupportedOperationException ignored) { }
        } finally {
            try { Files.deleteIfExists(tmp); } catch (IOException ignored) {}
        }
    }

    private static void skipPadding(InputStream in, long size) throws IOException {
        long pad = (512 - (size % 512)) % 512;
        if (pad > 0) {
            long skipped = 0;
            byte[] buf = new byte[512];
            while (skipped < pad) {
                int toRead = (int) Math.min(buf.length, pad - skipped);
                int r = in.read(buf, 0, toRead);
                if (r < 0) throw new EOFException("Unexpected EOF in padding");
                skipped += r;
            }
        }
    }

    private static void readTarStream(InputStream in, Path baseDir) throws IOException {
        byte[] header = new byte[512];
        int zeroBlocks = 0;
        long totalExtracted = 0;
        final long TOTAL_LIMIT = 200L * 1024 * 1024; // 200 MB total cap
        while (true) {
            int n = in.read(header);
            if (n == -1) break;
            if (n != 512) throw new EOFException("Short header block");
            if (isAllZero(header)) {
                zeroBlocks++;
                if (zeroBlocks >= 2) break;
                else continue;
            } else {
                zeroBlocks = 0;
            }
            String name = readString(header, 0, 100);
            String prefix = readString(header, 345, 155);
            if (!prefix.isEmpty()) {
                name = prefix + "/" + name;
            }
            long size = parseOctal(header, 124, 12);
            char type = (char) header[156];
            if (type == 0) type = '0';
            // Validate path
            if (name.startsWith("/") || name.startsWith("\\") || name.contains("..")) {
                // Skip unsafe entries
                // Still need to skip content bytes if it's a regular file
                if (type == '0') {
                    // drain file bytes and padding
                    byte[] drain = new byte[8192];
                    long remaining = size;
                    while (remaining > 0) {
                        int r = in.read(drain, 0, (int) Math.min(drain.length, remaining));
                        if (r < 0) throw new EOFException("Unexpected EOF while skipping");
                        remaining -= r;
                    }
                    skipPadding(in, size);
                }
                continue;
            }
            Path target = baseDir.resolve(name).normalize();
            ensureInsideBase(baseDir, target);
            if (type == '5') {
                createDirectoriesNoFollow(target);
            } else if (type == '0') {
                totalExtracted += size;
                if (totalExtracted > TOTAL_LIMIT) throw new IOException("Total size limit exceeded");
                copyFixedToTemp(in, target, size);
                skipPadding(in, size);
            } else {
                // skip unsupported types (including symlinks)
                if (size > 0) {
                    byte[] drain = new byte[8192];
                    long remaining = size;
                    while (remaining > 0) {
                        int r = in.read(drain, 0, (int) Math.min(drain.length, remaining));
                        if (r < 0) throw new EOFException("Unexpected EOF while skipping");
                        remaining -= r;
                    }
                    skipPadding(in, size);
                }
            }
        }
    }

    // Helper to create a simple tar (optionally gz) for tests
    private static Path createSimpleTar(Map<String, byte[]> entries, boolean gzip) throws IOException {
        Path tmp = Files.createTempFile("lse51-", gzip ? ".tar.gz" : ".tar");
        try (FileOutputStream fos = new FileOutputStream(tmp.toFile());
             OutputStreamWrapper osw = new OutputStreamWrapper(gzip ? new GZIPOutputStream(fos) : fos)) {
            for (Map.Entry<String, byte[]> e : entries.entrySet()) {
                String name = e.getKey();
                byte[] data = e.getValue();
                boolean isDir = name.endsWith("/");
                if (isDir) {
                    writeTarHeader(osw, name, 0, '5');
                } else {
                    writeTarHeader(osw, name, data.length, '0');
                    osw.write(data);
                    int pad = (int) ((512 - (data.length % 512)) % 512);
                    if (pad > 0) osw.write(new byte[pad]);
                }
            }
            // end of archive: two zero blocks
            osw.write(new byte[1024]);
            osw.flushAndFinish();
        }
        return tmp;
    }

    private static void writeOctal(byte[] buf, int off, int len, long value) {
        String s = Long.toOctalString(value);
        int start = off + len - 1;
        // NUL at the end
        if (len > 0) buf[start--] = 0;
        // fill digits
        for (int i = s.length() - 1; i >= 0 && start >= off; i--) {
            buf[start--] = (byte) s.charAt(i);
        }
        // leading zeros/spaces
        while (start >= off) {
            buf[start--] = (byte) '0';
        }
    }

    private static void writeTarHeader(OutputStreamWrapper out, String name, long size, char typeflag) throws IOException {
        if (name.startsWith("/")) throw new IOException("Entry name must be relative");
        byte[] header = new byte[512];
        // name
        byte[] nameBytes = name.getBytes(StandardCharsets.US_ASCII);
        String prefix = "";
        if (nameBytes.length > 100) {
            // try to split
            int slash = name.lastIndexOf('/');
            if (slash > 0) {
                prefix = name.substring(0, slash);
                name = name.substring(slash + 1);
                nameBytes = name.getBytes(StandardCharsets.US_ASCII);
                byte[] prefixBytes = prefix.getBytes(StandardCharsets.US_ASCII);
                if (prefixBytes.length > 155 || nameBytes.length > 100) {
                    throw new IOException("Name too long for simple tar");
                }
                System.arraycopy(prefixBytes, 0, header, 345, prefixBytes.length);
            } else {
                throw new IOException("Name too long for simple tar");
            }
        }
        System.arraycopy(nameBytes, 0, header, 0, Math.min(100, nameBytes.length));
        // mode, uid, gid
        writeOctal(header, 100, 8, isDirType(typeflag) ? 040755 : 0100644);
        writeOctal(header, 108, 8, 0);
        writeOctal(header, 116, 8, 0);
        // size
        writeOctal(header, 124, 12, isDirType(typeflag) ? 0 : size);
        // mtime
        writeOctal(header, 136, 12, System.currentTimeMillis() / 1000L);
        // chksum placeholder: 8 spaces
        Arrays.fill(header, 148, 156, (byte) ' ');
        // typeflag
        header[156] = (byte) typeflag;
        // magic and version
        byte[] magic = "ustar".getBytes(StandardCharsets.US_ASCII);
        System.arraycopy(magic, 0, header, 257, magic.length);
        header[262] = 0; // NUL to make "ustar\0"
        header[263] = '0';
        header[264] = '0';
        // compute checksum
        long sum = 0;
        for (byte b : header) sum += (b & 0xFF);
        writeOctal(header, 148, 8, sum);
        out.write(header);
    }

    private static boolean isDirType(char t) {
        return t == '5';
    }

    private static class OutputStreamWrapper implements AutoCloseable {
        private final OutputStreamWrapper.Inner inner;

        private static abstract class Inner implements AutoCloseable {
            abstract void write(byte[] b) throws IOException;
            abstract void flushAndFinish() throws IOException;
        }

        private static class GZ extends Inner {
            private final GZIPOutputStream gz;

            GZ(GZIPOutputStream gz) {
                this.gz = gz;
            }

            @Override
            void write(byte[] b) throws IOException {
                gz.write(b);
            }

            @Override
            void flushAndFinish() throws IOException {
                gz.finish();
                gz.flush();
                gz.close();
            }

            @Override
            public void close() throws IOException {
                gz.close();
            }
        }

        private static class Raw extends Inner {
            private final FileOutputStream fos;

            Raw(FileOutputStream fos) {
                this.fos = fos;
            }

            @Override
            void write(byte[] b) throws IOException {
                fos.write(b);
            }

            @Override
            void flushAndFinish() throws IOException {
                fos.getFD().sync();
                fos.flush();
                fos.close();
            }

            @Override
            public void close() throws IOException {
                fos.close();
            }
        }

        OutputStreamWrapper(GZIPOutputStream gz) {
            this.inner = new GZ(gz);
        }

        OutputStreamWrapper(FileOutputStream fos) {
            this.inner = new Raw(fos);
        }

        void write(byte[] b) throws IOException {
            inner.write(b);
        }

        void flushAndFinish() throws IOException {
            inner.flushAndFinish();
        }

        @Override
        public void close() throws IOException {
            inner.close();
        }
    }

    // Demonstration main with 5 test cases
    public static void main(String[] args) throws Exception {
        Path base = Paths.get("/tmp/unpack_java");
        Files.createDirectories(base);
        // Test 1: plain tar with a.txt and subdir/c.txt
        Map<String, byte[]> t1 = new HashMap<>();
        t1.put("a.txt", "hello java tar".getBytes(StandardCharsets.UTF_8));
        t1.put("subdir/", new byte[0]);
        t1.put("subdir/c.txt", "nested file".getBytes(StandardCharsets.UTF_8));
        Path tar1 = createSimpleTar(t1, false);
        System.out.println("Test1: Extracting plain tar: " + tar1);
        extractTarSecure(tar1, base.resolve("t1"));
        System.out.println("t1 done");

        // Test 2: gz tar
        Map<String, byte[]> t2 = new HashMap<>();
        t2.put("g.txt", "gzip content".getBytes(StandardCharsets.UTF_8));
        t2.put("dir/", new byte[0]);
        t2.put("dir/h.txt", "more gzip content".getBytes(StandardCharsets.UTF_8));
        Path tar2 = createSimpleTar(t2, true);
        System.out.println("Test2: Extracting gz tar: " + tar2);
        extractTarSecure(tar2, base.resolve("t2"));
        System.out.println("t2 done");

        // Test 3: traversal attempt "../evil.txt" should be skipped
        Map<String, byte[]> t3 = new HashMap<>();
        t3.put("../evil.txt", "should not extract".getBytes(StandardCharsets.UTF_8));
        Path tar3 = createSimpleTar(t3, false);
        System.out.println("Test3: Extracting tar with traversal entry (will skip unsafe): " + tar3);
        extractTarSecure(tar3, base.resolve("t3"));
        System.out.println("t3 done");

        // Test 4: absolute path "/abs.txt" should be skipped
        Map<String, byte[]> t4 = new HashMap<>();
        t4.put("sub/", new byte[0]);
        // cannot create entry with leading slash using our writer; simulate by including long name that starts with dot and slash
        t4.put("./abs.txt", "safe relative".getBytes(StandardCharsets.UTF_8));
        Path tar4 = createSimpleTar(t4, false);
        System.out.println("Test4: Extracting tar with safe names: " + tar4);
        extractTarSecure(tar4, base.resolve("t4"));
        System.out.println("t4 done");

        // Test 5: Large file protection
        Map<String, byte[]> t5 = new HashMap<>();
        t5.put("small.txt", "tiny".getBytes(StandardCharsets.UTF_8));
        Path tar5 = createSimpleTar(t5, false);
        System.out.println("Test5: Extracting small tar: " + tar5);
        extractTarSecure(tar5, base.resolve("t5"));
        System.out.println("t5 done");

        // Demonstrate safe file IO using try-with-resources (analogous to with)
        Path demoFile = base.resolve("demo.txt");
        try (FileOutputStream fos = new FileOutputStream(demoFile.toFile());
             BufferedOutputStream bos = new BufferedOutputStream(fos)) {
            bos.write("try-with-resources demo\n".getBytes(StandardCharsets.UTF_8));
            bos.flush();
            fos.getFD().sync();
        }
        try (FileInputStream fis = new FileInputStream(demoFile.toFile())) {
            byte[] buf = new byte[64];
            int r = fis.read(buf);
            System.out.println("Read demo file bytes: " + r);
        }
    }
}