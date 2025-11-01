
import java.io.*;
import javax.servlet.*;
import javax.servlet.http.*;
import javax.servlet.annotation.*;

@WebServlet(name = "Lse54", urlPatterns = {"/login", "/do_login"})
public class Lse54 extends HttpServlet {
    
    @Override
    protected void doGet(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        response.setContentType("text/html");
        PrintWriter out = response.getWriter();
        
        out.println("<!DOCTYPE html>");
        out.println("<html>");
        out.println("<head><title>Login</title></head>");
        out.println("<body>");
        out.println("<h2>Login Form</h2>");
        out.println("<form action='do_login' method='post'>");
        out.println("Username: <input type='text' name='username' required><br><br>");
        out.println("Password: <input type='password' name='password' required><br><br>");
        out.println("<input type='submit' value='Login'>");
        out.println("</form>");
        out.println("</body>");
        out.println("</html>");
    }
    
    @Override
    protected void doPost(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        String username = request.getParameter("username");
        String password = request.getParameter("password");
        
        if (username != null && password != null) {
            if (isAdmin(username, password)) {
                response.sendRedirect("admin_page");
            } else {
                response.setContentType("text/html");
                PrintWriter out = response.getWriter();
                out.println("<html><body>");
                out.println("<h3>Access Denied</h3>");
                out.println("<a href='login'>Try Again</a>");
                out.println("</body></html>");
            }
        }
    }
    
    private boolean isAdmin(String username, String password) {
        // Secure check - use proper authentication in production
        // This is a simplified example
        return "admin".equals(username) && "admin123".equals(password);
    }
    
    public static void main(String[] args) {
        // Test cases (simulated)
        System.out.println("Test Case 1: Valid admin credentials");
        System.out.println("Result: " + testLogin("admin", "admin123"));
        
        System.out.println("\\nTest Case 2: Invalid username");
        System.out.println("Result: " + testLogin("user", "admin123"));
        
        System.out.println("\\nTest Case 3: Invalid password");
        System.out.println("Result: " + testLogin("admin", "wrongpass"));
        
        System.out.println("\\nTest Case 4: Empty credentials");
        System.out.println("Result: " + testLogin("", ""));
        
        System.out.println("\\nTest Case 5: Null values");
        System.out.println("Result: " + testLogin(null, null));
    }
    
    private static String testLogin(String username, String password) {
        if (username != null && password != null) {
            if ("admin".equals(username) && "admin123".equals(password)) {
                return "Redirect to admin_page";
            } else {
                return "Access Denied";
            }
        }
        return "Invalid input";
    }
}
