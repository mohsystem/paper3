
import yaml
import re
from typing import Dict, Any

# Maximum payload size to prevent memory exhaustion (1KB)
MAX_PAYLOAD_SIZE = 1024

# Pattern to validate product codes (alphanumeric, 1-20 chars)
PRODUCT_CODE_PATTERN = re.compile(r'^[a-zA-Z0-9]{1,20}$')

# Maximum quantity to prevent integer overflow
MAX_QUANTITY = 1000000

# Hardcoded price database (in production, use a secure database)
PRICE_DATABASE = {
    'PROD001': 10.50,
    'PROD002': 25.99,
    'PROD003': 5.75,
    'PROD004': 100.00,
    'PROD005': 15.25
}


def get_price_for_product_code(product_code: str) -> float:
    """\n    Returns the price for a given product code.\n    Uses input validation to prevent injection attacks.\n    """
    # Input validation: check None and empty
    if not product_code or not isinstance(product_code, str):
        raise ValueError("Product code cannot be null or empty")
    
    product_code = product_code.strip()
    
    # Input validation: check format (alphanumeric only)
    if not PRODUCT_CODE_PATTERN.match(product_code):
        raise ValueError("Invalid product code format")
    
    # Check if product exists
    if product_code not in PRICE_DATABASE:
        raise ValueError("Product code not found")
    
    return PRICE_DATABASE[product_code]


def calculate_price(yaml_payload: str) -> str:
    """\n    Calculates total price from YAML payload.\n    Uses safe_load to prevent arbitrary code execution during YAML deserialization.\n    """
    try:
        # Input validation: check None and size limits
        if yaml_payload is None:
            return create_error_response("Payload cannot be null")
        
        if len(yaml_payload) > MAX_PAYLOAD_SIZE:
            return create_error_response("Payload exceeds maximum size")
        
        # Use safe_load to prevent arbitrary code execution (CWE-502)
        # safe_load only constructs simple Python objects (dict, list, str, int, float, bool, None)
        data = yaml.safe_load(yaml_payload)
        
        # Type validation: ensure it's a dict\n        if not isinstance(data, dict):\n            return create_error_response("Invalid payload format")\n        \n        # Extract and validate product_code\n        product_code = data.get('product_code')\n        if not product_code or not isinstance(product_code, str):\n            return create_error_response("Missing or invalid product_code")\n        \n        product_code = product_code.strip()\n        \n        # Extract and validate quantity\n        quantity = data.get('quantity')\n        if quantity is None:\n            return create_error_response("Missing quantity")\n        \n        # Convert to integer if needed\n        try:\n            if isinstance(quantity, str):\n                quantity = int(quantity)\n            elif not isinstance(quantity, int):\n                return create_error_response("Invalid quantity type")\n        except (ValueError, TypeError):\n            return create_error_response("Invalid quantity format")\n        \n        # Validate quantity range\n        if quantity <= 0 or quantity > MAX_QUANTITY:\n            return create_error_response("Quantity out of valid range")\n        \n        # Get price for product code (with validation inside)\n        unit_price = get_price_for_product_code(product_code)\n        \n        # Calculate total with overflow check\n        total_price = unit_price * quantity\n        \n        # Check for overflow\n        if total_price > 1e15:  # Reasonable upper limit\n            return create_error_response("Calculation overflow")\n        \n        # Create response with safe YAML dump\n        return create_success_response(total_price)\n        \n    except ValueError:\n        # Return generic error without exposing internal details\n        return create_error_response("Invalid input")\n    except yaml.YAMLError:\n        return create_error_response("Invalid YAML format")\n    except Exception:\n        # Log detailed error internally (not shown here)\n        # Return generic error to client\n        return create_error_response("Processing error")\n\n\ndef create_success_response(total_price: float) -> str:\n    """Creates a safe YAML success response"""\n    response = {\n        'status': 'success',\n        'total_price': f'{total_price:.2f}'\n    }\n    return yaml.dump(response, default_flow_style=False)\n\n\ndef create_error_response(message: str) -> str:\n    """Creates a safe YAML error response without leaking details"""\n    response = {\n        'status': 'error',\n        'message': message
    }
    return yaml.dump(response, default_flow_style=False)


def main():
    print("Test Case 1: Valid input")
    payload1 = "product_code: PROD001\\nquantity: 5"
    print(calculate_price(payload1))
    
    print("\\nTest Case 2: Different product")
    payload2 = "product_code: PROD003\\nquantity: 10"
    print(calculate_price(payload2))
    
    print("\\nTest Case 3: Invalid product code")
    payload3 = "product_code: INVALID\\nquantity: 5"
    print(calculate_price(payload3))
    
    print("\\nTest Case 4: Negative quantity")
    payload4 = "product_code: PROD002\\nquantity: -5"
    print(calculate_price(payload4))
    
    print("\\nTest Case 5: Missing quantity")
    payload5 = "product_code: PROD001"
    print(calculate_price(payload5))


if __name__ == "__main__":
    main()
