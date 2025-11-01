class Lse58 {

    private String username;
    private String password;
    private String email;
    private boolean isLoggedIn;

    public Lse58(String username, String password, String email) {
        this.username = username;
        this.password = password; // In a real app, this should be a hashed password
        this.email = email;
        this.isLoggedIn = false;
    }

    public boolean login(String username, String password) {
        if (this.username.equals(username) && this.password.equals(password)) {
            this.isLoggedIn = true;
            System.out.println("Login successful for user: " + this.username);
            return true;
        }
        System.out.println("Login failed for user: " + username);
        return false;
    }

    public void logout() {
        this.isLoggedIn = false;
        System.out.println(this.username + " has been logged out.");
    }

    public boolean changeEmail(String oldEmail, String confirmPassword, String newEmail) {
        if (!this.isLoggedIn) {
            System.out.println("Error: You must be logged in to change your email.");
            return false;
        }

        if (this.email.equals(oldEmail) && this.password.equals(confirmPassword)) {
            this.email = newEmail;
            System.out.println("Success: Email has been changed to " + newEmail);
            return true;
        } else {
            System.out.println("Error: Incorrect old email or password. Email not changed.");
            return false;
        }
    }

    public String getEmail() {
        return this.email;
    }

    public static void main(String[] args) {
        Lse58 user = new Lse58("testuser", "password123", "user@example.com");

        System.out.println("--- Test Case 1: Successful Email Change ---");
        user.login("testuser", "password123");
        user.changeEmail("user@example.com", "password123", "newuser@example.com");
        System.out.println("Current email: " + user.getEmail());
        user.logout();
        System.out.println();

        // Reset user for next tests
        user = new Lse58("testuser", "password123", "user@example.com");

        System.out.println("--- Test Case 2: Failed Change (Not Logged In) ---");
        user.changeEmail("user@example.com", "password123", "another@example.com");
        System.out.println("Current email: " + user.getEmail());
        System.out.println();

        System.out.println("--- Test Case 3: Failed Change (Wrong Password) ---");
        user.login("testuser", "password123");
        user.changeEmail("user@example.com", "wrongpassword", "another@example.com");
        System.out.println("Current email: " + user.getEmail());
        user.logout();
        System.out.println();
        
        // Reset user
        user = new Lse58("testuser", "password123", "user@example.com");

        System.out.println("--- Test Case 4: Failed Change (Wrong Old Email) ---");
        user.login("testuser", "password123");
        user.changeEmail("wrong@example.com", "password123", "another@example.com");
        System.out.println("Current email: " + user.getEmail());
        System.out.println();
        
        System.out.println("--- Test Case 5: Failed Change (Logged out then attempt) ---");
        user.logout(); // Ensuring user is logged out
        user.changeEmail("user@example.com", "password123", "final@example.com");
        System.out.println("Current email: " + user.getEmail());
    }
}