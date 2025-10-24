import java.util.ArrayList;
import java.util.List;

public class Task176 {

    public static final class ProductOfNumbers {
        private final List<Long> prefix;

        public ProductOfNumbers() {
            prefix = new ArrayList<>();
            prefix.add(1L); // sentinel
        }

        public void add(int num) {
            if (num == 0) {
                prefix.clear();
                prefix.add(1L);
                return;
            }
            long last = prefix.get(prefix.size() - 1);
            long next = last * (long) num;
            prefix.add(next);
        }

        public int getProduct(int k) {
            if (k < 0) {
                throw new IllegalArgumentException("k must be non-negative");
            }
            // If k >= size, it crosses a zero boundary -> product is 0
            if (k >= prefix.size()) {
                return 0;
            }
            int sz = prefix.size();
            long num = prefix.get(sz - 1);
            long den = prefix.get(sz - 1 - k);
            if (den == 0L) {
                return 0; // defensive, though den should never be 0 with this design
            }
            long res = num / den;
            return (int) res;
        }
    }

    public static void main(String[] args) {
        // Test case 1: Example from prompt
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

        // Test case 2: No zeros
        ProductOfNumbers p2 = new ProductOfNumbers();
        p2.add(1);
        p2.add(2);
        p2.add(3);
        p2.add(4);
        System.out.println(p2.getProduct(4)); // 24

        // Test case 3: Multiple zeros with short queries
        ProductOfNumbers p3 = new ProductOfNumbers();
        p3.add(0);
        p3.add(7);
        p3.add(0);
        p3.add(10);
        System.out.println(p3.getProduct(1)); // 10
        System.out.println(p3.getProduct(2)); // 0

        // Test case 4: Larger k within segment
        ProductOfNumbers p4 = new ProductOfNumbers();
        p4.add(2);
        p4.add(2);
        p4.add(2);
        p4.add(2);
        p4.add(2);
        System.out.println(p4.getProduct(5)); // 32

        // Test case 5: Zero boundary checks
        ProductOfNumbers p5 = new ProductOfNumbers();
        p5.add(9);
        p5.add(9);
        p5.add(0);
        p5.add(9);
        p5.add(9);
        p5.add(9);
        System.out.println(p5.getProduct(3)); // 729
        System.out.println(p5.getProduct(4)); // 0
    }
}