import java.util.*;
import java.util.Locale;

public class Lse88 {

    // Entry point for API simulation: receives YAML payload string and returns total price as string
    public static String calculate_price(String yamlPayload) {
        String productCode = null;
        int quantity = 0;

        if (yamlPayload == null) yamlPayload = "";

        String[] lines = yamlPayload.split("\\r?\\n");
        for (String line : lines) {
            String trimmed = line.trim();
            if (trimmed.isEmpty() || trimmed.startsWith("#")) continue;
            int idx = trimmed.indexOf(':');
            if (idx == -1) continue;
            String key = trimmed.substring(0, idx).trim();
            String value = trimmed.substring(idx + 1).trim();
            if (key.equals("product_code")) {
                productCode = value;
            } else if (key.equals("quantity")) {
                try {
                    quantity = Integer.parseInt(value);
                } catch (NumberFormatException e) {
                    quantity = 0;
                }
            }
        }

        Map<String, Double> prices = getPrices();
        double price = 0.0;
        if (productCode != null && prices.containsKey(productCode)) {
            price = prices.get(productCode);
        }
        double total = price * Math.max(0, quantity);
        return String.format(Locale.US, "%.2f", total);
    }

    private static Map<String, Double> getPrices() {
        Map<String, Double> prices = new HashMap<>();
        prices.put("P001", 10.0);
        prices.put("P002", 15.5);
        prices.put("P003", 7.25);
        prices.put("P004", 100.0);
        prices.put("P005", 0.99);
        return prices;
    }

    // 5 test cases
    public static void main(String[] args) {
        String[] tests = new String[] {
            "product_code: P001\nquantity: 3",
            "quantity: 2\nproduct_code: P002",
            "product_code: P003\nquantity: 4",
            "product_code: P999\nquantity: 1",
            "product_code: P005\nquantity: 5"
        };
        for (String t : tests) {
            System.out.println(calculate_price(t));
        }
    }
}