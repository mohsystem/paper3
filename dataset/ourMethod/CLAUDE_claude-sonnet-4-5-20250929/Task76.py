
#!/usr/bin/env python3
"""\nSecure HTTPS web page title extractor.\nFetches content from an HTTPS URL and extracts the page title.\n"""

import sys
import re
import urllib.parse
import urllib.request
import ssl
from typing import Optional


def validate_url(url: str) -> bool:
    """\n    Validate that the URL is well-formed and uses HTTPS protocol.\n    \n    Args:\n        url: The URL string to validate\n        \n    Returns:\n        True if valid HTTPS URL, False otherwise\n    """
    # Check basic length constraints to prevent excessive memory usage
    if not url or len(url) > 2048:
        return False
    
    # Parse URL to validate structure
    try:
        parsed = urllib.parse.urlparse(url)
    except ValueError:
        return False
    
    # Only allow HTTPS scheme for security
    if parsed.scheme != 'https':
        return False
    
    # Ensure netloc (hostname) is present and reasonable
    if not parsed.netloc or len(parsed.netloc) > 253:
        return False
    
    return True


def extract_title(html_content: str) -> Optional[str]:
    """\n    Extract the title from HTML content using safe regex parsing.\n    \n    Args:\n        html_content: The HTML content as a string\n        \n    Returns:\n        The extracted title or None if not found\n    """
    # Limit HTML content size to prevent excessive memory usage
    max_html_size = 10 * 1024 * 1024  # 10 MB limit
    if len(html_content) > max_html_size:
        html_content = html_content[:max_html_size]
    
    # Use regex to find title tag (case-insensitive, with multiline support)
    # This is a safe operation as we're only reading, not executing\n    title_pattern = re.compile(r'<title[^>]*>(.*?)</title>', re.IGNORECASE | re.DOTALL)\n    match = title_pattern.search(html_content)\n    \n    if match:\n        title = match.group(1)\n        # Strip whitespace and decode HTML entities safely\n        title = title.strip()\n        # Limit title length to prevent excessive output\n        if len(title) > 1000:\n            title = title[:1000]\n        return title\n    \n    return None\n\n\ndef fetch_page_title(url: str) -> str:\n    """\n    Fetch a webpage via HTTPS and extract its title.\n    \n    Args:\n        url: The HTTPS URL to fetch\n        \n    Returns:\n        The page title or an error message\n    """\n    # Validate URL before making any network request\n    if not validate_url(url):\n        return "Error: Invalid URL. Only HTTPS URLs are allowed and must be well-formed."\n    \n    try:\n        # Create SSL context with certificate verification enabled\n        # This ensures proper SSL/TLS certificate validation (CWE-295, CWE-297)\n        ssl_context = ssl.create_default_context()\n        # Enforce hostname verification\n        ssl_context.check_hostname = True\n        # Require certificate verification\n        ssl_context.verify_mode = ssl.CERT_REQUIRED\n        \n        # Set a reasonable timeout to prevent indefinite hangs\n        timeout = 30  # seconds\n        \n        # Create request with User-Agent to avoid being blocked by some servers\n        headers = {\n            'User-Agent': 'Mozilla/5.0 (compatible; SecurePageTitleExtractor/1.0)'\n        }\n        request = urllib.request.Request(url, headers=headers)\n        \n        # Make HTTPS request with SSL context and timeout\n        with urllib.request.urlopen(request, context=ssl_context, timeout=timeout) as response:\n            # Check response status\n            if response.status != 200:\n                return f"Error: HTTP status {response.status}"\n            \n            # Limit response size to prevent excessive memory usage\n            max_response_size = 10 * 1024 * 1024  # 10 MB\n            content = response.read(max_response_size)\n            \n            # Decode content safely\n            # Try to get charset from Content-Type header\n            content_type = response.headers.get('Content-Type', '')\n            encoding = 'utf-8'  # default\n            \n            if 'charset=' in content_type:\n                try:\n                    encoding = content_type.split('charset=')[-1].split(';')[0].strip()\n                except (IndexError, AttributeError):\n                    encoding = 'utf-8'\n            \n            # Decode with error handling\n            try:\n                html_content = content.decode(encoding, errors='replace')\n            except (UnicodeDecodeError, LookupError):\n                html_content = content.decode('utf-8', errors='replace')
            
            # Extract and return title
            title = extract_title(html_content)
            if title:
                return title
            else:
                return "Error: No title found in HTML content"
    
    except ssl.SSLError as e:
        # SSL/TLS certificate validation failed
        return f"Error: SSL certificate validation failed: {str(e)}"
    except urllib.error.HTTPError as e:
        # HTTP error (4xx, 5xx)
        return f"Error: HTTP error {e.code}: {e.reason}"
    except urllib.error.URLError as e:
        # Network error or invalid URL
        return f"Error: Network error: {str(e.reason)}"
    except TimeoutError:
        return "Error: Request timed out"
    except Exception as e:
        # Catch any other unexpected errors
        return f"Error: Unexpected error: {str(type(e).__name__)}"


def main() -> int:
    """\n    Main entry point for the program.\n    \n    Returns:\n        Exit code (0 for success, 1 for error)\n    """
    # Validate command line arguments
    if len(sys.argv) != 2:
        print("Usage: python solution.py <HTTPS_URL>", file=sys.stderr)
        print("Example: python solution.py https://www.example.com", file=sys.stderr)
        return 1
    
    # Get URL from command line argument (treating it as untrusted input)
    url = sys.argv[1]
    
    # Validate input length
    if len(url) > 2048:
        print("Error: URL too long (max 2048 characters)", file=sys.stderr)
        return 1
    
    # Fetch and print the page title
    result = fetch_page_title(url)
    print(result)
    
    # Return success if no error message
    return 0 if not result.startswith("Error:") else 1


# Test cases
if __name__ == "__main__":
    # Check if running in test mode
    if len(sys.argv) > 1 and sys.argv[1] == "--test":
        print("Running test cases...")
        print()
        
        # Test case 1: Valid HTTPS URL (example.com)
        print("Test 1: Valid HTTPS URL")
        test_url_1 = "https://www.example.com"
        result_1 = fetch_page_title(test_url_1)
        print(f"URL: {test_url_1}")
        print(f"Result: {result_1}")
        print()
        
        # Test case 2: Invalid protocol (HTTP instead of HTTPS)
        print("Test 2: Invalid protocol (HTTP)")
        test_url_2 = "http://www.example.com"
        result_2 = fetch_page_title(test_url_2)
        print(f"URL: {test_url_2}")
        print(f"Result: {result_2}")
        print()
        
        # Test case 3: Malformed URL
        print("Test 3: Malformed URL")
        test_url_3 = "not_a_valid_url"
        result_3 = fetch_page_title(test_url_3)
        print(f"URL: {test_url_3}")
        print(f"Result: {result_3}")
        print()
        
        # Test case 4: Empty string
        print("Test 4: Empty string")
        test_url_4 = ""
        result_4 = fetch_page_title(test_url_4)
        print(f"URL: (empty)")
        print(f"Result: {result_4}")
        print()
        
        # Test case 5: URL too long
        print("Test 5: URL too long")
        test_url_5 = "https://example.com/" + "a" * 3000
        result_5 = fetch_page_title(test_url_5)
        print(f"URL: (very long URL)")
        print(f"Result: {result_5}")
        print()
        
        sys.exit(0)
    else:
        sys.exit(main())
