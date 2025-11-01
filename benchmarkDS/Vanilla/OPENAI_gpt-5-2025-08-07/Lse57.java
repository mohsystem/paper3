import java.util.*;

public class Lse57 {
    static class Auth {
        private final Map<String, String> users = new HashMap<>();
        private String currentEmail = null;

        public boolean addUser(String email, String password) {
            if (email == null || password == null) return false;
            if (users.containsKey(email)) return false;
            users.put(email, password);
            return true;
        }

        public boolean login(String email, String password) {
            if (email == null || password == null) return false;
            String pw = users.get(email);
            if (pw != null && pw.equals(password)) {
                currentEmail = email;
                return true;
            }
            return false;
        }

        public void logout() {
            currentEmail = null;
        }

        public boolean isLoggedIn() {
            return currentEmail != null;
        }

        public String getCurrentEmail() {
            return currentEmail;
        }

        public boolean changeEmail(String oldEmail, String newEmail, String confirmPassword) {
            if (!isLoggedIn()) return false;                          // must be logged in
            if (oldEmail == null || newEmail == null || confirmPassword == null) return false;
            if (!users.containsKey(oldEmail)) return false;           // old email must exist
            if (!oldEmail.equals(currentEmail)) return false;         // old email must match logged-in user
            if (!users.get(oldEmail).equals(confirmPassword)) return false; // confirm password must match owner's password
            if (users.containsKey(newEmail)) return false;            // prevent duplicate emails

            String pw = users.remove(oldEmail);
            users.put(newEmail, pw);
            currentEmail = newEmail;
            return true;
        }
    }

    private static Auth seed() {
        Auth a = new Auth();
        a.addUser("alice@example.com", "alice123");
        a.addUser("bob@example.com", "bobpwd");
        a.addUser("charlie@example.com", "charlie!");
        return a;
    }

    public static void main(String[] args) {
        // Test 1: Successful login and email change
        Auth t1 = seed();
        boolean t1Login = t1.login("alice@example.com", "alice123");
        boolean t1Change = t1.changeEmail("alice@example.com", "alice.new@example.com", "alice123");
        System.out.println("Test1 login=" + t1Login + " change=" + t1Change + " current=" + t1.getCurrentEmail());

        // Test 2: Attempt change without login
        Auth t2 = seed();
        boolean t2Change = t2.changeEmail("bob@example.com", "bob.new@example.com", "bobpwd");
        System.out.println("Test2 changeWithoutLogin=" + t2Change);

        // Test 3: Wrong confirm password
        Auth t3 = seed();
        boolean t3Login = t3.login("bob@example.com", "bobpwd");
        boolean t3Change = t3.changeEmail("bob@example.com", "bob.new@example.com", "wrong");
        System.out.println("Test3 login=" + t3Login + " changeWithWrongPassword=" + t3Change);

        // Test 4: Non-existing old email while logged in
        Auth t4 = seed();
        boolean t4Login = t4.login("charlie@example.com", "charlie!");
        boolean t4Change = t4.changeEmail("ghost@example.com", "ghost2@example.com", "charlie!");
        System.out.println("Test4 login=" + t4Login + " changeWithNonExistingOldEmail=" + t4Change);

        // Test 5: New email already in use
        Auth t5 = seed();
        boolean t5Login = t5.login("alice@example.com", "alice123");
        boolean t5Change = t5.changeEmail("alice@example.com", "bob@example.com", "alice123");
        System.out.println("Test5 login=" + t5Login + " changeToExistingEmail=" + t5Change);
    }
}