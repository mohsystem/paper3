
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.attribute.PosixFilePermission;
import java.security.SecureRandom;
import java.util.HashSet;
import java.util.Set;

public class Task88 {
    private static final String CHARACTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    private static final int RANDOM_STRING_LENGTH = 20;
    
    public static String createTempFileWithRandomString() throws IOException {
        // Create temporary file with UTF-8 encoding
        Path tempFile = Files.createTempFile("secure_temp_", ".txt");
        File file = tempFile.toFile();
        
        // Set restrictive file permissions (owner read/write only)
        try {
            Set<PosixFilePermission> perms = new HashSet<>();
            perms.add(PosixFilePermission.OWNER_READ);
            perms.add(PosixFilePermission.OWNER_WRITE);
            Files.setPosixFilePermissions(tempFile, perms);
        } catch (UnsupportedOperationException e) {
            // Windows doesn't support POSIX permissions
            file.setReadable(true, true);
            file.setWritable(true, true);
            file.setExecutable(false, false);
        }
        
        // Generate secure random string
        String randomString = generateSecureRandomString(RANDOM_STRING_LENGTH);
        
        // Convert to Unicode (UTF-8) and write to file
        try (BufferedWriter writer = new BufferedWriter(
                new FileWriter(file, StandardCharsets.UTF_8))) {
            writer.write(randomString);
            writer.flush();
        }
        
        // Return absolute path
        return tempFile.toAbsolutePath().toString();
    }
    
    private static String generateSecureRandomString(int length) {
        SecureRandom secureRandom = new SecureRandom();
        StringBuilder sb = new StringBuilder(length);
        
        for (int i = 0; i < length; i++) {
            int randomIndex = secureRandom.nextInt(CHARACTERS.length());
            sb.append(CHARACTERS.charAt(randomIndex));
        }
        
        return sb.toString();
    }
    
    public static void main(String[] args) {
        System.out.println("Testing temporary file creation with random Unicode strings:\\n");
        
        for (int i = 1; i <= 5; i++) {
            try {
                String filePath = createTempFileWithRandomString();
                System.out.println("Test " + i + ": " + filePath);
                
                // Verify file content
                String content = Files.readString(Path.of(filePath), StandardCharsets.UTF_8);
                System.out.println("  Content: " + content);
                System.out.println("  Length: " + content.length() + " characters\\n");
            } catch (IOException e) {
                System.err.println("Test " + i + " failed: " + e.getMessage());
            }
        }
    }
}
