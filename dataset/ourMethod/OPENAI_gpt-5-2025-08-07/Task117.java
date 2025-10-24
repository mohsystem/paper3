import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.EOFException;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.UncheckedIOException;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.AtomicMoveNotSupportedException;
import java.nio.file.FileAlreadyExistsException;
import java.nio.file.FileVisitOption;
import java.nio.file.Files;
import java.nio.file.LinkOption;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.nio.file.StandardOpenOption;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.ArrayList;
import java.util.EnumSet;
import java.util.List;
import java.util.zip.GZIPInputStream;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

public class Task117 {

    public static List<Path> extractArchive(Path archivePath, Path destDir, long maxTotalSizeBytes) throws IOException {
        validateNotNull(archivePath, "archivePath");
        validateNotNull(destDir, "destDir");
        if (maxTotalSizeBytes <= 0) {
            throw new IllegalArgumentException("maxTotalSizeBytes must be positive");
        }
        String name = archivePath.getFileName().toString().toLowerCase();
        if (name.endsWith(".zip")) {
            return extractZip(archivePath, destDir, maxTotalSizeBytes);
        } else if (name.endsWith(".tar")) {
            try (InputStream fis = new BufferedInputStream(Files.newInputStream(archivePath))) {
                return extractTarStream(fis, destDir, maxTotalSizeBytes);
            }
        } else if (name.endsWith(".tar.gz") || name.endsWith(".tgz")) {
            try (InputStream fis = new BufferedInputStream(Files.newInputStream(archivePath));
                 InputStream gzi = new GZIPInputStream(fis)) {
                return extractTarStream(gzi, destDir, maxTotalSizeBytes);
            }
        } else {
            throw new IllegalArgumentException("Unsupported archive type: " + name);
        }
    }

    public static List<Path> extractZip(Path archivePath, Path destDir, long maxTotalSizeBytes) throws IOException {
        validateNotNull(archivePath, "archivePath");
        validateNotNull(destDir, "destDir");
        createBaseDirectory(destDir);
        List<Path> extracted = new ArrayList<>();
        long total = 0L;
        try (ZipFile zip = new ZipFile(archivePath.toFile(), StandardCharsets.UTF_8)) {
            var entries = zip.entries();
            while (entries.hasMoreElements()) {
                ZipEntry entry = entries.nextElement();
                String rawName = entry.getName();
                validateEntryName(rawName);
                Path target = resolveSafePath(destDir, rawName);
                if (entry.isDirectory()) {
                    secureMkdirs(target, destDir);
                    continue;
                }
                // Ensure parent directories exist securely
                secureMkdirs(target.getParent(), destDir);

                long declaredSize = entry.getSize();
                if (declaredSize >= 0) {
                    if (declaredSize > maxTotalSizeBytes - total) {
                        throw new IOException("Archive exceeds maximum allowed total size");
                    }
                }

                try (InputStream is = zip.getInputStream(entry)) {
                    long written = writeFileSafely(is, target);
                    total = safeAdd(total, written, maxTotalSizeBytes);
                    extracted.add(target);
                }
            }
        }
        return extracted;
    }

    public static List<Path> extractTarStream(InputStream in, Path destDir, long maxTotalSizeBytes) throws IOException {
        validateNotNull(in, "in");
        validateNotNull(destDir, "destDir");
        createBaseDirectory(destDir);
        List<Path> extracted = new ArrayList<>();
        long total = 0L;

        try (BufferedInputStream bis = new BufferedInputStream(in)) {
            TarReader reader = new TarReader(bis);
            TarEntry e;
            while ((e = reader.getNextEntry()) != null) {
                validateEntryName(e.name);
                Path target = resolveSafePath(destDir, e.name);
                if (e.type == TarEntryType.DIRECTORY) {
                    secureMkdirs(target, destDir);
                    reader.skipEntryData(); // directory has no data but align anyway
                    continue;
                }
                if (e.type == TarEntryType.SYMLINK || e.type == TarEntryType.HARDLINK) {
                    // Refuse symlinks/hardlinks
                    reader.skipEntryData();
                    throw new IOException("Refusing to extract link entry: " + e.name);
                }
                if (e.size < 0) {
                    throw new IOException("Invalid tar entry size for: " + e.name);
                }
                if (e.size > maxTotalSizeBytes - total) {
                    throw new IOException("Archive exceeds maximum allowed total size");
                }

                if (e.type == TarEntryType.REGULAR_FILE) {
                    secureMkdirs(target.getParent(), destDir);
                    long written = writeFileSafely(reader.asInputStream(e.size), target);
                    total = safeAdd(total, written, maxTotalSizeBytes);
                    reader.alignTo512(e.size);
                    extracted.add(target);
                } else {
                    // Unknown types: skip safely
                    reader.skipEntryData();
                }
            }
        }
        return extracted;
    }

