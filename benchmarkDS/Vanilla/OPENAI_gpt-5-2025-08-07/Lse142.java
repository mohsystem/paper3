import java.util.*;

public class Lse142 {
    private final Set<String> database;

    public Lse142(Set<String> initialEmails) {
        this.database = new HashSet<>(initialEmails);
    }

    private Set<String> connectToDatabase() {
        // Simulate a DB connection by returning the in-memory set.
        return database;
    }

    // Unsubscribe function: accepts email as parameter, returns a message.
    public String unsubscribe(String email) {
        Set<String> db = connectToDatabase();
        if (db.contains(email)) {
            db.remove(email);
            return "Unsubscribed: " + email;
        } else {
            return "Email not found: " + email;
        }
    }

    public static void main(String[] args) {
        Set<String> initial = new HashSet<>(Arrays.asList(
                "alice@example.com",
                "bob@example.com",
                "carol@example.com"
        ));
        Lse142 service = new Lse142(initial);

        String[] tests = new String[] {
                "alice@example.com",   // present -> removed
                "dave@example.com",    // not present
                "carol@example.com",   // present -> removed
                "eve@example.com",     // not present
                "bob@example.com"      // present -> removed
        };

        for (String email : tests) {
            System.out.println(service.unsubscribe(email));
        }

        System.out.println("Remaining in database: " + service.database);
    }
}