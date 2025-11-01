public class Lse16 {
    public static int getValueAtIndex(int index) {
        int[] arr = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
        return arr[index];
    }

    public static void main(String[] args) {
        int[] tests = {0, 1, 5, 8, 9};
        for (int idx : tests) {
            try {
                int result = getValueAtIndex(idx);
                System.out.println("Index " + idx + " -> " + result);
            } catch (ArrayIndexOutOfBoundsException e) {
                System.out.println("Index " + idx + " -> Index out of bounds");
            }
        }
    }
}