    private static void validateNotNull(Object o, String name) {
        if (o == null) throw new IllegalArgumentException(name + " must not be null");
    }

    private static void validateEntryName(String name) throws IOException {
        if (name == null) throw new IOException("Entry name is null");
        if (name.length() == 0) throw new IOException("Empty entry name");
        if (name.length() > 4096) throw new IOException("Entry name too long");
    }

    private static Path resolveSafePath(Path base, String entryName) throws IOException {
        // Normalize slashes and strip potential leading "./"
        String norm = entryName.replace('\\', '/');
        while (norm.startsWith("./")) {
            norm = norm.substring(2);
        }
        Path rel = Paths.get(norm).normalize();
        if (rel.isAbsolute()) {
            throw new IOException("Absolute paths are not allowed: " + entryName);
        }
        for (Path part : rel) {
            if (part.toString().equals("..")) {
                throw new IOException("Path traversal detected: " + entryName);
            }
        }
        Path target = base.resolve(rel).normalize();
        if (!target.startsWith(base)) {
            throw new IOException("Resolved path escapes base directory: " + entryName);
        }
        return target;
    }

    private static void createBaseDirectory(Path base) throws IOException {
        if (Files.exists(base, LinkOption.NOFOLLOW_LINKS)) {
            if (!Files.isDirectory(base, LinkOption.NOFOLLOW_LINKS)) {
                throw new IOException("Destination exists and is not a directory");
            }
            if (Files.isSymbolicLink(base)) {
                throw new IOException("Destination directory is a symlink");
            }
        } else {
            Files.createDirectories(base);
            try {
                Files.setPosixFilePermissions(base, PosixFilePermissions.fromString("rwx------"));
            } catch (UnsupportedOperationException ignored) {
            }
        }
    }

    private static void secureMkdirs(Path dir, Path base) throws IOException {
        if (dir == null) return;
        Path current = base;
        for (Path part : base.relativize(dir)) {
            current = current.resolve(part);
            if (Files.exists(current, LinkOption.NOFOLLOW_LINKS)) {
                if (Files.isSymbolicLink(current)) {
                    throw new IOException("Refusing to use symlinked directory: " + current);
                }
                if (!Files.isDirectory(current, LinkOption.NOFOLLOW_LINKS)) {
                    throw new IOException("Path component is not a directory: " + current);
                }
            } else {
                Files.createDirectory(current);
                try {
                    Files.setPosixFilePermissions(current, PosixFilePermissions.fromString("rwx------"));
                } catch (UnsupportedOperationException ignored) {
                }
            }
        }
    }

