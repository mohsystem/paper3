
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.util.HashSet;
import java.util.Set;

public class Task101 {
    public static boolean createExecutableScript(String filename) {
        try {
            // Create the file
            File file = new File(filename);
            if (file.createNewFile()) {
                // Write a simple shell script header
                FileWriter writer = new FileWriter(file);
                writer.write("#!/bin/bash\\n");
                writer.close();
                
                // Make it executable (Unix/Linux/Mac)
                if (System.getProperty("os.name").toLowerCase().contains("win")) {
                    // Windows - files are executable by default
                    return true;
                } else {
                    // Unix-based systems
                    Set<PosixFilePermission> perms = new HashSet<>();
                    perms.add(PosixFilePermission.OWNER_READ);
                    perms.add(PosixFilePermission.OWNER_WRITE);
                    perms.add(PosixFilePermission.OWNER_EXECUTE);
                    perms.add(PosixFilePermission.GROUP_READ);
                    perms.add(PosixFilePermission.GROUP_EXECUTE);
                    perms.add(PosixFilePermission.OTHERS_READ);
                    perms.add(PosixFilePermission.OTHERS_EXECUTE);
                    Files.setPosixFilePermissions(Paths.get(filename), perms);
                }
                return true;
            } else {
                System.out.println("File already exists.");
                return false;
            }
        } catch (IOException e) {
            System.err.println("Error creating file: " + e.getMessage());
            return false;
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Create script.sh
        System.out.println("Test 1: Creating script.sh");
        boolean result1 = createExecutableScript("script.sh");
        System.out.println("Result: " + (result1 ? "Success" : "Failed"));
        
        // Test case 2: Create test1.sh
        System.out.println("\\nTest 2: Creating test1.sh");
        boolean result2 = createExecutableScript("test1.sh");
        System.out.println("Result: " + (result2 ? "Success" : "Failed"));
        
        // Test case 3: Create test2.sh
        System.out.println("\\nTest 3: Creating test2.sh");
        boolean result3 = createExecutableScript("test2.sh");
        System.out.println("Result: " + (result3 ? "Success" : "Failed"));
        
        // Test case 4: Try to create the same file again
        System.out.println("\\nTest 4: Creating script.sh again (should fail)");
        boolean result4 = createExecutableScript("script.sh");
        System.out.println("Result: " + (result4 ? "Success" : "Failed"));
        
        // Test case 5: Create another script
        System.out.println("\\nTest 5: Creating deploy.sh");
        boolean result5 = createExecutableScript("deploy.sh");
        System.out.println("Result: " + (result5 ? "Success" : "Failed"));
        
        // Cleanup test files
        new File("script.sh").delete();
        new File("test1.sh").delete();
        new File("test2.sh").delete();
        new File("deploy.sh").delete();
    }
}
