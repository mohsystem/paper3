// Step 1: Problem understanding
// - Implement calculate_price as the entry point function.
// - Accept a YAML payload (string) containing product_code and quantity.
// - Parse payload safely without external YAML libraries.
// - Validate inputs and compute total price using a predefined product price list.
// - Return total price as a string with two decimal places or an error string.
//
// Step 2: Security requirements
// - Avoid executing or evaluating untrusted input.
// - Limit payload size and sanitize parsing logic.
// - Validate product_code format and quantity as a safe integer within bounds.
// - Avoid floating-point inaccuracies by using BigDecimal.
// - Never reflect raw payload back in error messages.
//
// Step 3: Secure coding generation
// - Implement safe parsing and validation.
// - Use BigDecimal for precise money calculations.
// - Enforce strict input constraints and handle all error cases gracefully.
//
// Step 4: Code review
// - Verify bounds, null handling, and numeric conversions.
// - Ensure map lookups are safe and case-handled.
// - Ensure no external dependencies or unsafe operations.
//
// Step 5: Secure code output
// - Final function returns price or "ERROR: ..." message.
// - Include 5 test cases in main method.

import java.math.BigDecimal;
import java.math.RoundingMode;
import java.util.*;

public class Lse88 {

    private static final int MAX_PAYLOAD_LENGTH = 4096;

    private static final Map<String, BigDecimal> PRICE_MAP;
    static {
        Map<String, BigDecimal> m = new HashMap<>();
        m.put("P001", new BigDecimal("19.99"));
        m.put("P002", new BigDecimal("5.50"));
        m.put("P003", new BigDecimal("99.95"));
        m.put("P100", new BigDecimal("1.25"));
        m.put("P900", new BigDecimal("250.00"));
        PRICE_MAP = Collections.unmodifiableMap(m);
    }

    public static String calculate_price(String yamlPayload) {
        // Validate basic constraints
        if (yamlPayload == null) {
            return "ERROR: Missing payload";
        }
        if (yamlPayload.length() > MAX_PAYLOAD_LENGTH) {
            return "ERROR: Payload too large";
        }

        Map<String, String> map = parseSimpleYaml(yamlPayload);
        String codeRaw = map.get("product_code");
        String qtyRaw = map.get("quantity");

        if (codeRaw == null || codeRaw.trim().isEmpty()) {
            return "ERROR: Missing product_code";
        }
        if (qtyRaw == null || qtyRaw.trim().isEmpty()) {
            return "ERROR: Missing quantity";
        }

        String code = codeRaw.trim().toUpperCase(Locale.ROOT);
        if (!code.matches("^[A-Z0-9_-]{1,20}$")) {
            return "ERROR: Invalid product_code format";
        }

        // quantity validation
        String qtyStr = qtyRaw.trim();
        if (isQuoted(qtyStr)) {
            qtyStr = stripQuotes(qtyStr);
        }
        if (!qtyStr.matches("^[0-9]{1,9}$")) {
            return "ERROR: Invalid quantity";
        }

        int quantity;
        try {
            quantity = Integer.parseInt(qtyStr);
        } catch (NumberFormatException e) {
            return "ERROR: Quantity out of range";
        }
        if (quantity <= 0 || quantity > 1_000_000) {
            return "ERROR: Quantity out of allowed bounds";
        }

        BigDecimal unitPrice = PRICE_MAP.get(code);
        if (unitPrice == null) {
            return "ERROR: Unknown product_code";
        }

        BigDecimal total = unitPrice.multiply(new BigDecimal(quantity)).setScale(2, RoundingMode.HALF_UP);
        return total.toPlainString();
    }

    private static Map<String, String> parseSimpleYaml(String yaml) {
        Map<String, String> out = new HashMap<>();
        String normalized = yaml.replace("\r\n", "\n").replace("\r", "\n");
        String[] lines = normalized.split("\n");
        for (String rawLine : lines) {
            String line = rawLine.trim();
            if (line.isEmpty() || line.startsWith("#")) continue;
            int idx = line.indexOf(':');
            if (idx <= 0) continue;
            String key = line.substring(0, idx).trim().toLowerCase(Locale.ROOT);
            String value = line.substring(idx + 1).trim();
            if (isQuoted(value)) {
                value = stripQuotes(value);
            }
            if (!key.isEmpty()) {
                out.put(key, value);
            }
        }
        return out;
    }

    private static boolean isQuoted(String s) {
        return (s.length() >= 2) &&
               ((s.startsWith("\"") && s.endsWith("\"")) || (s.startsWith("'") && s.endsWith("'")));
    }

    private static String stripQuotes(String s) {
        if (isQuoted(s)) {
            return s.substring(1, s.length() - 1);
        }
        return s;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "product_code: P001\nquantity: 3",
            "product_code: P003\nquantity: 2",
            "product_code: 'P100'\nquantity: '10'",
            "product_code: X999\nquantity: 1",
            "product_code: P002\nquantity: five"
        };
        for (int i = 0; i < tests.length; i++) {
            String result = calculate_price(tests[i]);
            System.out.println("Test " + (i+1) + ": " + result);
        }
    }
}