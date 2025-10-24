public class Task11 {

    /**
     * Calculates the number of years required for a population to reach a target.
     *
     * @param p0      Initial population. Must be > 0.
     * @param percent Annual percentage growth. Must be >= 0.
     * @param aug     Annual net change in inhabitants (can be positive or negative).
     * @param p       Target population. Must be > 0.
     * @return The number of full years to reach or exceed the target population.
     */
    public static int nbYear(int p0, double percent, int aug, int p) {
        int years = 0;
        double currentPopulation = p0;
        double percentDecimal = percent / 100.0;

        while (currentPopulation < p) {
            currentPopulation = Math.floor(currentPopulation * (1 + percentDecimal) + aug);
            years++;
        }
        return years;
    }

    public static void main(String[] args) {
        // Test Case 1: Standard growth
        System.out.println("Test Case 1: Expected 15, Got " + nbYear(1500, 5, 100, 5000));

        // Test Case 2: Large numbers
        System.out.println("Test Case 2: Expected 10, Got " + nbYear(1500000, 2.5, 10000, 2000000));

        // Test Case 3: Example from prompt
        System.out.println("Test Case 3: Expected 3, Got " + nbYear(1000, 2, 50, 1200));

        // Test Case 4: Initial population already meets target
        System.out.println("Test Case 4: Expected 0, Got " + nbYear(1200, 2, 50, 1200));

        // Test Case 5: Slow growth
        System.out.println("Test Case 5: Expected 4, Got " + nbYear(1000, 0.5, 20, 1100));
    }
}