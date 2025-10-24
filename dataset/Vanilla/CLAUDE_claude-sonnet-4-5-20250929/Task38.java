
import java.io.*;
import javax.servlet.*;
import javax.servlet.http.*;
import javax.servlet.annotation.WebServlet;

@WebServlet("/Task38")
public class Task38 extends HttpServlet {
    protected void doGet(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        response.setContentType("text/html");
        PrintWriter out = response.getWriter();
        
        out.println("<!DOCTYPE html>");
        out.println("<html>");
        out.println("<head><title>User Input Form</title></head>");
        out.println("<body>");
        out.println("<h2>Enter Your Information</h2>");
        out.println("<form method='post' action='Task38'>");
        out.println("Name: <input type='text' name='name' required><br><br>");
        out.println("Email: <input type='email' name='email' required><br><br>");
        out.println("Message: <textarea name='message' rows='4' cols='50' required></textarea><br><br>");
        out.println("<input type='submit' value='Submit'>");
        out.println("</form>");
        out.println("</body>");
        out.println("</html>");
    }
    
    protected void doPost(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        response.setContentType("text/html");
        PrintWriter out = response.getWriter();
        
        String name = request.getParameter("name");
        String email = request.getParameter("email");
        String message = request.getParameter("message");
        
        out.println("<!DOCTYPE html>");
        out.println("<html>");
        out.println("<head><title>Display User Input</title></head>");
        out.println("<body>");
        out.println("<h2>Your Submitted Information</h2>");
        out.println("<p><strong>Name:</strong> " + escapeHtml(name) + "</p>");
        out.println("<p><strong>Email:</strong> " + escapeHtml(email) + "</p>");
        out.println("<p><strong>Message:</strong> " + escapeHtml(message) + "</p>");
        out.println("<br><a href='Task38'>Go Back</a>");
        out.println("</body>");
        out.println("</html>");
    }
    
    private String escapeHtml(String input) {
        if (input == null) return "";
        return input.replace("&", "&amp;")
                   .replace("<", "&lt;")
                   .replace(">", "&gt;")
                   .replace("\\"", "&quot;")
                   .replace("'", "&#x27;");
    }
    
    public static void main(String[] args) {
        System.out.println("This is a web application that requires a servlet container like Tomcat to run.");
        System.out.println("Test cases would be executed through HTTP requests in a web browser.");
    }
}
