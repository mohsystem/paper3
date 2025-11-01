import java.io.IOException;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermission;
import java.util.HashSet;
import java.util.Set;

public class Lse106 {

    /**
     * Creates a file, writes content to it, and sets its permissions to owner-read-only.
     * This permission setting is POSIX-specific. On other systems like Windows,
     * it will print a warning as equivalent permissions are not directly available.
     *
     * @param fileName The name of the file to create.
     * @param content  The super secret info to write to the file.
     * @return The name of the file created.
     */
    public static String createSecretFile(String fileName, String content) {
        Path filePath = Paths.get(fileName);

        try {
            // 1. & 2. Create a file and write the super secret info to it.
            Files.write(filePath, content.getBytes(), StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);

            // 3. Change the permissions of the file to read-only for the owner.
            // This is primarily for POSIX-compliant file systems (Linux, macOS, etc.)
            if (FileSystems.getDefault().supportedFileAttributeViews().contains("posix")) {
                Set<PosixFilePermission> perms = new HashSet<>();
                perms.add(PosixFilePermission.OWNER_READ);
                Files.setPosixFilePermissions(filePath, perms);
            } else {
                System.err.println("\nWarning: POSIX permissions not supported. Could not set owner-only read permission for " + fileName);
            }

            // 4. Print the name of the file to the screen.
            System.out.println(fileName);
            return fileName;

        } catch (IOException e) {
            System.err.println("An error occurred while processing " + fileName + ": " + e.getMessage());
            return null;
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Running Java Test Cases ---");
        for (int i = 0; i < 5; i++) {
            String fileName = "java_secret_file_" + i + ".txt";
            String secretInfo = "This is Java secret #" + i + ". The password is 'alohomora'.";
            System.out.print("Created file: ");
            createSecretFile(fileName, secretInfo);
        }
        System.out.println("-----------------------------\n");
    }
}