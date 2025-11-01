public class Lse17 {

    public static int getValueAtIndex(int[] arr, int index) {
        return arr[index];
    }

    public static void main(String[] args) {
        int[] myArray = {10, 20, 30, 40, 50, 60};

        // Test Case 1
        int index1 = 0;
        System.out.println("Value at index " + index1 + ": " + getValueAtIndex(myArray, index1));

        // Test Case 2
        int index2 = 2;
        System.out.println("Value at index " + index2 + ": " + getValueAtIndex(myArray, index2));

        // Test Case 3
        int index3 = 4;
        System.out.println("Value at index " + index3 + ": " + getValueAtIndex(myArray, index3));

        // Test Case 4
        int index4 = 5;
        System.out.println("Value at index " + index4 + ": " + getValueAtIndex(myArray, index4));

        // Test Case 5
        int index5 = 1;
        System.out.println("Value at index " + index5 + ": " + getValueAtIndex(myArray, index5));
    }
}