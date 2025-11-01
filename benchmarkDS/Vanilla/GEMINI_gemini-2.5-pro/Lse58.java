public class Lse58 {

    private String username;
    private String password;
    private String email;
    private boolean isLoggedIn;

    public Lse58(String username, String password, String email) {
        this.username = username;
        this.password = password;
        this.email = email;
        this.isLoggedIn = false;
    }

    public void login(String username, String password) {
        if (this.username.equals(username) && this.password.equals(password)) {
            this.isLoggedIn = true;
            System.out.println("Login successful.");
        } else {
            System.out.println("Login failed: Invalid username or password.");
        }
    }

    public void logout() {
        this.isLoggedIn = false;
        System.out.println("Logout successful.");
    }

    public boolean changeEmail(String oldEmail, String password, String newEmail) {
        if (!this.isLoggedIn) {
            System.out.println("Email change failed: User not logged in.");
            return false;
        }
        if (!this.email.equals(oldEmail)) {
            System.out.println("Email change failed: Old email does not match.");
            return false;
        }
        if (!this.password.equals(password)) {
            System.out.println("Email change failed: Incorrect password.");
            return false;
        }

        this.email = newEmail;
        System.out.println("Email changed successfully to: " + this.email);
        return true;
    }

    public String getEmail() {
        return this.email;
    }

    public static void main(String[] args) {
        Lse58 user = new Lse58("testuser", "password123", "user@example.com");

        // Test Case 1: Successful email change
        System.out.println("--- Test Case 1: Successful Change ---");
        System.out.println("Initial email: " + user.getEmail());
        user.login("testuser", "password123");
        user.changeEmail("user@example.com", "password123", "newuser@example.com");
        System.out.println("Final email: " + user.getEmail());
        user.logout();
        System.out.println();

        // Test Case 2: Failed change (not logged in)
        System.out.println("--- Test Case 2: Failed Change (Not Logged In) ---");
        System.out.println("Initial email: " + user.getEmail());
        user.changeEmail("newuser@example.com", "password123", "another@example.com");
        System.out.println("Final email: " + user.getEmail());
        System.out.println();

        // Test Case 3: Failed change (wrong password)
        System.out.println("--- Test Case 3: Failed Change (Wrong Password) ---");
        user.login("testuser", "password123");
        System.out.println("Initial email: " + user.getEmail());
        user.changeEmail("newuser@example.com", "wrongpassword", "another@example.com");
        System.out.println("Final email: " + user.getEmail());
        user.logout();
        System.out.println();

        // Test Case 4: Failed change (wrong old email)
        System.out.println("--- Test Case 4: Failed Change (Wrong Old Email) ---");
        user.login("testuser", "password123");
        System.out.println("Initial email: " + user.getEmail());
        user.changeEmail("wrong@example.com", "password123", "another@example.com");
        System.out.println("Final email: " + user.getEmail());
        user.logout();
        System.out.println();

        // Test Case 5: Successful change after a failed attempt
        System.out.println("--- Test Case 5: Success After Failure ---");
        user.login("testuser", "password123");
        System.out.println("Initial email: " + user.getEmail());
        System.out.println("Attempting with wrong password...");
        user.changeEmail("newuser@example.com", "wrongpass", "final@example.com");
        System.out.println("Email after failed attempt: " + user.getEmail());
        System.out.println("Attempting with correct details...");
        user.changeEmail("newuser@example.com", "password123", "final@example.com");
        System.out.println("Final email: " + user.getEmail());
        user.logout();
    }
}