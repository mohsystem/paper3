// Chain-of-Through secure code generation:
// 1) Problem understanding: Provide getValueFromArray to safely access array values by index.
// 2) Security requirements: Validate inputs, avoid out-of-bounds access, and normalize exit codes.
// 3) Secure coding generation: Implement bounds checks and handle input parsing safely.
// 4) Code review: Avoid undefined behavior and ensure consistent behavior across environments.
// 5) Secure code output: Final code reflects mitigations and robust handling.

import java.io.*;
import java.util.*;

public class Lse11 {

    // Returns the value at the given index if valid; otherwise returns 0.
    public static int getValueFromArray(int[] arr, int size, int index) {
        if (arr == null || size < 0) {
            return 0;
        }
        if (index < 0 || index >= size) {
            return 0;
        }
        return arr[index];
    }

    // Simple test harness with 5 test cases.
    private static void runTests() {
        int[] id_sequence = new int[] {1, 12, 123, 200, 255};
        int size = id_sequence.length;

        int[] testIndices = new int[] {0, 4, 2, -1, 10};
        for (int idx : testIndices) {
            int val = getValueFromArray(id_sequence, size, idx);
            System.out.println("Index: " + idx + " -> Value: " + val);
        }
    }

    public static void main(String[] args) {
        int[] id_sequence = new int[] {1, 12, 123, 200, 255};
        int size = id_sequence.length;

        // If any argument is provided, run tests instead of stdin flow.
        if (args != null && args.length > 0) {
            runTests();
            System.exit(0);
            return;
        }

        // Read an integer index from standard input.
        int index = 0;
        boolean haveInput = false;
        try {
            // Use Scanner for safe parsing
            Scanner sc = new Scanner(System.in);
            if (sc.hasNextInt()) {
                index = sc.nextInt();
                haveInput = true;
            }
            sc.close();
        } catch (Exception e) {
            // Ignore and fall back to default
            haveInput = false;
        }

        int value = 0;
        if (haveInput) {
            value = getValueFromArray(id_sequence, size, index);
        }
        // Normalize exit code to 0..255
        int exitCode = value & 0xFF;
        System.exit(exitCode);
    }
}