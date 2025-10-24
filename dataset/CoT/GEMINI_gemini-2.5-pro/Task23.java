public class Task23 {

    public static long findNb(long m) {
        long totalVolume = 0;
        long n = 0;
        while (totalVolume < m) {
            n++;
            // Use long for cubeVolume to prevent overflow during multiplication
            long cubeVolume = n * n * n;
            totalVolume += cubeVolume;
        }

        if (totalVolume == m) {
            return n;
        } else {
            return -1;
        }
    }

    public static void main(String[] args) {
        System.out.println("Java Test Cases:");
        // Test Case 1: Example from description
        System.out.println("findNb(1071225) -> Expected: 45, Got: " + findNb(1071225));
        // Test Case 2: Example from description, no solution
        System.out.println("findNb(91716553919377L) -> Expected: -1, Got: " + findNb(91716553919377L));
        // Test Case 3: A large number with a solution
        System.out.println("findNb(4183059834009L) -> Expected: 2022, Got: " + findNb(4183059834009L));
        // Test Case 4: Smallest possible input with a solution
        System.out.println("findNb(1) -> Expected: 1, Got: " + findNb(1));
        // Test Case 5: Another large number with a solution
        System.out.println("findNb(135440716410000L) -> Expected: 4824, Got: " + findNb(135440716410000L));
    }
}