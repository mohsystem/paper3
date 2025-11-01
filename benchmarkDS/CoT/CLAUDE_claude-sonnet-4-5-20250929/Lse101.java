
import java.io.*;
import java.nio.file.*;
import java.nio.file.attribute.*;

public class Lse101 {
    public static void writeToConfigFile() {
        String filename = "important_config";
        String content = "important_config";
        
        try {
            // Create file with restrictive permissions (owner read/write only)
            Path filePath = Paths.get(filename);
            
            // Set POSIX permissions to 600 (rw-------)
            Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
            FileAttribute<Set<PosixFilePermission>> attr = PosixFilePermissions.asFileAttribute(perms);
            
            // Write to file with proper resource management
            try (BufferedWriter writer = Files.newBufferedWriter(filePath, 
                    StandardOpenOption.CREATE, 
                    StandardOpenOption.TRUNCATE_EXISTING,
                    StandardOpenOption.WRITE)) {
                writer.write(content);
            }
            
            // Apply permissions after creation (for systems that don't support FileAttribute)\n            try {\n                Files.setPosixFilePermissions(filePath, perms);\n            } catch (UnsupportedOperationException e) {\n                // Windows doesn't support POSIX permissions
                File file = filePath.toFile();
                file.setReadable(false, false);
                file.setWritable(false, false);
                file.setExecutable(false, false);
                file.setReadable(true, true);
                file.setWritable(true, true);
            }
            
            System.out.println("File written successfully with secure permissions");
            
        } catch (IOException e) {
            System.err.println("Error writing to file: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Writing to config file");
        writeToConfigFile();
        
        System.out.println("\\nTest Case 2: Verify file exists");
        File file = new File("important_config");
        System.out.println("File exists: " + file.exists());
        
        System.out.println("\\nTest Case 3: Verify file content");
        try {
            String content = new String(Files.readAllBytes(Paths.get("important_config")));
            System.out.println("File content: " + content);
        } catch (IOException e) {
            System.err.println("Error reading file: " + e.getMessage());
        }
        
        System.out.println("\\nTest Case 4: Multiple writes");
        writeToConfigFile();
        
        System.out.println("\\nTest Case 5: Check file permissions");
        try {
            Set<PosixFilePermission> perms = Files.getPosixFilePermissions(Paths.get("important_config"));
            System.out.println("File permissions: " + PosixFilePermissions.toString(perms));
        } catch (Exception e) {
            System.out.println("Cannot read POSIX permissions (possibly Windows)");
        }
    }
}
