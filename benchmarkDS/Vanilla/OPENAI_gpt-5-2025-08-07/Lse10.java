import java.io.BufferedReader;
import java.io.InputStreamReader;

public class Lse10 {
    public static int getValueFromArray(int[] arr, int size, int index) {
        if (index >= 0 && index < size) {
            return arr[index];
        }
        return 0;
    }

    public static void main(String[] args) throws Exception {
        int[] id_sequence = new int[] {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
        int size = id_sequence.length;

        // 5 test cases
        int[] testIdx = new int[] {0, 4, 9, -1, 100};
        for (int i = 0; i < testIdx.length; i++) {
            int v = getValueFromArray(id_sequence, size, testIdx[i]);
            System.out.println("test" + (i + 1) + ": idx=" + testIdx[i] + ", val=" + v);
        }

        BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
        String line = br.readLine();
        int idx = 0;
        if (line != null) {
            line = line.trim();
            try {
                if (!line.isEmpty()) {
                    idx = Integer.parseInt(line);
                }
            } catch (Exception e) {
                idx = 0;
            }
        }
        int res = getValueFromArray(id_sequence, size, idx);
        System.exit(res);
    }
}