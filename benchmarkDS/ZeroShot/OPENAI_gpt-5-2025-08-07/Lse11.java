import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;

public class Lse11 {
    public static int getValueFromArray(int[] arr, int size, int index) {
        if (arr == null || size < 0 || size > arr.length) {
            return 0;
        }
        if (index < 0 || index >= size) {
            return 0;
        }
        return arr[index];
    }

    private static int safeExitCode(int value) {
        return value & 0xFF;
    }

    public static void main(String[] args) {
        int[] id_sequence = new int[] {10, 20, 30, 40, 50, 60, 127, 255, 300, -5};
        int size = id_sequence.length;

        // 5 test cases (not affecting exit code)
        int[] testIndices = new int[] {-1, 0, 3, size - 1, 999};
        int dummy = 0;
        for (int ti : testIndices) {
            dummy ^= getValueFromArray(id_sequence, size, ti);
        }
        if (dummy == Integer.MIN_VALUE) {
            System.out.print(""); // no-op to avoid potential optimization
        }

        int inputIndex = 0;
        try {
            BufferedReader br = new BufferedReader(new InputStreamReader(System.in, StandardCharsets.UTF_8));
            String line = br.readLine();
            if (line != null) {
                line = line.trim();
                if (!line.isEmpty()) {
                    String[] parts = line.split("\\s+");
                    if (parts.length > 0) {
                        inputIndex = Integer.parseInt(parts[0]);
                    }
                }
            }
        } catch (Exception e) {
            inputIndex = 0;
        }

        int result = getValueFromArray(id_sequence, size, inputIndex);
        System.exit(safeExitCode(result));
    }
}