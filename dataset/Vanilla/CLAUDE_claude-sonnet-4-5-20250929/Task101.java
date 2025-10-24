
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
            FileWriter writer = new FileWriter(file);
            writer.write("#!/bin/bash\\n");
            writer.write("echo 'Hello from script.sh'\\n");
            writer.close();
            
            // Make it executable
            Set<PosixFilePermission> perms = new HashSet<>();
            perms.add(PosixFilePermission.OWNER_READ);
            perms.add(PosixFilePermission.OWNER_WRITE);
            perms.add(PosixFilePermission.OWNER_EXECUTE);
            perms.add(PosixFilePermission.GROUP_READ);
            perms.add(PosixFilePermission.GROUP_EXECUTE);
            perms.add(PosixFilePermission.OTHERS_READ);
            perms.add(PosixFilePermission.OTHERS_EXECUTE);
            
            Files.setPosixFilePermissions(Paths.get(filename), perms);
            return true;
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Create script.sh
        System.out.println("Test 1: " + createExecutableScript("script.sh"));
        
        // Test case 2: Create test1.sh
        System.out.println("Test 2: " + createExecutableScript("test1.sh"));
        
        // Test case 3: Create test2.sh
        System.out.println("Test 3: " + createExecutableScript("test2.sh"));
        
        // Test case 4: Create test3.sh
        System.out.println("Test 4: " + createExecutableScript("test3.sh"));
        
        // Test case 5: Create test4.sh
        System.out.println("Test 5: " + createExecutableScript("test4.sh"));
    }
}
