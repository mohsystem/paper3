public class Task6 {

    public static double findUniq(double[] arr) {
        // The array is guaranteed to have at least 3 elements.
        // We compare the first three elements to determine the common number.
        if (arr[0] != arr[1]) {
            // If the first two are different, the third element will match one of them.
            // The one that doesn't match is the unique number.
            return arr[0] == arr[2] ? arr[1] : arr[0];
        } else {
            // If the first two are the same, this is the common number.
            // We then iterate through the rest of the array to find the different one.
            double common = arr[0];
            for (int i = 2; i < arr.length; i++) {
                if (arr[i] != common) {
                    return arr[i];
                }
            }
        }
        // This part is unreachable under the problem's guarantees.
        // Added for completeness to satisfy the compiler.
        return 0;
    }

    public static void main(String[] args) {
        // Test cases
        System.out.println(findUniq(new double[]{ 1, 1, 1, 2, 1, 1 }));
        System.out.println(findUniq(new double[]{ 0, 0, 0.55, 0, 0 }));
        System.out.println(findUniq(new double[]{ 3, 10, 3, 3, 3 }));
        System.out.println(findUniq(new double[]{ 5, 5, 5, 5, 0.2 }));
        System.out.println(findUniq(new double[]{ -2, -2, -2, -1, -2 }));
    }
}