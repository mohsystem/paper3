
import java.io.*;
import javax.servlet.*;
import javax.servlet.http.*;
import javax.servlet.annotation.*;

@WebServlet("/info")
public class Lse40 extends HttpServlet {
    
    @Override
    protected void doGet(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        
        HttpSession session = request.getSession();
        
        String username = (String) session.getAttribute("username");
        String email = (String) session.getAttribute("email");
        String ssn = (String) session.getAttribute("ssn");
        
        String last4SSN = "";
        if (ssn != null && ssn.length() >= 4) {
            last4SSN = ssn.substring(ssn.length() - 4);
        }
        
        response.setContentType("text/html");
        PrintWriter out = response.getWriter();
        
        out.println("<html><body>");
        out.println("<h1>User Information</h1>");
        out.println("<p>Username: " + (username != null ? username : "N/A") + "</p>");
        out.println("<p>Email: " + (email != null ? email : "N/A") + "</p>");
        out.println("<p>Last 4 digits of SSN: " + (last4SSN.isEmpty() ? "N/A" : last4SSN) + "</p>");
        out.println("</body></html>");
    }
    
    public static void main(String[] args) {
        System.out.println("This is a servlet application. Deploy to a servlet container like Tomcat.");
        System.out.println("Test cases would be executed through HTTP requests to /info endpoint.");
        System.out.println("Example: Set session variables and access http://localhost:8080/info");
    }
}