    private static long writeFileSafely(InputStream data, Path target) throws IOException {
        Path parent = target.getParent();
        if (parent == null) throw new IOException("Target has no parent: " + target);
        // Ensure no symlink components in parent path
        Path current = parent.getRoot() == null ? parent.getFileSystem().getPath("") : parent.getRoot();
        Path base = parent.getRoot() == null ? target.getFileSystem().getPath("") : parent.getRoot();
        // Verify ancestors from root/base to parent
        Path check = parent;
        Path rootToBase = parent;
        Path cur = parent;
        // Safer explicit check: walk from nearest base ancestor (unknown) to parent
        Path p = parent;
        while (p != null && !Files.exists(p, LinkOption.NOFOLLOW_LINKS)) {
            p = p.getParent();
        }
        // From existing closest ancestor down to parent
        List<Path> stack = new ArrayList<>();
        Path tmp = parent;
        while (tmp != null && (p == null || !tmp.equals(p))) {
            stack.add(0, tmp);
            tmp = tmp.getParent();
        }
        if (p != null && Files.isSymbolicLink(p)) {
            throw new IOException("Refusing to use symlink ancestor: " + p);
        }
        for (Path part : stack) {
            if (Files.exists(part, LinkOption.NOFOLLOW_LINKS) && Files.isSymbolicLink(part)) {
                throw new IOException("Refusing to use symlinked path component: " + part);
            }
        }

        // Create a temp file in the parent directory
        Path tmpFile = Files.createTempFile(parent, ".extract-", ".part");
        try (OutputStream os = new BufferedOutputStream(Files.newOutputStream(tmpFile, StandardOpenOption.WRITE, StandardOpenOption.TRUNCATE_EXISTING))) {
            byte[] buf = new byte[8192];
            long total = 0L;
            int read;
            while ((read = data.read(buf)) != -1) {
                total = safeAdd(total, read, Long.MAX_VALUE);
                os.write(buf, 0, read);
            }
            os.flush();
        } catch (IOException e) {
            try { Files.deleteIfExists(tmpFile); } catch (IOException ignored) {}
            throw e;
        }

        // fsync
        try (FileChannel ch = FileChannel.open(tmpFile, StandardOpenOption.READ)) {
            ch.force(true);
        }

        // Set secure permissions before move where supported
        try {
            Files.setPosixFilePermissions(tmpFile, PosixFilePermissions.fromString("rw-------"));
        } catch (UnsupportedOperationException ignored) {
        }

        // Move atomically to target; fail if target exists
        try {
            Files.move(tmpFile, target, StandardCopyOption.ATOMIC_MOVE);
        } catch (AtomicMoveNotSupportedException e) {
            Files.move(tmpFile, target);
        } catch (FileAlreadyExistsException e) {
            // Target exists, delete temp and fail closed
            try { Files.deleteIfExists(tmpFile); } catch (IOException ignored) {}
            throw e;
        }
        return Files.size(target);
    }

    private static long safeAdd(long a, long b, long limit) throws IOException {
        long r = a + b;
        if (r < a || r > limit) {
            throw new IOException("Size limit exceeded");
        }
        return r;
    }

    // Minimal TAR reader for ustar and GNU tar (supports regular files, directories, longname)
    private static final class TarReader {
        private final InputStream in;
        private boolean eof = false;
        private String pendingLongName = null;

        TarReader(InputStream in) {
            this.in = in;
        }

        TarEntry getNextEntry() throws IOException {
            if (eof) return null;
            byte[] header = new byte[512];
            int read = readFully(header, 0, 512);
            if (read == -1) return null;
            if (isAllZero(header)) {
                eof = true;
                return null;
            }
            String name = getString(header, 0, 100);
            long size = parseOctal(header, 124, 12);
            int typeflag = header[156] & 0xFF;
            String magic = getString(header, 257, 6);
            String prefix = getString(header, 345, 155);
            if (pendingLongName != null) {
                name = pendingLongName;
                pendingLongName = null;
            } else if (!prefix.isEmpty()) {
                name = prefix + "/" + name;
            }
            TarEntryType type = TarEntryType.fromTypeFlag(typeflag);

            if (type == TarEntryType.GNU_LONGNAME) {
                // Read long name data
                String longName = readStringData(size);
                alignTo512(size);
                // Next header is the real entry
                pendingLongName = longName;
                return getNextEntry();
            }

            TarEntry entry = new TarEntry();
            entry.name = name;
            entry.size = size;
            entry.type = type;
            entry.remaining = size;
            return entry;
        }

