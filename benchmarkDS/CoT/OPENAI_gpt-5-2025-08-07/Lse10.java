/*
Chain-of-Through process:
1) Problem understanding: Implement getValueFromArray(arr, size, idx) returning arr[idx]. Main reads an integer, calls getValueFromArray with id_sequence and exits with the returned value as exit code.
2) Security requirements: Validate inputs (nulls, size, bounds). Avoid out-of-bounds. Sanitize exit codes to OS-supported range (0..255). Handle malformed input safely.
3) Secure coding: Perform bounds checks; catch exceptions; avoid undefined behavior. Do not expose sensitive info.
4) Code review: Ensure safe index access, proper resource handling, and predictable exit behavior.
5) Secure output: Exit with sanitized code; provide test mode when no input is provided.
*/
import java.util.*;

public class Lse10 {

    public static int getValueFromArray(int[] arr, int size, int index) {
        if (arr == null) throw new IllegalArgumentException("Array is null");
        if (size < 0 || size > arr.length) throw new IllegalArgumentException("Invalid size");
        if (index < 0 || index >= size) throw new IndexOutOfBoundsException("Index out of range");
        return arr[index];
    }

    private static int toExitCode(int value) {
        return value & 0xFF; // sanitize to 0..255
    }

    private static void runTests() {
        int[] id_sequence = new int[] {10, 20, 30, 40, 50, 60, 70};
        int size = id_sequence.length;
        int[] testIndexes = new int[] {0, 3, 4, 6, 10}; // last is out-of-bounds to test handling
        System.out.println("Test mode: demonstrating 5 test cases");
        for (int idx : testIndexes) {
            try {
                int val = getValueFromArray(id_sequence, size, idx);
                System.out.println("Index " + idx + " -> " + val);
            } catch (Exception e) {
                System.out.println("Index " + idx + " -> error: " + e.getMessage());
            }
        }
    }

    public static void main(String[] args) {
        int[] id_sequence = new int[] {10, 20, 30, 40, 50, 60, 70};
        int size = id_sequence.length;

        Scanner sc = new Scanner(System.in);
        int exitCode = 0;

        if (sc.hasNextInt()) {
            int idx = sc.nextInt();
            try {
                int value = getValueFromArray(id_sequence, size, idx);
                exitCode = toExitCode(value);
            } catch (Exception e) {
                System.err.println("Error: " + e.getMessage());
                exitCode = 1;
            }
        } else {
            // No integer provided; run 5 test cases
            runTests();
            exitCode = 0;
        }
        sc.close();
        System.exit(exitCode);
    }
}