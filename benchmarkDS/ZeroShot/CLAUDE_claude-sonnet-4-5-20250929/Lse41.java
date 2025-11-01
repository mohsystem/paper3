
import java.io.*;
import javax.servlet.*;
import javax.servlet.http.*;
import javax.servlet.annotation.WebServlet;

@WebServlet("/info")
public class Lse41 extends HttpServlet {
    // Mock database - In reality, never store SSN like this!
    private static class User {
        String username;
        String email;
        String ssn; // INSECURE: Should never store full SSN
        
        User(String username, String email, String ssn) {
            this.username = username;
            this.email = email;
            this.ssn = ssn;
        }
    }
    
    // Mock user data
    private User getCurrentUser() {
        // INSECURE: Hardcoded sensitive data for demonstration
        return new User("john_doe", "john@example.com", "123456789");
    }
    
    protected void doGet(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        response.setContentType("text/html");
        PrintWriter out = response.getWriter();
        
        User user = getCurrentUser();
        String lastFour = user.ssn.substring(user.ssn.length() - 4);
        
        out.println("<html><body>");
        out.println("<h1>User Information</h1>");
        out.println("<p>Username: " + user.username + "</p>");
        out.println("<p>Email: " + user.email + "</p>");
        out.println("<p>SSN (Last 4): ***-**-" + lastFour + "</p>");
        out.println("</body></html>");
    }
    
    // Test method
    public static void main(String[] args) {
        Lse41 app = new Lse41();
        
        System.out.println("Test Cases:");
        System.out.println("1. User: john_doe");
        User user1 = new User("john_doe", "john@example.com", "123456789");
        System.out.println("   Last 4 SSN: " + user1.ssn.substring(user1.ssn.length() - 4));
        
        System.out.println("2. User: jane_smith");
        User user2 = new User("jane_smith", "jane@example.com", "987654321");
        System.out.println("   Last 4 SSN: " + user2.ssn.substring(user2.ssn.length() - 4));
        
        System.out.println("3. User: bob_jones");
        User user3 = new User("bob_jones", "bob@example.com", "555443333");
        System.out.println("   Last 4 SSN: " + user3.ssn.substring(user3.ssn.length() - 4));
        
        System.out.println("4. User: alice_wonder");
        User user4 = new User("alice_wonder", "alice@example.com", "111223333");
        System.out.println("   Last 4 SSN: " + user4.ssn.substring(user4.ssn.length() - 4));
        
        System.out.println("5. User: charlie_brown");
        User user5 = new User("charlie_brown", "charlie@example.com", "999887777");
        System.out.println("   Last 4 SSN: " + user5.ssn.substring(user5.ssn.length() - 4));
    }
}
