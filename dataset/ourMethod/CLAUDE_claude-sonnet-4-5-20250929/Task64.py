
import sys
import json
import urllib.request
import urllib.error
import ssl
from typing import Optional

# Maximum allowed response size (10 MB)
MAX_RESPONSE_SIZE = 10 * 1024 * 1024

def validate_url(url: str) -> bool:
    """Validate URL format and enforce HTTPS only."""
    if not url or not isinstance(url, str):
        return False
    
    if len(url) < 8 or len(url) > 2048:
        return False
    
    # Enforce HTTPS only for security
    if not url.startswith("https://"):
        return False
    
    return True

def fetch_json_from_url(url: str) -> Optional[dict]:
    """\n    Fetch JSON from a URL with secure HTTPS settings.\n    \n    Args:\n        url: HTTPS URL to fetch JSON from\n        \n    Returns:\n        Parsed JSON object or None on error\n    """
    # Validate URL before any network operation
    if not validate_url(url):
        print("Error: Invalid or insecure URL. Only HTTPS URLs are allowed.", file=sys.stderr)
        return None
    
    try:
        # Create secure SSL context with certificate verification enabled
        # and TLS 1.2 minimum
        ssl_context = ssl.create_default_context()
        ssl_context.minimum_version = ssl.TLSVersion.TLSv1_2
        ssl_context.check_hostname = True
        ssl_context.verify_mode = ssl.CERT_REQUIRED
        
        # Create request with timeout
        request = urllib.request.Request(url)
        request.add_header('User-Agent', 'Python-JSON-Fetcher/1.0')
        
        # Open URL with secure context and size limit
        with urllib.request.urlopen(request, context=ssl_context, timeout=30) as response:
            # Check response code
            if response.status != 200:
                print(f"Error: HTTP request failed with code: {response.status}", file=sys.stderr)
                return None
            
            # Read response with size limit to prevent memory exhaustion
            content_length = response.headers.get('Content-Length')
            if content_length and int(content_length) > MAX_RESPONSE_SIZE:
                print("Error: Response size exceeds maximum allowed size.", file=sys.stderr)
                return None
            
            # Read in chunks with size checking
            data = bytearray()
            chunk_size = 8192
            
            while True:
                chunk = response.read(chunk_size)
                if not chunk:
                    break
                
                if len(data) + len(chunk) > MAX_RESPONSE_SIZE:
                    print("Error: Response size exceeds maximum allowed size.", file=sys.stderr)
                    return None
                
                data.extend(chunk)
            
            # Decode and parse JSON
            json_str = data.decode('utf-8')
            json_obj = json.loads(json_str)
            
            return json_obj
            
    except urllib.error.HTTPError as e:
        print(f"Error: HTTP error occurred: {e.code} {e.reason}", file=sys.stderr)
        return None
    except urllib.error.URLError as e:
        print(f"Error: URL error occurred: {e.reason}", file=sys.stderr)
        return None
    except json.JSONDecodeError as e:
        print(f"Error: Invalid JSON format: {e}", file=sys.stderr)
        return None
    except ssl.SSLError as e:
        print(f"Error: SSL/TLS error: {e}", file=sys.stderr)
        return None
    except Exception as e:
        print(f"Error: Unexpected error occurred: {type(e).__name__}", file=sys.stderr)
        return None

def main():
    """Main function with test cases."""
    if len(sys.argv) < 2:
        print("Running test cases...\\n")
        
        # Test case 1: Valid HTTPS URL
        print("Test 1: Valid HTTPS URL")
        result1 = fetch_json_from_url("https://jsonplaceholder.typicode.com/posts/1")
        if result1:
            print("Success: Fetched JSON data")
            print(f"Data: {str(result1)[:100]}...\\n")
        else:
            print("Failed to fetch data\\n")
        
        # Test case 2: Invalid URL (HTTP)
        print("Test 2: Invalid URL (HTTP not allowed)")
        result2 = fetch_json_from_url("http://example.com/data.json")
        if result2:
            print("Unexpected success\\n")
        else:
            print("Correctly rejected HTTP URL\\n")
        
        # Test case 3: None URL
        print("Test 3: None URL")
        result3 = fetch_json_from_url(None)
        if result3:
            print("Unexpected success\\n")
        else:
            print("Correctly rejected None URL\\n")
        
        # Test case 4: Empty URL
        print("Test 4: Empty URL")
        result4 = fetch_json_from_url("")
        if result4:
            print("Unexpected success\\n")
        else:
            print("Correctly rejected empty URL\\n")
        
        # Test case 5: Another valid HTTPS URL
        print("Test 5: Another valid HTTPS URL")
        result5 = fetch_json_from_url("https://jsonplaceholder.typicode.com/users/1")
        if result5:
            print("Success: Fetched JSON data")
            print(f"Data: {str(result5)[:100]}...\\n")
        else:
            print("Failed to fetch data\\n")
        
        return 0
    
    # Validate command line argument length
    url = sys.argv[1]
    if len(url) > 2048:
        print("Error: URL too long.", file=sys.stderr)
        return 1
    
    # Fetch JSON from provided URL
    json_data = fetch_json_from_url(url)
    
    if json_data is not None:
        print("Successfully fetched JSON:")
        print(json.dumps(json_data, indent=2))
        return 0
    else:
        print("Failed to fetch JSON from URL.", file=sys.stderr)
        return 1

if __name__ == "__main__":
    sys.exit(main())
