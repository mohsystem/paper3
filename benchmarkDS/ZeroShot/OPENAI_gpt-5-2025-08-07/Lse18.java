public class Lse18 {
    private static final int[] DATA = new int[] {11, 22, 33, 44, 55, 66, 77, 88, 99, 110};

    public static int getValueAtIndex(int index) {
        if (index < 0 || index >= DATA.length) {
            throw new IndexOutOfBoundsException("Index out of bounds");
        }
        return DATA[index];
    }

    public static void main(String[] args) {
        int[] tests = {0, 5, 9, 3, 7};
        for (int t : tests) {
            try {
                int value = getValueAtIndex(t);
                System.out.println("Index " + t + " -> " + value);
            } catch (IndexOutOfBoundsException ex) {
                System.out.println("Index " + t + " -> error: " + ex.getMessage());
            }
        }
    }
}