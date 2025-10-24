
public class Task23 {
    public static long findNb(long m) {
        // Input validation
        if (m <= 0) {
            return -1;
        }
        
        long n = 0;
        long sum = 0;
        
        // Iterate until sum equals or exceeds m
        while (sum < m) {
            n++;
            // Check for potential overflow before multiplication
            if (n > Math.sqrt(Long.MAX_VALUE)) {
                return -1;
            }
            sum += n * n * n;
        }
        
        // Return n if exact match, otherwise -1
        return (sum == m) ? n : -1;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: " + findNb(1071225) + " (Expected: 45)");
        System.out.println("Test 2: " + findNb(91716553919377L) + " (Expected: -1)");
        System.out.println("Test 3: " + findNb(4183059834009L) + " (Expected: 2022)");
        System.out.println("Test 4: " + findNb(24723578342962L) + " (Expected: -1)");
        System.out.println("Test 5: " + findNb(135) + " (Expected: 5)");
    }
}
