
import yaml
import re
from decimal import Decimal, InvalidOperation
from typing import Optional

# Hardcoded product prices - in production, fetch from secure database
PRODUCT_PRICES = {
    "PROD-001": Decimal("10.50"),
    "PROD-002": Decimal("25.00"),
    "PROD-003": Decimal("5.99"),
    "PROD-004": Decimal("100.00"),
    "PROD-005": Decimal("15.75")
}

# Maximum YAML payload size to prevent DoS (10KB)
MAX_YAML_SIZE = 10240

# Pattern to validate product codes (alphanumeric and hyphens only)
PRODUCT_CODE_PATTERN = re.compile(r'^[a-zA-Z0-9-]{1,50}$')

def calculate_price(yaml_payload: str) -> str:
    """\n    Calculates price based on YAML payload.\n    Security measures:\n    - Input size validation to prevent DoS\n    - Safe YAML parsing using safe_load (no arbitrary code execution)\n    - Type validation for all extracted values\n    - Range validation for quantity\n    - Whitelist validation for product codes\n    - Overflow prevention using Decimal\n    - Generic error messages to prevent information leakage\n    """
    try:
        # Validate input is not None or empty
        if not yaml_payload or not yaml_payload.strip():
            return "Error: Invalid request"
        
        # Validate payload size to prevent DoS attacks
        if len(yaml_payload) > MAX_YAML_SIZE:
            return "Error: Request too large"
        
        # Parse YAML safely - safe_load prevents code execution
        # and does not resolve external entities
        try:
            data = yaml.safe_load(yaml_payload)
        except yaml.YAMLError:
            # Generic error message - don't leak parsing details\n            return "Error: Invalid format"\n        \n        # Validate data is a dictionary\n        if not isinstance(data, dict):\n            return "Error: Invalid format"\n        \n        # Validate required fields exist\n        if "product_code" not in data or "quantity" not in data:\n            return "Error: Missing required fields"\n        \n        # Extract and validate product_code\n        product_code = data.get("product_code")\n        if not isinstance(product_code, str):\n            return "Error: Invalid product code type"\n        \n        product_code = product_code.strip()\n        \n        # Validate product code format against whitelist pattern\n        if not PRODUCT_CODE_PATTERN.match(product_code):\n            return "Error: Invalid product code format"\n        \n        # Extract and validate quantity\n        quantity = data.get("quantity")\n        \n        # Handle quantity as int or string\n        if isinstance(quantity, int):\n            qty = quantity\n        elif isinstance(quantity, str):\n            try:\n                qty = int(quantity)\n            except ValueError:\n                return "Error: Invalid quantity format"\n        else:\n            return "Error: Invalid quantity type"\n        \n        # Validate quantity range (1-10000)\n        if qty < 1 or qty > 10000:\n            return "Error: Quantity out of range"\n        \n        # Check if product exists in catalog\n        if product_code not in PRODUCT_PRICES:\n            return "Error: Product not found"\n        \n        # Calculate total price using Decimal to prevent overflow\n        unit_price = PRODUCT_PRICES[product_code]\n        total_price = unit_price * Decimal(qty)\n        \n        # Return formatted result (2 decimal places)\n        return f"{total_price:.2f}"\n        \n    except Exception:\n        # Generic error - log details securely but don't expose to user
        # In production: log to secure logging system without sensitive data
        return "Error: Processing failed"

def main():
    print("Test Case 1 - Valid request:")
    test1 = "product_code: PROD-001\\nquantity: 5"
    print(f"Input: {test1.replace(chr(10), ' | ')}")
    print(f"Output: {calculate_price(test1)}")
    print()
    
    print("Test Case 2 - Different product:")
    test2 = "product_code: PROD-003\\nquantity: 10"
    print(f"Input: {test2.replace(chr(10), ' | ')}")
    print(f"Output: {calculate_price(test2)}")
    print()
    
    print("Test Case 3 - Invalid product code:")
    test3 = "product_code: INVALID\\nquantity: 2"
    print(f"Input: {test3.replace(chr(10), ' | ')}")
    print(f"Output: {calculate_price(test3)}")
    print()
    
    print("Test Case 4 - Quantity out of range:")
    test4 = "product_code: PROD-002\\nquantity: 20000"
    print(f"Input: {test4.replace(chr(10), ' | ')}")
    print(f"Output: {calculate_price(test4)}")
    print()
    
    print("Test Case 5 - Missing field:")
    test5 = "product_code: PROD-001"
    print(f"Input: {test5}")
    print(f"Output: {calculate_price(test5)}")

if __name__ == "__main__":
    main()
