public class Lse16 {
    private static final int[] DATA = new int[] {10, 20, 30, 40, 50, 60, 70, 80};

    public static int getValueAtIndex(int index) {
        if (index < 0 || index >= DATA.length) {
            throw new IllegalArgumentException("Index out of bounds");
        }
        return DATA[index];
    }

    public static void main(String[] args) {
        int[] tests = {0, 1, 2, 5, 7};
        for (int t : tests) {
            try {
                int val = getValueAtIndex(t);
                System.out.println("Index " + t + " -> " + val);
            } catch (IllegalArgumentException e) {
                System.out.println("Index " + t + " -> error: " + e.getMessage());
            }
        }
    }
}