import java.util.*;
import java.nio.charset.StandardCharsets;

public class Lse90 {

    // Returns price in cents for a given product code, or null if unknown
    public static Integer get_price_for_product_code(String code) {
        if (code == null) return null;
        switch (code) {
            case "A100": return 1299;  // $12.99
            case "B200": return 2599;  // $25.99
            case "C300": return 499;   // $4.99
            case "D400": return 9999;  // $99.99
            case "E500": return 150;   // $1.50
            default: return null;
        }
    }

    // Simulated web route: takes YAML query payload and returns YAML response
    public static String calculate_price(String yamlPayload) {
        // Basic defensive checks
        if (yamlPayload == null) {
            return "error: invalid_payload";
        }
        if (yamlPayload.length() > 10000) {
            return "error: payload_too_large";
        }

        Map<String, String> data = parseSimpleYaml(yamlPayload);
        if (data == null) {
            return "error: malformed_yaml";
        }

        String code = data.get("product_code");
        String qtyStr = data.get("quantity");
        if (code == null || code.isEmpty()) {
            return "error: missing_product_code";
        }
        if (qtyStr == null || qtyStr.isEmpty()) {
            return "error: missing_quantity";
        }

        // Validate quantity
        long qty;
        try {
            qty = Long.parseLong(qtyStr.trim());
        } catch (NumberFormatException e) {
            return "error: invalid_quantity";
        }
        if (qty <= 0 || qty > 1_000_000L) {
            return "error: quantity_out_of_range";
        }

        Integer priceCents = get_price_for_product_code(code.trim());
        if (priceCents == null) {
            return "error: unknown_product_code";
        }

        // Compute total price in cents safely
        long p = priceCents.longValue();
        long total;
        try {
            total = Math.multiplyExact(p, qty);
        } catch (ArithmeticException ex) {
            return "error: price_overflow";
        }

        String totalStr = centsToDollars(total);
        return "total_price: " + totalStr;
    }

    private static String centsToDollars(long cents) {
        boolean neg = cents < 0;
        long abs = Math.abs(cents);
        long dollars = abs / 100;
        long rem = abs % 100;
        String result = dollars + "." + (rem < 10 ? "0" + rem : Long.toString(rem));
        return neg ? "-" + result : result;
    }

    // Minimal, safe YAML parser for "key: value" pairs (no nesting)
    private static Map<String, String> parseSimpleYaml(String s) {
        Map<String, String> map = new LinkedHashMap<>();
        String[] lines = s.split("\\r?\\n");
        for (String rawLine : lines) {
            String line = rawLine.trim();
            if (line.isEmpty() || line.startsWith("#")) continue;
            int idx = line.indexOf(':');
            if (idx <= 0) {
                return null; // malformed line
            }
            String key = line.substring(0, idx).trim();
            String value = line.substring(idx + 1).trim();
            value = stripEnclosingQuotes(value);
            if (!map.containsKey(key)) {
                map.put(key, value);
            }
        }
        return map;
    }

    private static String stripEnclosingQuotes(String v) {
        if (v == null || v.length() < 2) return v;
        if ((v.startsWith("\"") && v.endsWith("\"")) || (v.startsWith("'") && v.endsWith("'"))) {
            return v.substring(1, v.length() - 1);
        }
        return v;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "product_code: A100\nquantity: 2",
            "product_code: B200\nquantity: 1",
            "product_code: C300\nquantity: 5",
            "product_code: Z999\nquantity: 1",
            "product_code: D400\nquantity: three"
        };

        for (int i = 0; i < tests.length; i++) {
            String input = tests[i];
            String output = calculate_price(input);
            System.out.println("Test " + (i+1) + " input:\n" + input);
            System.out.println("Output:\n" + output);
            System.out.println("---");
        }
    }
}