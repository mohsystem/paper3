import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;

public class Lse12 {
    // Returns the value at index if valid; otherwise returns 1 (error code).
    public static int getValueFromArray(int[] arr, int size, int index) {
        if (arr == null) {
            return 1;
        }
        if (size < 0 || size > arr.length) {
            return 1;
        }
        if (index < 0 || index >= size) {
            return 1;
        }
        return arr[index];
    }

    // Simple internal tests (5 cases)
    private static void runTests() {
        int[] data = new int[] {10, 20, 30, 40, 50};
        if (getValueFromArray(data, data.length, 0) != 10) throw new IllegalStateException("Test 1 failed");
        if (getValueFromArray(data, data.length, 1) != 20) throw new IllegalStateException("Test 2 failed");
        if (getValueFromArray(data, data.length, 2) != 30) throw new IllegalStateException("Test 3 failed");
        if (getValueFromArray(data, data.length, 4) != 50) throw new IllegalStateException("Test 4 failed");
        if (getValueFromArray(data, data.length, 5) != 1)  throw new IllegalStateException("Test 5 failed"); // out of bounds -> 1
    }

    public static void main(String[] args) {
        // Run internal tests first
        runTests();

        int[] data = new int[] {10, 20, 30, 40, 50};
        int idx = -1; // invalid by default
        try (BufferedReader br = new BufferedReader(new InputStreamReader(System.in, StandardCharsets.UTF_8))) {
            String line = br.readLine();
            if (line != null) {
                line = line.trim();
                try {
                    idx = Integer.parseInt(line);
                } catch (NumberFormatException e) {
                    idx = -1; // invalid index, will return error code 1
                }
            }
        } catch (Exception e) {
            // swallow and use default invalid index
            idx = -1;
        }
        int result = getValueFromArray(data, data.length, idx);
        System.exit(result & 0xFF);
    }
}