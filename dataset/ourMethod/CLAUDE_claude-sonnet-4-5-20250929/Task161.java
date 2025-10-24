
import java.util.*;
import java.nio.charset.StandardCharsets;

public class Task161 {
    private static final int MAX_CAPACITY = 1000000;
    private static final int MAX_ITEMS = 10000;
    private static final int MAX_WEIGHT = 1000000;
    private static final int MAX_VALUE = 1000000;

    public static class Item {
        private final int weight;
        private final int value;

        public Item(int weight, int value) {
            if (weight < 0 || weight > MAX_WEIGHT) {
                throw new IllegalArgumentException("Weight must be between 0 and " + MAX_WEIGHT);
            }
            if (value < 0 || value > MAX_VALUE) {
                throw new IllegalArgumentException("Value must be between 0 and " + MAX_VALUE);
            }
            this.weight = weight;
            this.value = value;
        }

        public int getWeight() {
            return weight;
        }

        public int getValue() {
            return value;
        }
    }

    public static int knapsack(int capacity, List<Item> items) {
        if (capacity < 0 || capacity > MAX_CAPACITY) {
            throw new IllegalArgumentException("Capacity must be between 0 and " + MAX_CAPACITY);
        }
        if (items == null) {
            throw new IllegalArgumentException("Items list cannot be null");
        }
        if (items.size() > MAX_ITEMS) {
            throw new IllegalArgumentException("Number of items exceeds maximum allowed: " + MAX_ITEMS);
        }

        if (items.isEmpty() || capacity == 0) {
            return 0;
        }

        int n = items.size();
        int[] dp = new int[capacity + 1];
        Arrays.fill(dp, 0);

        for (int i = 0; i < n; i++) {
            Item item = items.get(i);
            if (item == null) {
                throw new IllegalArgumentException("Item at index " + i + " is null");
            }
            
            int weight = item.getWeight();
            int value = item.getValue();

            for (int w = capacity; w >= weight; w--) {
                int newValue = dp[w - weight] + value;
                if (newValue > dp[w]) {
                    dp[w] = newValue;
                }
            }
        }

        return dp[capacity];
    }

    public static void main(String[] args) {
        try {
            // Test case 1: Basic case
            List<Item> items1 = new ArrayList<>();
            items1.add(new Item(2, 3));
            items1.add(new Item(3, 4));
            items1.add(new Item(4, 5));
            items1.add(new Item(5, 6));
            int result1 = knapsack(5, items1);
            System.out.println("Test 1 - Capacity 5: " + result1);

            // Test case 2: Empty items
            List<Item> items2 = new ArrayList<>();
            int result2 = knapsack(10, items2);
            System.out.println("Test 2 - Empty items: " + result2);

            // Test case 3: Zero capacity
            List<Item> items3 = new ArrayList<>();
            items3.add(new Item(5, 10));
            int result3 = knapsack(0, items3);
            System.out.println("Test 3 - Zero capacity: " + result3);

            // Test case 4: Multiple items
            List<Item> items4 = new ArrayList<>();
            items4.add(new Item(1, 1));
            items4.add(new Item(3, 4));
            items4.add(new Item(4, 5));
            items4.add(new Item(5, 7));
            int result4 = knapsack(7, items4);
            System.out.println("Test 4 - Capacity 7: " + result4);

            // Test case 5: Large values
            List<Item> items5 = new ArrayList<>();
            items5.add(new Item(10, 60));
            items5.add(new Item(20, 100));
            items5.add(new Item(30, 120));
            int result5 = knapsack(50, items5);
            System.out.println("Test 5 - Capacity 50: " + result5);

        } catch (IllegalArgumentException e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
