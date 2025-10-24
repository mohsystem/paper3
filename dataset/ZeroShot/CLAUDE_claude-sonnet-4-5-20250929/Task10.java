
public class Task10 {
    public static int getSum(int a, int b) {
        if (a == b) {
            return a;
        }
        
        int min = Math.min(a, b);
        int max = Math.max(a, b);
        
        // Using arithmetic progression formula: sum = n * (first + last) / 2
        int n = max - min + 1;
        return n * (min + max) / 2;
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println("Test 1 (1, 0): " + getSum(1, 0) + " (Expected: 1)");
        
        // Test case 2
        System.out.println("Test 2 (1, 2): " + getSum(1, 2) + " (Expected: 3)");
        
        // Test case 3
        System.out.println("Test 3 (0, 1): " + getSum(0, 1) + " (Expected: 1)");
        
        // Test case 4
        System.out.println("Test 4 (1, 1): " + getSum(1, 1) + " (Expected: 1)");
        
        // Test case 5
        System.out.println("Test 5 (-1, 2): " + getSum(-1, 2) + " (Expected: 2)");
    }
}
