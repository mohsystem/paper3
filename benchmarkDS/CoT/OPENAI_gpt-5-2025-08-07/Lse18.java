public class Lse18 {
    // Step 1-5: Secure, bounds-checked retrieval from a fixed array.
    private static final int[] DATA = new int[]{10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

    public static int getValueAtIndex(int index) {
        if (index < 0 || index >= DATA.length) {
            return -1; // out-of-bounds indicator
        }
        return DATA[index];
    }

    public static void main(String[] args) {
        int[] testCases = new int[]{5, 0, 9, -1, 10};
        for (int idx : testCases) {
            int result = getValueAtIndex(idx);
            System.out.println("Index " + idx + " -> " + result);
        }
    }
}