
import re
import urllib.parse
from typing import Optional

# Secure pattern: matches alphanumeric/hyphen characters followed by a literal period
# Pattern is anchored to prevent partial matches and injection attacks
SAFE_TARGET_PATTERN = re.compile(r'^[a-zA-Z0-9-]+\\.$')

# Maximum length to prevent ReDoS and excessive resource consumption
MAX_TARGET_LENGTH = 100

def process_redirect(target_query_param: Optional[str]) -> str:
    """\n    Validates and processes a redirect target based on secure pattern matching.\n    \n    Args:\n        target_query_param: The target query parameter value (untrusted input)\n    \n    Returns:\n        The validated redirect URL or root path "/"\n    """
    # Rule#3: Input validation - check for None or empty input
    if target_query_param is None or target_query_param == "":
        return "/"  # Fail closed - redirect to root
    
    try:
        # Decode URL-encoded input to prevent bypass via encoding
        target = urllib.parse.unquote(target_query_param)
        
        # Rule#3: Validate length to prevent resource exhaustion
        if len(target) > MAX_TARGET_LENGTH:
            return "/"  # Fail closed
        
        # Rule#3: Additional validation - reject directory traversal attempts
        # Reject paths containing .. or absolute paths
        if ".." in target or target.startswith("/") or target.startswith("\\\\"):\n            return "/"  # Fail closed\n        \n        # Rule#3: Reject URL schemes to prevent open redirect to external sites\n        if "://" in target or target.startswith("//"):\n            return "/"  # Fail closed - no external redirects\n        \n        # Apply the regex pattern match\n        if SAFE_TARGET_PATTERN.match(target):\n            # Even if pattern matches, construct safe relative path\n            # Prepend with ./ to ensure relative redirect within site\n            return "./" + target\n        else:\n            # Pattern does not match - redirect to root\n            return "/"\n    \n    except Exception as e:\n        # Rule#1: Fail closed on any error, log generically\n        # Do not expose internal details to caller\n        print("Redirect validation error occurred", file=__import__('sys').stderr)\n        return "/"\n\n\ndef main():\n    # Test case 1: Valid input matching pattern\n    test1 = "test123."\n    print(f"Test 1 input: '{test1}' -> {process_redirect(test1)}")\n    \n    # Test case 2: Valid input with hyphens\n    test2 = "my-page."\n    print(f"Test 2 input: '{test2}' -> {process_redirect(test2)}")\n    \n    # Test case 3: Invalid - missing period\n    test3 = "test123"\n    print(f"Test 3 input: '{test3}' -> {process_redirect(test3)}")\n    \n    # Test case 4: Invalid - directory traversal attempt\n    test4 = "../etc."\n    print(f"Test 4 input: '{test4}' -> {process_redirect(test4)}")\n    \n    # Test case 5: Invalid - open redirect attempt\n    test5 = "http://evil.com."\n    print(f"Test 5 input: '{test5}' -> {process_redirect(test5)}")\n\n\nif __name__ == "__main__":
    main()
