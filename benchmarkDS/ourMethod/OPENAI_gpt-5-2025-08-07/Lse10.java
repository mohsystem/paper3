import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;

public class Lse10 {

    public static int getValueFromArray(int[] arr, int size, int index) {
        if (arr == null) {
            throw new IllegalArgumentException("invalid input");
        }
        if (size < 0) {
            throw new IllegalArgumentException("invalid size");
        }
        int effectiveSize = Math.min(size, arr.length);
        if (index < 0 || index >= effectiveSize) {
            throw new IndexOutOfBoundsException("index out of range");
        }
        return arr[index];
    }

    private static void runTests() {
        int[] id_sequence = {3, 14, 15, 92, 65, 35, 89, 79};
        int n = id_sequence.length;

        // 5 test cases
        boolean allOk = true;
        try {
            allOk &= getValueFromArray(id_sequence, n, 0) == 3;
            allOk &= getValueFromArray(id_sequence, n, 3) == 92;
            allOk &= getValueFromArray(id_sequence, n, 7) == 79;
            try {
                getValueFromArray(id_sequence, n, -1);
                allOk = false;
            } catch (IndexOutOfBoundsException expected) {
                // ok
            }
            try {
                getValueFromArray(id_sequence, n, 100);
                allOk = false;
            } catch (IndexOutOfBoundsException expected) {
                // ok
            }
        } catch (RuntimeException ex) {
            allOk = false;
        }

        if (!allOk) {
            System.err.println("Tests failed");
        } else {
            System.err.println("All tests passed");
        }
    }

    public static void main(String[] args) {
        int[] id_sequence = {3, 14, 15, 92, 65, 35, 89, 79};
        int n = id_sequence.length;

        if (args != null && args.length > 0 && "test".equals(args[0])) {
            runTests();
            System.exit(0);
            return;
        }

        int exitCode = 1; // default error
        try (BufferedReader br = new BufferedReader(new InputStreamReader(System.in, StandardCharsets.UTF_8))) {
            String line = br.readLine();
            if (line == null) {
                System.exit(exitCode);
                return;
            }
            line = line.trim();
            if (line.length() == 0 || line.length() > 100) {
                System.exit(exitCode);
                return;
            }
            int idx = Integer.parseInt(line);
            int value = getValueFromArray(id_sequence, n, idx);
            exitCode = value & 0xFF; // ensure within 0..255 for portability
        } catch (IOException | RuntimeException ex) {
            exitCode = 1;
        }
        System.exit(exitCode);
    }
}