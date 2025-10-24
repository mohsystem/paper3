import java.io.*;
import java.net.*;
import java.util.*;
import java.util.concurrent.*;

public class Task90 {
    private static final int PORT = 12345;
    // Using a thread-safe set to store client writers.
    private static final Set<PrintWriter> clientWriters = new CopyOnWriteArraySet<>();

    public static void main(String[] args) {
        System.out.println("Java Chat Server is running on port " + PORT);
        // Using a thread pool to manage client threads efficiently.
        ExecutorService pool = Executors.newCachedThreadPool();
        try (ServerSocket serverSocket = new ServerSocket(PORT)) {
            while (true) {
                try {
                    // Accept a new client connection.
                    Socket clientSocket = serverSocket.accept();
                    // Submit a new task to the thread pool to handle the client.
                    pool.submit(new ClientHandler(clientSocket));
                } catch (IOException e) {
                    System.err.println("Error accepting client connection: " + e.getMessage());
                }
            }
        } catch (IOException e) {
            System.err.println("Could not listen on port " + PORT + ": " + e.getMessage());
        } finally {
            pool.shutdown();
        }
    }

    private static class ClientHandler implements Runnable {
        private final Socket socket;
        private PrintWriter writer;
        private String clientName;

        public ClientHandler(Socket socket) {
            this.socket = socket;
        }

        @Override
        public void run() {
            try {
                InputStream input = socket.getInputStream();
                BufferedReader reader = new BufferedReader(new InputStreamReader(input));
                OutputStream output = socket.getOutputStream();
                writer = new PrintWriter(output, true);

                // Add the new client's writer to the shared set.
                clientWriters.add(writer);
                
                clientName = socket.getRemoteSocketAddress().toString();
                System.out.println("New client connected: " + clientName);
                broadcastMessage("Server: " + clientName + " has joined the chat.");

                String message;
                while ((message = reader.readLine()) != null) {
                    if ("exit".equalsIgnoreCase(message.trim())) {
                        break;
                    }
                    String broadcastMsg = clientName + ": " + message;
                    System.out.println("Received: " + broadcastMsg);
                    broadcastMessage(broadcastMsg);
                }
            } catch (IOException e) {
                // This might happen if the client disconnects abruptly.
                System.out.println("Error with client " + clientName + ": " + e.getMessage());
            } finally {
                // Cleanup resources.
                if (writer != null) {
                    clientWriters.remove(writer);
                }
                if (clientName != null) {
                    String departureMessage = "Server: " + clientName + " has left the chat.";
                    System.out.println(departureMessage);
                    broadcastMessage(departureMessage);
                }
                try {
                    socket.close();
                } catch (IOException e) {
                    // Ignore
                }
            }
        }

        private void broadcastMessage(String message) {
            for (PrintWriter writer : clientWriters) {
                writer.println(message);
            }
        }
    }
}
/*
How to Test:
1. Compile and run this Java file:
   javac Task90.java
   java Task90
2. The server will start and print "Java Chat Server is running...".
3. Open multiple (e.g., 5) separate terminal/command prompt windows.
4. In each terminal, connect to the server using a tool like telnet or netcat:
   telnet localhost 12345
   (or nc localhost 12345)
5. Type a message in any of the client terminals and press Enter.
6. The message should appear in all other client terminals, prefixed with the sender's address.
7. To disconnect a client, close its terminal window or type 'exit' and press Enter.
   A "has left the chat" message should appear for the remaining clients.
*/