
import java.util.Arrays;

public class Task163 {
    public static int longestRun(int[] arr) {
        if (arr == null) {
            throw new IllegalArgumentException("Array cannot be null");
        }
        
        if (arr.length == 0) {
            throw new IllegalArgumentException("Array cannot be empty");
        }
        
        if (arr.length > 100000) {
            throw new IllegalArgumentException("Array length exceeds maximum allowed size");
        }
        
        if (arr.length == 1) {
            return 1;
        }
        
        int maxRun = 1;
        int currentRun = 1;
        
        for (int i = 1; i < arr.length; i++) {
            long diff = (long)arr[i] - (long)arr[i - 1];
            
            if (diff == 1 || diff == -1) {
                if (i == 1 || (long)arr[i - 1] - (long)arr[i - 2] == diff) {
                    currentRun++;
                } else {
                    currentRun = 2;
                }
                maxRun = Math.max(maxRun, currentRun);
            } else {
                currentRun = 1;
            }
        }
        
        return maxRun;
    }
    
    public static void main(String[] args) {
        System.out.println("Test 1: " + longestRun(new int[]{1, 2, 3, 5, 6, 7, 8, 9}));
        System.out.println("Test 2: " + longestRun(new int[]{1, 2, 3, 10, 11, 15}));
        System.out.println("Test 3: " + longestRun(new int[]{5, 4, 2, 1}));
        System.out.println("Test 4: " + longestRun(new int[]{3, 5, 7, 10, 15}));
        System.out.println("Test 5: " + longestRun(new int[]{10}));
    }
}