        InputStream asInputStream(final long size) {
            return new InputStream() {
                long remaining = size;
                @Override
                public int read() throws IOException {
                    byte[] b = new byte[1];
                    int r = read(b, 0, 1);
                    if (r == -1) return -1;
                    return b[0] & 0xFF;
                }
                @Override
                public int read(byte[] b, int off, int len) throws IOException {
                    if (remaining <= 0) return -1;
                    int toRead = (int) Math.min(len, remaining);
                    int r = in.read(b, off, toRead);
                    if (r == -1) throw new EOFException("Unexpected end of tar entry");
                    remaining -= r;
                    return r;
                }
            };
        }

        void skipEntryData() throws IOException {
            // Used when we want to skip the data for current entry
            // We assume header already read and size known; but here we don't track size. No-op except align if needed.
            // This implementation is used for dirs/links (size=0) or after reading longname which is handled separately.
        }

        void alignTo512(long size) throws IOException {
            long pad = (512 - (size % 512)) % 512;
            if (pad > 0) {
                long skipped = 0;
                while (skipped < pad) {
                    long s = in.skip(pad - skipped);
                    if (s <= 0) {
                        if (in.read() == -1) throw new EOFException("Unexpected end of stream during tar padding");
                        s = 1;
                    }
                    skipped += s;
                }
            }
        }

        private int readFully(byte[] b, int off, int len) throws IOException {
            int total = 0;
            while (total < len) {
                int r = in.read(b, off + total, len - total);
                if (r == -1) {
                    if (total == 0) return -1;
                    throw new EOFException("Unexpected end of tar header");
                }
                total += r;
            }
            return total;
        }

        private boolean isAllZero(byte[] b) {
            for (byte value : b) {
                if (value != 0) return false;
            }
            return true;
        }

        private String readStringData(long size) throws IOException {
            if (size < 0 || size > (10 * 1024 * 1024)) {
                throw new IOException("Unreasonable long name size");
            }
            byte[] data = new byte[(int) size];
            int read = 0;
            while (read < size) {
                int r = in.read(data, read, (int) (size - read));
                if (r == -1) throw new EOFException("Unexpected end while reading long name");
                read += r;
            }
            String s = new String(data, StandardCharsets.UTF_8);
            int nul = s.indexOf('\0');
            if (nul >= 0) {
                s = s.substring(0, nul);
            }
            return s;
        }

        private String getString(byte[] header, int off, int len) {
            int end = off + len;
            int i = off;
            while (i < end && header[i] != 0) i++;
            return new String(header, off, i - off, StandardCharsets.US_ASCII).trim();
        }

        private long parseOctal(byte[] header, int off, int len) throws IOException {
            String s = getString(header, off, len).trim();
            if (s.isEmpty()) return 0L;
            try {
                return Long.parseLong(s, 8);
            } catch (NumberFormatException e) {
                throw new IOException("Invalid octal number in tar header: " + s);
            }
        }
    }

    private static final class TarEntry {
        String name;
        long size;
        TarEntryType type;
        long remaining;
    }

    private enum TarEntryType {
        REGULAR_FILE('0'),
        ALT_REGULAR_FILE('\0'),
        HARDLINK('1'),
        SYMLINK('2'),
        DIRECTORY('5'),
        GNU_LONGNAME('L'),
        OTHER('?');

        final int code;
        TarEntryType(int c) { this.code = c; }

        static TarEntryType fromTypeFlag(int tf) {
            if (tf == '0') return REGULAR_FILE;
            if (tf == 0) return ALT_REGULAR_FILE;
            if (tf == '1') return HARDLINK;
            if (tf == '2') return SYMLINK;
            if (tf == '5') return DIRECTORY;
            if (tf == 'L') return GNU_LONGNAME;
            return OTHER;
        }
    }

