
import socket
import threading
import sys
import select
import signal
from typing import Set, Optional

# Maximum message size to prevent excessive memory usage
MAX_MESSAGE_SIZE = 4096
# Maximum username length
MAX_USERNAME = 32
# Server port
SERVER_PORT = 8888
# Maximum number of clients
MAX_CLIENTS = 100


def validate_input(data: str, max_length: int) -> bool:
    """Validate input string: check length and reject control characters"""
    if not data or not isinstance(data, str):
        return False
    
    # Enforce maximum length to prevent buffer overflow
    if len(data) > max_length:
        return False
    
    # Check for control characters (allow newline and tab)
    for char in data:
        code = ord(char)
        if (code < 32 and code not in (9, 10, 13)) or code == 127:
            return False
    
    return True


class ClientHandler:
    """Handle individual client connection"""
    
    def __init__(self, client_socket: socket.socket, address: tuple, server: 'ChatServer'):
        """Initialize client handler with validation"""
        if client_socket is None or server is None:
            raise ValueError("Socket and server cannot be None")
        
        self.socket = client_socket
        self.address = address
        self.server = server
        self.username: Optional[str] = None
        self.running = True
        
    def handle(self):
        """Main handler for client communication"""
        try:
            # Set socket timeout to prevent indefinite blocking
            self.socket.settimeout(300)  # 5 minutes
            
            # Receive username with size limit
            username_bytes = self.socket.recv(MAX_USERNAME)
            if not username_bytes:
                return
            
            # Decode with UTF-8 and validate
            try:
                self.username = username_bytes.decode('utf-8').strip()
            except UnicodeDecodeError:
                self.send_message("Invalid encoding\\n")
                return
            
            if not validate_input(self.username, MAX_USERNAME):
                self.send_message("Invalid username\\n")
                return
            
            # Add client to server
            if not self.server.add_client(self):
                self.send_message("Server full\\n")
                return
            
            # Broadcast join message
            self.server.broadcast(f"[Server] {self.username} joined the chat\\n", self)
            
            # Main message loop
            buffer = b''
            while self.running:
                try:
                    # Receive data with size limit
                    chunk = self.socket.recv(MAX_MESSAGE_SIZE)
                    if not chunk:
                        break
                    
                    buffer += chunk
                    
                    # Process complete messages (lines)
                    while b'\\n' in buffer:
                        # Enforce maximum buffer size
                        if len(buffer) > MAX_MESSAGE_SIZE:
                            buffer = buffer[:MAX_MESSAGE_SIZE]
                        
                        line, buffer = buffer.split(b'\\n', 1)
                        
                        try:
                            message = line.decode('utf-8').strip()
                        except UnicodeDecodeError:
                            continue  # Skip invalid messages
                        
                        # Validate message before processing
                        if validate_input(message, MAX_MESSAGE_SIZE):
                            formatted = f"[{self.username}]: {message}\\n"
                            self.server.broadcast(formatted, self)
                    
                except socket.timeout:
                    continue
                except OSError:
                    break
                    
        except Exception as e:
            # Log error without exposing internal details
            print(f"Client handler error: connection issue", file=sys.stderr)
        finally:
            # Clean up
            self.server.remove_client(self)
            if self.username:
                self.server.broadcast(f"[Server] {self.username} left the chat\\n", self)
            self.close()
    
    def send_message(self, message: str):
        """Send message to this client with error handling"""
        try:
            if message and self.socket:
                # Encode to UTF-8 bytes
                data = message.encode('utf-8')
                self.socket.sendall(data)
        except Exception:
            # Silently fail - client may have disconnected
            pass
    
    def close(self):
        """Close socket connection"""
        self.running = False
        try:
            if self.socket:
                self.socket.close()
        except Exception:
            pass


