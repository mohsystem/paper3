import java.util.*;
import java.util.stream.*;
import java.text.DecimalFormat;

public class Task137 {

    static class Product {
        int id;
        String name;
        String category;
        double price;
        int stock;

        Product(int id, String name, String category, double price, int stock) {
            this.id = id;
            this.name = name;
            this.category = category;
            this.price = price;
            this.stock = stock;
        }
    }

    private static final List<Product> TABLE = Arrays.asList(
        new Product(1001, "Widget", "Tools", 19.99, 120),
        new Product(1002, "Gadget", "Electronics", 99.50, 55),
        new Product(1003, "Gizmo", "Electronics", 49.00, 200),
        new Product(1004, "Pro Headphones", "Audio", 129.99, 35),
        new Product(1005, "Office Chair", "Furniture", 199.99, 15),
        new Product(1006, "Pro Keyboard", "Computers", 89.99, 60)
    );

    private static String escapeJson(String s) {
        StringBuilder sb = new StringBuilder();
        for (char c : s.toCharArray()) {
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
                        sb.append(String.format("\\u%04x", (int)c));
                    } else {
                        sb.append(c);
                    }
            }
        }
        return sb.toString();
    }

    private static String productsToJson(List<Product> products) {
        StringBuilder sb = new StringBuilder();
        DecimalFormat df = new DecimalFormat("#0.00");
        sb.append("[");
        for (int i = 0; i < products.size(); i++) {
            Product p = products.get(i);
            sb.append("{");
            sb.append("\"id\":").append(p.id).append(",");
            sb.append("\"name\":\"").append(escapeJson(p.name)).append("\",");
            sb.append("\"category\":\"").append(escapeJson(p.category)).append("\",");
            sb.append("\"price\":").append(df.format(p.price)).append(",");
            sb.append("\"stock\":").append(p.stock);
            sb.append("}");
            if (i < products.size() - 1) sb.append(",");
        }
        sb.append("]");
        return sb.toString();
    }

    private static String trim(String s) {
        return s == null ? "" : s.trim();
    }

    public static String processInput(String input) {
        String in = trim(input);
        if (in.isEmpty()) return "[]";
        String lower = in.toLowerCase(Locale.ROOT);

        List<Product> result = new ArrayList<>();

        if (lower.equals("all")) {
            result = TABLE;
        } else if (in.contains("=")) {
            int idx = in.indexOf('=');
            String key = trim(in.substring(0, idx)).toLowerCase(Locale.ROOT);
            String value = trim(in.substring(idx + 1));
            String valueLower = value.toLowerCase(Locale.ROOT);

            switch (key) {
                case "id": {
                    try {
                        int id = Integer.parseInt(value);
                        for (Product p : TABLE) {
                            if (p.id == id) { result.add(p); break; }
                        }
                    } catch (NumberFormatException e) {
                        // no result
                    }
                    break;
                }
                case "name": {
                    for (Product p : TABLE) {
                        if (p.name.equalsIgnoreCase(value)) result.add(p);
                    }
                    break;
                }
                case "category": {
                    for (Product p : TABLE) {
                        if (p.category.equalsIgnoreCase(value)) result.add(p);
                    }
                    break;
                }
                case "search": {
                    for (Product p : TABLE) {
                        if (p.name.toLowerCase(Locale.ROOT).contains(valueLower)) result.add(p);
                    }
                    break;
                }
                default:
                    // invalid key -> empty result
                    break;
            }
        } else {
            // unsupported format -> empty result
        }

        return productsToJson(result);
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "id=1002",
            "name=Gizmo",
            "category=Audio",
            "search=Pro",
            "all"
        };
        for (String t : tests) {
            System.out.println("Query: " + t);
            System.out.println(processInput(t));
        }
    }
}