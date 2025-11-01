import java.io.BufferedWriter;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.time.Instant;
import java.time.ZoneOffset;
import java.time.format.DateTimeFormatter;
import java.util.EnumSet;
import java.util.Objects;
import java.util.Set;
import java.util.regex.Pattern;

public class Lse146 {

    // Secure validation patterns
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9_]{3,32}$");
    private static final Pattern STOCK_PATTERN = Pattern.compile("^[A-Za-z0-9.-]{1,16}$");

    private static final Path DB_PATH = Paths.get("orders_db.csv");
    private static final String HEADER = "timestamp,username,stock,quantity";

    public static void main(String[] args) {
        // 5 test cases
        test("alice_01", "AAPL", 10);
        test("bob", "GOOG", 100);
        test("charlie", "msft", 5000);
        test("bad name", "TSLA", 10);  // invalid username
        test("dave", "BRK.B", 0);      // invalid quantity
    }

    private static void test(String username, String stock, int qty) {
        String result = processBuyOrder(username, stock, qty);
        System.out.println("Input: user=" + username + ", stock=" + stock + ", qty=" + qty);
        System.out.println("Result: " + (result != null ? result : "ERROR"));
        System.out.println("----");
    }

    // Main function as per requirement
    public static String processBuyOrder(String username, String stock_name, int stock_quantity) {
        try {
            // Validate inputs
            if (!isValidUsername(username) || !isValidStock(stock_name) || !isValidQuantity(stock_quantity)) {
                return null;
            }

            String stockNormalized = stock_name.toUpperCase();

            // Insert into "database" (CSV file with secure handling)
            if (!appendOrder(username, stockNormalized, stock_quantity)) {
                return null;
            }

            // Redirect to profile page
            return "/users/" + username + "/profile";
        } catch (Exception e) {
            return null;
        }
    }

    private static boolean isValidUsername(String username) {
        if (username == null) return false;
        return USERNAME_PATTERN.matcher(username).matches();
    }

    private static boolean isValidStock(String stock) {
        if (stock == null) return false;
        return STOCK_PATTERN.matcher(stock).matches();
    }

    private static boolean isValidQuantity(int q) {
        return q >= 1 && q <= 1_000_000;
    }

    private static void ensureDbWithHeader() throws IOException {
        if (Files.notExists(DB_PATH)) {
            // Try to create with restrictive permissions on POSIX
            try {
                Set<PosixFilePermission> perms = EnumSet.of(
                        PosixFilePermission.OWNER_READ,
                        PosixFilePermission.OWNER_WRITE
                );
                Files.createFile(DB_PATH, PosixFilePermissions.asFileAttribute(perms));
            } catch (UnsupportedOperationException e) {
                // Non-POSIX systems
                Files.createFile(DB_PATH);
            }
            try (BufferedWriter w = Files.newBufferedWriter(DB_PATH, StandardCharsets.UTF_8, StandardOpenOption.APPEND)) {
                w.write(HEADER);
                w.newLine();
            }
            return;
        }
        // If exists but empty, write header
        if (Files.size(DB_PATH) == 0L) {
            try (BufferedWriter w = Files.newBufferedWriter(DB_PATH, StandardCharsets.UTF_8, StandardOpenOption.APPEND)) {
                w.write(HEADER);
                w.newLine();
            }
        }
    }

    private static synchronized boolean appendOrder(String username, String stock, int qty) {
        Objects.requireNonNull(username);
        Objects.requireNonNull(stock);
        try {
            ensureDbWithHeader();
            String timestamp = DateTimeFormatter.ISO_INSTANT.format(Instant.now().atOffset(ZoneOffset.UTC));
            String line = timestamp + "," + username + "," + stock + "," + qty;
            // Since inputs are tightly validated, no commas/newlines are present
            try (BufferedWriter w = Files.newBufferedWriter(DB_PATH, StandardCharsets.UTF_8,
                    StandardOpenOption.APPEND)) {
                w.write(line);
                w.newLine();
            }
            return true;
        } catch (IOException e) {
            return false;
        }
    }
}