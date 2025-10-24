
// Java: FTP operations require external libraries like Apache Commons Net
// This is a basic example showing the structure
// Note: Add Apache Commons Net dependency to use this code
// Maven: <dependency>
//   <groupId>commons-net</groupId>
//   <artifactId>commons-net</artifactId>
//   <version>3.9.0</version>
// </dependency>

import org.apache.commons.net.ftp.FTP;
import org.apache.commons.net.ftp.FTPClient;
import org.apache.commons.net.ftp.FTPSClient;
import java.io.FileOutputStream;
import java.io.IOException;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;
import java.security.cert.X509Certificate;

public class Task77 {
    
    public static boolean downloadFile(String hostname, String username, String password, String remoteFile, String localFile) {
        FTPSClient ftpClient = new FTPSClient("TLS", false);
        
        try {
            // Create a trust manager that accepts all certificates (for demo only - not secure for production)
            TrustManager[] trustAllCerts = new TrustManager[]{
                new X509TrustManager() {
                    public X509Certificate[] getAcceptedIssuers() { return null; }
                    public void checkClientTrusted(X509Certificate[] certs, String authType) { }
                    public void checkServerTrusted(X509Certificate[] certs, String authType) { }
                }
            };
            
            SSLContext sc = SSLContext.getInstance("TLS");
            sc.init(null, trustAllCerts, new java.security.SecureRandom());
            ftpClient.setTrustManager(trustAllCerts[0]);
            
            // Connect to FTP server
            ftpClient.connect(hostname, 21);
            
            // Login
            boolean loginSuccess = ftpClient.login(username, password);
            if (!loginSuccess) {
                System.err.println("Login failed");
                return false;
            }
            
            // Enter passive mode
            ftpClient.enterLocalPassiveMode();
            
            // Set file type to binary
            ftpClient.setFileType(FTP.BINARY_FILE_TYPE);
            
            // Download file
            try (FileOutputStream fos = new FileOutputStream(localFile)) {
                boolean success = ftpClient.retrieveFile(remoteFile, fos);
                if (success) {
                    System.out.println("File downloaded successfully: " + localFile);
                } else {
                    System.err.println("File download failed");
                }
                return success;
            }
            
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
            e.printStackTrace();
            return false;
        } finally {
            try {
                if (ftpClient.isConnected()) {
                    ftpClient.logout();
                    ftpClient.disconnect();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
    
    public static void main(String[] args) {
        if (args.length < 4) {
            System.out.println("Usage: java Task77 <hostname> <username> <password> <remote_file> [local_file]");
            System.out.println("\\nRunning test cases with sample data...\\n");
            
            // Test cases (these will fail without actual FTP server)
            System.out.println("Test Case 1:");
            downloadFile("ftp.example.com", "testuser", "testpass", "/data/file1.txt", "downloaded_file1.txt");
            
            System.out.println("\\nTest Case 2:");
            downloadFile("ftp.test.com", "admin", "admin123", "/sensitive/report.pdf", "report.pdf");
            
            System.out.println("\\nTest Case 3:");
            downloadFile("192.168.1.100", "user1", "password1", "/docs/document.docx", "document.docx");
            
            System.out.println("\\nTest Case 4:");
            downloadFile("ftp.secure.com", "secureuser", "securepass", "/files/data.csv", "data.csv");
            
            System.out.println("\\nTest Case 5:");
            downloadFile("localhost", "localuser", "localpass", "/temp/test.txt", "test.txt");
            
            return;
        }
        
        String hostname = args[0];
        String username = args[1];
        String password = args[2];
        String remoteFile = args[3];
        String localFile = args.length > 4 ? args[4] : remoteFile.substring(remoteFile.lastIndexOf('/') + 1);
        
        downloadFile(hostname, username, password, remoteFile, localFile);
    }
}
