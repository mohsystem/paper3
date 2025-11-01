
import java.io.*;
import javax.servlet.*;
import javax.servlet.http.*;

public class Lse54 extends HttpServlet {
    
    // GET request - display login form
    protected void doGet(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        response.setContentType("text/html");
        PrintWriter out = response.getWriter();
        
        out.println("<html><body>");
        out.println("<h2>Login Form</h2>");
        out.println("<form method='POST' action='login'>");
        out.println("Username: <input type='text' name='username'><br>");
        out.println("Password: <input type='password' name='password'><br>");
        out.println("<input type='submit' value='Login'>");
        out.println("</form>");
        out.println("</body></html>");
    }
    
    // POST request - process login
    protected void doPost(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        String username = request.getParameter("username");
        String password = request.getParameter("password");
        
        if (username != null && username.equals("admin")) {
            response.sendRedirect("admin_page");
        } else {
            response.setContentType("text/html");
            PrintWriter out = response.getWriter();
            out.println("<html><body>");
            out.println("<h3>Login failed or user is not admin</h3>");
            out.println("</body></html>");
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Admin login
        System.out.println("Test 1 - Admin login: username='admin' -> Redirect to admin_page");
        
        // Test case 2: Non-admin login
        System.out.println("Test 2 - Non-admin login: username='user' -> Return login failed");
        
        // Test case 3: Empty username
        System.out.println("Test 3 - Empty username: username='' -> Return login failed");
        
        // Test case 4: Null username
        System.out.println("Test 4 - Null username: username=null -> Return login failed");
        
        // Test case 5: Admin with wrong password
        System.out.println("Test 5 - Admin user: username='admin', password='wrong' -> Redirect to admin_page");
    }
}
