import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.zip.GzipInputStream;

public class Lse51 {

    /**
     * Extracts a .tar.gz file to a destination directory.
     * This is a simplified implementation that only handles regular files
     * and does not handle directories or other special file types within the tar.
     *
     * @param tarGzPath The path to the .tar.gz file.
     * @param destPath  The path to the destination directory.
     * @return true if extraction was successful, false otherwise.
     */
    public static boolean extractTarGz(String tarGzPath, String destPath) {
        File destDir = new File(destPath);
        if (!destDir.exists()) {
            destDir.mkdirs();
        }

        try (FileInputStream fis = new FileInputStream(tarGzPath);
             GzipInputStream gzis = new GzipInputStream(fis);
             InputStream taris = new BufferedInputStream(gzis)) {

            byte[] header = new byte[512];
            while (taris.read(header) != -1) {

                // Check for end-of-archive marker (two consecutive null blocks)
                boolean isAllZeros = true;
                for (byte b : header) {
                    if (b != 0) {
                        isAllZeros = false;
                        break;
                    }
                }
                if (isAllZeros) {
                    break;
                }

                // Parse filename (bytes 0-99)
                String fileName = new String(header, 0, 100).trim();
                if (fileName.isEmpty()) {
                    continue;
                }
                
                // File type flag (byte 156), '0' or '\0' is a regular file
                char typeFlag = (char) header[156];
                if (typeFlag != '0' && typeFlag != '\0') {
                    // Skip non-regular files for this simple implementation
                    long sizeToSkip = parseOctal(header, 124, 12);
                    long blocksToSkip = (sizeToSkip % 512 == 0) ? sizeToSkip / 512 : (sizeToSkip / 512) + 1;
                    taris.skip(blocksToSkip * 512);
                    continue;
                }

                // Parse file size (bytes 124-135)
                long fileSize = parseOctal(header, 124, 12);

                File outputFile = new File(destDir, fileName);
                // Ensure parent directory exists for nested files
                if (outputFile.getParentFile() != null) {
                    outputFile.getParentFile().mkdirs();
                }


                try (FileOutputStream fos = new FileOutputStream(outputFile)) {
                    byte[] buffer = new byte[1024];
                    long bytesRemaining = fileSize;
                    while (bytesRemaining > 0) {
                        int bytesToRead = (int) Math.min(bytesRemaining, buffer.length);
                        int bytesRead = taris.read(buffer, 0, bytesToRead);
                        if (bytesRead == -1) {
                            break;
                        }
                        fos.write(buffer, 0, bytesRead);
                        bytesRemaining -= bytesRead;
                    }
                }

                // Skip padding to the next 512-byte boundary
                long padding = (512 - (fileSize % 512)) % 512;
                if (taris.skip(padding) != padding) {
                    System.err.println("Warning: Failed to skip padding bytes for " + fileName);
                }
            }
        } catch (IOException e) {
            System.err.println("Error during extraction: " + e.getMessage());
            return false;
        }
        return true;
    }

    /**
     * Parses an octal number from a byte array as specified in the TAR format.
     */
    private static long parseOctal(byte[] buffer, int offset, int length) {
        String octalStr = new String(buffer, offset, length).trim();
        if (octalStr.length() > 0) {
            // TAR format pads with null characters, which we trim.
            octalStr = octalStr.substring(0, octalStr.indexOf('\0'));
            if(octalStr.isEmpty()) return 0;
            return Long.parseLong(octalStr, 8);
        }
        return 0;
    }

    public static void main(String[] args) {
        System.out.println("--- Running Java Tests ---");
        System.out.println("Note: These tests assume the Python script has been run to create test archives in /tmp/.");

        // Test Case 1: Extract a standard tar.gz archive
        System.out.println("\n[Test 1] Extracting simple.tar.gz to /tmp/java_unpack_1");
        boolean success1 = extractTarGz("/tmp/simple.tar.gz", "/tmp/java_unpack_1");
        System.out.println("Test 1 Success: " + success1);

        // Test Case 2: Extract an archive with subdirectories
        System.out.println("\n[Test 2] Extracting nested.tar.gz to /tmp/java_unpack_2");
        boolean success2 = extractTarGz("/tmp/nested.tar.gz", "/tmp/java_unpack_2");
        System.out.println("Test 2 Success: " + success2);

        // Test Case 3: Attempt to extract a non-existent file
        System.out.println("\n[Test 3] Extracting non_existent.tar.gz to /tmp/java_unpack_3 (expecting error)");
        boolean success3 = extractTarGz("/tmp/non_existent.tar.gz", "/tmp/java_unpack_3");
        System.out.println("Test 3 Success: " + success3);

        // Test Case 4: Extract to a new, non-existent directory
        System.out.println("\n[Test 4] Extracting simple.tar.gz to /tmp/java_new_dir (should be created)");
        boolean success4 = extractTarGz("/tmp/simple.tar.gz", "/tmp/java_new_dir");
        System.out.println("Test 4 Success: " + success4);
        
        // Test Case 5: Extract an empty archive
        System.out.println("\n[Test 5] Extracting empty.tar.gz to /tmp/java_unpack_5");
        boolean success5 = extractTarGz("/tmp/empty.tar.gz", "/tmp/java_unpack_5");
        System.out.println("Test 5 Success: " + success5);
    }
}