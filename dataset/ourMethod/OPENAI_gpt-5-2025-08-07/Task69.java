import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.BasicFileAttributes;
import java.nio.file.attribute.BasicFileAttributeView;
import java.util.ArrayList;
import java.util.List;

public class Task69 {

    // Return codes:
    // 0 = success
    // 1 = invalid input
    // 2 = target path resolves outside base
    // 3 = file not found
    // 4 = not a regular file
    // 5 = is a symlink
    // 6 = delete failed
    public static int secureDelete(String baseDir, String userPath) {
        if (baseDir == null || userPath == null) return 1;
        if (baseDir.length() == 0 || userPath.length() == 0) return 1;
        if (userPath.length() > 4096) return 1;
        // Reject absolute user paths
        try {
            if (Paths.get(userPath).isAbsolute()) return 1;
        } catch (Exception e) {
            return 1;
        }
        // Basic control characters rejection
        for (char c : userPath.toCharArray()) {
            if (Character.isISOControl(c)) return 1;
        }
        try {
            Path baseReal = Paths.get(baseDir).toRealPath(LinkOption.NOFOLLOW_LINKS);
            Path candidate = baseReal.resolve(userPath).normalize();

            // Ensure the path stays within base
            if (!candidate.startsWith(baseReal)) {
                return 2;
            }
            // Existence check without following symlinks
            if (!Files.exists(candidate, LinkOption.NOFOLLOW_LINKS)) {
                return 3;
            }
            // Refuse if symlink
            if (Files.isSymbolicLink(candidate)) {
                return 5;
            }
            // Must be a regular file (no symlink follow)
            BasicFileAttributes attrs = Files.readAttributes(candidate, BasicFileAttributes.class, LinkOption.NOFOLLOW_LINKS);
            if (!attrs.isRegularFile()) {
                return 4;
            }

            // Attempt to use SecureDirectoryStream to reduce TOCTOU risks
            Path parent = candidate.getParent() != null ? candidate.getParent() : baseReal;
            Path fileName = candidate.getFileName();
            if (fileName == null) return 1;

            try (DirectoryStream<Path> ds = Files.newDirectoryStream(parent)) {
                if (ds instanceof SecureDirectoryStream) {
                    @SuppressWarnings("unchecked")
                    SecureDirectoryStream<Path> sds = (SecureDirectoryStream<Path>) ds;

                    BasicFileAttributeView view = sds.getFileAttributeView(fileName, BasicFileAttributeView.class, LinkOption.NOFOLLOW_LINKS);
                    if (view == null) return 6;
                    BasicFileAttributes a2 = view.readAttributes();
                    if (a2.isSymbolicLink()) return 5;
                    if (!a2.isRegularFile()) return 4;

                    sds.deleteFile(fileName);
                    return 0;
                }
            } catch (UnsupportedOperationException ignored) {
                // Fallback below
            } catch (IOException ioe) {
                // If secure delete path failed, try fallback
            }

            // Fallback: re-check then delete
            if (Files.isSymbolicLink(candidate)) return 5;
            attrs = Files.readAttributes(candidate, BasicFileAttributes.class, LinkOption.NOFOLLOW_LINKS);
            if (!attrs.isRegularFile()) return 4;

            Files.delete(candidate);
            return 0;
        } catch (NoSuchFileException e) {
            return 3;
        } catch (AccessDeniedException e) {
            return 6;
        } catch (IOException e) {
            return 6;
        }
    }

    private static String codeToMessage(int code) {
        switch (code) {
            case 0: return "Deleted";
            case 1: return "Invalid input";
            case 2: return "Path outside base";
            case 3: return "Not found";
            case 4: return "Not a regular file";
            case 5: return "Is a symlink";
            case 6: return "Delete failed";
            default: return "Unknown";
        }
    }

    public static void main(String[] args) {
        // Prepare a controlled test environment
        try {
            Path base = Files.createTempDirectory("safe-del-java-");
            Path f1 = base.resolve("file1.txt");
            Path sub = base.resolve("sub");
            Path nested = sub.resolve("inside.txt");
            Path f2 = base.resolve("file2.txt");
            Files.write(f1, "one".getBytes(StandardCharsets.UTF_8));
            Files.createDirectories(sub);
            Files.write(nested, "two".getBytes(StandardCharsets.UTF_8));
            Files.write(f2, "three".getBytes(StandardCharsets.UTF_8));
            Path sym = base.resolve("link_to_file2");
            try {
                Files.createSymbolicLink(sym, f2.getFileName()); // relative link within base
            } catch (UnsupportedOperationException | IOException | SecurityException e) {
                // Symlink may not be supported; ignore
            }

            List<String[]> tests = new ArrayList<>();
            tests.add(new String[]{base.toString(), "file1.txt"});            // success
            tests.add(new String[]{base.toString(), "sub/inside.txt"});       // success
            tests.add(new String[]{base.toString(), "sub"});                  // reject: directory
            tests.add(new String[]{base.toString(), "link_to_file2"});        // reject: symlink (if created) else not found
            tests.add(new String[]{base.toString(), "../outside.txt"});       // reject: traversal

            for (int i = 0; i < tests.size(); i++) {
                int rc = secureDelete(tests.get(i)[0], tests.get(i)[1]);
                System.out.println("Test " + (i + 1) + " => " + codeToMessage(rc) + " (" + rc + ")");
            }
        } catch (IOException e) {
            System.out.println("Setup failed: " + e.getMessage());
        }

        // If CLI args provided: expect baseDir and userPath
        if (args != null && args.length >= 2) {
            int rc = secureDelete(args[0], args[1]);
            System.out.println(codeToMessage(rc) + " (" + rc + ")");
        }
    }
}