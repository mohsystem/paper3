// Chain-of-Through process in code generation:
// 1) Problem understanding: Program deletes a specified file path passed via command-line. Provide a function that accepts a path as parameter and returns success/failure.
// 2) Security requirements: Validate input, avoid deleting directories or symlinks, handle exceptions safely, and operate without following symlinks.
// 3) Secure coding generation: Use java.nio.file with NOFOLLOW_LINKS, ensure path refers to a regular file, and delete using Files.delete with error handling.
// 4) Code review: Checks for null/blank path, invalid path, non-existence, symlink, directory, and exceptions on delete.
// 5) Secure code output: Final method and main with five test cases (if no args). If args exist, process them as requested.

import java.nio.file.*;
import java.nio.file.attribute.*;
import java.util.*;
import java.io.IOException;

public class Task69 {

    // Secure deletion method: returns true if file deleted, false otherwise
    public static boolean deleteFileSecure(String inputPath) {
        if (inputPath == null) return false;
        String trimmed = inputPath.trim();
        if (trimmed.isEmpty()) return false;

        final Path p;
        try {
            p = Paths.get(trimmed);
        } catch (InvalidPathException e) {
            return false;
        }

        try {
            // Do not follow symlinks; only allow regular files
            if (!Files.exists(p, LinkOption.NOFOLLOW_LINKS)) return false;
            if (Files.isDirectory(p, LinkOption.NOFOLLOW_LINKS)) return false;
            if (!Files.isRegularFile(p, LinkOption.NOFOLLOW_LINKS)) return false;

            // Attempt deletion
            Files.delete(p);
            return true;
        } catch (IOException | SecurityException ex) {
            return false;
        }
    }

    public static void main(String[] args) {
        if (args != null && args.length > 0) {
            for (String a : args) {
                boolean result = deleteFileSecure(a);
                System.out.println(a + " -> " + (result ? "deleted" : "failed"));
            }
            return;
        }

        // Five test cases when no arguments are provided
        List<String> pathsToClean = new ArrayList<>();
        try {
            // Test 1: Create temp file and delete (expect true)
            Path temp1 = Files.createTempFile("Task69_", ".tmp");
            pathsToClean.add(temp1.toString()); // In case deletion fails, still track
            boolean t1 = deleteFileSecure(temp1.toString());
            System.out.println("Test1 delete temp file: " + (t1 ? "PASS" : "FAIL"));

            // Test 2: Delete same file again (expect false)
            boolean t2 = deleteFileSecure(temp1.toString());
            System.out.println("Test2 delete non-existent file: " + (!t2 ? "PASS" : "FAIL"));

            // Test 3: Attempt delete a directory (expect false)
            Path tempDir = Files.createTempDirectory("Task69_dir_");
            pathsToClean.add(tempDir.toString());
            boolean t3 = deleteFileSecure(tempDir.toString());
            System.out.println("Test3 delete directory: " + (!t3 ? "PASS" : "FAIL"));

            // Test 4: Empty path (expect false)
            boolean t4 = deleteFileSecure("   ");
            System.out.println("Test4 empty path: " + (!t4 ? "PASS" : "FAIL"));

            // Test 5: Another temp file delete (expect true)
            Path temp2 = Files.createTempFile("Task69_", ".tmp");
            pathsToClean.add(temp2.toString());
            boolean t5 = deleteFileSecure(temp2.toString());
            System.out.println("Test5 second temp file: " + (t5 ? "PASS" : "FAIL"));

            // Cleanup: remove temp directory if still exists
            try {
                if (Files.exists(tempDir, LinkOption.NOFOLLOW_LINKS)) {
                    // Ensure directory is empty then delete
                    try (DirectoryStream<Path> ds = Files.newDirectoryStream(tempDir)) {
                        if (!ds.iterator().hasNext()) {
                            Files.delete(tempDir);
                        }
                    }
                }
            } catch (IOException ignored) {}
        } catch (IOException e) {
            System.out.println("Setup error: " + e.getMessage());
        }
    }
}