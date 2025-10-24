
public class Task163 {
    public static int longestRun(int[] arr) {
        if (arr == null || arr.length == 0) {
            return 0;
        }
        
        if (arr.length == 1) {
            return 1;
        }
        
        int maxRun = 1;
        int currentRun = 1;
        
        for (int i = 1; i < arr.length; i++) {
            int diff = arr[i] - arr[i - 1];
            
            if (diff == 1 || diff == -1) {
                if (i > 1) {
                    int prevDiff = arr[i - 1] - arr[i - 2];
                    if (diff == prevDiff) {
                        currentRun++;
                    } else {
                        currentRun = 2;
                    }
                } else {
                    currentRun++;
                }
                maxRun = Math.max(maxRun, currentRun);
            } else {
                currentRun = 1;
            }
        }
        
        return maxRun;
    }
    
    public static void main(String[] args) {
        // Test case 1
        int[] test1 = {1, 2, 3, 5, 6, 7, 8, 9};
        System.out.println("Test 1: " + longestRun(test1) + " (Expected: 5)");
        
        // Test case 2
        int[] test2 = {1, 2, 3, 10, 11, 15};
        System.out.println("Test 2: " + longestRun(test2) + " (Expected: 3)");
        
        // Test case 3
        int[] test3 = {5, 4, 2, 1};
        System.out.println("Test 3: " + longestRun(test3) + " (Expected: 2)");
        
        // Test case 4
        int[] test4 = {3, 5, 7, 10, 15};
        System.out.println("Test 4: " + longestRun(test4) + " (Expected: 1)");
        
        // Test case 5
        int[] test5 = {10, 9, 8, 7, 6, 5};
        System.out.println("Test 5: " + longestRun(test5) + " (Expected: 6)");
    }
}
