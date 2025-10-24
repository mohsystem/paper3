public class Task11 {

    public static int nbYear(int p0, double percent, int aug, int p) {
        int years = 0;
        double currentPopulation = p0;
        double percentDecimal = percent / 100.0;

        while (currentPopulation < p) {
            currentPopulation = Math.floor(currentPopulation + currentPopulation * percentDecimal + aug);
            years++;
        }
        return years;
    }

    public static void main(String[] args) {
        // Test Case 1
        System.out.println(nbYear(1500, 5, 100, 5000));
        // Test Case 2
        System.out.println(nbYear(1500000, 2.5, 10000, 2000000));
        // Test Case 3
        System.out.println(nbYear(1000, 2, 50, 1200));
        // Test Case 4: Target population is already met or surpassed
        System.out.println(nbYear(1200, 2, 50, 1200));
        // Test Case 5: Zero percent growth
        System.out.println(nbYear(1000, 0, 100, 2000));
    }
}