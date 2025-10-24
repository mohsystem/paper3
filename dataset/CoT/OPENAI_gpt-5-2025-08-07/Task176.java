import java.util.*;

public class Task176 {
    // ProductOfNumbers implementation using prefix products
    static class ProductOfNumbers {
        private final List<Integer> prefix;

        public ProductOfNumbers() {
            this.prefix = new ArrayList<>();
            this.prefix.add(1);
        }

        public void add(int num) {
            if (num == 0) {
                prefix.clear();
                prefix.add(1);
            } else {
                long prod = (long) prefix.get(prefix.size() - 1) * (long) num;
                // Safe cast as per problem constraints (fits in 32-bit)
                prefix.add((int) prod);
            }
        }

        public int getProduct(int k) {
            int n = prefix.size();
            if (k >= n) return 0;
            int last = prefix.get(n - 1);
            int prev = prefix.get(n - 1 - k);
            return last / prev;
        }
    }

    // Helper to run a test and print outputs
    private static void printList(List<Integer> list) {
        System.out.println(list.toString());
    }

    public static void main(String[] args) {
        // Test 1: Example provided
        {
            ProductOfNumbers pon = new ProductOfNumbers();
            pon.add(3);
            pon.add(0);
            pon.add(2);
            pon.add(5);
            pon.add(4);
            List<Integer> out = new ArrayList<>();
            out.add(pon.getProduct(2)); // 20
            out.add(pon.getProduct(3)); // 40
            out.add(pon.getProduct(4)); // 0
            pon.add(8);
            out.add(pon.getProduct(2)); // 32
            printList(out);
        }

        // Test 2: No zeros
        {
            ProductOfNumbers pon = new ProductOfNumbers();
            pon.add(1);
            pon.add(2);
            pon.add(3);
            pon.add(4);
            List<Integer> out = new ArrayList<>();
            out.add(pon.getProduct(1)); // 4
            out.add(pon.getProduct(3)); // 2*3*4=24
            out.add(pon.getProduct(4)); // 1*2*3*4=24
            printList(out);
        }

        // Test 3: Zeros resetting
        {
            ProductOfNumbers pon = new ProductOfNumbers();
            pon.add(0);
            pon.add(0);
            pon.add(7);
            pon.add(3);
            List<Integer> out = new ArrayList<>();
            out.add(pon.getProduct(1)); // 3
            out.add(pon.getProduct(2)); // 7*3=21
            printList(out);
        }

        // Test 4: Long k equals length
        {
            ProductOfNumbers pon = new ProductOfNumbers();
            pon.add(2);
            pon.add(2);
            pon.add(2);
            pon.add(2);
            pon.add(2);
            List<Integer> out = new ArrayList<>();
            out.add(pon.getProduct(5)); // 32
            printList(out);
        }

        // Test 5: Mix with ones and zero
        {
            ProductOfNumbers pon = new ProductOfNumbers();
            pon.add(1);
            pon.add(1);
            pon.add(1);
            pon.add(1);
            pon.add(0);
            pon.add(10);
            List<Integer> out = new ArrayList<>();
            out.add(pon.getProduct(1)); // 10
            out.add(pon.getProduct(2)); // 0
            pon.add(2);
            out.add(pon.getProduct(2)); // 10*2=20
            printList(out);
        }
    }
}