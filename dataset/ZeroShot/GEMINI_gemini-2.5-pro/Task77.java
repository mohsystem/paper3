import org.apache.commons.net.ftp.FTP;
import org.apache.commons.net.ftp.FTPReply;
import org.apache.commons.net.ftp.FTPSClient;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.file.InvalidPathException;
import java.nio.file.Paths;

// Note: This code requires the Apache Commons Net library.
// You can add it to your project, e.g., via Maven/Gradle.
// <dependency>
//     <groupId>commons-net</groupId>
//     <artifactId>commons-net</artifactId>
//     <version>3.9.0</version>
// </dependency>
public class Task77 {

    /**
     * Connects to an FTP server using FTPS (explicit TLS) and downloads a file.
     *
     * @param host       The FTP server hostname or IP address.
     * @param username   The username for authentication.
     * @param password   The password for authentication.
     * @param remoteFile The full path to the file to download on the server.
     * @param localFile  The name of the file to save locally. Path traversal is prevented.
     * @return true if the download was successful, false otherwise.
     */
    public static boolean downloadFileSecureFTP(String host, String username, String password, String remoteFile, String localFile) {
        // Security: Prevent path traversal attacks on the local file system.
        try {
            // Check if the filename is valid and doesn't contain path elements.
            String baseName = Paths.get(localFile).getFileName().toString();
            if (!baseName.equals(localFile)) {
                System.err.println("Error: Local filename cannot contain path separators.");
                return false;
            }
        } catch (InvalidPathException e) {
            System.err.println("Error: Invalid local filename: " + e.getMessage());
            return false;
        }

        FTPSClient ftps = new FTPSClient();
        try {
            ftps.connect(host);
            int reply = ftps.getReplyCode();
            if (!FTPReply.isPositiveCompletion(reply)) {
                ftps.disconnect();
                System.err.println("FTP server refused connection.");
                return false;
            }

            if (!ftps.login(username, password)) {
                System.err.println("Login failed.");
                ftps.logout();
                return false;
            }
            System.out.println("Login successful.");

            // Set protection level to private, encrypting the data channel.
            ftps.execPROT("P");

            ftps.enterLocalPassiveMode();
            ftps.setFileType(FTP.BINARY_FILE_TYPE);

            try (OutputStream outputStream = new FileOutputStream(localFile)) {
                System.out.println("Starting download of " + remoteFile + " to " + localFile);
                boolean success = ftps.retrieveFile(remoteFile, outputStream);
                if (success) {
                    System.out.println("File has been downloaded successfully.");
                } else {
                    System.err.println("File download failed. Check file path and permissions.");
                }
                return success;
            }

        } catch (IOException ex) {
            System.err.println("Error: " + ex.getMessage());
            ex.printStackTrace();
            return false;
        } finally {
            if (ftps.isConnected()) {
                try {
                    ftps.logout();
                    ftps.disconnect();
                } catch (IOException f) {
                    // do nothing
                }
            }
        }
    }

    public static void main(String[] args) {
        // SECURITY WARNING: Passing credentials as command-line arguments is insecure.
        // They can be viewed by other users on the system via the process list and
        // may be stored in shell history. In a production environment, use a more
        // secure method like environment variables, a secure vault, or interactive prompts.
        if (args.length != 5) {
            System.out.println("Usage: java Task77 <host> <username> <password> <remote_file_path> <local_file_name>");
            System.out.println("\n--- Test Case Examples (replace with actual server details) ---");
            System.out.println("1. Download 'report.csv' from 'ftp.example.com':");
            System.out.println("   java Task77 ftp.example.com user1 pass1 /reports/report.csv report_local.csv");
            System.out.println("2. Download 'archive.zip' from an IP address:");
            System.out.println("   java Task77 192.168.1.100 admin secret /backups/archive.zip backup.zip");
            System.out.println("3. Download 'image.jpg' from a public test server (credentials may vary):");
            System.out.println("   java Task77 test.rebex.net demo password /pub/example/image.jpg example.jpg");
            System.out.println("4. Attempt to download a non-existent file:");
            System.out.println("   java Task77 ftp.example.com user1 pass1 /path/to/nonexistent.file missing.txt");
            System.out.println("5. Use invalid credentials:");
            System.out.println("   java Task77 ftp.example.com wronguser wrongpass /reports/report.csv report.csv");
            return;
        }

        String host = args[0];
        String username = args[1];
        String password = args[2];
        String remoteFile = args[3];
        String localFile = args[4];

        downloadFileSecureFTP(host, username, password, remoteFile, localFile);
    }
}