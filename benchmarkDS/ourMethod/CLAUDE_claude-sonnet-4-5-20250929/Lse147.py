
import sqlite3
import os
import re
from typing import Optional

# Input validation: Whitelist pattern for stock names (alphanumeric and common symbols)
STOCK_NAME_PATTERN = re.compile(r'^[A-Z0-9._-]{1,10}$')
MAX_QUANTITY = 1000000
MIN_QUANTITY = 1

def get_connection() -> sqlite3.Connection:
    """\n    Secure database connection using environment variables (avoid hardcoded credentials - CWE-798)\n    """
    db_path = os.getenv('DB_PATH', 'stocks.db')
    # Validate db_path to prevent path traversal (CWE-22)
    if '..' in db_path or db_path.startswith('/'):
        raise ValueError("Invalid database path")
    
    conn = sqlite3.connect(db_path, check_same_thread=False)
    conn.row_factory = sqlite3.Row
    return conn

def is_valid_stock_name(stock_name: Optional[str]) -> bool:
    """\n    Input validation function to prevent injection attacks (CWE-89)\n    """
    if not stock_name or not isinstance(stock_name, str):
        return False
    return bool(STOCK_NAME_PATTERN.match(stock_name))

def is_valid_quantity(quantity: int) -> bool:
    """\n    Validate quantity is within acceptable range\n    """
    if not isinstance(quantity, int):
        return False
    return MIN_QUANTITY <= quantity <= MAX_QUANTITY

def buy_order(stock_name: str, quantity: int) -> bool:
    """\n    Secure buy order function using parameterized queries (prevents SQL injection)\n    \n    Args:\n        stock_name: Stock symbol to purchase\n        quantity: Number of shares to purchase\n    \n    Returns:\n        bool: True if order successful, False otherwise\n    """
    # Input validation: Check all inputs before processing (Rules#3)
    if not is_valid_stock_name(stock_name):
        print("Invalid stock name format", file=os.sys.stderr)
        return False
    
    if not is_valid_quantity(quantity):
        print("Invalid quantity range", file=os.sys.stderr)
        return False
    
    conn = None
    try:
        conn = get_connection()
        cursor = conn.cursor()
        
        # Use parameterized query to prevent SQL injection (CWE-89)
        sql = "INSERT INTO orders (stock_name, quantity, order_date) VALUES (?, ?, datetime('now'))"
        cursor.execute(sql, (stock_name, quantity))
        
        conn.commit()
        
        if cursor.rowcount > 0:
            # Log success without exposing sensitive data (Rules#1)
            print("Order placed successfully")
            return True
        else:
            print("Order insertion failed", file=os.sys.stderr)
            return False
            
    except sqlite3.Error as e:
        # Error handling: Don't expose internal details to user (Rules#1)\n        print("Database error occurred", file=os.sys.stderr)\n        # In production, log detailed error to secure log only\n        print(f"Details: {str(e)}", file=os.sys.stderr)\n        return False\n    except Exception as e:\n        print("Unexpected error occurred", file=os.sys.stderr)\n        print(f"Details: {str(e)}", file=os.sys.stderr)\n        return False\n    finally:\n        # Resource cleanup: Always close connection\n        if conn:\n            conn.close()\n\ndef initialize_database() -> None:\n    """\n    Initialize database table if it doesn't exist
    """\n    try:\n        conn = get_connection()\n        cursor = conn.cursor()\n        \n        create_table = """
        CREATE TABLE IF NOT EXISTS orders (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            stock_name TEXT NOT NULL,
            quantity INTEGER NOT NULL,
            order_date TEXT NOT NULL
        )
        """\n        cursor.execute(create_table)\n        conn.commit()\n        conn.close()\n        \n    except Exception as e:\n        print("Database initialization failed", file=os.sys.stderr)\n        print(f"Details: {str(e)}", file=os.sys.stderr)\n\nif __name__ == "__main__":\n    # Initialize database\n    initialize_database()\n    \n    # Test cases with validation\n    print("Test Case 1: Valid order")\n    print(f"Result: {buy_order('AAPL', 100)}")\n    \n    print("\
Test Case 2: Valid order with different stock")\n    print(f"Result: {buy_order('GOOGL', 50)}")\n    \n    print("\
Test Case 3: Invalid stock name (SQL injection attempt)")\n    print(f"Result: {buy_order(\\"'; DROP TABLE orders; --\\", 10)}")
    
    print("\\nTest Case 4: Invalid quantity (negative)")
    print(f"Result: {buy_order('MSFT', -5)}")
    
    print("\\nTest Case 5: Invalid quantity (exceeds maximum)")
    print(f"Result: {buy_order('TSLA', 2000000)}")
