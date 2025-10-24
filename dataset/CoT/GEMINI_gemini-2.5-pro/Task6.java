public class Task6 {

    public static double findUniq(double[] arr) {
        // It's guaranteed that array contains at least 3 numbers.
        // If the first two numbers are different, one of them is the unique number.
        // We can check the third number to decide which one.
        if (arr[0] != arr[1]) {
            // If arr[0] is equal to arr[2], then arr[1] must be the unique number.
            // Otherwise, arr[0] is the unique number.
            return arr[0] == arr[2] ? arr[1] : arr[0];
        } else {
            // If the first two numbers are the same, this is the common number.
            // We can then iterate through the rest of the array to find the different one.
            double commonNumber = arr[0];
            for (int i = 2; i < arr.length; i++) {
                if (arr[i] != commonNumber) {
                    return arr[i];
                }
            }
        }
        // This part should be unreachable given the problem constraints.
        // We return a default value, though an exception would also be appropriate.
        return -1;
    }

    public static void main(String[] args) {
        // Test Case 1
        System.out.println(findUniq(new double[]{ 1, 1, 1, 2, 1, 1 })); // Expected: 2.0
        // Test Case 2
        System.out.println(findUniq(new double[]{ 0, 0, 0.55, 0, 0 })); // Expected: 0.55
        // Test Case 3
        System.out.println(findUniq(new double[]{ 3, 10, 3, 3, 3 })); // Expected: 10.0
        // Test Case 4
        System.out.println(findUniq(new double[]{ 5, 5, 5, 5, 4 })); // Expected: 4.0
        // Test Case 5
        System.out.println(findUniq(new double[]{ 8, 8, 8, 8, 8, 8, 8, 7 })); // Expected: 7.0
    }
}