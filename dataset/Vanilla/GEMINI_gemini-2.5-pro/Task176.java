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
            // If a zero is added, reset the prefix products.
            // Any product including this zero will be 0.
            prefixProducts.clear();
            prefixProducts.add(1);
        } else {
            // Append the new prefix product.
            prefixProducts.add(prefixProducts.get(prefixProducts.size() - 1) * num);
        }
    }

    public int getProduct(int k) {
        int n = prefixProducts.size();
        // If k is greater than or equal to the number of elements since the last zero,
        // it means a zero is included in the range of the last k numbers.
        if (k >= n) {
            return 0;
        }
        // The product of the last k numbers is P[n-1] / P[n-1-k].
        int lastProduct = prefixProducts.get(n - 1);
        int productBeforeK = prefixProducts.get(n - 1 - k);
        return lastProduct / productBeforeK;
    }
}

public class Task176 {
    public static void main(String[] args) {
        // Test Case 1 (from example)
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

        // Test Case 2: No zeros
        System.out.println("Test Case 2:");
        ProductOfNumbers p2 = new ProductOfNumbers();
        p2.add(1);
        p2.add(2);
        p2.add(3);
        p2.add(4);
        System.out.println("getProduct(1): " + p2.getProduct(1));
        System.out.println("getProduct(4): " + p2.getProduct(4));
        System.out.println();

        // Test Case 3: Starts with zero
        System.out.println("Test Case 3:");
        ProductOfNumbers p3 = new ProductOfNumbers();
        p3.add(0);
        p3.add(5);
        p3.add(10);
        System.out.println("getProduct(1): " + p3.getProduct(1));
        System.out.println("getProduct(2): " + p3.getProduct(2));
        System.out.println("getProduct(3): " + p3.getProduct(3));
        System.out.println();

        // Test Case 4: Multiple zeros
        System.out.println("Test Case 4:");
        ProductOfNumbers p4 = new ProductOfNumbers();
        p4.add(5);
        p4.add(6);
        p4.add(0);
        p4.add(7);
        p4.add(0);
        p4.add(8);
        p4.add(9);
        System.out.println("getProduct(1): " + p4.getProduct(1));
        System.out.println("getProduct(2): " + p4.getProduct(2));
        System.out.println("getProduct(3): " + p4.getProduct(3));
        System.out.println();

        // Test Case 5: k equals the number of elements
        System.out.println("Test Case 5:");
        ProductOfNumbers p5 = new ProductOfNumbers();
        p5.add(10);
        p5.add(20);
        p5.add(30);
        System.out.println("getProduct(3): " + p5.getProduct(3));
        System.out.println();
    }
}