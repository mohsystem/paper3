import java.util.*;

public class Lse58 {
    static class User {
        String username;
        String email;
        String password;
        boolean loggedIn;

        User(String username, String email, String password) {
            this.username = username;
            this.email = email;
            this.password = password;
            this.loggedIn = false;
        }
    }

    // Logs in the user if password matches; returns true if logged in.
    public static boolean login(User user, String password) {
        if (user == null) return false;
        if (user.password.equals(password)) {
            user.loggedIn = true;
            return true;
        }
        return false;
    }

    // Changes email if user is logged in, old email matches, and password is correct. Returns true if changed.
    public static boolean changeEmail(User user, String oldEmail, String password, String newEmail) {
        if (user == null) return false;
        if (!user.loggedIn) return false;
        if (!Objects.equals(user.email, oldEmail)) return false;
        if (!Objects.equals(user.password, password)) return false;
        user.email = newEmail;
        return true;
    }

    // Optional helper to log out
    public static void logout(User user) {
        if (user != null) user.loggedIn = false;
    }

    public static void main(String[] args) {
        // Test 1: Attempt change without login -> should fail
        User u1 = new User("bob", "bob@example.com", "bobpass");
        boolean t1 = changeEmail(u1, "bob@example.com", "bobpass", "newbob@example.com");
        System.out.println("Test1 (not logged in, expect false): " + t1 + " | email=" + u1.email);

        // Test 2: Correct login and correct old email/password -> should succeed
        User u2 = new User("alice", "alice@example.com", "pass123");
        boolean login2 = login(u2, "pass123");
        boolean t2 = changeEmail(u2, "alice@example.com", "pass123", "alice_new@example.com");
        System.out.println("Test2 (logged in, correct creds, expect true): " + (login2 && t2) + " | email=" + u2.email);

        // Test 3: Logged in but wrong old email -> should fail
        User u3 = new User("carol", "carol@example.com", "cpass");
        login(u3, "cpass");
        boolean t3 = changeEmail(u3, "wrong@example.com", "cpass", "carol_new@example.com");
        System.out.println("Test3 (wrong old email, expect false): " + t3 + " | email=" + u3.email);

        // Test 4: Logged in, correct old email, wrong password -> should fail
        User u4 = new User("dan", "dan@example.com", "dpass");
        login(u4, "dpass");
        boolean t4 = changeEmail(u4, "dan@example.com", "wrongpass", "dan_new@example.com");
        System.out.println("Test4 (wrong password, expect false): " + t4 + " | email=" + u4.email);

        // Test 5: After successful change, try again using original old email -> should fail
        User u5 = new User("eve", "eve@example.com", "epass");
        login(u5, "epass");
        boolean firstChange = changeEmail(u5, "eve@example.com", "epass", "eve_new@example.com"); // succeed
        boolean secondChange = changeEmail(u5, "eve@example.com", "epass", "eve_new2@example.com"); // fail (old email no longer matches)
        System.out.println("Test5 (first true then second false): first=" + firstChange + ", second=" + secondChange + " | email=" + u5.email);
    }
}