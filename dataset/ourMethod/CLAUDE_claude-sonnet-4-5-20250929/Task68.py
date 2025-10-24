
import re
import sys
from typing import Optional

try:
    import ldap
    LDAP_AVAILABLE = True
except ImportError:
    LDAP_AVAILABLE = False

MAX_INPUT_LENGTH = 256
DC_PATTERN = re.compile(r'^[a-zA-Z0-9][a-zA-Z0-9-]*[a-zA-Z0-9]$')
USERNAME_PATTERN = re.compile(r'^[a-zA-Z0-9._-]+$')
LDAP_URL = "ldap://localhost:389"


def sanitize_ldap_filter(value: str) -> str:
    """Sanitize input for LDAP filter to prevent injection."""
    escape_map = {
        '\\\\': r'\\5c',\n        '*': r'\\2a',\n        '(': r'\\28',\n        ')': r'\\29',\n        '\\x00': r'\\00'\n    }\n    result = value\n    for char, escaped in escape_map.items():\n        result = result.replace(char, escaped)\n    return result\n\n\ndef search_user(dc_input: Optional[str], username: Optional[str]) -> str:\n    """Search for a user in LDAP directory."""\n    if not LDAP_AVAILABLE:\n        return "Error: python-ldap module not available"\n    \n    if dc_input is None or username is None:\n        return "Error: Input cannot be null"\n    \n    if len(dc_input) > MAX_INPUT_LENGTH or len(username) > MAX_INPUT_LENGTH:\n        return "Error: Input exceeds maximum length"\n    \n    if not dc_input or not username:\n        return "Error: Input cannot be empty"\n    \n    if not USERNAME_PATTERN.match(username):\n        return "Error: Invalid username format"\n    \n    dc_components = dc_input.split('.')\n    if not dc_components or len(dc_components) > 10:\n        return "Error: Invalid domain component format"\n    \n    for dc in dc_components:\n        if not DC_PATTERN.match(dc):\n            return f"Error: Invalid domain component: {dc}"\n    \n    base_dn = ','.join([f"dc={dc}" for dc in dc_components])\n    \n    sanitized_username = sanitize_ldap_filter(username)\n    ldap_filter = f"(uid={sanitized_username})"\n    \n    conn = None\n    try:\n        conn = ldap.initialize(LDAP_URL)\n        conn.set_option(ldap.OPT_NETWORK_TIMEOUT, 5.0)\n        conn.set_option(ldap.OPT_TIMEOUT, 5.0)\n        conn.set_option(ldap.OPT_REFERRALS, 0)\n        \n        conn.simple_bind_s()\n        \n        results = conn.search_s(\n            base_dn,\n            ldap.SCOPE_SUBTREE,\n            ldap_filter,\n            ['uid', 'cn', 'mail'],\n            sizelimit=10\n        )\n        \n        if not results:\n            return "No user found"\n        \n        output = []\n        for dn, attrs in results[:10]:\n            output.append(f"DN: {dn}")\n            for attr_name, attr_values in attrs.items():\n                for value in attr_values:\n                    if isinstance(value, bytes):\n                        value = value.decode('utf-8', errors='replace')\n                    output.append(f"{attr_name}: {value}")\n            output.append("---")\n        \n        return '\
'.join(output[:-1])
        
    except ldap.LDAPError as e:
        return f"Error: LDAP operation failed - {type(e).__name__}"
    finally:
        if conn is not None:
            try:
                conn.unbind_s()
            except:
                pass


def main() -> None:
    """Main function with test cases."""
    print("Test 1: Valid input")
    print(search_user("example.com", "testuser"))
    print()
    
    print("Test 2: Invalid username with special chars")
    print(search_user("example.com", "test*user"))
    print()
    
    print("Test 3: Null input")
    print(search_user(None, "testuser"))
    print()
    
    print("Test 4: Empty input")
    print(search_user("", "testuser"))
    print()
    
    print("Test 5: Invalid domain component")
    print(search_user("exam ple.com", "testuser"))


if __name__ == "__main__":
    main()
