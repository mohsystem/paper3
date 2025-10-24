
public class Task20 {
    public static long digPow(int n, int p) {
        String numStr = String.valueOf(n);
        long sum = 0;
        
        for (int i = 0; i < numStr.length(); i++) {
            int digit = numStr.charAt(i) - '0';
            sum += Math.pow(digit, p + i);
        }
        
        if (sum % n == 0) {
            return sum / n;
        }
        return -1;
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println("Test 1: n=89, p=1 -> " + digPow(89, 1)); // Expected: 1
        
        // Test case 2
        System.out.println("Test 2: n=92, p=1 -> " + digPow(92, 1)); // Expected: -1
        
        // Test case 3
        System.out.println("Test 3: n=695, p=2 -> " + digPow(695, 2)); // Expected: 2
        
        // Test case 4
        System.out.println("Test 4: n=46288, p=3 -> " + digPow(46288, 3)); // Expected: 51
        
        // Test case 5
        System.out.println("Test 5: n=114, p=3 -> " + digPow(114, 3)); // Expected: 9
    }
}
