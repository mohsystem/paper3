
public class Task11 {
    public static int nb_year(int p0, double percent, int aug, int p) {
        // Input validation to prevent security issues
        if (p0 <= 0 || p <= 0 || percent < 0) {
            throw new IllegalArgumentException("Invalid input parameters");
        }
        
        // Prevent integer overflow
        if (p0 > Integer.MAX_VALUE || p > Integer.MAX_VALUE) {
            throw new IllegalArgumentException("Population values too large");
        }
        
        int years = 0;
        int currentPopulation = p0;
        
        // Convert percent to decimal (e.g., 2 -> 0.02)
        double percentDecimal = percent / 100.0;
        
        // Calculate years needed, with overflow protection
        while (currentPopulation < p) {
            // Prevent infinite loop
            if (years > 10000) {
                throw new ArithmeticException("Calculation exceeded maximum iterations");
            }
            
            // Calculate new population with integer conversion (floor)
            long newPopulation = (long) currentPopulation + (long) (currentPopulation * percentDecimal) + aug;
            
            // Check for overflow
            if (newPopulation > Integer.MAX_VALUE || newPopulation < 0) {
                throw new ArithmeticException("Population overflow occurred");
            }
            
            currentPopulation = (int) newPopulation;
            years++;
        }
        
        return years;
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println("Test 1: " + nb_year(1000, 2, 50, 1200) + " (Expected: 3)");
        
        // Test case 2
        System.out.println("Test 2: " + nb_year(1500, 5, 100, 5000) + " (Expected: 15)");
        
        // Test case 3
        System.out.println("Test 3: " + nb_year(1500000, 2.5, 10000, 2000000) + " (Expected: 10)");
        
        // Test case 4
        System.out.println("Test 4: " + nb_year(1000, 0, 50, 1200) + " (Expected: 4)");
        
        // Test case 5
        System.out.println("Test 5: " + nb_year(1500, 10, 200, 10000) + " (Expected: 14)");
    }
}
