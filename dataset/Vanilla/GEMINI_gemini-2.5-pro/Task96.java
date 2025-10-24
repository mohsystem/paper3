import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.ServerSocket;
import java.net.Socket;

public class Task96 {

    /**
     * Starts the echo server on a specified port.
     * @param port The port number to listen on.
     */
    public void startServer(int port) {
        ServerSocket serverSocket = null;
        try {
            serverSocket = new ServerSocket(port);
            System.out.println("Server started on port: " + port);
            System.out.println("Waiting for client connections...");

            while (true) {
                // Accepts a client connection. This is a blocking call.
                Socket clientSocket = serverSocket.accept();
                // Create a new thread to handle the client connection
                new ClientHandler(clientSocket).start();
            }
        } catch (IOException e) {
            System.err.println("Could not listen on port: " + port);
            e.printStackTrace();
        } finally {
            if (serverSocket != null) {
                try {
                    serverSocket.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    /**
     * Inner class to handle communication with a single client in a separate thread.
     */
    private static class ClientHandler extends Thread {
        private Socket clientSocket;

        public ClientHandler(Socket socket) {
            this.clientSocket = socket;
            System.out.println("Client connected: " + clientSocket.getInetAddress().getHostAddress());
        }

        public void run() {
            try (
                PrintWriter out = new PrintWriter(clientSocket.getOutputStream(), true);
                BufferedReader in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()))
            ) {
                String inputLine;
                // Read from client and echo back until client disconnects
                while ((inputLine = in.readLine()) != null) {
                    System.out.println("Received from client " + clientSocket.getInetAddress().getHostAddress() + ": " + inputLine);
                    out.println(inputLine); // Echo back to the client
                }
            } catch (IOException e) {
                // This might happen if the client disconnects abruptly
                // System.err.println("Error handling client: " + e.getMessage());
            } finally {
                try {
                    clientSocket.close();
                    System.out.println("Client disconnected: " + clientSocket.getInetAddress().getHostAddress());
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    public static void main(String[] args) {
        // --- Test Cases ---
        // The server will run indefinitely. To test, you need a client program
        // or a tool like `telnet` or `netcat`.
        //
        // How to test (5 test cases):
        // 1. Run this Java program.
        // 2. Open a new terminal/command prompt.
        // 3. Test Case 1: Connect using telnet: `telnet localhost 12345`
        // 4. Test Case 2: Type "Hello Server" and press Enter. The server should echo it back.
        // 5. Test Case 3: Type "This is a test" and press Enter.
        // 6. Test Case 4: Open another new terminal and connect again: `telnet localhost 12345`.
        //    Send a message from this new client. The server handles multiple clients.
        // 7. Test Case 5: Close one of the telnet sessions (Ctrl+] then type 'quit').
        //    The other session should remain active.
        
        int port = 12345;
        Task96 server = new Task96();
        server.startServer(port);
    }
}