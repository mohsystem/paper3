// Chain-of-Through steps applied: understanding, secure design, implementation, review, and safe output.
// Java implementation with main class Task11 and 5 test cases.
public class Task11 {
    // Compute years needed for population to reach or exceed target.
    // Returns -1 if target is unreachable given the parameters.
    public static int nb_year(int p0, double percent, int aug, int p) {
        if (p0 >= p) return 0;
        double rate = percent / 100.0;
        long pop = p0;
        int years = 0;
        // Prevent infinite loops by detecting non-increasing population or excessive iterations.
        while (pop < p) {
            long prev = pop;
            long delta = (long) Math.floor(pop * rate) + aug; // floor as per requirement
            pop += delta;
            years++;
            if (pop <= prev || years > 10000000) {
                return -1; // unreachable
            }
        }
        return years;
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println(nb_year(1000, 2, 50, 1200));        // expected 3
        System.out.println(nb_year(1500, 5, 100, 5000));       // expected 15
        System.out.println(nb_year(1500000, 2.5, 10000, 2000000)); // expected 10
        System.out.println(nb_year(1500000, 0, 10000, 2000000));   // expected 50
        System.out.println(nb_year(1000, 0, -1, 1200));        // expected -1 (unreachable)
    }
}