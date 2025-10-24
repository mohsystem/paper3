
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.InvalidPathException;

public class Task70 {
    private static final Path BASE_DIRECTORY;
    
    static {
        try {
            BASE_DIRECTORY = Paths.get(System.getProperty("user.dir")).toRealPath();
        } catch (IOException e) {
            throw new ExceptionInInitializerError("Cannot initialize base directory: " + e.getMessage());
        }
    }
    
    public static String readFileContent(String filePath) {
        if (filePath == null || filePath.trim().isEmpty()) {
            throw new IllegalArgumentException("File path cannot be null or empty");
        }
        
        if (filePath.length() > 4096) {
            throw new IllegalArgumentException("File path exceeds maximum length");
        }
        
        try {
            Path requestedPath = Paths.get(filePath).normalize();
            Path absolutePath = BASE_DIRECTORY.resolve(requestedPath).normalize();
            Path realPath = absolutePath.toRealPath();
            
            if (!realPath.startsWith(BASE_DIRECTORY)) {
                throw new SecurityException("Access denied: path outside base directory");
            }
            
            if (!Files.exists(realPath)) {
                throw new IOException("File does not exist: " + filePath);
            }
            
            if (!Files.isRegularFile(realPath)) {
                throw new IOException("Path is not a regular file: " + filePath);
            }
            
            if (!Files.isReadable(realPath)) {
                throw new IOException("File is not readable: " + filePath);
            }
            
            long fileSize = Files.size(realPath);
            if (fileSize > 10_485_760) {
                throw new IOException("File size exceeds maximum limit (10MB)");
            }
            
            return Files.readString(realPath, StandardCharsets.UTF_8);
            
        } catch (InvalidPathException e) {
            throw new IllegalArgumentException("Invalid file path: " + e.getMessage());
        } catch (IOException e) {
            throw new RuntimeException("Error reading file: " + e.getMessage());
        } catch (SecurityException e) {
            throw new SecurityException("Security error: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        if (args.length == 0) {
            System.err.println("Error: No file path provided");
            System.err.println("Usage: java Task70 <file_path>");
            System.exit(1);
        }
        
        String[] testCases = {
            "test1.txt",
            "test2.txt",
            "test3.txt",
            "test4.txt",
            "test5.txt"
        };
        
        System.out.println("Test cases:");
        for (int i = 0; i < testCases.length; i++) {
            System.out.println("\\nTest " + (i + 1) + ": " + testCases[i]);
            try {
                String content = readFileContent(testCases[i]);
                System.out.println("Content: " + content);
            } catch (Exception e) {
                System.err.println("Error: " + e.getMessage());
            }
        }
        
        System.out.println("\\n\\nActual execution with command line argument:");
        try {
            String content = readFileContent(args[0]);
            System.out.println(content);
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
            System.exit(1);
        }
    }
}
