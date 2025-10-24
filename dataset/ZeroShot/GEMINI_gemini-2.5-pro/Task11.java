public class Task11 {

    /**
     * Calculates the number of years required for a population to reach a target.
     *
     * @param p0      The initial population.
     * @param percent The annual percentage increase.
     * @param aug     The number of inhabitants coming or leaving each year.
     * @param p       The target population to surpass.
     * @return The number of full years needed.
     */
    public static int nbYear(int p0, double percent, int aug, int p) {
        int years = 0;
        int currentPopulation = p0;
        double percentDecimal = percent / 100.0;

        while (currentPopulation < p) {
            // Calculate the population for the next year and truncate to the nearest whole number.
            currentPopulation = (int) (currentPopulation + currentPopulation * percentDecimal + aug);
            years++;
        }
        return years;
    }

    public static void main(String[] args) {
        // Test Case 1: From example
        System.out.println(nbYear(1500, 5, 100, 5000));
        // Test Case 2: From example
        System.out.println(nbYear(1500000, 2.5, 10000, 2000000));
        // Test Case 3: Initial prompt example
        System.out.println(nbYear(1000, 2, 50, 1200));
        // Test Case 4: Initial population is already at or above target
        System.out.println(nbYear(1200, 2, 50, 1200));
        // Test Case 5: Small growth
        System.out.println(nbYear(1000, 0.5, 2, 1010));
    }
}