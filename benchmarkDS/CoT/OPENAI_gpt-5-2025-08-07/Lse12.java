import java.io.BufferedReader;
import java.io.InputStreamReader;

public class Lse12 {
    public static int getValueFromArray(int[] arr, int size, int index) {
        if (arr == null || size <= 0) {
            return 0;
        }
        if (size > arr.length) {
            size = arr.length;
        }
        if (index >= 0 && index < size) {
            return arr[index];
        }
        return 0;
    }

    private static void runTests() {
        int[] arr = new int[]{10, 20, 30, 40, 50};
        int size = arr.length;
        int[] testIdx = new int[]{0, 2, 4, -1, 10};
        for (int i = 0; i < testIdx.length; i++) {
            int res = getValueFromArray(arr, size, testIdx[i]);
            System.out.println("Test " + (i + 1) + ": index=" + testIdx[i] + " value=" + res);
        }
    }

    public static void main(String[] args) {
        runTests();

        int[] arr = new int[]{10, 20, 30, 40, 50};
        int size = arr.length;
        int index = 0;
        try {
            BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
            String line = br.readLine();
            if (line != null) {
                line = line.trim();
                if (!line.isEmpty()) {
                    index = Integer.parseInt(line);
                }
            }
        } catch (Exception e) {
            index = 0;
        }
        int value = getValueFromArray(arr, size, index);
        System.exit(value);
    }
}