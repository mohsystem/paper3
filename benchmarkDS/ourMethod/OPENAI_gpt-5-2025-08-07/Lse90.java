import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;

public class Lse90 {

    // Price catalog
    private static final Map<String, Double> PRICE_MAP = new HashMap<>();
    static {
        PRICE_MAP.put("A1", 9.99);
        PRICE_MAP.put("B2", 5.00);
        PRICE_MAP.put("C3", 2.50);
        PRICE_MAP.put("D4", 100.00);
        PRICE_MAP.put("E5", 0.75);
    }

    public static double get_price_for_product_code(String code) {
        if (code == null) return -1.0;
        Double price = PRICE_MAP.get(code);
        return price != null ? price : -1.0;
    }

    public static String calculate_price(String yamlPayload) {
        // Fail closed on null or excessive size
        if (yamlPayload == null) {
            return "error: invalid_request";
        }
        if (yamlPayload.length() > 4096) {
            return "error: invalid_request";
        }
        // Validate ASCII printable (allow newline/carriage return and space)
        for (int i = 0; i < yamlPayload.length(); i++) {
            char c = yamlPayload.charAt(i);
            if (!(c == '\n' || c == '\r' || c == '\t' || (c >= 32 && c <= 126))) {
                return "error: invalid_request";
            }
        }

        Map<String, String> map = parseSimpleYamlToMap(yamlPayload);
        if (map == null) {
            return "error: invalid_request";
        }

        String productCode = map.get("product_code");
        String quantityStr = map.get("quantity");
        if (productCode == null || quantityStr == null) {
            return "error: invalid_request";
        }

        if (!isValidProductCode(productCode)) {
            return "error: invalid_request";
        }

        Long qty = parsePositiveInt(quantityStr, 1, 1_000_000);
        if (qty == null) {
            return "error: invalid_request";
        }

        double price = get_price_for_product_code(productCode);
        if (price < 0.0) {
            return "error: unknown_product_code";
        }

        double total = price * qty;
        return String.format(Locale.ROOT, "total_price: %.2f", total);
    }

    private static boolean isValidProductCode(String s) {
        if (s == null || s.length() < 1 || s.length() > 20) return false;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            boolean ok = (c >= 'A' && c <= 'Z') ||
                         (c >= 'a' && c <= 'z') ||
                         (c >= '0' && c <= '9') ||
                         c == '_' || c == '-';
            if (!ok) return false;
        }
        return true;
    }

    private static String stripQuotes(String v) {
        String s = v.trim();
        if ((s.startsWith("'") && s.endsWith("'") && s.length() >= 2) ||
            (s.startsWith("\"") && s.endsWith("\"") && s.length() >= 2)) {
            return s.substring(1, s.length() - 1);
        }
        return s;
    }

    private static Map<String, String> parseSimpleYamlToMap(String yaml) {
        String[] lines = yaml.split("\\r?\\n", -1);
        if (lines.length > 200) {
            return null;
        }
        Map<String, String> out = new HashMap<>();
        for (String rawLine : lines) {
            String line = rawLine.trim();
            if (line.isEmpty()) continue;
            if (line.startsWith("#")) continue;
            int idx = line.indexOf(':');
            if (idx <= 0) continue;
            String key = line.substring(0, idx).trim();
            String val = line.substring(idx + 1).trim();
            val = stripQuotes(val);
            if ("product_code".equals(key) || "quantity".equals(key)) {
                out.put(key, val);
            }
        }
        return out;
    }

    private static Long parsePositiveInt(String s, long min, long max) {
        String t = s.trim();
        if (t.isEmpty()) return null;
        // No sign allowed
        for (int i = 0; i < t.length(); i++) {
            char c = t.charAt(i);
            if (c < '0' || c > '9') return null;
        }
        try {
            long v = Long.parseLong(t);
            if (v < min || v > max) return null;
            return v;
        } catch (NumberFormatException ex) {
            return null;
        }
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "product_code: A1\nquantity: 3\n",
            "product_code: B2\nquantity: 10\n",
            "product_code: Z9\nquantity: 1\n",
            "product_code: C3\nquantity: -2\n",
            " product_code : 'E5' \n quantity : 4 \n"
        };
        for (int i = 0; i < tests.length; i++) {
            String result = calculate_price(tests[i]);
            System.out.println("Test " + (i + 1) + " -> " + result);
        }
    }
}