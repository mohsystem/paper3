import java.util.ArrayList;

public class Task176 {
    static class ProductOfNumbers {
        private ArrayList<Integer> prefix;
        public ProductOfNumbers() {
            prefix = new ArrayList<>();
            prefix.add(1);
        }
        public void add(int num) {
            if (num == 0) {
                prefix.clear();
                prefix.add(1);
            } else {
                int last = prefix.get(prefix.size() - 1);
                prefix.add(last * num);
            }
        }
        public int getProduct(int k) {
            if (k >= prefix.size()) return 0;
            int n = prefix.size();
            return prefix.get(n - 1) / prefix.get(n - 1 - k);
        }
    }

    public static void main(String[] args) {
        // Test case 1 (sample)
        ProductOfNumbers p1 = new ProductOfNumbers();
        p1.add(3); p1.add(0); p1.add(2); p1.add(5); p1.add(4);
        System.out.println(p1.getProduct(2));
        System.out.println(p1.getProduct(3));
        System.out.println(p1.getProduct(4));
        p1.add(8);
        System.out.println(p1.getProduct(2));

        // Test case 2
        ProductOfNumbers p2 = new ProductOfNumbers();
        p2.add(1); p2.add(2); p2.add(3); p2.add(4);
        System.out.println(p2.getProduct(1));
        System.out.println(p2.getProduct(4));

        // Test case 3
        ProductOfNumbers p3 = new ProductOfNumbers();
        p3.add(0); p3.add(0); p3.add(2); p3.add(3);
        System.out.println(p3.getProduct(1));
        System.out.println(p3.getProduct(3));

        // Test case 4
        ProductOfNumbers p4 = new ProductOfNumbers();
        p4.add(10); p4.add(10); p4.add(10);
        System.out.println(p4.getProduct(2));
        System.out.println(p4.getProduct(3));

        // Test case 5
        ProductOfNumbers p5 = new ProductOfNumbers();
        p5.add(100); p5.add(0); p5.add(100); p5.add(100);
        System.out.println(p5.getProduct(1));
        System.out.println(p5.getProduct(2));
        System.out.println(p5.getProduct(3));
    }
}