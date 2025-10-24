
public class Task6 {
    public static double findUniq(double[] arr) {
        if (arr == null || arr.length < 3) {
            throw new IllegalArgumentException("Array must contain at least 3 numbers");
        }
        
        // Compare first three elements to determine which is the unique one
        if (arr[0] == arr[1]) {
            // arr[0] and arr[1] are the same, so they represent the repeated value
            for (int i = 2; i < arr.length; i++) {
                if (arr[i] != arr[0]) {
                    return arr[i];
                }
            }
            return arr[0]; // Should not reach here given problem constraints
        } else if (arr[0] == arr[2]) {
            // arr[0] and arr[2] are the same, so arr[1] is unique
            return arr[1];
        } else {
            // arr[1] and arr[2] must be the same, so arr[0] is unique
            return arr[0];
        }
    }
    
    public static void main(String[] args) {
        // Test case 1
        double result1 = findUniq(new double[]{1, 1, 1, 2, 1, 1});
        System.out.println("Test 1: " + result1 + " (Expected: 2.0)");
        
        // Test case 2
        double result2 = findUniq(new double[]{0, 0, 0.55, 0, 0});
        System.out.println("Test 2: " + result2 + " (Expected: 0.55)");
        
        // Test case 3
        double result3 = findUniq(new double[]{5, 5, 5, 5, 10});
        System.out.println("Test 3: " + result3 + " (Expected: 10.0)");
        
        // Test case 4
        double result4 = findUniq(new double[]{3.14, 2.71, 2.71, 2.71});
        System.out.println("Test 4: " + result4 + " (Expected: 3.14)");
        
        // Test case 5
        double result5 = findUniq(new double[]{-1, -1, -1, -5, -1, -1, -1});
        System.out.println("Test 5: " + result5 + " (Expected: -5.0)");
    }
}
