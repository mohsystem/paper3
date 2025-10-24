import java.util.ArrayList;
import java.util.List;

public class Task176 {

    private List<Integer> prefixProducts;

    public Task176() {
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
        int prevProduct = prefixProducts.get(n - 1 - k);
        return totalProduct / prevProduct;
    }

    public static void main(String[] args) {
        // Test Case 1
        Task176 pon1 = new Task176();
        pon1.add(3);
        pon1.add(0);
        pon1.add(2);
        pon1.add(5);
        pon1.add(4);
        System.out.println(pon1.getProduct(2)); // 20
        System.out.println(pon1.getProduct(3)); // 40
        System.out.println(pon1.getProduct(4)); // 0
        pon1.add(8);
        System.out.println(pon1.getProduct(2)); // 32
        System.out.println();

        // Test Case 2
        Task176 pon2 = new Task176();
        pon2.add(1);
        pon2.add(2);
        pon2.add(3);
        pon2.add(4);
        pon2.add(5);
        System.out.println(pon2.getProduct(1)); // 5
        System.out.println(pon2.getProduct(2)); // 20
        System.out.println(pon2.getProduct(5)); // 120
        System.out.println();

        // Test Case 3
        Task176 pon3 = new Task176();
        pon3.add(0);
        pon3.add(10);
        pon3.add(20);
        System.out.println(pon3.getProduct(1)); // 20
        System.out.println(pon3.getProduct(2)); // 200
        System.out.println(pon3.getProduct(3)); // 0
        System.out.println();

        // Test Case 4
        Task176 pon4 = new Task176();
        pon4.add(5);
        pon4.add(6);
        pon4.add(0);
        pon4.add(7);
        pon4.add(0);
        pon4.add(8);
        pon4.add(9);
        System.out.println(pon4.getProduct(1)); // 9
        System.out.println(pon4.getProduct(2)); // 72
        System.out.println(pon4.getProduct(3)); // 0
        System.out.println();

        // Test Case 5
        Task176 pon5 = new Task176();
        pon5.add(42);
        System.out.println(pon5.getProduct(1)); // 42
        pon5.add(0);
        System.out.println(pon5.getProduct(1)); // 0
        pon5.add(1);
        System.out.println(pon5.getProduct(1)); // 1
        System.out.println(pon5.getProduct(2)); // 0
        System.out.println();
    }
}