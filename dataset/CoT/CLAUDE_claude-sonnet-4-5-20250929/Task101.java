
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.util.HashSet;
import java.util.Set;

public class Task101 {
    
    public static boolean createExecutableScript(String fileName) {
        File file = null;
        FileWriter writer = null;
        
        try {
            // Create the file
            file = new File(fileName);
            
            // Create new file, returns false if file already exists
            if (!file.exists()) {
                file.createNewFile();
            }
            
            // Write a simple shell script header
            writer = new FileWriter(file);
            writer.write("#!/bin/bash\\n");
            writer.write("# This is an executable script\\n");
            writer.flush();
            
            // Make it executable (Unix/Linux/Mac)
            if (System.getProperty("os.name").toLowerCase().contains("win")) {
                // Windows doesn't use the same permission model\n                System.out.println("File created on Windows: " + fileName);\n                return file.setExecutable(true, false);\n            } else {\n                // Unix-based systems - use POSIX permissions\n                Set<PosixFilePermission> perms = new HashSet<>();\n                perms.add(PosixFilePermission.OWNER_READ);\n                perms.add(PosixFilePermission.OWNER_WRITE);\n                perms.add(PosixFilePermission.OWNER_EXECUTE);\n                perms.add(PosixFilePermission.GROUP_READ);\n                perms.add(PosixFilePermission.GROUP_EXECUTE);\n                perms.add(PosixFilePermission.OTHERS_READ);\n                perms.add(PosixFilePermission.OTHERS_EXECUTE);\n                \n                Files.setPosixFilePermissions(Paths.get(fileName), perms);\n                return true;\n            }\n            \n        } catch (IOException e) {\n            System.err.println("Error creating executable script: " + e.getMessage());\n            return false;\n        } catch (UnsupportedOperationException e) {\n            // Fallback for systems that don't support POSIX permissions
            return file != null && file.setExecutable(true, false);
        } finally {
            if (writer != null) {
                try {
                    writer.close();
                } catch (IOException e) {
                    System.err.println("Error closing file writer: " + e.getMessage());
                }
            }
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Create script.sh");
        boolean result1 = createExecutableScript("script.sh");
        System.out.println("Result: " + (result1 ? "Success" : "Failed") + "\\n");
        
        System.out.println("Test Case 2: Create test_script.sh");
        boolean result2 = createExecutableScript("test_script.sh");
        System.out.println("Result: " + (result2 ? "Success" : "Failed") + "\\n");
        
        System.out.println("Test Case 3: Create another_script.sh");
        boolean result3 = createExecutableScript("another_script.sh");
        System.out.println("Result: " + (result3 ? "Success" : "Failed") + "\\n");
        
        System.out.println("Test Case 4: Create deploy.sh");
        boolean result4 = createExecutableScript("deploy.sh");
        System.out.println("Result: " + (result4 ? "Success" : "Failed") + "\\n");
        
        System.out.println("Test Case 5: Create backup.sh");
        boolean result5 = createExecutableScript("backup.sh");
        System.out.println("Result: " + (result5 ? "Success" : "Failed") + "\\n");
    }
}
