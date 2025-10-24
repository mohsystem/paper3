
# WARNING: This code demonstrates a dangerous pattern and should NOT be used in production.
# Downloading and executing arbitrary executables is a critical security vulnerability.
# This violates secure software distribution practices.

import sys
import os
import ssl
import urllib.request
import urllib.parse
import subprocess
import tempfile
import secrets

def download_and_execute(url):
    """\n    SECURITY WARNING: This function downloads and executes arbitrary code.\n    In production, you MUST:\n    1. Verify cryptographic signatures using the vendor's public key\n    2. Validate file hashes against known-good values from a trusted channel\n    3. Implement user consent with clear security warnings\n    4. Use sandboxing/containerization for execution\n    5. Maintain audit logs\n    """
    
    # Input validation - check URL format
    if not url or not isinstance(url, str):
        print("Error: Invalid URL provided", file=sys.stderr)
        return False
    
    parsed = urllib.parse.urlparse(url)
    if parsed.scheme not in ('https',):  # Only HTTPS allowed per security rules
        print("Error: Only HTTPS URLs are permitted", file=sys.stderr)
        return False
    
    if not parsed.netloc:
        print("Error: Invalid URL format", file=sys.stderr)
        return False
    
    # Create secure TLS context - enforce TLS 1.2+ per Rules #3, #4, #13
    ctx = ssl.create_default_context()
    ctx.check_hostname = True
    ctx.verify_mode = ssl.CERT_REQUIRED
    
    # Enforce TLS 1.2+ for all Python versions per Rules #13
    if hasattr(ssl, "TLSVersion"):  # Python 3.10+
        ctx.minimum_version = ssl.TLSVersion.TLSv1_2
    else:  # Older Python versions
        ctx.options |= getattr(ssl, "OP_NO_SSLv2", 0)
        ctx.options |= getattr(ssl, "OP_NO_SSLv3", 0)
        ctx.options |= getattr(ssl, "OP_NO_TLSv1", 0)
        ctx.options |= getattr(ssl, "OP_NO_TLSv1_1", 0)
    
    temp_file = None
    
    try:
        # Create temporary file with restrictive permissions (0600) per Rules #1
        # Use secure random for filename to prevent predictability per Rules #15
        random_suffix = secrets.token_hex(16)
        fd = os.open(
            os.path.join(tempfile.gettempdir(), f"download_{random_suffix}.exe"),
            os.O_WRONLY | os.O_CREAT | os.O_EXCL,
            0o600  # Owner read/write only
        )
        temp_file = os.fdopen(fd, 'wb')
        temp_path = temp_file.name
        
        print(f"Downloading from: {url}")
        print("WARNING: Executing downloaded executables is dangerous!")
        
        # Download with secure TLS settings
        request = urllib.request.Request(url, headers={'User-Agent': 'SecureDownloader/1.0'})
        
        with urllib.request.urlopen(request, context=ctx, timeout=30) as response:
            # Limit download size to prevent resource exhaustion per Rules #6
            MAX_SIZE = 500 * 1024 * 1024  # 500MB limit
            downloaded = 0
            
            while True:
                chunk = response.read(8192)
                if not chunk:
                    break
                
                downloaded += len(chunk)
                if downloaded > MAX_SIZE:
                    print(f"Error: File exceeds maximum size of {MAX_SIZE} bytes", file=sys.stderr)
                    return False
                
                temp_file.write(chunk)
        
        temp_file.close()
        
        print(f"Download complete: {downloaded} bytes")
        print(f"Saved to: {temp_path}")
        
        # CRITICAL SECURITY ISSUE: No signature verification!
        # In production, you MUST verify cryptographic signatures here
        print("\\nWARNING: No signature verification performed!")
        print("This executable could be malicious!")
        
        # Execute the downloaded file
        # SECURITY ISSUE: Running untrusted code with user privileges
        print(f"\\nExecuting: {temp_path}")
        result = subprocess.run([temp_path], capture_output=True, timeout=300)
        
        print(f"Exit code: {result.returncode}")
        
        return True
        
    except urllib.error.URLError as e:
        print(f"Error: Download failed - {e}", file=sys.stderr)
        return False
    except subprocess.TimeoutExpired:
        print("Error: Execution timeout", file=sys.stderr)
        return False
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        return False
    finally:
        # Cleanup: remove temporary file
        if temp_file:
            try:
                temp_file.close()
            except:
                pass
            try:
                os.unlink(temp_file.name)
            except:
                pass

if __name__ == "__main__":
    print("="*70)
    print("SECURITY WARNING: This program downloads and executes code!")
    print("This is a DANGEROUS operation and should only be used for demonstration.")
    print("="*70)
    print()
    
    # Test cases (DO NOT RUN with real executables)
    test_urls = [
        "https://example.com/test.exe",  # Would fail - not a real file
        "http://insecure.com/test.exe",  # Rejected - not HTTPS
        "",  # Rejected - empty URL
        "https://",  # Rejected - invalid format
        "https://example.com/large.exe",  # Would test size limits
    ]
    
    print("Test case validation (not executing):")
    for i, test_url in enumerate(test_urls, 1):
        print(f"\\nTest {i}: {test_url if test_url else '(empty)'}")
        if not test_url or not test_url.startswith("https://") or test_url == "https://":
            print("  -> Would be rejected by validation")
