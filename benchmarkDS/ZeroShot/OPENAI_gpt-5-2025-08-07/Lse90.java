import java.math.BigDecimal;
import java.math.RoundingMode;
import java.util.*;

public class Lse90 {
    private static final int MAX_PAYLOAD_LEN = 4096;
    private static final Map<String, BigDecimal> PRICE_MAP;
    static {
        Map<String, BigDecimal> m = new HashMap<>();
        m.put("A100", new BigDecimal("19.99"));
        m.put("B200", new BigDecimal("5.99"));
        m.put("C300", new BigDecimal("25.00"));
        m.put("D400", new BigDecimal("123.45"));
        m.put("E500", new BigDecimal("0.01"));
        PRICE_MAP = Collections.unmodifiableMap(m);
    }

    public static BigDecimal get_price_for_product_code(String productCode) {
        if (productCode == null) return null;
        String code = productCode.trim().toUpperCase(Locale.ROOT);
        if (!code.matches("^[A-Z0-9_-]{1,32}$")) {
            return null;
        }
        return PRICE_MAP.get(code);
    }

    public static String calculate_price(String yamlPayload) {
        try {
            if (yamlPayload == null) {
                return "error: \"missing payload\"\n";
            }
            if (yamlPayload.length() > MAX_PAYLOAD_LEN) {
                return "error: \"payload too large\"\n";
            }
            Map<String, String> data = parseSimpleYaml(yamlPayload);
            String rawCode = data.get("product_code");
            String rawQty = data.get("quantity");
            if (rawCode == null || rawCode.isEmpty()) {
                return "error: \"missing product_code\"\n";
            }
            String code = rawCode.trim().toUpperCase(Locale.ROOT);
            if (!code.matches("^[A-Z0-9_-]{1,32}$")) {
                return "error: \"invalid product_code format\"\n";
            }
            BigDecimal price = get_price_for_product_code(code);
            if (price == null) {
                return "error: \"unknown product_code\"\n";
            }
            if (rawQty == null || rawQty.isEmpty()) {
                return "error: \"missing quantity\"\n";
            }
            long qty;
            try {
                rawQty = rawQty.trim();
                if (rawQty.startsWith("+")) rawQty = rawQty.substring(1);
                qty = Long.parseLong(rawQty);
            } catch (NumberFormatException e) {
                return "error: \"quantity must be an integer\"\n";
            }
            if (qty <= 0) {
                return "error: \"quantity must be positive\"\n";
            }
            if (qty > 1_000_000L) {
                return "error: \"quantity too large\"\n";
            }
            BigDecimal total = price.multiply(BigDecimal.valueOf(qty)).setScale(2, RoundingMode.HALF_UP);
            return "total_price: \"" + total.toPlainString() + "\"\n";
        } catch (Exception ex) {
            return "error: \"" + escapeYaml(ex.getMessage() == null ? "internal error" : ex.getMessage()) + "\"\n";
        }
    }

    private static Map<String, String> parseSimpleYaml(String s) {
        Map<String, String> map = new HashMap<>();
        String[] lines = s.split("\\r?\\n");
        for (String line : lines) {
            String t = line.trim();
            if (t.isEmpty() || t.startsWith("#")) continue;
            int idx = t.indexOf(':');
            if (idx <= 0) continue;
            String key = t.substring(0, idx).trim().toLowerCase(Locale.ROOT);
            String value = t.substring(idx + 1).trim();
            value = stripQuotes(value);
            map.put(key, value);
        }
        return map;
    }

    private static String stripQuotes(String v) {
        if (v == null) return null;
        if ((v.startsWith("\"") && v.endsWith("\"")) || (v.startsWith("'") && v.endsWith("'"))) {
            if (v.length() >= 2) return v.substring(1, v.length() - 1);
        }
        return v;
    }

    private static String escapeYaml(String s) {
        if (s == null) return "";
        String r = s.replace("\\", "\\\\").replace("\"", "\\\"");
        r = r.replace("\r", "\\r").replace("\n", "\\n");
        return r;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "product_code: A100\nquantity: 2",
            "product_code: \"B200\"\nquantity: 3",
            "product_code: Z999\nquantity: 1",
            "product_code: C300\nquantity: -5",
            "product_code: D400\nquantity: two"
        };
        for (int i = 0; i < tests.length; i++) {
            String out = calculate_price(tests[i]);
            System.out.println(out);
        }
    }
}