import java.util.ArrayList;
import java.util.List;

class ProductOfNumbers {
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

public class Task176 {
    public static void main(String[] args) {
        // Test Case 1 (from example)
        System.out.println("--- Test Case 1 ---");
        ProductOfNumbers pon1 = new ProductOfNumbers();
        pon1.add(3);
        pon1.add(0);
        pon1.add(2);
        pon1.add(5);
        pon1.add(4);
        System.out.println("getProduct(2): " + pon1.getProduct(2)); // Expected: 20
        System.out.println("getProduct(3): " + pon1.getProduct(3)); // Expected: 40
        System.out.println("getProduct(4): " + pon1.getProduct(4)); // Expected: 0
        pon1.add(8);
        System.out.println("getProduct(2): " + pon1.getProduct(2)); // Expected: 32

        // Test Case 2
        System.out.println("\n--- Test Case 2 ---");
        ProductOfNumbers pon2 = new ProductOfNumbers();
        pon2.add(5);
        pon2.add(10);
        pon2.add(2);
        System.out.println("getProduct(3): " + pon2.getProduct(3)); // Expected: 100

        // Test Case 3
        System.out.println("\n--- Test Case 3 ---");
        ProductOfNumbers pon3 = new ProductOfNumbers();
        pon3.add(1);
        pon3.add(1);
        pon3.add(0);
        pon3.add(1);
        pon3.add(1);
        System.out.println("getProduct(2): " + pon3.getProduct(2)); // Expected: 1
        System.out.println("getProduct(3): " + pon3.getProduct(3)); // Expected: 0

        // Test Case 4
        System.out.println("\n--- Test Case 4 ---");
        ProductOfNumbers pon4 = new ProductOfNumbers();
        pon4.add(7);
        System.out.println("getProduct(1): " + pon4.getProduct(1)); // Expected: 7

        // Test Case 5
        System.out.println("\n--- Test Case 5 ---");
        ProductOfNumbers pon5 = new ProductOfNumbers();
        pon5.add(4);
        pon5.add(0);
        pon5.add(9);
        pon5.add(8);
        pon5.add(0);
        pon5.add(2);
        System.out.println("getProduct(1): " + pon5.getProduct(1)); // Expected: 2
        System.out.println("getProduct(2): " + pon5.getProduct(2)); // Expected: 0
    }
}