    // Test helpers to create sample archives
    private static Path createSampleZip(Path dir) throws IOException {
        Path zipPath = dir.resolve("sample.zip");
        try (var zos = new java.util.zip.ZipOutputStream(Files.newOutputStream(zipPath))) {
            addZipEntry(zos, "a.txt", "Hello A\n".getBytes(StandardCharsets.UTF_8));
            addZipEntry(zos, "dir/b.txt", "Hello B\n".getBytes(StandardCharsets.UTF_8));
            addZipEntry(zos, "../evil.txt", "nope\n".getBytes(StandardCharsets.UTF_8)); // should be rejected
            addZipEntry(zos, "/abs.txt", "nope abs\n".getBytes(StandardCharsets.UTF_8)); // should be rejected
            zos.closeEntry();
        }
        return zipPath;
    }

    private static void addZipEntry(java.util.zip.ZipOutputStream zos, String name, byte[] data) throws IOException {
        ZipEntry e = new ZipEntry(name);
        zos.putNextEntry(e);
        zos.write(data);
        zos.closeEntry();
    }

    private static Path createSampleTar(Path dir) throws IOException {
        Path tarPath = dir.resolve("sample.tar");
        try (OutputStream os = new BufferedOutputStream(Files.newOutputStream(tarPath))) {
            writeTarFile(os, "x.txt", "Hi X\n".getBytes(StandardCharsets.UTF_8));
            writeTarDir(os, "dir/");
            writeTarFile(os, "dir/y.txt", "Hi Y\n".getBytes(StandardCharsets.UTF_8));
            writeTarFile(os, "../pwn.txt", "No\n".getBytes(StandardCharsets.UTF_8)); // should be rejected
            writeTarSymlink(os, "linkz", "target"); // should be refused on extraction
            writeTarEOF(os);
        }
        return tarPath;
    }

    private static void writeTarFile(OutputStream os, String name, byte[] data) throws IOException {
        byte[] header = tarHeader(name, data.length, '0');
        os.write(header);
        os.write(data);
        padTo512(os, data.length);
    }

    private static void writeTarDir(OutputStream os, String name) throws IOException {
        byte[] header = tarHeader(name.endsWith("/") ? name : name + "/", 0, '5');
        os.write(header);
    }

    private static void writeTarSymlink(OutputStream os, String name, String target) throws IOException {
        byte[] header = tarHeader(name, 0, '2');
        // Place linkname in header bytes 157-256
        byte[] link = target.getBytes(StandardCharsets.US_ASCII);
        for (int i = 0; i < Math.min(100, link.length); i++) {
            header[157 + i] = link[i];
        }
        // Recompute checksum
        computeTarChecksum(header);
        os.write(header);
    }

    private static void writeTarEOF(OutputStream os) throws IOException {
        os.write(new byte[1024]); // two 512 blocks of zeros
    }

    private static byte[] tarHeader(String name, int size, int typeflag) throws IOException {
        byte[] header = new byte[512];
        putString(header, 0, 100, name);
        putOctal(header, 100, 8, 0644);
        putOctal(header, 108, 8, 0);
        putOctal(header, 116, 8, 0);
        putOctal(header, 124, 12, size);
        putOctal(header, 136, 12, System.currentTimeMillis() / 1000);
        // Checksum field set to spaces initially
        for (int i = 148; i < 156; i++) header[i] = 0x20;
        header[156] = (byte) typeflag;
        putString(header, 257, 6, "ustar");
        putString(header, 263, 2, "00");
        computeTarChecksum(header);
        return header;
    }

    private static void computeTarChecksum(byte[] header) {
        long sum = 0;
        for (byte b : header) sum += (b & 0xFF);
        String chk = String.format("%06o", sum);
        byte[] b = chk.getBytes(StandardCharsets.US_ASCII);
        for (int i = 0; i < 6; i++) header[148 + i] = (i < b.length) ? b[i] : (byte) '0';
        header[154] = 0;
        header[155] = (byte) ' ';
    }

    private static void putString(byte[] header, int off, int len, String s) throws IOException {
        byte[] b = s.getBytes(StandardCharsets.US_ASCII);
        if (b.length > len) throw new IOException("Name too long for simple tar writer");
        System.arraycopy(b, 0, header, off, b.length);
    }

