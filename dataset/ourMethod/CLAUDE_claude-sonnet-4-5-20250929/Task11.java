
public class Task11 {
    public static int nbYear(int p0, double percent, int aug, int p) {
        // Validate inputs
        if (p0 <= 0 || p <= 0) {
            throw new IllegalArgumentException("p0 and p must be positive integers");
        }
        if (percent < 0.0) {
            throw new IllegalArgumentException("percent must be positive or zero");
        }
        if (Double.isNaN(percent) || Double.isInfinite(percent)) {
            throw new IllegalArgumentException("percent must be a valid number");
        }
        
        // Convert percent to decimal
        double percentDecimal = percent / 100.0;
        
        int years = 0;
        int currentPopulation = p0;
        
        // Calculate years needed
        while (currentPopulation < p) {
            // Calculate next year population: current + (current * percent) + aug
            // Cast to int to floor the result (no fractions of people)
            currentPopulation = (int)(currentPopulation + currentPopulation * percentDecimal + aug);
            years++;
            
            // Safety check to prevent infinite loop
            if (years > 1000000) {
                throw new ArithmeticException("Calculation exceeded maximum iterations");
            }
        }
        
        return years;
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println("Test 1: " + nbYear(1000, 2, 50, 1200) + " (expected: 3)");
        
        // Test case 2
        System.out.println("Test 2: " + nbYear(1500, 5, 100, 5000) + " (expected: 15)");
        
        // Test case 3
        System.out.println("Test 3: " + nbYear(1500000, 2.5, 10000, 2000000) + " (expected: 10)");
        
        // Test case 4
        System.out.println("Test 4: " + nbYear(1000, 0, 50, 1200) + " (expected: 4)");
        
        // Test case 5
        System.out.println("Test 5: " + nbYear(1000, 2, 0, 1200) + " (expected: 10)");
    }
}
