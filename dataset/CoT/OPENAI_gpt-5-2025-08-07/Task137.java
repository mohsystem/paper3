import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.regex.Pattern;
import java.util.regex.Matcher;
import java.text.DecimalFormat;

/*
 Steps applied:
 1) Problem understanding
 2) Security requirements
 3) Secure coding generation
 4) Code review
 5) Secure code output
*/

public class Task137 {

    // Product entity
    public static final class Product {
        private final int id;
        private final String name;
        private final double price;
        private final int stock;

        public Product(int id, String name, double price, int stock) {
            this.id = id;
            this.name = name;
            this.price = price;
            this.stock = stock;
        }

        public int getId() { return id; }
        public String getName() { return name; }
        public double getPrice() { return price; }
        public int getStock() { return stock; }
    }

    // Repository simulating a product table
    public static final class ProductRepository {
        private final Map<Integer, Product> byId = new ConcurrentHashMap<>();
        private final Map<String, Product> byNameNormalized = new ConcurrentHashMap<>();
        private static final Pattern NAME_ALLOWED = Pattern.compile("[A-Za-z0-9 _\\-]+");

        public ProductRepository(Collection<Product> products) {
            for (Product p : products) {
                byId.put(p.getId(), p);
                byNameNormalized.put(normalizeName(p.getName()), p);
            }
        }

        private static String normalizeName(String input) {
            if (input == null) return null;
            String trimmed = input.trim();
            // Collapse multiple spaces
            String collapsed = trimmed.replaceAll("\\s+", " ");
            // Validate allowed characters
            if (!NAME_ALLOWED.matcher(collapsed).matches()) return null;
            return collapsed.toLowerCase(Locale.ROOT);
        }

        public Product findByIdSecure(int id) {
            if (id < 0 || id > 1_000_000_000) return null;
            return byId.get(id);
        }

        public Product findByNameSecure(String inputName) {
            String normalized = normalizeName(inputName);
            if (normalized == null) return null;
            return byNameNormalized.get(normalized);
        }
    }

    // Service handling user input
    public static final class ProductService {
        private final ProductRepository repo;
        private static final Pattern ID_PATTERN = Pattern.compile("^\\s*id\\s*:\\s*(\\d{1,9})\\s*$", Pattern.CASE_INSENSITIVE);
        private static final Pattern NAME_PATTERN = Pattern.compile("^\\s*name\\s*:\\s*(.+)\\s*$", Pattern.CASE_INSENSITIVE);
        private static final DecimalFormat PRICE_FMT = new DecimalFormat("#0.00");

        public ProductService(ProductRepository repo) {
            this.repo = repo;
        }

        public String processUserInput(String input) {
            if (input == null) return error("Invalid input");
            if (input.length() > 1024) return error("Input too long");

            Matcher mId = ID_PATTERN.matcher(input);
            if (mId.matches()) {
                try {
                    int id = Integer.parseInt(mId.group(1));
                    Product p = repo.findByIdSecure(id);
                    if (p == null) return notFound();
                    return toJson(p);
                } catch (NumberFormatException ex) {
                    return error("Invalid ID");
                }
            }

            Matcher mName = NAME_PATTERN.matcher(input);
            if (mName.matches()) {
                String nameVal = mName.group(1);
                Product p = repo.findByNameSecure(nameVal);
                if (p == null) {
                    // Could be invalid characters or not found
                    // Distinguish only generically to avoid information leaks
                    return notFound();
                }
                return toJson(p);
            }

            return error("Unsupported query. Use 'id:<digits>' or 'name:<text>'");
        }

        private String toJson(Product p) {
            return "{\"id\":" + p.getId() +
                   ",\"name\":\"" + escapeJson(p.getName()) + "\"" +
                   ",\"price\":" + PRICE_FMT.format(p.getPrice()) +
                   ",\"stock\":" + p.getStock() + "}";
        }

        private String escapeJson(String s) {
            StringBuilder sb = new StringBuilder(s.length() + 16);
            for (int i = 0; i < s.length(); i++) {
                char c = s.charAt(i);
                switch (c) {
                    case '\"': sb.append("\\\""); break;
                    case '\\': sb.append("\\\\"); break;
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

        private String error(String msg) {
            return "{\"error\":\"" + escapeJson(msg) + "\"}";
        }

        private String notFound() {
            return "{\"message\":\"Product not found\"}";
        }
    }

    // Build repository with sample data
    public static ProductRepository buildRepository() {
        List<Product> products = Arrays.asList(
            new Product(1001, "Wireless Mouse", 25.99, 42),
            new Product(1002, "Mechanical Keyboard", 79.49, 12),
            new Product(1003, "USB-C Cable", 9.99, 150),
            new Product(1004, "27-inch Monitor", 199.99, 8),
            new Product(1005, "Webcam Pro", 49.95, 5)
        );
        return new ProductRepository(products);
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        ProductRepository repo = buildRepository();
        ProductService service = new ProductService(repo);

        String[] tests = new String[] {
            "id:1001",
            "name:Wireless Mouse",
            "name:   usb-c   cable ",
            "id:9999",
            "name:DROP TABLE products;--"
        };

        for (String t : tests) {
            String result = service.processUserInput(t);
            System.out.println("Query: " + t);
            System.out.println("Result: " + result);
        }
    }
}