import java.math.BigDecimal;
import java.math.RoundingMode;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;

public class Lse88 {

    private static final int MAX_PAYLOAD_SIZE = 10_000;
    private static final Map<String, BigDecimal> PRICE_MAP = new HashMap<>();
    static {
        PRICE_MAP.put("P001", new BigDecimal("9.99"));
        PRICE_MAP.put("P002", new BigDecimal("19.99"));
        PRICE_MAP.put("P003", new BigDecimal("25.00"));
        PRICE_MAP.put("P004", new BigDecimal("123.45"));
        PRICE_MAP.put("P005", new BigDecimal("0.50"));
    }

    public static String calculate_price(String yamlPayload) {
        if (yamlPayload == null) {
            return "error: payload is null";
        }
        if (yamlPayload.length() > MAX_PAYLOAD_SIZE) {
            return "error: payload too large";
        }

        String productCode = null;
        String quantityStr = null;
        String[] lines = yamlPayload.split("\\r?\\n");
        for (String rawLine : lines) {
            String line = rawLine.trim();
            if (line.isEmpty() || line.startsWith("#")) continue;
            int idx = line.indexOf(':');
            if (idx <= 0) continue;
            String key = line.substring(0, idx).trim();
            String value = line.substring(idx + 1).trim();
            if ((value.startsWith("\"") && value.endsWith("\"") && value.length() >= 2)
                || (value.startsWith("'") && value.endsWith("'") && value.length() >= 2)) {
                value = value.substring(1, value.length() - 1);
            }
            if ("product_code".equals(key)) {
                productCode = value;
            } else if ("quantity".equals(key)) {
                quantityStr = value;
            }
        }

        if (productCode == null || productCode.isEmpty()) {
            return "error: missing product_code";
        }
        if (!productCode.matches("[A-Za-z0-9_-]{1,32}")) {
            return "error: invalid product_code";
        }

        if (quantityStr == null || quantityStr.isEmpty()) {
            return "error: missing quantity";
        }
        int quantity;
        try {
            if (!quantityStr.matches("[0-9]{1,9}")) {
                return "error: invalid quantity";
            }
            quantity = Integer.parseInt(quantityStr);
        } catch (NumberFormatException e) {
            return "error: invalid quantity";
        }
        if (quantity <= 0 || quantity > 1_000_000) {
            return "error: quantity out of range";
        }

        BigDecimal price = PRICE_MAP.get(productCode);
        if (price == null) {
            return "error: unknown product_code";
        }

        BigDecimal total = price.multiply(new BigDecimal(quantity)).setScale(2, RoundingMode.HALF_UP);
        return total.toPlainString();
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "product_code: P001\nquantity: 3",
            "# Sample order\nproduct_code: \"P002\"\nquantity: 1",
            "product_code: P999\nquantity: 2",
            "product_code: P001\nquantity: -5",
            "product_code: P004\nquantity: 1000000"
        };
        for (int i = 0; i < tests.length; i++) {
            String result = calculate_price(tests[i]);
            System.out.println("Test " + (i + 1) + ": " + result);
        }
    }
}