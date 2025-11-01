import java.util.*;
import java.util.regex.Pattern;
import java.nio.charset.StandardCharsets;

public class Lse145 {

    // Simple in-memory database
    static final class Database {
        private static final long MAX_RECORDS = 1_000_000L;
        private final List<Purchase> purchases = new ArrayList<>();
        private long nextId = 1;

        public synchronized long insert(String stockName, int quantity) {
            if (purchases.size() >= MAX_RECORDS) {
                throw new IllegalStateException("Capacity reached");
            }
            Purchase p = new Purchase(nextId, stockName, quantity);
            purchases.add(p);
            return nextId++;
        }

        public synchronized List<Purchase> getAll() {
            return new ArrayList<>(purchases);
        }
    }

    static final class Purchase {
        final long id;
        final String stockName;
        final int quantity;

        Purchase(long id, String stockName, int quantity) {
            this.id = id;
            this.stockName = stockName;
            this.quantity = quantity;
        }
    }

    // Allowed ticker set simulating a market
    private static final Set<String> ALLOWED_TICKERS = Collections.unmodifiableSet(new HashSet<>(Arrays.asList(
            "AAPL", "GOOG", "MSFT", "TSLA", "AMZN"
    )));

    // Validation pattern: 1-15 chars, starts with A-Z, then A-Z, 0-9, '.', '-'
    private static final Pattern TICKER_PATTERN = Pattern.compile("^[A-Z][A-Z0-9.\\-]{0,14}$");

    // buy_function: checks if the stock exists/allowed
    public static boolean buy_function(String stockName) {
        return ALLOWED_TICKERS.contains(stockName);
    }

    // Sanitizes and validates stock name: trims, normalizes to uppercase, validates format
    private static Optional<String> sanitizeAndValidateStockName(String input) {
        if (input == null) return Optional.empty();
        String trimmed = new String(input.trim().getBytes(StandardCharsets.UTF_8), StandardCharsets.UTF_8);
        if (trimmed.isEmpty()) return Optional.empty();
        String upper = trimmed.toUpperCase(Locale.ROOT);
        if (upper.length() > 15) return Optional.empty();
        if (!TICKER_PATTERN.matcher(upper).matches()) return Optional.empty();
        return Optional.of(upper);
    }

    // Input validation for quantity
    private static boolean isValidQuantity(int q) {
        return q >= 1 && q <= 1_000_000;
    }

    // buy_stock: takes stock name and quantity, validates, calls buy_function, inserts to DB
    public static String buy_stock(Database db, String stockNameInput, int quantity) {
        if (db == null) {
            return "ERROR: Internal database unavailable";
        }
        if (!isValidQuantity(quantity)) {
            return "ERROR: Invalid quantity";
        }
        Optional<String> sanitizedOpt = sanitizeAndValidateStockName(stockNameInput);
        if (sanitizedOpt.isEmpty()) {
            return "ERROR: Invalid stock name";
        }
        String sanitized = sanitizedOpt.get();

        if (!buy_function(sanitized)) {
            return "ERROR: Stock not available";
        }

        try {
            long id = db.insert(sanitized, quantity);
            return "SUCCESS: id=" + id + " stock=" + sanitized + " qty=" + quantity;
        } catch (IllegalStateException ex) {
            return "ERROR: Database capacity reached";
        } catch (RuntimeException ex) {
            return "ERROR: Internal error";
        }
    }

    // 5 test cases
    public static void main(String[] args) {
        Database db = new Database();
        String[] stocks = { "AAPL", " msft ", "AAPL;DELETE", "GOOG", "IBM" };
        int[] qtys =    {      10,        5,              1,       0,      1 };

        for (int i = 0; i < stocks.length; i++) {
            String result = buy_stock(db, stocks[i], qtys[i]);
            System.out.println("Test " + (i + 1) + ": " + result);
        }

        // Show inserted purchases
        for (Purchase p : db.getAll()) {
            System.out.println("Record: id=" + p.id + " stock=" + p.stockName + " qty=" + p.quantity);
        }
    }
}