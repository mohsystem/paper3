import org.apache.commons.net.ftp.FTP;
import org.apache.commons.net.ftp.FTPClient;
import org.apache.commons.net.ftp.FTPReply;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

// NOTE: This code requires the Apache Commons Net library.
// You can add it as a dependency in your build tool (e.g., Maven, Gradle).
// Maven dependency:
// <dependency>
//     <groupId>commons-net</groupId>
//     <artifactId>commons-net</artifactId>
//     <version>3.9.0</version>
// </dependency>

public class Task77 {

    /**
     * Connects to an FTP server and downloads a file.
     *
     * @param host       The FTP server hostname or IP address.
     * @param username   The username for authentication.
     * @param password   The password for authentication.
     * @param remoteFile The full path of the file to download from the server.
     * @return true if the file was downloaded successfully, false otherwise.
     */
    public static boolean downloadFile(String host, String username, String password, String remoteFile) {
        FTPClient ftpClient = new FTPClient();
        // WARNING: Standard FTP sends credentials and data in plaintext.
        // For production systems, always use FTPS (FTP over SSL/TLS) or SFTP.
        try {
            ftpClient.connect(host);
            int reply = ftpClient.getReplyCode();
            if (!FTPReply.isPositiveCompletion(reply)) {
                ftpClient.disconnect();
                System.err.println("FTP server refused connection.");
                return false;
            }

            if (!ftpClient.login(username, password)) {
                System.err.println("Login failed for user " + username);
                ftpClient.logout();
                return false;
            }

            System.out.println("Logged in successfully to " + host);

            ftpClient.setFileType(FTP.BINARY_FILE_TYPE);
            ftpClient.enterLocalPassiveMode();

            // Security: Prevent path traversal attacks.
            // Use only the basename of the remote file for the local filename.
            String localFileName = new File(remoteFile).getName();
            if (localFileName.isEmpty()) {
                 System.err.println("Could not determine a local filename from remote path: " + remoteFile);
                 return false;
            }

            try (FileOutputStream fos = new FileOutputStream(localFileName)) {
                System.out.println("Downloading file " + remoteFile + " to " + localFileName + "...");
                boolean success = ftpClient.retrieveFile(remoteFile, fos);
                if (success) {
                    System.out.println("File downloaded successfully.");
                    return true;
                } else {
                    System.err.println("Failed to download file. Server reply: " + ftpClient.getReplyString());
                    // Clean up the potentially empty local file if download failed
                    new File(localFileName).delete();
                    return false;
                }
            }
        } catch (IOException e) {
            System.err.println("Error: " + e.getMessage());
            return false;
        } finally {
            try {
                if (ftpClient.isConnected()) {
                    ftpClient.logout();
                    ftpClient.disconnect();
                }
            } catch (IOException ex) {
                // Silently ignore errors on disconnect
            }
        }
    }

    public static void main(String[] args) {
        // WARNING: Passing credentials as command-line arguments is insecure.
        // They can be viewed by other users on the system via the process list.
        // This is for demonstration purposes only.
        if (args.length == 4) {
            String host = args[0];
            String user = args[1];
            String pass = args[2];
            String remoteFile = args[3];
            downloadFile(host, user, pass, remoteFile);
        } else {
            System.out.println("Running test cases. Please replace placeholder values if needed.");
            System.out.println("Usage: java Task77 <host> <username> <password> <remoteFilePath>");
            
            // Test cases use a public test FTP server.
            // Note: The remote files do not exist, so download will fail,
            // but connection and login should succeed, demonstrating the process.
            
            // Test Case 1: Example with valid-looking data
            System.out.println("\n--- Test Case 1 ---");
            downloadFile("ftp.dlptest.com", "dlpuser", "rNrKYTX9g7z3RgJR", "some-file.txt");

            // Test Case 2: Example with a file in a subdirectory
            System.out.println("\n--- Test Case 2 ---");
            downloadFile("ftp.dlptest.com", "dlpuser", "rNrKYTX9g7z3RgJR", "/folder/some-other-file.zip");

            // Test Case 3: Invalid Host (will fail to connect)
            System.out.println("\n--- Test Case 3 ---");
            downloadFile("invalid.ftp.host", "user", "pass", "file.txt");

            // Test Case 4: Invalid Credentials (will fail to login)
            System.out.println("\n--- Test Case 4 ---");
            downloadFile("ftp.dlptest.com", "wronguser", "wrongpass", "file.txt");

            // Test Case 5: File Not Found (will fail to download)
            System.out.println("\n--- Test Case 5 ---");
            downloadFile("ftp.dlptest.com", "dlpuser", "rNrKYTX9g7z3RgJR", "non-existent-file.dat");
        }
    }
}