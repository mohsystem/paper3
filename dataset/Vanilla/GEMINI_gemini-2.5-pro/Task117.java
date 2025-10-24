import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Comparator;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipOutputStream;

public class Task117 {

    /**
     * Extracts a zip file to a specified destination directory.
     * Note: Standard Java library does not support TAR archives without external libraries.
     *
     * @param archivePath Path to the archive file.
     * @param destDirPath Path to the destination directory.
     */
    public static boolean extractArchive(String archivePath, String destDirPath) {
        if (!archivePath.toLowerCase().endsWith(".zip")) {
            System.err.println("Unsupported archive type: " + archivePath + ". Only .zip is supported.");
            return false;
        }

        File destDir = new File(destDirPath);
        if (!destDir.exists()) {
            destDir.mkdirs();
        }

        byte[] buffer = new byte[1024];
        try (ZipInputStream zis = new ZipInputStream(new FileInputStream(archivePath))) {
            ZipEntry zipEntry = zis.getNextEntry();
            while (zipEntry != null) {
                File newFile = newFile(destDir, zipEntry);
                if (zipEntry.isDirectory()) {
                    if (!newFile.isDirectory() && !newFile.mkdirs()) {
                        throw new IOException("Failed to create directory " + newFile);
                    }
                } else {
                    // fix for Windows-created archives
                    File parent = newFile.getParentFile();
                    if (!parent.isDirectory() && !parent.mkdirs()) {
                        throw new IOException("Failed to create directory " + parent);
                    }

                    // write file content
                    try (FileOutputStream fos = new FileOutputStream(newFile)) {
                        int len;
                        while ((len = zis.read(buffer)) > 0) {
                            fos.write(buffer, 0, len);
                        }
                    }
                }
                zipEntry = zis.getNextEntry();
            }
            zis.closeEntry();
            return true;
        } catch (IOException e) {
            System.err.println("Error extracting archive: " + e.getMessage());
            return false;
        }
    }

    /**
     * Helper to prevent Zip Slip vulnerability.
     */
    private static File newFile(File destinationDir, ZipEntry zipEntry) throws IOException {
        File destFile = new File(destinationDir, zipEntry.getName());
        String destDirPath = destinationDir.getCanonicalPath();
        String destFilePath = destFile.getCanonicalPath();
        if (!destFilePath.startsWith(destDirPath + File.separator)) {
            throw new IOException("Entry is outside of the target dir: " + zipEntry.getName());
        }
        return destFile;
    }

    // Helper to create a test zip file
    private static void createTestZip(String zipFilePath, String[] fileNames, String[] contents) throws IOException {
        try (FileOutputStream fos = new FileOutputStream(zipFilePath);
             ZipOutputStream zos = new ZipOutputStream(fos)) {
            for (int i = 0; i < fileNames.length; i++) {
                ZipEntry entry = new ZipEntry(fileNames[i]);
                zos.putNextEntry(entry);
                if (contents[i] != null && !contents[i].isEmpty()) {
                    zos.write(contents[i].getBytes());
                }
                zos.closeEntry();
            }
        }
    }
    
    // Helper to recursively delete a directory
    private static void deleteDirectory(File directory) throws IOException {
        if (directory.exists()) {
             Files.walk(directory.toPath())
                .sorted(Comparator.reverseOrder())
                .map(Path::toFile)
                .forEach(File::delete);
        }
    }


    public static void main(String[] args) {
        String testDir = "java_test_area";
        String outputDir = testDir + File.separator + "output";
        
        // Create test directory
        new File(testDir).mkdirs();

        try {
            // Test Case 1: Simple ZIP
            System.out.println("--- Test Case 1: Simple ZIP ---");
            String zip1 = testDir + File.separator + "test1.zip";
            String out1 = outputDir + File.separator + "test1";
            createTestZip(zip1, new String[]{"hello.txt"}, new String[]{"Hello World!"});
            System.out.println("Extracting " + zip1 + " to " + out1);
            if (extractArchive(zip1, out1)) {
                System.out.println("Extraction successful.\n");
            } else {
                 System.out.println("Extraction failed.\n");
            }

            // Test Case 2: ZIP with subdirectory
            System.out.println("--- Test Case 2: ZIP with subdirectory ---");
            String zip2 = testDir + File.separator + "test2.zip";
            String out2 = outputDir + File.separator + "test2";
            createTestZip(zip2, new String[]{"folder/", "folder/file.txt"}, new String[]{"", "Content in a folder."});
            System.out.println("Extracting " + zip2 + " to " + out2);
            if(extractArchive(zip2, out2)) {
                System.out.println("Extraction successful.\n");
            } else {
                 System.out.println("Extraction failed.\n");
            }

            // Test Case 3: Empty ZIP
            System.out.println("--- Test Case 3: Empty ZIP ---");
            String zip3 = testDir + File.separator + "test3.zip";
            String out3 = outputDir + File.separator + "test3";
            createTestZip(zip3, new String[]{}, new String[]{});
            System.out.println("Extracting " + zip3 + " to " + out3);
            if(extractArchive(zip3, out3)) {
                System.out.println("Extraction successful.\n");
            } else {
                 System.out.println("Extraction failed.\n");
            }

            // Test Case 4: Non-existent archive
            System.out.println("--- Test Case 4: Non-existent archive ---");
            String zip4 = testDir + File.separator + "nonexistent.zip";
            String out4 = outputDir + File.separator + "test4";
            System.out.println("Extracting " + zip4 + " to " + out4);
            if(extractArchive(zip4, out4)) {
                System.out.println("Extraction successful.\n");
            } else {
                 System.out.println("Extraction failed as expected.\n");
            }

            // Test Case 5: Unsupported file type
            System.out.println("--- Test Case 5: Unsupported file type ---");
            String tar5 = testDir + File.separator + "test5.tar";
            String out5 = outputDir + File.separator + "test5";
            new File(tar5).createNewFile(); // Create empty dummy file
            System.out.println("Extracting " + tar5 + " to " + out5);
            if(extractArchive(tar5, out5)) {
                System.out.println("Extraction successful.\n");
            } else {
                 System.out.println("Extraction failed as expected.\n");
            }

        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            // Cleanup
            try {
                System.out.println("Cleaning up test files and directories...");
                deleteDirectory(new File(testDir));
                System.out.println("Cleanup complete.");
            } catch (IOException e) {
                System.err.println("Error during cleanup: " + e.getMessage());
            }
        }
    }
}