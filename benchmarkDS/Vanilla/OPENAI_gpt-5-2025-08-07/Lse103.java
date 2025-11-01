import java.io.IOException;
import java.nio.channels.SeekableByteChannel;
import java.nio.file.*;
import java.nio.file.attribute.FileAttribute;
import java.util.HashSet;
import java.util.Set;

public class Lse103 {
    public static int openFileWithFlags(String path, boolean create, boolean trunc, boolean excl, boolean writeOnly) {
        Path p = Paths.get(path);
        Set<OpenOption> options = new HashSet<>();
        // Access mode
        if (writeOnly || trunc) {
            options.add(StandardOpenOption.WRITE);
        } else {
            options.add(StandardOpenOption.READ);
        }
        // Creation flags
        if (create && excl) {
            options.add(StandardOpenOption.CREATE_NEW); // fails if exists
        } else if (create) {
            options.add(StandardOpenOption.CREATE); // create if not exist
        }
        // Truncate flag
        if (trunc) {
            options.add(StandardOpenOption.TRUNCATE_EXISTING);
        }

        SeekableByteChannel ch = null;
        try {
            ch = Files.newByteChannel(p, options, new FileAttribute<?>[0]);
            ch.close();
            return 0;
        } catch (IOException e) {
            return -1;
        } finally {
            if (ch != null) {
                try { ch.close(); } catch (IOException ignore) {}
            }
        }
    }

    private static void prepareFile(String path, boolean create) {
        Path p = Paths.get(path);
        try {
            if (!create) {
                Files.deleteIfExists(p);
            } else {
                if (!Files.exists(p)) {
                    Files.write(p, "seed".getBytes(), StandardOpenOption.CREATE, StandardOpenOption.WRITE);
                }
            }
        } catch (IOException ignore) {}
    }

    public static void main(String[] args) {
        // Test 1: Create new file with O_CREAT | O_WRONLY
        String f1 = "java_t1_create.txt";
        prepareFile(f1, false);
        System.out.println("T1: " + openFileWithFlags(f1, true, false, false, true));

        // Test 2: Truncate existing file with O_TRUNC | O_WRONLY
        String f2 = "java_t2_trunc.txt";
        prepareFile(f2, true);
        System.out.println("T2: " + openFileWithFlags(f2, false, true, false, true));

        // Test 3: O_CREAT | O_EXCL on existing file should fail
        String f3 = "java_t3_excl.txt";
        prepareFile(f3, true);
        System.out.println("T3: " + openFileWithFlags(f3, true, false, true, true));

        // Test 4: Open non-existent file without O_CREAT should fail
        String f4 = "java_t4_no_create.txt";
        prepareFile(f4, false);
        System.out.println("T4: " + openFileWithFlags(f4, false, false, false, true));

        // Test 5: Open existing file O_WRONLY
        String f5 = "java_t5_open_existing.txt";
        prepareFile(f5, true);
        System.out.println("T5: " + openFileWithFlags(f5, false, false, false, true));
    }
}