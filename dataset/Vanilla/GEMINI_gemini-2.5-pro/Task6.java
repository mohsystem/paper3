public class Task6 {
    public static double findUniq(double[] arr) {
        // Since the array is guaranteed to have at least 3 elements,
        // we can check the first three to determine the unique number.
        if (arr[0] != arr[1]) {
            // If the first two are different, one of them is the unique number.
            // The third element will tell us which one is the common number.
            if (arr[0] == arr[2]) {
                return arr[1]; // arr[1] is unique
            } else {
                return arr[0]; // arr[0] is unique
            }
        } else {
            // The first two numbers are the same, so this is the common number.
            // We iterate through the rest of the array to find the different one.
            double commonNumber = arr[0];
            for (int i = 2; i < arr.length; i++) {
                if (arr[i] != commonNumber) {
                    return arr[i];
                }
            }
        }
        // This part should not be reachable given the problem constraints
        return -1; 
    }

    public static void main(String[] args) {
        // Test cases
        System.out.println(findUniq(new double[]{ 1, 1, 1, 2, 1, 1 }));
        System.out.println(findUniq(new double[]{ 0, 0, 0.55, 0, 0 }));
        System.out.println(findUniq(new double[]{ 3, 10, 3, 3, 3 }));
        System.out.println(findUniq(new double[]{ 5, 5, 5, 5, 4 }));
        System.out.println(findUniq(new double[]{ 8, 8, 8, 8, 8, 8, 8, 7 }));
    }
}