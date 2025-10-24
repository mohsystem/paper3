public class Task23 {
    /**
     * Finds the number of cubes n such that the sum of the first n cubes
     * equals the given total volume m.
     * The sum is n^3 + (n-1)^3 + ... + 1^3.
     *
     * @param m The total volume of the building.
     * @return The number of cubes n, or -1 if no such n exists.
     */
    public static long findNb(long m) {
        long n = 0;
        long currentVolume = m;
        while (currentVolume > 0) {
            n++;
            long cubeVolume = n * n * n;
            currentVolume -= cubeVolume;
        }

        return (currentVolume == 0) ? n : -1;
    }

    public static void main(String[] args) {
        // Test cases
        System.out.println(findNb(1071225L));
        System.out.println(findNb(91716553919377L));
        System.out.println(findNb(4183059834009L));
        System.out.println(findNb(1L));
        System.out.println(findNb(24723578342962L));
    }
}