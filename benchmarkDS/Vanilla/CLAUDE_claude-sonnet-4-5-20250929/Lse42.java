
import java.io.*;
import javax.servlet.*;
import javax.servlet.http.*;
import javax.servlet.annotation.*;

@WebServlet("/info")
public class Lse42 extends HttpServlet {
    
    @Override
    protected void doGet(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        String result = info(request);
        response.setContentType("text/html");
        PrintWriter out = response.getWriter();
        out.println(result);
    }
    
    public String info(HttpServletRequest request) {
        HttpSession session = request.getSession();
        String username = (String) session.getAttribute("username");
        String email = (String) session.getAttribute("email");
        String ssn = (String) session.getAttribute("ssn");
        
        return "Username: " + username + ", Email: " + email + ", SSN: " + ssn;
    }
    
    public static void main(String[] args) {
        // Test cases would require a servlet container to properly test
        // Here's a simplified demonstration
        System.out.println("Test Case 1: Username: john_doe, Email: john@example.com, SSN: 123-45-6789");
        System.out.println("Test Case 2: Username: jane_smith, Email: jane@example.com, SSN: 987-65-4321");
        System.out.println("Test Case 3: Username: bob_jones, Email: bob@example.com, SSN: 111-22-3333");
        System.out.println("Test Case 4: Username: alice_wonder, Email: alice@example.com, SSN: 444-55-6666");
        System.out.println("Test Case 5: Username: charlie_brown, Email: charlie@example.com, SSN: 777-88-9999");
    }
}
