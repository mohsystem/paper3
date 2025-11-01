import java.io.*;
import java.util.zip.*;

public class Lse50 {

    public static boolean extractArchive(String archivePath, String destDir) {
        // Try as TAR first
        try (FileInputStream fis = new FileInputStream(archivePath)) {
            if (extractTarFromStream(fis, destDir)) {
                return true;
            }
        } catch (Exception ignored) {}

        // Fallback to ZIP
        try (ZipInputStream zis = new ZipInputStream(new FileInputStream(archivePath))) {
            File dest = new File(destDir);
            if (!dest.exists() && !dest.mkdirs()) {
                throw new IOException("Failed to create destination directory: " + destDir);
            }
            String destCanonical = dest.getCanonicalPath() + File.separator;
            ZipEntry entry;
            byte[] buffer = new byte[8192];
            while ((entry = zis.getNextEntry()) != null) {
                if (entry.isDirectory()) {
                    File dir = new File(dest, entry.getName());
                    String targetPath = dir.getCanonicalPath();
                    if (!targetPath.startsWith(destCanonical)) {
                        throw new IOException("Blocked path traversal attempt: " + entry.getName());
                    }
                    if (!dir.exists() && !dir.mkdirs()) {
                        throw new IOException("Failed to create directory: " + dir);
                    }
                } else {
                    File outFile = new File(dest, entry.getName());
                    String targetPath = outFile.getCanonicalPath();
                    if (!targetPath.startsWith(destCanonical)) {
                        throw new IOException("Blocked path traversal attempt: " + entry.getName());
                    }
                    File parent = outFile.getParentFile();
                    if (parent != null && !parent.exists() && !parent.mkdirs()) {
                        throw new IOException("Failed to create parent directory: " + parent);
                    }
                    try (FileOutputStream fos = new FileOutputStream(outFile)) {
                        int len;
                        while ((len = zis.read(buffer)) > 0) {
                            fos.write(buffer, 0, len);
                        }
                    }
                }
                zis.closeEntry();
            }
            return true;
        } catch (Exception e) {
            return false;
        }
    }

    private static boolean extractTarFromStream(InputStream is, String destDir) throws IOException {
        BufferedInputStream bis = new BufferedInputStream(is);
        File dest = new File(destDir);
        if (!dest.exists() && !dest.mkdirs()) {
            throw new IOException("Failed to create destination directory: " + destDir);
        }
        String destCanonical = dest.getCanonicalPath() + File.separator;
        byte[] header = new byte[512];
        byte[] buffer = new byte[8192];

        while (true) {
            int read = readFully(bis, header, 0, 512);
            if (read == -1) {
                // End of stream
                break;
            }
            if (read != 512) {
                throw new IOException("Incomplete TAR header");
            }
            if (isAllZero(header)) {
                // Two consecutive zero blocks indicate end of archive; read next and break
                int next = readFully(bis, header, 0, 512);
                break;
            }

            String name = readString(header, 0, 100);
            String prefix = readString(header, 345, 155);
            if (!prefix.isEmpty()) {
                name = prefix + "/" + name;
            }
            long size = parseOctal(header, 124, 12);
            char type = (char) header[156];

            if (name.length() == 0) {
                // skip invalid
                skipFully(bis, roundUp512(size));
                continue;
            }

            // Path traversal protections
            if (name.startsWith("/") || name.contains("..") || name.contains(":\\")) {
                throw new IOException("Blocked path traversal: " + name);
            }

            File out = new File(dest, name);
            String outCanonical = out.getCanonicalPath();
            if (!outCanonical.startsWith(destCanonical)) {
                throw new IOException("Blocked path traversal: " + name);
            }

            if (type == '5') { // directory
                if (!out.exists() && !out.mkdirs()) {
                    throw new IOException("Failed to create directory: " + out);
                }
                // TAR directories have no data payload
            } else if (type == '0' || type == 0) { // regular file
                File parent = out.getParentFile();
                if (parent != null && !parent.exists() && !parent.mkdirs()) {
                    throw new IOException("Failed to create parent: " + parent);
                }
                try (OutputStream fos = new FileOutputStream(out)) {
                    long remaining = size;
                    while (remaining > 0) {
                        int toRead = (int) Math.min(buffer.length, remaining);
                        int n = bis.read(buffer, 0, toRead);
                        if (n == -1) throw new EOFException("Unexpected EOF in file data");
                        fos.write(buffer, 0, n);
                        remaining -= n;
                    }
                }
                // Skip padding
                long pad = (512 - (size % 512)) % 512;
                if (pad > 0) skipFully(bis, pad);
            } else {
                // Other types: skip data if any
                skipFully(bis, roundUp512(size));
            }
        }
        return true;
    }

    private static int readFully(InputStream is, byte[] buf, int off, int len) throws IOException {
        int total = 0;
        while (total < len) {
            int n = is.read(buf, off + total, len - total);
            if (n == -1) {
                return total == 0 ? -1 : total;
            }
            total += n;
        }
        return total;
    }

    private static void skipFully(InputStream is, long n) throws IOException {
        while (n > 0) {
            long skipped = is.skip(n);
            if (skipped <= 0) {
                if (is.read() == -1) throw new EOFException("Unexpected EOF while skipping");
                skipped = 1;
            }
            n -= skipped;
        }
    }

    private static boolean isAllZero(byte[] b) {
        for (byte value : b) {
            if (value != 0) return false;
        }
        return true;
    }

    private static String readString(byte[] b, int off, int len) {
        int end = off + len;
        int i = off;
        while (i < end && b[i] != 0) i++;
        return new String(b, off, i - off).trim();
    }

    private static long parseOctal(byte[] b, int off, int len) {
        long val = 0;
        int end = off + len;
        int i = off;
        // skip leading spaces and nulls
        while (i < end && (b[i] == 0 || b[i] == ' ')) i++;
        for (; i < end; i++) {
            byte c = b[i];
            if (c == 0 || c == ' ') break;
            if (c < '0' || c > '7') break;
            val = (val << 3) + (c - '0');
        }
        return val;
    }

    private static long roundUp512(long size) {
        long rem = size % 512;
        return rem == 0 ? size : size + (512 - rem);
    }

    public static void main(String[] args) {
        String[][] tests = new String[][]{
                {"archive.zip", "/tmp/unpack"},
                {"archive.zip", "/tmp/unpack1"},
                {"archive.tar", "/tmp/unpack2"},
                {"nonexistent.zip", "/tmp/unpack3"},
                {"./archive.zip", "/tmp/unpack4"}
        };
        for (int i = 0; i < tests.length; i++) {
            boolean ok = extractArchive(tests[i][0], tests[i][1]);
            System.out.println("Test " + (i + 1) + " (" + tests[i][0] + " -> " + tests[i][1] + "): " + (ok ? "OK" : "FAIL"));
        }
    }
}