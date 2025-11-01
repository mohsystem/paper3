import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.regex.Pattern;
import java.time.Instant;

public class Lse145 {

    // Simple in-memory "database" record
    public static final class Record {
        public final String stock;
        public final int quantity;
        public final long tsEpochSeconds;

        public Record(String stock, int quantity) {
            this.stock = stock;
            this.quantity = quantity;
            this.tsEpochSeconds = Instant.now().getEpochSecond();
        }

        @Override
        public String toString() {
            return "Record{stock='" + stock + "', quantity=" + quantity + ", ts=" + tsEpochSeconds + "}";
        }
    }

    // Simulated "database"
    private static final List<Record> DB = new ArrayList<>();

    // Allow uppercase letters, digits, dot and hyphen. 1..15 length.
    private static final Pattern STOCK_PATTERN = Pattern.compile("^[A-Z0-9.-]{1,15}$");

    // Normalizes and validates the stock name.
    public static String buy_function(String stockName) {
        Objects.requireNonNull(stockName, "stockName must not be null");
        String trimmed = stockName.trim();
        if (trimmed.isEmpty()) {
            throw new IllegalArgumentException("Stock name must not be empty");
        }
        String normalized = trimmed.toUpperCase();
        if (!STOCK_PATTERN.matcher(normalized).matches()) {
            throw new IllegalArgumentException("Invalid stock name format");
        }
        return normalized;
    }

    // Inserts the stock purchase into the "database" after validation.
    public static Record buy_stock(String stockName, int quantity) {
        String normalized = buy_function(stockName); // validate and normalize
        if (quantity <= 0 || quantity > 1_000_000) {
            throw new IllegalArgumentException("Quantity out of allowed range");
        }
        Record rec = new Record(normalized, quantity);
        synchronized (DB) {
            DB.add(rec);
        }
        return rec;
    }

    // Helper to print DB content securely
    private static void printDb() {
        synchronized (DB) {
            System.out.println("DB size: " + DB.size());
            for (Record r : DB) {
                System.out.println(r);
            }
        }
    }

    public static void main(String[] args) {
        // 5 test cases
        try {
            System.out.println("Test 1: Normal insert AAPL, 10");
            System.out.println(buy_stock("AAPL", 10));
        } catch (Exception e) {
            System.out.println("Test 1 failed: " + e.getMessage());
        }

        try {
            System.out.println("Test 2: Normalize msft, 25");
            System.out.println(buy_stock("  msft ", 25));
        } catch (Exception e) {
            System.out.println("Test 2 failed: " + e.getMessage());
        }

        try {
            System.out.println("Test 3: Invalid name with injection attempt");
            System.out.println(buy_stock("AAPL; DROP TABLES", 5));
        } catch (Exception e) {
            System.out.println("Test 3 expected failure: " + e.getMessage());
        }

        try {
            System.out.println("Test 4: Invalid quantity -5");
            System.out.println(buy_stock("GOOG", -5));
        } catch (Exception e) {
            System.out.println("Test 4 expected failure: " + e.getMessage());
        }

        try {
            System.out.println("Test 5: Edge symbol BRK.B, 100");
            System.out.println(buy_stock("BRK.B", 100));
        } catch (Exception e) {
            System.out.println("Test 5 failed: " + e.getMessage());
        }

        printDb();
    }
}