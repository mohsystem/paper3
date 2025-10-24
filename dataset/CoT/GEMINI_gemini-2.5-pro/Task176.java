import java.util.ArrayList;
import java.util.List;

public class Task176 {

    static class ProductOfNumbers {
        private List<Integer> prefixProducts;

        public ProductOfNumbers() {
            prefixProducts = new ArrayList<>();
            prefixProducts.add(1);
        }

        public void add(int num) {
            if (num == 0) {
                prefixProducts.clear();
                prefixProducts.add(1);
            } else {
                int lastProduct = prefixProducts.get(prefixProducts.size() - 1);
                prefixProducts.add(lastProduct * num);
            }
        }

        public int getProduct(int k) {
            int n = prefixProducts.size();
            if (k >= n) {
                return 0;
            }
            int totalProduct = prefixProducts.get(n - 1);
            int productBeforeK = prefixProducts.get(n - 1 - k);
            return totalProduct / productBeforeK;
        }
    }

    public static void main(String[] args) {
        // Test Case 1: Example from prompt
        System.out.println("Test Case 1:");
        ProductOfNumbers p1 = new ProductOfNumbers();
        p1.add(3);
        p1.add(0);
        p1.add(2);
        p1.add(5);
        p1.add(4);
        System.out.println("getProduct(2): " + p1.getProduct(2));
        System.out.println("getProduct(3): " + p1.getProduct(3));
        System.out.println("getProduct(4): " + p1.getProduct(4));
        p1.add(8);
        System.out.println("getProduct(2): " + p1.getProduct(2));
        System.out.println();

        // Test Case 2: Multiple zeros
        System.out.println("Test Case 2:");
        ProductOfNumbers p2 = new ProductOfNumbers();
        p2.add(1);
        p2.add(2);
        p2.add(0);
        p2.add(4);
        p2.add(5);
        System.out.println("getProduct(2): " + p2.getProduct(2));
        p2.add(0);
        p2.add(10);
        System.out.println("getProduct(1): " + p2.getProduct(1));
        System.out.println();
        
        // Test Case 3: k equals the number of non-zero elements
        System.out.println("Test Case 3:");
        ProductOfNumbers p3 = new ProductOfNumbers();
        p3.add(5);
        p3.add(6);
        p3.add(7);
        System.out.println("getProduct(3): " + p3.getProduct(3));
        System.out.println();
        
        // Test Case 4: getProduct(1)
        System.out.println("Test Case 4:");
        ProductOfNumbers p4 = new ProductOfNumbers();
        p4.add(100);
        p4.add(50);
        System.out.println("getProduct(1): " + p4.getProduct(1));
        System.out.println("getProduct(2): " + p4.getProduct(2));
        System.out.println();
        
        // Test Case 5: k spans across a zero
        System.out.println("Test Case 5:");
        ProductOfNumbers p5 = new ProductOfNumbers();
        p5.add(1);
        p5.add(2);
        p5.add(3);
        p5.add(0);
        p5.add(4);
        p5.add(5);
        p5.add(6);
        System.out.println("getProduct(3): " + p5.getProduct(3));
        System.out.println("getProduct(4): " + p5.getProduct(4));
        System.out.println();
    }
}