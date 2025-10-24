import java.util.*;
import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Task137 {

    public static final class Product {
        private final int id;
        private final String name;
        private final double price;
        private final String description;

        public Product(int id, String name, double price, String description) {
            this.id = id;
            this.name = Objects.requireNonNull(name, "name");
            this.price = price;
            this.description = Objects.requireNonNull(description, "description");
        }

        public int getId() { return id; }
        public String getName() { return name; }
        public double getPrice() { return price; }
        public String getDescription() { return description; }

        @Override
        public String toString() {
            return "Product{id=" + id + ", name='" + name + "', price=" + price + ", description='" + description + "'}";
        }
    }

    private static final Pattern SAFE_CHARS = Pattern.compile("^[A-Za-z0-9 _'\\-\\.]{1,100}$");

    // Securely validate and normalize a query string
    private static String normalizeQuery(String q) {
        if (q == null) {
            throw new IllegalArgumentException("Query cannot be null.");
        }
        String trimmed = q.trim();
        if (trimmed.isEmpty()) {
            throw new IllegalArgumentException("Query cannot be empty.");
        }
        if (trimmed.length() > 100) {
            throw new IllegalArgumentException("Query too long.");
        }
        Matcher m = SAFE_CHARS.matcher(trimmed);
        if (!m.matches()) {
            throw new IllegalArgumentException("Query contains disallowed characters.");
        }
        return trimmed.toLowerCase(Locale.ROOT);
    }

    // Securely validate an ID
    private static int normalizeId(Integer id) {
        if (id == null) {
            throw new IllegalArgumentException("ID cannot be null.");
        }
        if (id < 0 || id > 1_000_000_000) {
            throw new IllegalArgumentException("ID out of allowed range.");
        }
        return id;
    }

    // Function: query product details by ID
    public static Optional<Product> getProductDetailsById(Map<Integer, Product> productTable, Integer id) {
        Objects.requireNonNull(productTable, "productTable");
        int safeId = normalizeId(id);
        return Optional.ofNullable(productTable.get(safeId));
    }

    // Function: query product details by name (case-insensitive substring)
    public static List<Product> getProductDetailsByName(Map<Integer, Product> productTable, String nameQuery) {
        Objects.requireNonNull(productTable, "productTable");
        String q = normalizeQuery(nameQuery);
        List<Product> results = new ArrayList<>();
        for (Product p : productTable.values()) {
            String nameLower = p.getName().toLowerCase(Locale.ROOT);
            if (nameLower.contains(q)) {
                results.add(p);
            }
        }
        return results;
    }

    // Build sample "product table"
    private static Map<Integer, Product> buildSampleTable() {
        Map<Integer, Product> table = new HashMap<>();
        table.put(1, new Product(1, "Apple iPhone 14", 799.00, "Latest Apple smartphone"));
        table.put(2, new Product(2, "Samsung Galaxy S23", 749.99, "Flagship Android phone"));
        table.put(3, new Product(3, "Google Pixel 8", 699.00, "Google's premium device"));
        table.put(4, new Product(4, "Apple AirPods Pro", 249.00, "Noise-cancelling earbuds"));
        table.put(5, new Product(5, "Dell XPS 13", 1199.00, "Ultrabook laptop"));
        return Collections.unmodifiableMap(table);
    }

    public static void main(String[] args) {
        Map<Integer, Product> table = buildSampleTable();

        // Test case 1: by ID existing
        System.out.println("Test1 (ID=3): " + getProductDetailsById(table, 3).orElse(null));

        // Test case 2: by ID not existing
        System.out.println("Test2 (ID=999): " + getProductDetailsById(table, 999).orElse(null));

        // Test case 3: by name "apple"
        System.out.println("Test3 (name='apple'):");
        for (Product p : getProductDetailsByName(table, "apple")) {
            System.out.println("  " + p);
        }

        // Test case 4: by name "XPS"
        System.out.println("Test4 (name='XPS'):");
        for (Product p : getProductDetailsByName(table, "XPS")) {
            System.out.println("  " + p);
        }

        // Test case 5: invalid query input
        try {
            System.out.println("Test5 (invalid name):");
            System.out.println(getProductDetailsByName(table, "bad; DROP TABLE product;--"));
        } catch (IllegalArgumentException ex) {
            System.out.println("  Caught expected error: " + ex.getMessage());
        }
    }
}