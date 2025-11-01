import java.util.*;
import java.time.*;

public class Lse145 {
    static class Record {
        int id;
        String stock;
        int qty;
        long ts;
    }

    private static final List<Record> db = new ArrayList<>();
    private static int nextId = 1;

    public static String buy_function(String stockName) {
        if (stockName == null) return null;
        String s = stockName.trim();
        if (s.isEmpty()) return null;
        return s.toUpperCase();
    }

    public static int buy_stock(String stockName, int quantity) {
        String normalized = buy_function(stockName);
        if (normalized == null || quantity <= 0) return -1;
        Record r = new Record();
        r.id = nextId++;
        r.stock = normalized;
        r.qty = quantity;
        r.ts = System.currentTimeMillis();
        db.add(r);
        return r.id;
    }

    public static List<Record> getAllRecords() {
        return new ArrayList<>(db);
    }

    public static void main(String[] args) {
        int id1 = buy_stock("AAPL", 10);
        int id2 = buy_stock("goog", 5);
        int id3 = buy_stock("  msft  ", 20);
        int id4 = buy_stock("", 7);      // invalid
        int id5 = buy_stock("TSLA", -3); // invalid

        System.out.println("Inserted IDs: " + Arrays.asList(id1, id2, id3, id4, id5));
        System.out.println("Database contents:");
        for (Record r : getAllRecords()) {
            System.out.println("ID=" + r.id + " Stock=" + r.stock + " Qty=" + r.qty + " Ts=" + r.ts);
        }
    }
}