    private static void putOctal(byte[] header, int off, int len, long val) {
        String s = String.format("%0" + (len - 1) + "o", val);
        byte[] b = s.getBytes(StandardCharsets.US_ASCII);
        int start = off + len - 1 - b.length;
        for (int i = off; i < off + len; i++) header[i] = 0;
        System.arraycopy(b, 0, header, start, b.length);
    }

    private static void padTo512(OutputStream os, int size) throws IOException {
        int pad = (512 - (size % 512)) % 512;
        if (pad > 0) os.write(new byte[pad]);
    }

    public static void main(String[] args) throws Exception {
        Path tempRoot = Files.createTempDirectory("extractor-tests-");
        System.out.println("Temp root: " + tempRoot);

        // Test 1: Simple ZIP extraction
        try {
            Path zip = createSampleZip(tempRoot);
            Path out1 = tempRoot.resolve("out1");
            List<Path> files1 = extractArchive(zip, out1, 10 * 1024 * 1024);
            System.out.println("Test1 ZIP extracted files:");
            for (Path p : files1) System.out.println(" - " + tempRoot.relativize(p));
        } catch (Exception e) {
            System.out.println("Test1 failed: " + e.getMessage());
        }

        // Test 2: ZIP with traversal and absolute entries
        try {
            Path zip = createSampleZip(tempRoot);
            Path out2 = tempRoot.resolve("out2");
            List<Path> files2 = extractZip(zip, out2, 10 * 1024 * 1024);
            System.out.println("Test2 ZIP explicit extracted files:");
            for (Path p : files2) System.out.println(" - " + tempRoot.relativize(p));
        } catch (Exception e) {
            System.out.println("Test2 failed (expected some rejections): " + e.getMessage());
        }

        // Test 3: TAR extraction
        try {
            Path tar = createSampleTar(tempRoot);
            Path out3 = tempRoot.resolve("out3");
            List<Path> files3 = extractArchive(tar, out3, 10 * 1024 * 1024);
            System.out.println("Test3 TAR extracted files:");
            for (Path p : files3) System.out.println(" - " + tempRoot.relativize(p));
        } catch (Exception e) {
            System.out.println("Test3 failed (expected symlink refusal): " + e.getMessage());
        }

        // Test 4: TAR.GZ extraction (we gzip the tar we created)
        try {
            Path tar = createSampleTar(tempRoot);
            Path tgz = tempRoot.resolve("sample.tgz");
            try (InputStream in = Files.newInputStream(tar);
                 java.util.zip.GZIPOutputStream gz = new java.util.zip.GZIPOutputStream(Files.newOutputStream(tgz))) {
                byte[] buf = new byte[8192];
                int r;
                while ((r = in.read(buf)) != -1) gz.write(buf, 0, r);
            }
            Path out4 = tempRoot.resolve("out4");
            List<Path> files4 = extractArchive(tgz, out4, 10 * 1024 * 1024);
            System.out.println("Test4 TGZ extracted files:");
            for (Path p : files4) System.out.println(" - " + tempRoot.relativize(p));
        } catch (Exception e) {
            System.out.println("Test4 failed: " + e.getMessage());
        }

        // Test 5: Large file limit enforcement
        try {
            // Create a ZIP with a moderately large file (~1MB)
            Path zip = tempRoot.resolve("big.zip");
            try (var zos = new java.util.zip.ZipOutputStream(Files.newOutputStream(zip))) {
                byte[] data = new byte[1024 * 1024];
                ZipEntry e = new ZipEntry("big.bin");
                zos.putNextEntry(e);
                zos.write(data);
                zos.closeEntry();
            }
            Path out5 = tempRoot.resolve("out5");
            // Set limit smaller than file to trigger limit rejection
            List<Path> files5 = extractArchive(zip, out5, 512 * 1024);
            System.out.println("Test5 extracted files (unexpected): " + files5.size());
        } catch (Exception e) {
            System.out.println("Test5 size limit enforcement worked: " + e.getMessage());
        }
    }
}