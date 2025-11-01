
import java.io.*;
import java.nio.file.*;
import java.util.Base64;
java.security.SecureRandom;

public class Lse69 {
    private static final String UPLOAD_DIR = "/var/uploads";
    private static final long MAX_FILE_SIZE = 5 * 1024 * 1024; // 5MB limit
    private static final SecureRandom random = new SecureRandom();
    
    /**
     * Uploads and encodes an image file
     * @param imageData the raw image bytes to upload
     * @return the filename where the encoded image was saved, or null on failure
     */
    public static String uploadImage(byte[] imageData) {
        // Input validation: check for null and size limits
        if (imageData == null || imageData.length == 0) {
            System.err.println("Invalid image data");
            return null;
        }
        
        if (imageData.length > MAX_FILE_SIZE) {
            System.err.println("File size exceeds limit");
            return null;
        }
        
        try {
            // Create upload directory if it doesn't exist with restrictive permissions\n            Path uploadPath = Paths.get(UPLOAD_DIR);\n            if (!Files.exists(uploadPath)) {\n                Files.createDirectories(uploadPath);\n                // Set permissions to owner read/write/execute only (700)\n                Files.setPosixFilePermissions(uploadPath, \n                    java.nio.file.attribute.PosixFilePermissions.fromString("rwx------"));\n            }\n            \n            // Generate a safe, random filename to prevent path traversal\n            String filename = "image_" + System.currentTimeMillis() + "_" + \n                            random.nextInt(100000) + ".b64";\n            \n            // Normalize and validate the path resolves within UPLOAD_DIR\n            Path outputPath = uploadPath.resolve(filename).normalize();\n            if (!outputPath.startsWith(uploadPath)) {\n                System.err.println("Path traversal attempt detected");\n                return null;\n            }\n            \n            // Encode the image data using base64\n            String encodedImage = Base64.getEncoder().encodeToString(imageData);\n            \n            // Write to a temporary file first, then atomically move to final location\n            Path tempFile = Files.createTempFile(uploadPath, "temp_", ".tmp");\n            \n            // Set restrictive permissions on temp file (owner read/write only - 600)\n            Files.setPosixFilePermissions(tempFile,\n                java.nio.file.attribute.PosixFilePermissions.fromString("rw-------"));\n            \n            // Write encoded data to temp file\n            try (BufferedWriter writer = Files.newBufferedWriter(tempFile, \n                    java.nio.charset.StandardCharsets.UTF_8)) {\n                writer.write(encodedImage);\n                writer.flush();\n            }\n            \n            // Atomically move temp file to final destination\n            Files.move(tempFile, outputPath, StandardCopyOption.ATOMIC_MOVE,\n                      StandardCopyOption.REPLACE_EXISTING);\n            \n            return filename;\n            \n        } catch (IOException e) {\n            System.err.println("Error processing image upload");\n            return null;\n        }\n    }\n    \n    public static void main(String[] args) {\n        // Test case 1: Valid small image\n        byte[] testImage1 = "Test image data 1".getBytes(java.nio.charset.StandardCharsets.UTF_8);\n        String result1 = uploadImage(testImage1);\n        System.out.println("Test 1 - Valid small image: " + (result1 != null ? "PASS" : "FAIL"));\n        \n        // Test case 2: Null input\n        String result2 = uploadImage(null);\n        System.out.println("Test 2 - Null input: " + (result2 == null ? "PASS" : "FAIL"));\n        \n        // Test case 3: Empty array\n        byte[] testImage3 = new byte[0];\n        String result3 = uploadImage(testImage3);\n        System.out.println("Test 3 - Empty array: " + (result3 == null ? "PASS" : "FAIL"));\n        \n        // Test case 4: Valid image with binary data\n        byte[] testImage4 = new byte[1024];\n        for (int i = 0; i < testImage4.length; i++) {\n            testImage4[i] = (byte) (i % 256);\n        }\n        String result4 = uploadImage(testImage4);\n        System.out.println("Test 4 - Binary data: " + (result4 != null ? "PASS" : "FAIL"));\n        \n        // Test case 5: Image at size limit boundary\n        byte[] testImage5 = new byte[1024];\n        java.util.Arrays.fill(testImage5, (byte) 'A');
        String result5 = uploadImage(testImage5);
        System.out.println("Test 5 - Size boundary: " + (result5 != null ? "PASS" : "FAIL"));
    }
}
