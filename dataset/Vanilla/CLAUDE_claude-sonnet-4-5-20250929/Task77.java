
// Note: FTP operations in Java require external libraries like Apache Commons Net.
// This code demonstrates the structure but requires Apache Commons Net library to run.
// Add dependency: org.apache.commons:commons-net:3.9.0

import org.apache.commons.net.ftp.FTP;
import org.apache.commons.net.ftp.FTPClient;
import java.io.FileOutputStream;
import java.io.IOException;

public class Task77 {
    public static boolean downloadFile(String hostname, String username, String password, String remoteFilePath, String localFilePath) {
        FTPClient ftpClient = new FTPClient();
        try {
            // Connect to FTP server
            ftpClient.connect(hostname);
            
            // Login to FTP server
            boolean login = ftpClient.login(username, password);
            if (!login) {
                System.out.println("Login failed!");
                return false;
            }
            
            // Set file type to binary
            ftpClient.setFileType(FTP.BINARY_FILE_TYPE);
            
            // Download file
            try (FileOutputStream fos = new FileOutputStream(localFilePath)) {
                boolean success = ftpClient.retrieveFile(remoteFilePath, fos);
                if (success) {
                    System.out.println("File downloaded successfully: " + localFilePath);
                    return true;
                } else {
                    System.out.println("File download failed!");
                    return false;
                }
            }
        } catch (IOException ex) {
            System.out.println("Error: " + ex.getMessage());
            return false;
        } finally {
            try {
                if (ftpClient.isConnected()) {
                    ftpClient.logout();
                    ftpClient.disconnect();
                }
            } catch (IOException ex) {
                ex.printStackTrace();
            }
        }
    }
    
    public static void main(String[] args) {
        if (args.length < 4) {
            System.out.println("Usage: java Task77 <hostname> <username> <password> <remote_file_path>");
            System.out.println("\\nRunning test cases with dummy data...\\n");
            
            // Test cases with dummy data (these will fail without actual FTP server)
            System.out.println("Test Case 1:");
            downloadFile("ftp.example.com", "user1", "pass1", "/files/data.txt", "data1.txt");
            
            System.out.println("\\nTest Case 2:");
            downloadFile("ftp.test.com", "admin", "admin123", "/documents/report.pdf", "report2.pdf");
            
            System.out.println("\\nTest Case 3:");
            downloadFile("192.168.1.100", "ftpuser", "ftppass", "/backup/backup.zip", "backup3.zip");
            
            System.out.println("\\nTest Case 4:");
            downloadFile("ftp.server.org", "guest", "guest", "/public/readme.txt", "readme4.txt");
            
            System.out.println("\\nTest Case 5:");
            downloadFile("ftp.demo.net", "testuser", "test@123", "/files/config.xml", "config5.xml");
        } else {
            String hostname = args[0];
            String username = args[1];
            String password = args[2];
            String remoteFile = args[3];
            String localFile = remoteFile.substring(remoteFile.lastIndexOf("/") + 1);
            
            downloadFile(hostname, username, password, remoteFile, localFile);
        }
    }
}
