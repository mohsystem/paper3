
import urllib.parse
import urllib.request
import ssl
import socket
import ipaddress
from typing import Optional

MAX_URL_LENGTH = 2048
MAX_RESPONSE_SIZE = 1048576  # 1MB limit
TIMEOUT_SECONDS = 30

def is_private_ip(ip_str: str) -> bool:
    """Check if IP address is private/internal - SSRF protection"""
    try:
        ip = ipaddress.ip_address(ip_str)
        # Check if IP is private, loopback, link-local, or reserved
        return (
            ip.is_private or
            ip.is_loopback or
            ip.is_link_local or
            ip.is_reserved or
            ip.is_multicast
        )
    except ValueError:
        return True  # If invalid IP, treat as suspicious

def validate_url(url: str) -> bool:
    """Validate URL format and security requirements"""
    if not url or not isinstance(url, str):
        return False
    
    # Check length bounds
    if len(url) == 0 or len(url) > MAX_URL_LENGTH:
        return False
    
    # SECURITY: Only allow HTTPS protocol
    if not url.startswith('https://'):
        return False
    
    # Parse URL to extract components
    try:
        parsed = urllib.parse.urlparse(url)
    except Exception:
        return False
    
    # Must have a valid hostname
    if not parsed.netloc or len(parsed.netloc) == 0:
        return False
    
    # SECURITY: Reject URLs with credentials
    if '@' in parsed.netloc:
        return False
    
    # Extract hostname (remove port if present)
    hostname = parsed.hostname
    if not hostname:
        return False
    
    # SECURITY: Resolve hostname and check if it points to private IP
    try:
        ip_address = socket.gethostbyname(hostname)
        if is_private_ip(ip_address):
            return False
    except socket.error:
        # DNS resolution failed - reject
        return False
    
    return True

def make_secure_request(url: str) -> Optional[str]:
    """Make secure HTTPS request with all protections enabled"""
    if not url:
        print("Error: URL is required.", file=__import__('sys').stderr)
        return None
    
    # SECURITY: Validate URL before any network activity
    if not validate_url(url):
        print("Error: Invalid URL. Only HTTPS URLs to public hosts are allowed.", 
              file=__import__('sys').stderr)
        return None
    
    try:
        # SECURITY: Create SSL context with certificate verification enabled
        # Requires TLS 1.2 or higher
        ssl_context = ssl.create_default_context()
        ssl_context.minimum_version = ssl.TLSVersion.TLSv1_2
        ssl_context.check_hostname = True  # Verify hostname matches certificate
        ssl_context.verify_mode = ssl.CERT_REQUIRED  # Require valid certificate
        
        # Create request with security headers
        request = urllib.request.Request(
            url,
            headers={
                'User-Agent': 'SecureHTTPClient/1.0'
            }
        )
        
        # SECURITY: Open connection with timeout and SSL context
        with urllib.request.urlopen(
            request,
            timeout=TIMEOUT_SECONDS,
            context=ssl_context
        ) as response:
            
            # SECURITY: Check response size before reading
            content_length = response.getheader('Content-Length')
            if content_length and int(content_length) > MAX_RESPONSE_SIZE:
                print("Error: Response too large.", file=__import__('sys').stderr)
                return None
            
            # Read response with size limit
            data = response.read(MAX_RESPONSE_SIZE + 1)
            
            # SECURITY: Verify we didn't exceed size limit\n            if len(data) > MAX_RESPONSE_SIZE:\n                print("Error: Response exceeded size limit.", file=__import__('sys').stderr)\n                return None\n            \n            # Decode response safely\n            return data.decode('utf-8', errors='replace')\n            \n    except urllib.error.HTTPError as e:\n        # Generic error - don't leak internal details
        print(f"Error: HTTP request failed with status {e.code}.", 
              file=__import__('sys').stderr)
        return None
    except urllib.error.URLError as e:
        print("Error: Request failed.", file=__import__('sys').stderr)
        return None
    except socket.timeout:
        print("Error: Request timed out.", file=__import__('sys').stderr)
        return None
    except Exception as e:
        print("Error: Request failed.", file=__import__('sys').stderr)
        return None

def main():
    """Main function with test cases"""
    
    # Test case 1: Valid HTTPS URL
    print("Test 1: Valid HTTPS URL")
    response = make_secure_request("https://example.com")
    if response:
        print(f"Success! Response length: {len(response)} bytes")
        print(f"Response preview: {response[:100]}...")
    print()
    
    # Test case 2: Invalid URL (HTTP instead of HTTPS)
    print("Test 2: Invalid URL (HTTP not allowed)")
    response = make_secure_request("http://example.com")
    if response:
        print("Success (unexpected)")
    else:
        print("Rejected as expected.")
    print()
    
    # Test case 3: Empty URL
    print("Test 3: Empty URL")
    response = make_secure_request("")
    if response:
        print("Success (unexpected)")
    else:
        print("Rejected as expected.")
    print()
    
    # Test case 4: URL with credentials (should be rejected)
    print("Test 4: URL with credentials")
    response = make_secure_request("https://user:pass@example.com")
    if response:
        print("Success (unexpected)")
    else:
        print("Rejected as expected.")
    print()
    
    # Test case 5: Private IP address (should be rejected - SSRF protection)
    print("Test 5: Private IP address (SSRF protection)")
    response = make_secure_request("https://127.0.0.1")
    if response:
        print("Success (unexpected)")
    else:
        print("Rejected as expected.")
    print()

if __name__ == "__main__":
    main()
