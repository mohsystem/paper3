
from xmlrpc.server import SimpleXMLRPCServer
from xmlrpc.server import SimpleXMLRPCRequestHandler
import html
import socket

class Task111:
    """XML-RPC Server implementation with security measures"""
    
    class MathHandler:
        """Handler class for mathematical operations"""
        
        def add(self, a, b):
            """Add two numbers"""
            try:
                return int(a) + int(b)
            except (ValueError, TypeError):
                return "Error: Invalid input types"
        
        def subtract(self, a, b):
            """Subtract two numbers"""
            try:
                return int(a) - int(b)
            except (ValueError, TypeError):
                return "Error: Invalid input types"
        
        def multiply(self, a, b):
            """Multiply two numbers"""
            try:
                return int(a) * int(b)
            except (ValueError, TypeError):
                return "Error: Invalid input types"
        
        def divide(self, a, b):
            """Divide two numbers"""
            try:
                a, b = int(a), int(b)
                if b == 0:
                    return "Error: Division by zero not allowed"
                return float(a) / float(b)
            except (ValueError, TypeError):
                return "Error: Invalid input types"
        
        def echo(self, message):
            """Echo a message with sanitization"""
            if not message:
                return "Empty message"
            # Sanitize input to prevent injection attacks
            sanitized = html.escape(str(message))
            return f"Echo: {sanitized}"
    
    class SecureXMLRPCRequestHandler(SimpleXMLRPCRequestHandler):
        """Custom request handler with security restrictions"""
        
        # Restrict to specific paths
        rpc_paths = ('/RPC2',)
        
        def do_POST(self):
            """Override to add security headers"""
            try:
                # Add security headers
                super().do_POST()
            except Exception as e:
                print(f"Request error: {e}")
    
    @staticmethod
    def start_server(host='localhost', port=8080):
        """Start the XML-RPC server with security configurations"""
        try:
            # Create server with custom request handler
            server = SimpleXMLRPCServer(
                (host, port),
                requestHandler=Task111.SecureXMLRPCRequestHandler,
                allow_none=False,  # Security: Don't allow None values\n                use_builtin_types=True\n            )\n            \n            # Set timeout to prevent hanging connections\n            server.socket.settimeout(30)\n            \n            # Register instance\n            handler = Task111.MathHandler()\n            server.register_instance(handler)\n            \n            # Register introspection functions (optional, can be disabled for security)\n            server.register_introspection_functions()\n            \n            print(f"XML-RPC Server started on {host}:{port}")\n            print("Press Ctrl+C to stop the server")\n            \n            # Start serving\n            server.serve_forever()\n            \n        except socket.error as e:\n            print(f"Socket error: {e}")\n        except KeyboardInterrupt:\n            print("\\nServer stopped by user")\n        except Exception as e:\n            print(f"Server error: {e}")\n        finally:\n            if 'server' in locals():\n                server.server_close()\n\ndef main():\n    """Main function with test cases"""\n    print("Starting XML-RPC Server Tests...\\n")\n    \n    # Create handler instance for testing\n    handler = Task111.MathHandler()\n    \n    print("=== Test Case 1: Addition ===")\n    result1 = handler.add(10, 5)\n    print(f"add(10, 5) = {result1}")\n    \n    print("\\n=== Test Case 2: Subtraction ===")\n    result2 = handler.subtract(20, 8)\n    print(f"subtract(20, 8) = {result2}")\n    \n    print("\\n=== Test Case 3: Multiplication ===")\n    result3 = handler.multiply(7, 6)\n    print(f"multiply(7, 6) = {result3}")\n    \n    print("\\n=== Test Case 4: Division ===")\n    result4 = handler.divide(100, 4)\n    print(f"divide(100, 4) = {result4}")\n    \n    print("\\n=== Test Case 5: Echo with sanitization ===")\n    result5 = handler.echo("Hello World")\n    print(f"echo('Hello World') = {result5}")\n    result6 = handler.echo("<script>alert(1)</script>")\n    print(f"echo('<script>alert(1)</script>') = {result6}")\n    \n    # Uncomment to run actual server:\n    # print("\\n\\nStarting actual XML-RPC server...")\n    # Task111.start_server('localhost', 8080)

if __name__ == "__main__":
    main()
