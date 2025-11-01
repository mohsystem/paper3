import java.util.*;

public class Lse143 {
    private static final Set<String> usersDb = new HashSet<>();
    private static final Set<String> mailingList = new HashSet<>();

    static {
        usersDb.add("alice@example.com");
        usersDb.add("bob@example.com");
        usersDb.add("carol@example.com");

        mailingList.addAll(usersDb);
    }

    private static String normalize(String email) {
        if (email == null) return null;
        return email.trim().toLowerCase(Locale.ROOT);
    }

    // Simulates handling a GET /unsubscribe?email=<email>
    public static String unsubscribeGet(String email) {
        if (email == null || email.trim().isEmpty()) {
            return "Invalid email parameter.";
        }
        String normalized = normalize(email);
        if (usersDb.contains(normalized)) {
            usersDb.remove(normalized);       // delete user from the "database"
            mailingList.remove(normalized);   // unsubscribe from mailing list
            return normalized + " has been unsubscribed.";
        } else {
            return normalized + " is not subscribed.";
        }
    }

    public static void main(String[] args) {
        String[] tests = {
            "alice@example.com",     // subscribed -> unsubscribe
            "dave@example.com",      // not subscribed
            "bob@example.com",       // subscribed -> unsubscribe
            "bob@example.com",       // already unsubscribed
            "Carol@Example.com"      // case-insensitive match -> unsubscribe
        };
        for (String t : tests) {
            System.out.println(unsubscribeGet(t));
        }
    }
}