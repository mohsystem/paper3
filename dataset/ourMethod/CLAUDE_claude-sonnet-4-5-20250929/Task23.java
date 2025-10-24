
public class Task23 {
    public static long findNb(long m) {
        // Input validation
        if (m < 0) {
            return -1;
        }
        if (m == 0) {
            return 0;
        }
        
        long n = 0;
        long sum = 0;
        
        // Iterate to find n such that sum of cubes equals m
        // Using formula: 1^3 + 2^3 + ... + n^3 = [n(n+1)/2]^2
        // We can also iterate incrementally to avoid overflow
        while (sum < m) {
            n++;
            // Check for potential overflow before cubing
            if (n > 1000000) {
                return -1;
            }
            long cube = n * n * n;
            // Check for overflow in addition
            if (sum > Long.MAX_VALUE - cube) {
                return -1;
            }
            sum += cube;
        }
        
        return (sum == m) ? n : -1;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: findNb(1071225) = " + findNb(1071225));
        System.out.println("Test 2: findNb(91716553919377L) = " + findNb(91716553919377L));
        System.out.println("Test 3: findNb(4183059834009) = " + findNb(4183059834009L));
        System.out.println("Test 4: findNb(24723578342962) = " + findNb(24723578342962L));
        System.out.println("Test 5: findNb(135440716410000) = " + findNb(135440716410000L));
    }
}
