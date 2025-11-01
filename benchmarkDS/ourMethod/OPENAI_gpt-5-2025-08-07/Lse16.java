public class Lse16 {
    private static final int[] DATA = new int[] {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

    public static int getValueAtIndex(int index) {
        if (index < 0 || index >= DATA.length) {
            throw new IllegalArgumentException("Invalid index");
        }
        return DATA[index];
    }

    public static void main(String[] args) {
        int[] tests = {0, 5, 9, 3, 1};
        for (int t : tests) {
            try {
                int result = getValueAtIndex(t);
                System.out.println("Index " + t + " -> " + result);
            } catch (IllegalArgumentException e) {
                System.out.println("Index " + t + " -> error");
            }
        }
    }
}