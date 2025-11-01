import java.util.*;
import java.nio.charset.StandardCharsets;

public class Lse88 {
    private static final int MAX_INPUT_LEN = 4096;
    private static final int MAX_KEY_VAL_LEN = 64;
    private static final Map<String, Long> PRICE_CENTS = createCatalog();

    private static Map<String, Long> createCatalog() {
        Map<String, Long> m = new HashMap<>();
        m.put("P001", 999L);   // 9.99
        m.put("P002", 550L);   // 5.50
        m.put("P003", 2000L);  // 20.00
        return Collections.unmodifiableMap(m);
    }

    public static String calculate_price(String yamlPayload) {
        try {
            if (yamlPayload == null) {
                return "error: invalid input";
            }
            // Validate encoding and size
            byte[] bytes = yamlPayload.getBytes(StandardCharsets.UTF_8);
            if (bytes.length == 0 || bytes.length > MAX_INPUT_LEN) {
                return "error: invalid input";
            }
            if (yamlPayload.indexOf('\0') >= 0) {
                return "error: invalid input";
            }

            String[] lines = yamlPayload.replace("\r\n", "\n").replace('\r', '\n').split("\n", -1);
            String productCode = null;
            String quantityStr = null;

            for (String rawLine : lines) {
                String line = rawLine.trim();
                if (line.isEmpty() || line.startsWith("#")) {
                    continue;
                }
                int idx = line.indexOf(':');
                if (idx < 0) {
                    continue;
                }
                String k = line.substring(0, idx).trim();
                String v = line.substring(idx + 1).trim();
                if (k.length() == 0 || k.length() > MAX_KEY_VAL_LEN || v.length() > MAX_KEY_VAL_LEN) {
                    return "error: invalid input";
                }
                v = stripQuotes(v);

                if ("product_code".equals(k)) {
                    productCode = v;
                } else if ("quantity".equals(k)) {
                    quantityStr = v;
                }
            }

            if (productCode == null || quantityStr == null) {
                return "error: missing product_code or quantity";
            }

            if (!productCode.matches("[A-Za-z0-9_-]{1,32}")) {
                return "error: invalid product_code";
            }

            int quantity;
            try {
                if (!quantityStr.matches("[+-]?\\d{1,9}")) {
                    return "error: invalid quantity";
                }
                quantity = Integer.parseInt(quantityStr);
            } catch (NumberFormatException e) {
                return "error: invalid quantity";
            }
            if (quantity <= 0 || quantity > 1_000_000) {
                return "error: invalid quantity";
            }

            Long priceCents = PRICE_CENTS.get(productCode);
            if (priceCents == null) {
                return "error: unknown product_code";
            }

            if (quantity > Long.MAX_VALUE / priceCents) {
                return "error: overflow";
            }
            long totalCents = priceCents * (long) quantity;
            return formatCents(totalCents);
        } catch (Exception ex) {
            return "error: processing failed";
        }
    }

    private static String stripQuotes(String v) {
        if (v.length() >= 2) {
            char first = v.charAt(0);
            char last = v.charAt(v.length() - 1);
            if ((first == '\'' && last == '\'') || (first == '"' && last == '"')) {
                return v.substring(1, v.length() - 1);
            }
        }
        return v;
    }

    private static String formatCents(long cents) {
        long dollars = cents / 100;
        long rem = Math.abs(cents % 100);
        return dollars + "." + (rem < 10 ? "0" : "") + rem;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "product_code: P001\nquantity: 3\n",
            "quantity: 10\nproduct_code: P002\n",
            "product_code: UNKNOWN\nquantity: 1\n",
            "product_code: P001\nquantity: -5\n",
            "product_code: P003\n"
        };
        for (int i = 0; i < tests.length; i++) {
            String result = calculate_price(tests[i]);
            System.out.println("Test " + (i + 1) + ": " + result);
        }
    }
}