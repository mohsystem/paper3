
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.*;
import java.util.zip.*;

public class Task117 {
    private static final int MAX_FILE_SIZE = 100 * 1024 * 1024; // 100MB
    private static final int MAX_TOTAL_SIZE = 500 * 1024 * 1024; // 500MB
    private static final int MAX_FILES = 10000;
    private static final int BUFFER_SIZE = 8192;

    public static boolean extractArchive(String archivePath, String destDir) {
        if (archivePath == null || destDir == null || archivePath.isEmpty() || destDir.isEmpty()) {
            return false;
        }

        Path archivePathObj = Paths.get(archivePath);
        Path destPathObj = Paths.get(destDir);

        if (!Files.exists(archivePathObj) || !Files.isRegularFile(archivePathObj)) {
            return false;
        }

        try {
            Files.createDirectories(destPathObj);
        } catch (IOException e) {
            return false;
        }

        Path normalizedDest = destPathObj.toAbsolutePath().normalize();

        if (archivePath.toLowerCase().endsWith(".zip")) {
            return extractZip(archivePathObj, normalizedDest);
        } else if (archivePath.toLowerCase().endsWith(".tar")) {
            return extractTar(archivePathObj, normalizedDest);
        }

        return false;
    }

    private static boolean extractZip(Path archivePath, Path destDir) {
        long totalExtracted = 0;
        int fileCount = 0;

        try (ZipInputStream zis = new ZipInputStream(
                new BufferedInputStream(Files.newInputStream(archivePath)), StandardCharsets.UTF_8)) {
            
            ZipEntry entry;
            while ((entry = zis.getNextEntry()) != null) {
                if (fileCount >= MAX_FILES) {
                    return false;
                }

                String name = entry.getName();
                if (name == null || name.isEmpty() || name.contains("..") || 
                    name.startsWith("/") || name.startsWith("\\\\")) {\n                    zis.closeEntry();\n                    continue;\n                }\n\n                Path targetPath = destDir.resolve(name).normalize();\n                if (!targetPath.startsWith(destDir)) {\n                    zis.closeEntry();\n                    continue;\n                }\n\n                if (entry.isDirectory()) {\n                    Files.createDirectories(targetPath);\n                    zis.closeEntry();\n                    continue;\n                }\n\n                if (entry.getSize() > MAX_FILE_SIZE) {\n                    zis.closeEntry();\n                    continue;\n                }\n\n                Files.createDirectories(targetPath.getParent());\n\n                Path tempFile = Files.createTempFile(destDir, ".tmp", ".extract");\n                long bytesWritten = 0;\n\n                try (OutputStream fos = new BufferedOutputStream(Files.newOutputStream(tempFile))) {\n                    byte[] buffer = new byte[BUFFER_SIZE];\n                    int len;\n                    while ((len = zis.read(buffer)) > 0) {\n                        bytesWritten += len;\n                        totalExtracted += len;\n\n                        if (bytesWritten > MAX_FILE_SIZE || totalExtracted > MAX_TOTAL_SIZE) {\n                            Files.delete(tempFile);\n                            return false;\n                        }\n\n                        fos.write(buffer, 0, len);\n                    }\n                    fos.flush();\n                }\n\n                Files.move(tempFile, targetPath, StandardCopyOption.REPLACE_EXISTING, StandardCopyOption.ATOMIC_MOVE);\n                fileCount++;\n                zis.closeEntry();\n            }\n        } catch (IOException e) {\n            return false;\n        }\n\n        return true;\n    }\n\n    private static boolean extractTar(Path archivePath, Path destDir) {\n        long totalExtracted = 0;\n        int fileCount = 0;\n\n        try (InputStream fis = new BufferedInputStream(Files.newInputStream(archivePath))) {\n            byte[] header = new byte[512];\n            \n            while (true) {\n                int bytesRead = readFully(fis, header);\n                if (bytesRead < 512 || isEmptyBlock(header)) {\n                    break;\n                }\n\n                if (fileCount >= MAX_FILES) {\n                    return false;\n                }\n\n                String name = extractTarString(header, 0, 100);\n                long size = extractTarSize(header, 124, 12);\n                char typeFlag = (char) header[156];\n\n                if (name == null || name.isEmpty() || name.contains("..") || \n                    name.startsWith("/") || size < 0 || size > MAX_FILE_SIZE) {\n                    skipTarContent(fis, size);\n                    continue;\n                }\n\n                Path targetPath = destDir.resolve(name).normalize();\n                if (!targetPath.startsWith(destDir)) {\n                    skipTarContent(fis, size);\n                    continue;\n                }\n\n                if (typeFlag == '5' || name.endsWith("/")) {\n                    Files.createDirectories(targetPath);\n                    skipTarContent(fis, size);\n                    continue;\n                }\n\n                if (typeFlag != '0' && typeFlag != '\\0') {\n                    skipTarContent(fis, size);\n                    continue;\n                }\n\n                Files.createDirectories(targetPath.getParent());\n\n                Path tempFile = Files.createTempFile(destDir, ".tmp", ".extract");\n                long bytesWritten = 0;\n\n                try (OutputStream fos = new BufferedOutputStream(Files.newOutputStream(tempFile))) {\n                    byte[] buffer = new byte[BUFFER_SIZE];\n                    long remaining = size;\n\n                    while (remaining > 0) {\n                        int toRead = (int) Math.min(buffer.length, remaining);\n                        int len = fis.read(buffer, 0, toRead);\n                        if (len <= 0) {\n                            Files.delete(tempFile);\n                            return false;\n                        }\n\n                        bytesWritten += len;\n                        totalExtracted += len;\n\n                        if (totalExtracted > MAX_TOTAL_SIZE) {\n                            Files.delete(tempFile);\n                            return false;\n                        }\n\n                        fos.write(buffer, 0, len);\n                        remaining -= len;\n                    }\n                    fos.flush();\n                }\n\n                long padding = (512 - (size % 512)) % 512;\n                skipBytes(fis, padding);\n\n                Files.move(tempFile, targetPath, StandardCopyOption.REPLACE_EXISTING, StandardCopyOption.ATOMIC_MOVE);\n                fileCount++;\n            }\n        } catch (IOException e) {\n            return false;\n        }\n\n        return true;\n    }\n\n    private static String extractTarString(byte[] header, int offset, int length) {\n        int end = offset;\n        while (end < offset + length && header[end] != 0) {\n            end++;\n        }\n        return new String(header, offset, end - offset, StandardCharsets.UTF_8);\n    }\n\n    private static long extractTarSize(byte[] header, int offset, int length) {\n        try {\n            String sizeStr = extractTarString(header, offset, length).trim();\n            if (sizeStr.isEmpty()) {\n                return 0;\n            }\n            return Long.parseLong(sizeStr, 8);\n        } catch (NumberFormatException e) {\n            return -1;\n        }\n    }\n\n    private static boolean isEmptyBlock(byte[] block) {\n        for (byte b : block) {\n            if (b != 0) {\n                return false;\n            }\n        }\n        return true;\n    }\n\n    private static int readFully(InputStream is, byte[] buffer) throws IOException {\n        int total = 0;\n        while (total < buffer.length) {\n            int read = is.read(buffer, total, buffer.length - total);\n            if (read < 0) {\n                return total;\n            }\n            total += read;\n        }\n        return total;\n    }\n\n    private static void skipTarContent(InputStream is, long size) throws IOException {\n        long padding = (512 - (size % 512)) % 512;\n        skipBytes(is, size + padding);\n    }\n\n    private static void skipBytes(InputStream is, long bytes) throws IOException {\n        long remaining = bytes;\n        byte[] buffer = new byte[BUFFER_SIZE];\n        while (remaining > 0) {\n            long skipped = is.skip(remaining);\n            if (skipped > 0) {\n                remaining -= skipped;\n            } else {\n                int toRead = (int) Math.min(buffer.length, remaining);\n                int read = is.read(buffer, 0, toRead);\n                if (read <= 0) {\n                    break;\n                }\n                remaining -= read;\n            }\n        }\n    }\n\n    public static void main(String[] args) {\n        System.out.println("Test 1 - Extract valid ZIP:");\n        System.out.println(extractArchive("test.zip", "output1"));\n\n        System.out.println("\
Test 2 - Extract valid TAR:");\n        System.out.println(extractArchive("test.tar", "output2"));\n\n        System.out.println("\
Test 3 - Invalid archive path:");\n        System.out.println(extractArchive("nonexistent.zip", "output3"));\n\n        System.out.println("\
Test 4 - Null parameters:");\n        System.out.println(extractArchive(null, "output4"));\n\n        System.out.println("\
Test 5 - Empty destination:");\n        System.out.println(extractArchive("test.zip", ""));
    }
}