class ChatServer:
    """Chat server managing multiple clients"""
    
    def __init__(self):
        """Initialize server with thread-safe data structures"""
        self.clients: Set[ClientHandler] = set()
        self.clients_lock = threading.Lock()
        self.running = True
        self.server_socket: Optional[socket.socket] = None
        
    def add_client(self, client: ClientHandler) -> bool:
        """Add client with size limit enforcement"""
        with self.clients_lock:
            if len(self.clients) >= MAX_CLIENTS:
                return False
            self.clients.add(client)
            return True
    
    def remove_client(self, client: ClientHandler):
        """Remove client from active clients"""
        with self.clients_lock:
            self.clients.discard(client)
    
    def broadcast(self, message: str, sender: Optional[ClientHandler] = None):
        """Broadcast message to all clients except sender"""
        # Input validation
        if not message or not isinstance(message, str):
            return
        
        with self.clients_lock:
            # Create copy to avoid modification during iteration
            clients_copy = self.clients.copy()
        
        for client in clients_copy:
            if client != sender:
                try:
                    client.send_message(message)
                except Exception:
                    # Continue broadcasting to other clients on error
                    print("Failed to send to client", file=sys.stderr)
    
    def start(self):
        """Start server and accept connections"""
        try:
            # Create server socket
            self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            
            # Set socket options to reuse address
            self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            
            # Bind to all interfaces
            self.server_socket.bind(('0.0.0.0', SERVER_PORT))
            self.server_socket.listen(10)
            
            print(f"Server listening on port {SERVER_PORT}")
            
            # Accept client connections
            while self.running:
                try:
                    # Use select with timeout to allow checking running flag
                    readable, _, _ = select.select([self.server_socket], [], [], 1.0)
                    
                    if readable:
                        client_socket, address = self.server_socket.accept()
                        
                        # Create and start client handler thread
                        handler = ClientHandler(client_socket, address, self)
                        thread = threading.Thread(target=handler.handle, daemon=True)
                        thread.start()
                        
                except socket.error as e:
                    if not self.running:
                        break
                    print(f"Socket error: connection issue", file=sys.stderr)
                except Exception as e:
                    print("Failed to accept connection", file=sys.stderr)
                    
        except Exception as e:
            print("Failed to start server", file=sys.stderr)
        finally:
            self.shutdown()
    
    def shutdown(self):
        """Shutdown server and close all connections"""
        self.running = False
        
        # Close all client connections
        with self.clients_lock:
            clients_copy = self.clients.copy()
            self.clients.clear()
        
        for client in clients_copy:
            try:
                client.close()
            except Exception:
                pass
        
        # Close server socket
        if self.server_socket:
            try:
                self.server_socket.close()
            except Exception:
                pass


class ChatClient:
    """Chat client for connecting to server"""
    
    def __init__(self, username: str):
        """Initialize client with username validation"""
        if not username or not isinstance(username, str):
            raise ValueError("Invalid username")
        
        if len(username) > MAX_USERNAME:
            raise ValueError("Username too long")
        
        if not validate_input(username, MAX_USERNAME):
            raise ValueError("Username contains invalid characters")
        
        self.username = username
        self.socket: Optional[socket.socket] = None
        self.running = True
        
    def connect(self):
        """Connect to server and start communication"""
        try:
            # Create socket and connect
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.connect(('127.0.0.1', SERVER_PORT))
            
            # Send username
            self.socket.sendall(self.username.encode('utf-8'))
            
            print(f"Connected as {self.username}. Type messages to send:")
            
            # Start receiver thread
            receiver_thread = threading.Thread(target=self.receive_messages, daemon=True)
            receiver_thread.start()
            
            # Send messages from console
            self.send_messages()
            
        except Exception as e:
            print("Connection failed", file=sys.stderr)
        finally:
            self.disconnect()
    
    def receive_messages(self):
        """Receive messages from server"""
        try:
            buffer = b''
            while self.running:
                try:
                    # Receive data with size limit
                    chunk = self.socket.recv(MAX_MESSAGE_SIZE)
                    if not chunk:
                        break
                    
                    buffer += chunk
                    
                    # Process complete messages
                    while b'\\n' in buffer:
                        if len(buffer) > MAX_MESSAGE_SIZE:
                            buffer = buffer[:MAX_MESSAGE_SIZE]
                        
                        line, buffer = buffer.split(b'\\n', 1)
                        try:
                            message = line.decode('utf-8')
                            print(message)
                        except UnicodeDecodeError:
                            continue
                            
                except socket.timeout:
                    continue
                except OSError:
                    break
                    
        except Exception:
            print("Disconnected from server", file=sys.stderr)
    
    def send_messages(self):
        """Send messages to server from stdin"""
        try:
            while self.running:
                # Use select to check if input is available
                readable, _, _ = select.select([sys.stdin], [], [], 0.1)
                
                if readable:
                    message = sys.stdin.readline()
                    if not message:
                        break
                    
                    message = message.strip()
                    
                    # Validate message before sending
                    if message and len(message) <= MAX_MESSAGE_SIZE and \\
                       validate_input(message, MAX_MESSAGE_SIZE):
                        self.socket.sendall((message + '\\n').encode('utf-8'))
                        
        except Exception:
            print("Input error", file=sys.stderr)
    
    def disconnect(self):
        """Disconnect from server"""
        self.running = False
        if self.socket:
            try:
                self.socket.close()
            except Exception:
                pass


def signal_handler(signum, frame):
    """Handle shutdown signals"""
    print("\\nShutting down...")
    sys.exit(0)


def main():
    """Main entry point"""
    # Validate command line arguments
    if len(sys.argv) < 2:
        print("Usage:")
        print("  Server mode: python script.py server")
        print("  Client mode: python script.py client <username>")
        return
    
    # Set up signal handlers for graceful shutdown
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    
    try:
        if sys.argv[1] == "server":
            # Run server
            server = ChatServer()
            server.start()
        elif sys.argv[1] == "client" and len(sys.argv) >= 3:
            # Run client
            client = ChatClient(sys.argv[2])
            client.connect()
        else:
            print("Invalid arguments")
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)


if __name__ == "__main__":
    main()
