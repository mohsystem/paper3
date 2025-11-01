import java.util.*;
import java.text.DecimalFormat;

public class Lse90 {
    private static final Map<String, Double> PRICE_MAP = createPriceMap();

    private static Map<String, Double> createPriceMap() {
        Map<String, Double> m = new HashMap<>();
        m.put("A100", 10.00);
        m.put("B200", 5.50);
        m.put("C300", 2.25);
        m.put("D400", 12.75);
        m.put("E500", 99.99);
        return m;
    }

    public static double get_price_for_product_code(String productCode) {
        if (productCode == null) return 0.0;
        return PRICE_MAP.getOrDefault(productCode.trim(), 0.0);
    }

    private static Map<String, String> parseYamlToMap(String yaml) {
        Map<String, String> map = new HashMap<>();
        if (yaml == null) return map;
        String[] lines = yaml.split("\\r?\\n");
        for (String line : lines) {
            int idx = line.indexOf(':');
            if (idx >= 0) {
                String key = line.substring(0, idx).trim();
                String val = line.substring(idx + 1).trim();
                map.put(key, val);
            }
        }
        return map;
    }

    private static String formatTwoDecimals(double value) {
        DecimalFormat df = new DecimalFormat("0.00");
        return df.format(value);
    }

    public static String calculate_price(String yamlQueryParamPayload) {
        Map<String, String> data = parseYamlToMap(yamlQueryParamPayload);
        String code = data.getOrDefault("product_code", "");
        String qtyStr = data.getOrDefault("quantity", "0");
        int quantity = 0;
        try {
            quantity = Integer.parseInt(qtyStr.trim());
        } catch (Exception ignored) {}
        double price = get_price_for_product_code(code);
        double total = price * quantity;
        return "total_price: " + formatTwoDecimals(total);
    }

    public static void main(String[] args) {
        String[] testPayloads = new String[] {
            "product_code: A100\nquantity: 3",
            "product_code: B200\nquantity: 2",
            "product_code: C300\nquantity: 0",
            "product_code: UNKNOWN\nquantity: 5",
            "quantity: 4\nproduct_code: D400"
        };
        for (String payload : testPayloads) {
            String result = calculate_price(payload);
            System.out.println(result);
        }
    }
}