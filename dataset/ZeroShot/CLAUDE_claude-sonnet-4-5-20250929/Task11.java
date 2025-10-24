
public class Task11 {
    public static int nbYear(int p0, double percent, int aug, int p) {
        int years = 0;
        int currentPopulation = p0;
        double percentDecimal = percent / 100.0;
        
        while (currentPopulation < p) {
            currentPopulation = (int)(currentPopulation + currentPopulation * percentDecimal + aug);
            years++;
        }
        
        return years;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: " + nbYear(1000, 2, 50, 1200) + " (Expected: 3)");
        System.out.println("Test 2: " + nbYear(1500, 5, 100, 5000) + " (Expected: 15)");
        System.out.println("Test 3: " + nbYear(1500000, 2.5, 10000, 2000000) + " (Expected: 10)");
        System.out.println("Test 4: " + nbYear(1000, 2, 0, 1200) + " (Expected: 10)");
        System.out.println("Test 5: " + nbYear(1000, 0, 50, 1500) + " (Expected: 10)");
    }
}
