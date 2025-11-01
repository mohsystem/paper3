public class Lse18 {
    // Function that takes an integer index and returns the value at that index in the array
    public static int getValueAtIndex(int index) {
        int[] arr = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
        return arr[index];
    }

    public static void main(String[] args) {
        int[] testIndexes = {0, 3, 5, 7, 9};
        for (int idx : testIndexes) {
            int value = getValueAtIndex(idx);
            System.out.println("Index " + idx + " -> Value: " + value);
        }
    }
}