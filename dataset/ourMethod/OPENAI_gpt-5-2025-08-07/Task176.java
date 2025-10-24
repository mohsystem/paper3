import java.util.ArrayList;
import java.util.List;

public class Task176 {
    // ProductOfNumbers class using prefix products reset on zero
    public static class ProductOfNumbers {
        private final List<Integer> pref; // pref[0] = 1; pref[i] = product of first i numbers since last zero

        public ProductOfNumbers() {
            this.pref = new ArrayList<>();
            this.pref.add(1);
        }

        // Appends the integer num to the stream
        public void add(int num) {
            if (num < 0 || num > 100) {
                throw new IllegalArgumentException("num out of allowed range [0,100]");
            }
            if (num == 0) {
                // reset prefix products when zero encountered
                pref.clear();
                pref.add(1);
            } else {
                int last = pref.get(pref.size() - 1);
                int next = last * num; // per constraints, fits in 32-bit int
                pref.add(next);
            }
        }

        // Returns the product of the last k numbers
        public int getProduct(int k) {
            if (k <= 0) {
                throw new IllegalArgumentException("k must be positive");
            }
            if (k >= pref.size()) {
                // crosses a zero
                return 0;
            }
            int n = pref.size();
            int num = pref.get(n - 1);
            int den = pref.get(n - 1 - k);
            // den cannot be zero as prefix after zero contains no zeros
            return num / den;
        }
    }

    private static void printHeader(String title) {
        System.out.println("---- " + title + " ----");
    }

    public static void main(String[] args) {
        // Test Case 1: Example from prompt
        printHeader("Test Case 1: Example");
        ProductOfNumbers p1 = new ProductOfNumbers();
        p1.add(3);
        p1.add(0);
        p1.add(2);
        p1.add(5);
        p1.add(4);
        System.out.println(p1.getProduct(2)); // 20
        System.out.println(p1.getProduct(3)); // 40
        System.out.println(p1.getProduct(4)); // 0
        p1.add(8);
        System.out.println(p1.getProduct(2)); // 32

        // Test Case 2: No zeros
        printHeader("Test Case 2: No zeros");
        ProductOfNumbers p2 = new ProductOfNumbers();
        p2.add(1);
        p2.add(2);
        p2.add(3);
        p2.add(4);
        System.out.println(p2.getProduct(1)); // 4
        System.out.println(p2.getProduct(2)); // 12
        System.out.println(p2.getProduct(4)); // 24

        // Test Case 3: Multiple zeros and resets
        printHeader("Test Case 3: Multiple zeros");
        ProductOfNumbers p3 = new ProductOfNumbers();
        p3.add(0);
        p3.add(9);
        p3.add(9);
        System.out.println(p3.getProduct(2)); // 81
        p3.add(0);
        System.out.println(p3.getProduct(1)); // 0
        p3.add(7);
        System.out.println(p3.getProduct(2)); // 0 (crosses zero)
        System.out.println(p3.getProduct(1)); // 7

        // Test Case 4: Crossing zero with k larger than segment since zero
        printHeader("Test Case 4: Crossing zero");
        ProductOfNumbers p4 = new ProductOfNumbers();
        p4.add(2);
        p4.add(5);
        p4.add(0);
        p4.add(3);
        p4.add(4);
        System.out.println(p4.getProduct(2)); // 12
        System.out.println(p4.getProduct(3)); // 0 (includes zero)
        System.out.println(p4.getProduct(5)); // 0 (includes zero)

        // Test Case 5: Mixed sequence with multiple segments
        printHeader("Test Case 5: Mixed segments");
        ProductOfNumbers p5 = new ProductOfNumbers();
        p5.add(5);
        p5.add(2);
        p5.add(0);
        p5.add(1);
        p5.add(1);
        p5.add(1);
        p5.add(10);
        p5.add(0);
        p5.add(2);
        p5.add(2);
        p5.add(2);
        System.out.println(p5.getProduct(1)); // 2
        System.out.println(p5.getProduct(2)); // 4
        System.out.println(p5.getProduct(3)); // 8
        System.out.println(p5.getProduct(4)); // 0 (crosses zero)
    }
}