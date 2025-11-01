import java.util.*;

public class Lse142 {
    private final Set<String> database;

    public Lse142(Collection<String> initialEmails) {
        this.database = new HashSet<>(initialEmails);
    }

    public String unsubscribe(String email) {
        if (database.contains(email)) {
            database.remove(email);
            return "Email removed from database.";
        } else {
            return "Email not found in database.";
        }
    }

    public static void main(String[] args) {
        Lse142 service = new Lse142(Arrays.asList(
                "alice@example.com",
                "bob@example.com",
                "carol@example.com"
        ));

        String[] tests = {
                "alice@example.com",
                "dave@example.com",
                "bob@example.com",
                "alice@example.com",
                "carol@example.com"
        };

        for (String email : tests) {
            System.out.println(email + " -> " + service.unsubscribe(email));
        }
    }
}