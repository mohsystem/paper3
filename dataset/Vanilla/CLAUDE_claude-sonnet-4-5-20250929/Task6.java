
public class Task6 {
    public static double findUniq(double[] arr) {
        // Compare first three elements to determine which is unique
        if (arr[0] == arr[1]) {
            // arr[0] is the common element
            double common = arr[0];
            for (double num : arr) {
                if (num != common) {
                    return num;
                }
            }
        } else if (arr[0] == arr[2]) {
            // arr[0] is the common element, arr[1] is unique
            return arr[1];
        } else {
            // arr[1] or arr[2] is the common element, arr[0] is unique
            return arr[0];
        }
        return arr[0]; // Should not reach here
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println(findUniq(new double[]{1, 1, 1, 2, 1, 1})); // => 2.0
        
        // Test case 2
        System.out.println(findUniq(new double[]{0, 0, 0.55, 0, 0})); // => 0.55
        
        // Test case 3
        System.out.println(findUniq(new double[]{3, 3, 3, 3, 5})); // => 5.0
        
        // Test case 4
        System.out.println(findUniq(new double[]{10, 9, 9, 9, 9})); // => 10.0
        
        // Test case 5
        System.out.println(findUniq(new double[]{7.5, 7.5, 8.5, 7.5})); // => 8.5
    }
}
