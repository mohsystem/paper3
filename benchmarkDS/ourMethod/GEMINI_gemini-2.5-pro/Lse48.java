import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.LinkOption;
import java.nio.file.Path;
import java.nio.file.Paths;

public class Lse48 {

    private static final String BASE_IMAGE_DIR = "test_images_java";
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 MB

    /**
     * Safely reads a file specified by a filename from a base directory.
     * Prevents path traversal vulnerabilities.
     *
     * @param file The filename provided by the user.
     * @return A byte array with the file's content, or null on failure.
     */
    public static byte[] img(String file) {
        if (file == null || file.isEmpty()) {
            System.err.println("Error: Filename is null or empty.");
            return null;
        }

        // Rules #1 & #5: Sanitize input and prevent path traversal.
        // Get only the filename part, discarding any directory info.
        Path fileNamePath = Paths.get(file).getFileName();
        if (fileNamePath == null) {
             System.err.println("Error: Invalid filename format.");
             return null;
        }
        String sanitizedFileName = fileNamePath.toString();

        // An extra check to ensure the sanitized name is what was expected
        // and doesn't contain traversal sequences.
        if (!sanitizedFileName.equals(file) || sanitizedFileName.contains("..")) {
            System.err.println("Error: Directory traversal attempt detected in input '" + file + "'.");
            return null;
        }

        try {
            Path baseDir = Paths.get(BASE_IMAGE_DIR).toRealPath(LinkOption.NOFOLLOW_LINKS);
            Path requestedPath = baseDir.resolve(sanitizedFileName).normalize();

            // Final security check to ensure the resolved path is within the base directory.
            if (!requestedPath.startsWith(baseDir)) {
                System.err.println("Error: Path traversal attempt detected.");
                return null;
            }
            
            // Rules #6 & #7: Validate file properties securely before reading.
            // Files.isRegularFile with NOFOLLOW_LINKS checks type without following symlinks.
            if (!Files.exists(requestedPath, LinkOption.NOFOLLOW_LINKS) ||
                !Files.isRegularFile(requestedPath, LinkOption.NOFOLLOW_LINKS)) {
                System.err.println("Error: File does not exist or is not a regular file.");
                return null;
            }

            // Rule #3: Check file size before reading to prevent DoS.
            long fileSize = Files.size(requestedPath);
            if (fileSize > MAX_FILE_SIZE) {
                System.err.println("Error: File size exceeds the maximum limit of " + MAX_FILE_SIZE + " bytes.");
                return null;
            }

            return Files.readAllBytes(requestedPath);

        } catch (IOException | SecurityException e) {
            System.err.println("Error reading file: " + e.getMessage());
            return null;
        }
    }
    
    public static void main(String[] args) {
        // Setup test environment
        File baseDir = new File(BASE_IMAGE_DIR);
        if (!baseDir.exists()) baseDir.mkdirs();
        
        File validFile = new File(baseDir, "test.txt");
        File nestedDir = new File(baseDir, "subdir");
        if (!nestedDir.exists()) nestedDir.mkdirs();
        File nestedFile = new File(nestedDir, "nested.txt");

        try {
            Files.write(validFile.toPath(), "valid content".getBytes());
            Files.write(nestedFile.toPath(), "nested content".getBytes());
        } catch (IOException e) {
            System.err.println("Failed to set up test files.");
            e.printStackTrace();
            return;
        }

        System.out.println("--- Java Test Cases ---");

        // Test Case 1: Valid file
        System.out.println("\n1. Testing valid file 'test.txt'");
        byte[] content1 = img("test.txt");
        if (content1 != null) {
            System.out.println("Success. Content: " + new String(content1));
        } else {
            System.out.println("Failed.");
        }

        // Test Case 2: Non-existent file
        System.out.println("\n2. Testing non-existent file 'notfound.txt'");
        byte[] content2 = img("notfound.txt");
        if (content2 == null) {
            System.out.println("Success. File not read as expected.");
        } else {
            System.out.println("Failed.");
        }

        // Test Case 3: Path traversal attempt
        System.out.println("\n3. Testing path traversal '../test.txt'");
        byte[] content3 = img("../test.txt");
        if (content3 == null) {
            System.out.println("Success. Path traversal blocked as expected.");
        } else {
            System.out.println("Failed.");
        }

        // Test Case 4: Absolute path
        String absolutePath = validFile.getAbsolutePath();
        System.out.println("\n4. Testing absolute path '" + absolutePath + "'");
        byte[] content4 = img(absolutePath);
        if (content4 == null) {
            System.out.println("Success. Absolute path blocked as expected.");
        } else {
            System.out.println("Failed.");
        }

        // Test Case 5: Nested file (should be blocked by sanitization)
        System.out.println("\n5. Testing nested path 'subdir/nested.txt'");
        byte[] content5 = img("subdir/nested.txt");
        if (content5 == null) {
            System.out.println("Success. Nested path blocked as expected.");
        } else {
            System.out.println("Failed.");
        }

        // Cleanup
        nestedFile.delete();
        nestedDir.delete();
        validFile.delete();
        baseDir.delete();
    }
}