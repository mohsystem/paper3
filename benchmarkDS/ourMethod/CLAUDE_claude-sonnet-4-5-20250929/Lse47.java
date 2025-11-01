
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.File;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

public class Lse47 {
    private static final String BASE_DIR = "images";
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit

    /**
     * Validates and safely resolves a file path within the base directory.
     * Returns null if the path is invalid or outside the base directory.
     */
    private static Path validateAndResolvePath(String filename) {
        if (filename == null || filename.isEmpty()) {
            return null;
        }

        // Reject path traversal patterns
        if (filename.contains("..") || filename.contains("/") || filename.contains("\\\\")) {\n            return null;\n        }\n\n        try {\n            // Normalize and resolve the full path\n            Path baseDir = Paths.get(BASE_DIR).toRealPath();\n            Path requestedPath = baseDir.resolve(filename).normalize();\n\n            // Ensure the resolved path is within the base directory\n            if (!requestedPath.startsWith(baseDir)) {\n                return null;\n            }\n\n            return requestedPath;\n        } catch (IOException e) {\n            return null;\n        }\n    }\n\n    /**\n     * Handles image file requests with secure path validation.\n     */\n    public static byte[] serveImage(String filename) {\n        Path filePath = validateAndResolvePath(filename);\n        \n        if (filePath == null) {\n            return null;\n        }\n\n        try {\n            // Check if file exists and is a regular file (not directory or symlink)\n            if (!Files.exists(filePath) || !Files.isRegularFile(filePath, java.nio.file.LinkOption.NOFOLLOW_LINKS)) {\n                return null;\n            }\n\n            // Check file size before reading\n            long fileSize = Files.size(filePath);\n            if (fileSize > MAX_FILE_SIZE) {\n                return null;\n            }\n\n            // Read and return file contents\n            return Files.readAllBytes(filePath);\n        } catch (IOException e) {\n            return null;\n        }\n    }\n\n    public static void main(String[] args) {\n        // Create images directory if it doesn't exist\n        File imagesDir = new File(BASE_DIR);\n        if (!imagesDir.exists()) {\n            imagesDir.mkdir();\n        }\n\n        // Test case setup - create test files\n        try {\n            Files.write(Paths.get(BASE_DIR, "test1.jpg"), "test image 1".getBytes());\n            Files.write(Paths.get(BASE_DIR, "test2.png"), "test image 2".getBytes());\n            Files.write(Paths.get(BASE_DIR, "valid.gif"), "test image 3".getBytes());\n        } catch (IOException e) {\n            System.err.println("Failed to create test files");\n        }\n\n        // Test cases\n        System.out.println("Test 1 - Valid file:");\n        byte[] result1 = serveImage("test1.jpg");\n        System.out.println(result1 != null ? "SUCCESS - File served" : "FAILED");\n\n        System.out.println("\
Test 2 - Another valid file:");\n        byte[] result2 = serveImage("test2.png");\n        System.out.println(result2 != null ? "SUCCESS - File served" : "FAILED");\n\n        System.out.println("\
Test 3 - Non-existent file:");\n        byte[] result3 = serveImage("nonexistent.jpg");\n        System.out.println(result3 == null ? "SUCCESS - Rejected" : "FAILED");\n\n        System.out.println("\
Test 4 - Path traversal attempt:");\n        byte[] result4 = serveImage("../etc/passwd");\n        System.out.println(result4 == null ? "SUCCESS - Rejected" : "FAILED");\n\n        System.out.println("\
Test 5 - Path with slash:");\n        byte[] result5 = serveImage("subdir/file.jpg");\n        System.out.println(result5 == null ? "SUCCESS - Rejected" : "FAILED");
    }
}
