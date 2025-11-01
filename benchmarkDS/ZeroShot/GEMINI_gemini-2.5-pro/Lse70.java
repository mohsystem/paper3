// To compile and run this code, you need the Java Servlet API.
// 1. Compile the code with the servlet-api.jar in your classpath.
//    Example: javac -cp /path/to/servlet-api.jar Lse70.java
// 2. Package it into a .war file.
//    Example: jar -cvf Lse70.war Lse70.class
// 3. Deploy the Lse70.war file to a servlet container like Apache Tomcat or Jetty.
// 4. The upload endpoint will be accessible at http://localhost:8080/Lse70/upload (context path may vary)

import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;
import java.nio.file.Paths;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;
import java.util.UUID;
import javax.servlet.ServletException;
import javax.servlet.annotation.MultipartConfig;
import javax.servlet.annotation.WebServlet;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.Part;

@WebServlet("/upload")
@MultipartConfig(
    fileSizeThreshold = 1024 * 1024,      // 1 MB
    maxFileSize = 1024 * 1024 * 10,       // 10 MB
    maxRequestSize = 1024 * 1024 * 15     // 15 MB
)
public class Lse70 extends HttpServlet {

    private static final String UPLOAD_DIR = "uploads";
    private static final Set<String> ALLOWED_EXTENSIONS = new HashSet<>(Arrays.asList("txt", "jpg", "jpeg", "png", "pdf"));

    @Override
    protected void doPost(HttpServletRequest request, HttpServletResponse response)
            throws ServletException, IOException {
        response.setContentType("text/plain");
        PrintWriter out = response.getWriter();

        String applicationPath = request.getServletContext().getRealPath("");
        String uploadFilePath = applicationPath + File.separator + UPLOAD_DIR;

        File uploadDir = new File(uploadFilePath);
        if (!uploadDir.exists()) {
            if (!uploadDir.mkdirs()) {
                 response.setStatus(HttpServletResponse.SC_INTERNAL_SERVER_ERROR);
                 out.println("Failed to create upload directory.");
                 return;
            }
        }

        try {
            Part filePart = request.getPart("file");
            if (filePart == null || filePart.getSize() == 0) {
                response.setStatus(HttpServletResponse.SC_BAD_REQUEST);
                out.println("No file uploaded or file is empty.");
                return;
            }

            // Sanitize for path traversal by getting only the filename
            String submittedFileName = Paths.get(filePart.getSubmittedFileName()).getFileName().toString(); 
            if (submittedFileName.trim().isEmpty()) {
                response.setStatus(HttpServletResponse.SC_BAD_REQUEST);
                out.println("Invalid file name.");
                return;
            }

            String extension = getFileExtension(submittedFileName);
            if (extension == null || !ALLOWED_EXTENSIONS.contains(extension.toLowerCase())) {
                response.setStatus(HttpServletResponse.SC_BAD_REQUEST);
                out.println("File type not allowed. Allowed types are: " + ALLOWED_EXTENSIONS);
                return;
            }

            // Generate a unique filename to prevent overwrites and other attacks
            String uniqueFileName = UUID.randomUUID().toString() + "." + extension;
            String filePath = uploadFilePath + File.separator + uniqueFileName;

            filePart.write(filePath);

            response.setStatus(HttpServletResponse.SC_OK);
            out.println("File uploaded successfully as " + uniqueFileName);

        } catch (IOException | ServletException e) {
            // Log the exception in a real application
            response.setStatus(HttpServletResponse.SC_INTERNAL_SERVER_ERROR);
            out.println("There was an error during file upload: " + e.getMessage());
        }
    }

    private String getFileExtension(String fileName) {
        if (fileName != null && fileName.lastIndexOf(".") != -1 && fileName.lastIndexOf(".") != 0) {
            return fileName.substring(fileName.lastIndexOf(".") + 1);
        }
        return null;
    }
    
    public static void main(String[] args) {
        System.out.println("This is a Servlet-based web application for file uploads.");
        System.out.println("To run it, you must deploy it in a Servlet Container like Apache Tomcat.");
        System.out.println("Once deployed, you can use the following curl commands for testing:");
        System.out.println("The server is assumed to be running on http://localhost:8080/Lse70 (context path might vary).");
        System.out.println("\n--- 5 Test Cases (using curl) ---");

        System.out.println("\n1. Test Case: Successful upload of an allowed file type (.txt)");
        System.out.println("echo \"This is a test file.\" > test.txt");
        System.out.println("curl -X POST -F 'file=@test.txt' http://localhost:8080/Lse70/upload");

        System.out.println("\n2. Test Case: Attempted upload of a disallowed file type (.exe)");
        System.out.println("echo \"dummy exe\" > test.exe");
        System.out.println("curl -X POST -F 'file=@test.exe' http://localhost:8080/Lse70/upload");

        System.out.println("\n3. Test Case: Attempted upload of a file that is too large (e.g., > 10MB)");
        System.out.println("fallocate -l 11M largefile.dat");
        System.out.println("curl -X POST -F 'file=@largefile.dat' http://localhost:8080/Lse70/upload");

        System.out.println("\n4. Test Case: Attempted upload with a malicious filename (path traversal)");
        System.out.println("echo \"malicious content\" > evil.txt");
        System.out.println("curl -X POST -F 'file=@evil.txt;filename=\"../../evil.txt\"' http://localhost:8080/Lse70/upload");

        System.out.println("\n5. Test Case: Successful upload of another allowed file type (.jpg)");
        System.out.println("# (Assuming you have a file named 'image.jpg' in the current directory)");
        System.out.println("curl -X POST -F 'file=@image.jpg' http://localhost:8080/Lse70/upload");
    }
}