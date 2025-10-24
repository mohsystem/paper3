
import java.util.Arrays;

public class Task6 {
    public static double findUniq(double[] arr) {
        if (arr == null || arr.length < 3) {
            throw new IllegalArgumentException("Array must contain at least 3 elements");
        }
        
        // Sort first three elements to determine which is unique
        double a = arr[0];
        double b = arr[1];
        double c = arr[2];
        
        // If first two are equal, the unique is different from them
        if (Double.compare(a, b) == 0) {
            if (Double.compare(a, c) != 0) {
                return c;
            }
            // a == b == c, so unique must be later
            for (int i = 3; i < arr.length; i++) {
                if (Double.compare(arr[i], a) != 0) {
                    return arr[i];
                }
            }
        } else if (Double.compare(a, c) == 0) {
            // a == c != b, so b is unique
            return b;
        } else {
            // b == c != a, so a is unique
            return a;
        }
        
        throw new IllegalArgumentException("No unique element found");
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println(findUniq(new double[]{1, 1, 1, 2, 1, 1})); // 2.0
        
        // Test case 2
        System.out.println(findUniq(new double[]{0, 0, 0.55, 0, 0})); // 0.55
        
        // Test case 3
        System.out.println(findUniq(new double[]{5, 5, 5, 5, 3})); // 3.0
        
        // Test case 4
        System.out.println(findUniq(new double[]{10, 9, 9, 9, 9})); // 10.0
        
        // Test case 5
        System.out.println(findUniq(new double[]{-1, -1, -1, 0, -1})); // 0.0
    }
}
