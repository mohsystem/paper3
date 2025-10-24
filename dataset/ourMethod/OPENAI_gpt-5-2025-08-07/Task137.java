import java.util.*;
import java.util.regex.Pattern;

public final class Task137 {
    private static final class Product {
        final int id;
        final String name;
        final double price;
        final int stock;

        Product(int id, String name, double price, int stock) {
            this.id = id;
            this.name = name;
            this.price = price;
            this.stock = stock;
        }
    }

    private static final class ProductDB {
        private final Map<Integer, Product> byId;
        private final Map<String, Product> byName;

        ProductDB(List<Product> products) {
            Map<Integer, Product> idMap = new HashMap<>();
            Map<String, Product> nameMap = new HashMap<>();
            for (Product p : products) {
                idMap.put(p.id, p);
                nameMap.put(p.name, p);
            }
            this.byId = Collections.unmodifiableMap(idMap);
            this.byName = Collections.unmodifiableMap(nameMap);
        }

        Product findById(int id) {
            return byId.get(id);
        }

        Product findByName(String name) {
            return byName.get(name);
        }
    }

    private static final Pattern DIGITS = Pattern.compile("^[0-9]{1,9}$");
    private static final Pattern NAME = Pattern.compile("^[A-Za-z0-9 _-]{1,50}$");

    public static String processQuery(String query) {
        // Validation: treat all inputs as untrusted
        if (query == null) {
            return errorJson("Query must not be null");
        }
        if (query.length() == 0 || query.length() > 100) {
            return errorJson("Query length must be between 1 and 100");
        }

        // Initialize in-memory product table
        ProductDB db = new ProductDB(Arrays.asList(
                new Product(1001, "Widget", 9.99, 100),
                new Product(1002, "Gadget", 12.49, 50),
                new Product(1003, "Thingamajig", 7.95, 0),
                new Product(2001, "Doodad", 5.00, 25),
                new Product(3001, "Sprocket", 15.75, 10)
        ));

        if (query.startsWith("id=")) {
            String idStr = query.substring(3);
            if (!DIGITS.matcher(idStr).matches()) {
                return errorJson("Invalid id format. Expected 1-9 digits.");
            }
            int id;
            try {
                id = Integer.parseInt(idStr);
            } catch (NumberFormatException ex) {
                return errorJson("Invalid id value");
            }
            if (id <= 0 || id > 1_000_000_000) {
                return errorJson("Id out of allowed range");
            }
            Product p = db.findById(id);
            if (p == null) {
                return errorJson("Product not found");
            }
            return okJson(p);
        } else if (query.startsWith("name=")) {
            String name = query.substring(5);
            if (!NAME.matcher(name).matches()) {
                return errorJson("Invalid name. Allowed: A-Z a-z 0-9 space - _ (1-50 chars)");
            }
            Product p = db.findByName(name);
            if (p == null) {
                return errorJson("Product not found");
            }
            return okJson(p);
        } else {
            return errorJson("Invalid query. Use id=<digits> or name=<allowed_name>");
        }
    }

    private static String escapeJson(String s) {
        StringBuilder sb = new StringBuilder(Math.min(256, s.length() + 16));
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            switch (c) {
                case '\\': sb.append("\\\\"); break;
                case '"': sb.append("\\\""); break;
                case '\b': sb.append("\\b"); break;
                case '\f': sb.append("\\f"); break;
                case '\n': sb.append("\\n"); break;
                case '\r': sb.append("\\r"); break;
                case '\t': sb.append("\\t"); break;
                default:
                    if (c < 0x20) {
                        sb.append(String.format("\\u%04x", (int) c));
                    } else {
                        sb.append(c);
                    }
            }
        }
        return sb.toString();
    }

    private static String okJson(Product p) {
        return "{\"ok\":true,\"product\":{\"id\":" + p.id +
                ",\"name\":\"" + escapeJson(p.name) +
                "\",\"price\":" + String.format(java.util.Locale.ROOT, "%.2f", p.price) +
                ",\"stock\":" + p.stock + "}}";
    }

    private static String errorJson(String msg) {
        return "{\"ok\":false,\"error\":\"" + escapeJson(msg) + "\"}";
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
                "id=1002",
                "name=Widget",
                "id=9999",
                "name=Invalid*Name!",
                "foo=bar"
        };
        for (String q : tests) {
            String result = processQuery(q);
            System.out.println("Query: " + q);
            System.out.println("Result: " + result);
            System.out.println("---");
        }
    }
}