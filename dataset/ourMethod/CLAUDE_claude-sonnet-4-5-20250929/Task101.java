
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.Set;

public class Task101 {
    public static boolean createExecutableScript(String baseDirectory, String filename) {
        if (baseDirectory == null || filename == null || baseDirectory.isEmpty() || filename.isEmpty()) {
            return false;
        }
        
        if (filename.contains("..") || filename.contains("/") || filename.contains("\\\\")) {\n            return false;\n        }\n        \n        try {\n            Path basePath = Paths.get(baseDirectory).toAbsolutePath().normalize();\n            Path filePath = basePath.resolve(filename).normalize();\n            \n            if (!filePath.startsWith(basePath)) {\n                return false;\n            }\n            \n            if (Files.exists(filePath)) {\n                if (!Files.isRegularFile(filePath) || Files.isSymbolicLink(filePath)) {\n                    return false;\n                }\n            }\n            \n            Path tempFile = Files.createTempFile(basePath, ".tmp", ".sh");\n            \n            try {\n                Files.write(tempFile, "#!/bin/bash\
".getBytes(java.nio.charset.StandardCharsets.UTF_8));\n                \n                String os = System.getProperty("os.name").toLowerCase();\n                if (os.contains("win")) {\n                    Files.move(tempFile, filePath, java.nio.file.StandardCopyOption.ATOMIC_MOVE, \n                              java.nio.file.StandardCopyOption.REPLACE_EXISTING);\n                } else {\n                    Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rwxr-xr-x");\n                    Files.setPosixFilePermissions(tempFile, perms);\n                    Files.move(tempFile, filePath, java.nio.file.StandardCopyOption.ATOMIC_MOVE, \n                              java.nio.file.StandardCopyOption.REPLACE_EXISTING);\n                }\n                \n                return true;\n            } catch (Exception e) {\n                Files.deleteIfExists(tempFile);\n                throw e;\n            }\n        } catch (IOException | UnsupportedOperationException e) {\n            return false;\n        }\n    }\n    \n    public static void main(String[] args) {\n        String testDir = System.getProperty("java.io.tmpdir");\n        \n        System.out.println("Test 1 (valid): " + createExecutableScript(testDir, "script.sh"));\n        System.out.println("Test 2 (valid different name): " + createExecutableScript(testDir, "test_script.sh"));\n        System.out.println("Test 3 (path traversal attempt): " + createExecutableScript(testDir, "../script.sh"));\n        System.out.println("Test 4 (null filename): " + createExecutableScript(testDir, null));\n        System.out.println("Test 5 (empty filename): " + createExecutableScript(testDir, ""));
    }
}
