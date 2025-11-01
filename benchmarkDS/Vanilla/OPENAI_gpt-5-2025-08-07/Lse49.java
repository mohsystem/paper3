import java.io.*;
import java.nio.file.*;
import java.util.*;
import java.util.zip.*;

public class Lse49 {

    public static List<String> unzipArchive(String archivePath, String destDir) throws IOException {
        List<String> extracted = new ArrayList<>();
        File dest = new File(destDir);
        if (!dest.exists()) {
            if (!dest.mkdirs()) {
                throw new IOException("Failed to create destination directory: " + destDir);
            }
        }

        try (ZipInputStream zis = new ZipInputStream(new BufferedInputStream(new FileInputStream(archivePath)))) {
            ZipEntry entry;
            byte[] buffer = new byte[8192];
            while ((entry = zis.getNextEntry()) != null) {
                File newFile = safeNewFile(dest, entry);
                if (entry.isDirectory()) {
                    if (!newFile.isDirectory() && !newFile.mkdirs()) {
                        throw new IOException("Failed to create directory " + newFile);
                    }
                } else {
                    File parent = newFile.getParentFile();
                    if (parent != null && !parent.exists() && !parent.mkdirs()) {
                        throw new IOException("Failed to create directory " + parent);
                    }
                    try (FileOutputStream fos = new FileOutputStream(newFile)) {
                        int len;
                        while ((len = zis.read(buffer)) > 0) {
                            fos.write(buffer, 0, len);
                        }
                    }
                    extracted.add(newFile.getAbsolutePath());
                }
                zis.closeEntry();
            }
        }
        return extracted;
    }

    private static File safeNewFile(File destDir, ZipEntry zipEntry) throws IOException {
        File destFile = new File(destDir, zipEntry.getName());
        String destDirPath = destDir.getCanonicalPath();
        String destFilePath = destFile.getCanonicalPath();
        String sep = File.separator;
        if (!destFilePath.startsWith(destDirPath + sep) && !destFilePath.equals(destDirPath)) {
            throw new IOException("Blocked Zip Slip attempt: " + zipEntry.getName());
        }
        return destFile;
    }

    private static void deleteRecursively(File f) throws IOException {
        if (f == null || !f.exists()) return;
        if (f.isDirectory()) {
            File[] children = f.listFiles();
            if (children != null) {
                for (File c : children) {
                    deleteRecursively(c);
                }
            }
        }
        if (!f.delete()) {
            throw new IOException("Failed to delete: " + f.getAbsolutePath());
        }
    }

    private static void createSampleZip(String zipPath) throws IOException {
        try (ZipOutputStream zos = new ZipOutputStream(new BufferedOutputStream(new FileOutputStream(zipPath)))) {
            addEntry(zos, "file1.txt", "hello world 1\n");
            addDir(zos, "dir1/");
            addEntry(zos, "dir1/file2.txt", "hello world 2\n");
            addDir(zos, "dir1/dir2/");
            addEntry(zos, "dir1/dir2/file3.txt", "hello world 3\n");
            addEntry(zos, "dir1/dir2/file4.txt", "hello world 4\n");
            addEntry(zos, "dir1/file5.txt", "hello world 5\n");
        }
    }

    private static void addDir(ZipOutputStream zos, String dirName) throws IOException {
        ZipEntry dirEntry = new ZipEntry(dirName);
        zos.putNextEntry(dirEntry);
        zos.closeEntry();
    }

    private static void addEntry(ZipOutputStream zos, String name, String content) throws IOException {
        ZipEntry entry = new ZipEntry(name);
        zos.putNextEntry(entry);
        byte[] data = content.getBytes("UTF-8");
        zos.write(data, 0, data.length);
        zos.closeEntry();
    }

    public static void main(String[] args) {
        String archive = "archive.zip";
        String dest = "/tmp/unpack";
        try {
            // Prepare environment
            deleteRecursively(new File(dest));
        } catch (IOException ignored) {
        }
        try {
            createSampleZip(archive);

            // 5 test cases (same archive and destination as requested)
            for (int i = 1; i <= 5; i++) {
                List<String> paths = unzipArchive(archive, dest);
                System.out.println("Test " + i + " extracted files: " + paths.size());
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}