
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.InvalidPathException;

public class Task69 {
    private static final String BASE_DIRECTORY = System.getProperty("user.dir");
    
    public static boolean deleteFile(String filePath) {
        if (filePath == null || filePath.trim().isEmpty()) {
            System.err.println("Error: File path is null or empty");
            return false;
        }
        
        try {
            Path basePath = Paths.get(BASE_DIRECTORY).toRealPath();
            Path targetPath = Paths.get(filePath).normalize();
            
            if (!targetPath.isAbsolute()) {
                targetPath = basePath.resolve(targetPath).normalize();
            }
            
            Path realTargetPath = targetPath.toRealPath();
            
            if (!realTargetPath.startsWith(basePath)) {
                System.err.println("Error: Path traversal attempt detected");
                return false;
            }
            
            if (Files.isDirectory(realTargetPath)) {
                System.err.println("Error: Path is a directory, not a regular file");
                return false;
            }
            
            if (Files.isSymbolicLink(targetPath)) {
                System.err.println("Error: Symbolic links are not allowed");
                return false;
            }
            
            if (!Files.isRegularFile(realTargetPath)) {
                System.err.println("Error: Not a regular file");
                return false;
            }
            
            Files.delete(realTargetPath);
            System.out.println("File deleted successfully: " + realTargetPath.getFileName());
            return true;
            
        } catch (InvalidPathException e) {
            System.err.println("Error: Invalid path format");
            return false;
        } catch (IOException e) {
            System.err.println("Error: File does not exist or cannot be deleted");
            return false;
        } catch (SecurityException e) {
            System.err.println("Error: Permission denied");
            return false;
        }
    }
    
    public static void main(String[] args) {
        if (args.length == 0) {
            System.out.println("Test cases:");
            String[] testCases = {
                "test1.txt",
                "test2.txt",
                "../outside.txt",
                "test3.txt",
                ""
            };
            
            for (String testCase : testCases) {
                System.out.println("\\nTest: " + (testCase.isEmpty() ? "(empty)" : testCase));
                deleteFile(testCase);
            }
        } else {
            deleteFile(args[0]);
        }
    }
}
