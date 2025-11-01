// Chain-of-Through secure implementation in Java
// 1) Problem understanding: create "important_config" with secure permissions.
// 2) Security requirements: avoid race conditions, ensure restrictive permissions, validate filename.
// 3) Secure coding generation: use atomic creation, POSIX permissions where supported, safe fallbacks.
// 4) Code review: validated filename to avoid traversal, CREATE_NEW semantics, handled non-POSIX FS, minimal privileges.
// 5) Secure code output: final mitigations applied.

import java.nio.file.*;
import java.nio.file.attribute.*;
import java.util.*;
import java.io.*;

public class Lse100 {

    // Validate filename is a simple basename without any path traversal or separators
    private static boolean isValidBaseName(String name) {
        if (name == null || name.isEmpty()) return false;
        Path p = Paths.get(name);
        if (p.isAbsolute()) return false;
        if (p.getNameCount() != 1) return false; // prevent any separators
        String s = name;
        if (s.equals(".") || s.equals("..")) return false;
        return true;
    }

    // Securely creates a file with owner read/write (0600) permissions when supported
    public static boolean createSecureConfig(String filename) {
        if (!isValidBaseName(filename)) {
            return false;
        }
        Path path = Paths.get(filename).normalize();

        // Attempt POSIX-secure creation first (atomic with desired permissions)
        try {
            Set<PosixFilePermission> perms = EnumSet.of(
                    PosixFilePermission.OWNER_READ,
                    PosixFilePermission.OWNER_WRITE
            );
            FileAttribute<Set<PosixFilePermission>> attr = PosixFilePermissions.asFileAttribute(perms);
            Files.createFile(path, attr); // fails if file exists (CREATE_NEW semantics)
            return true;
        } catch (UnsupportedOperationException e) {
            // Non-POSIX filesystem (e.g., Windows). Fallback: create exclusively, then set permissions best-effort.
            try {
                Files.createFile(path); // CREATE_NEW semantics
                File f = path.toFile();
                // Best-effort restriction to owner only
                f.setReadable(false, false);
                f.setWritable(false, false);
                f.setExecutable(false, false);

                f.setReadable(true, true);
                f.setWritable(true, true);
                f.setExecutable(false, true);
                return true;
            } catch (FileAlreadyExistsException ex) {
                return false;
            } catch (IOException ex) {
                return false;
            }
        } catch (FileAlreadyExistsException e) {
            return false;
        } catch (IOException e) {
            return false;
        }
    }

    // Helper to remove any existing file before test to keep tests independent
    private static void cleanup(String filename) {
        try {
            Files.deleteIfExists(Paths.get(filename));
        } catch (IOException ignore) {
        }
    }

    // 5 test cases
    public static void main(String[] args) {
        String fname = "important_config";

        for (int i = 1; i <= 5; i++) {
            cleanup(fname);
            boolean result = createSecureConfig(fname);
            System.out.println("Test " + i + " createSecureConfig(\"" + fname + "\") -> " + result);
        }
    }
}