import java.io.IOException;
import java.io.OutputStream;
import java.nio.file.*;

public class Lse72 {

    /**
     * Securely uploads a file to a designated 'uploads' directory.
     * The 'uploads' directory is created in the current working directory.
     * This function prevents path traversal attacks and race conditions (TOCTOU).
     *
     * @param filename The name of the file to be saved. This will be sanitized.
     * @param content  The byte content of the file.
     * @return true if the upload was successful, false otherwise.
     */
    public static boolean uploadFile(String filename, byte[] content) {
        if (filename == null || filename.trim().isEmpty() || content == null) {
            System.err.println("Error: Filename or content is null or empty.");
            return false;
        }

        try {
            // Define the base directory for uploads in the current working directory.
            Path uploadsDir = Paths.get("uploads").toAbsolutePath();

            // Create the directory if it doesn't exist.
            if (!Files.exists(uploadsDir)) {
                Files.createDirectories(uploadsDir);
            } else if (!Files.isDirectory(uploadsDir)) {
                System.err.println("Error: 'uploads' exists but is not a directory.");
                return false;
            }

            // Sanitize the filename to prevent path traversal.
            // This extracts the final component of the path, discarding any directory info.
            Path fileNamePath = Paths.get(filename).getFileName();
            if (fileNamePath == null) {
                System.err.println("Error: Invalid filename provided.");
                return false;
            }
            String sanitizedFilename = fileNamePath.toString();
            if (sanitizedFilename.isEmpty()) {
                 System.err.println("Error: Filename is empty after sanitization.");
                 return false;
            }

            // Construct the destination path and normalize it.
            Path destPath = uploadsDir.resolve(sanitizedFilename).normalize();

            // Security Check: Ensure the final path is strictly within the 'uploads' directory.
            // This is a crucial step to prevent path traversal attacks that might bypass
            // the initial sanitization.
            if (!destPath.startsWith(uploadsDir)) {
                System.err.println("Error: Path traversal attempt detected.");
                return false;
            }

            // Write the file using CREATE_NEW to prevent overwriting existing files
            // and to perform an atomic "check-and-create" operation, mitigating TOCTOU vulnerabilities.
            try (OutputStream os = Files.newOutputStream(destPath, StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE)) {
                os.write(content);
            } catch (FileAlreadyExistsException e) {
                System.err.println("Error: File already exists: " + destPath);
                return false;
            } catch (IOException e) {
                System.err.println("Error writing to file: " + e.getMessage());
                return false;
            }

            System.out.println("Successfully uploaded file to: " + destPath);
            return true;

        } catch (InvalidPathException e) {
            System.err.println("Error: Invalid characters in filename: " + e.getMessage());
            return false;
        } catch (IOException e) {
            System.err.println("Error creating uploads directory: " + e.getMessage());
            return false;
        } catch (SecurityException e) {
            System.err.println("Security error: " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Running Test Cases ---");

        // Test Case 1: Valid upload
        System.out.println("\n--- Test Case 1: Valid upload ---");
        uploadFile("test1.txt", "This is a valid test file.".getBytes());

        // Test Case 2: Path traversal attempt
        System.out.println("\n--- Test Case 2: Path traversal attempt ---");
        uploadFile("../test2.txt", "This should fail.".getBytes());
        uploadFile("subdir/../../test2_b.txt", "This should also fail.".getBytes());

        // Test Case 3: Empty/null inputs
        System.out.println("\n--- Test Case 3: Empty/null inputs ---");
        uploadFile("", "Empty filename.".getBytes());
        uploadFile(null, "Null filename.".getBytes());
        uploadFile("test3.txt", null);

        // Test Case 4: Re-uploading the same file (should fail)
        System.out.println("\n--- Test Case 4: Re-uploading same file ---");
        uploadFile("test1.txt", "This should fail because it already exists.".getBytes());

        // Test Case 5: Filename with path components (should be sanitized)
        System.out.println("\n--- Test Case 5: Filename with path components ---");
        uploadFile("/etc/passwd", "This content should not be written to /etc/passwd".getBytes());
        uploadFile("C:\\Users\\test\\myfile.txt", "This should be saved as myfile.txt in uploads".getBytes());
        
        System.out.println("\n--- Test Cases Finished ---");
    }
}