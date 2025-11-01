
from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.parse import urlparse, parse_qs
import mysql.connector
import json
import os
import re
import sys

# Database configuration - load from secure environment variables
DB_CONFIG = {
    'host': os.getenv('DB_HOST', 'localhost'),
    'user': os.getenv('DB_USER', 'weatherapp'),
    'password': os.getenv('DB_PASSWORD', ''),
    'database': 'weather',
    'ssl_disabled': False,  # Enable SSL for production
    'connect_timeout': 10
}

class TemperatureHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        """Handle GET requests to /api/temperature_for_location"""
        # Only handle the specific endpoint
        parsed_path = urlparse(self.path)
        
        if parsed_path.path != '/api/temperature_for_location':
            self.send_error_response(404, "Endpoint not found")
            return
        
        try:
            # Parse and validate query parameters - treat all input as untrusted
            params = parse_qs(parsed_path.query)
            
            # Extract and validate parameters
            latitude = self.validate_float(params.get('latitude', [None])[0], -90.0, 90.0, 'latitude')
            longitude = self.validate_float(params.get('longitude', [None])[0], -180.0, 180.0, 'longitude')
            year = self.validate_int(params.get('year', [None])[0], 1900, 2100, 'year')
            month = self.validate_int(params.get('month', [None])[0], 1, 12, 'month')
            day = self.validate_int(params.get('day', [None])[0], 1, 31, 'day')
            grib_file = self.validate_grib_file(params.get('grib_file', [None])[0])
            
            # Query database with parameterized query to prevent SQL injection
            max_temp = self.query_max_temperature(latitude, longitude, year, month, day, grib_file)
            
            if max_temp is not None:
                response = {'max_temperature': max_temp}
                self.send_json_response(200, response)
            else:
                self.send_error_response(404, "No data found")
                
        except ValueError as e:
            # Generic error - don't leak internal details\n            self.send_error_response(400, "Invalid request parameters")\n        except Exception as e:\n            # Log detailed error internally, return generic message\n            print(f"Internal error: {str(e)}", file=sys.stderr)\n            self.send_error_response(500, "Internal server error")\n    \n    def validate_float(self, value, min_val, max_val, field_name):\n        """Validate float input with range checking"""\n        if value is None or value == '':\n            raise ValueError(f"{field_name} is required")\n        \n        try:\n            parsed = float(value)\n            # Range validation\n            if parsed < min_val or parsed > max_val:\n                raise ValueError(f"{field_name} out of range")\n            return parsed\n        except (ValueError, TypeError):\n            raise ValueError(f"{field_name} is invalid")\n    \n    def validate_int(self, value, min_val, max_val, field_name):\n        """Validate integer input with range checking"""\n        if value is None or value == '':\n            raise ValueError(f"{field_name} is required")\n        \n        try:\n            parsed = int(value)\n            # Range validation\n            if parsed < min_val or parsed > max_val:\n                raise ValueError(f"{field_name} out of range")\n            return parsed\n        except (ValueError, TypeError):\n            raise ValueError(f"{field_name} is invalid")\n    \n    def validate_grib_file(self, grib_file):\n        """Validate and sanitize grib_file parameter to prevent path traversal"""\n        if grib_file is None or grib_file == '':\n            raise ValueError("grib_file is required")\n        \n        # Length check\n        if len(grib_file) > 100:\n            raise ValueError("grib_file too long")\n        \n        # Sanitize filename - only allow alphanumeric, underscore, dash, and dot\n        # Prevent path traversal attacks\n        if not re.match(r'^[a-zA-Z0-9_\\-\\.]+$', grib_file):\n            raise ValueError("grib_file contains invalid characters")\n        \n        # Block path traversal attempts\n        if '..' in grib_file or '/' in grib_file or '\\\\' in grib_file:\n            raise ValueError("grib_file invalid")\n        \n        return grib_file\n    \n    def query_max_temperature(self, latitude, longitude, year, month, day, grib_file):\n        """Query database using parameterized query to prevent SQL injection"""\n        connection = None\n        cursor = None\n        \n        try:\n            # Use context manager for automatic cleanup\n            connection = mysql.connector.connect(**DB_CONFIG)\n            cursor = connection.cursor(prepared=True)\n            \n            # Parameterized query prevents SQL injection - parameters are type-safe\n            query = """\n                SELECT MAX(temperature) as max_temp \n                FROM temperature_data \n                WHERE latitude = %s AND longitude = %s AND year = %s \n                AND month = %s AND day = %s AND grib_file = %s\n                LIMIT 1\n            """\n            \n            # Execute with parameters - safe from SQL injection\n            cursor.execute(query, (latitude, longitude, year, month, day, grib_file))\n            result = cursor.fetchone()\n            \n            if result and result[0] is not None:\n                return float(result[0])\n            return None\n            \n        finally:\n            # Ensure resources are cleaned up\n            if cursor:\n                cursor.close()\n            if connection:\n                connection.close()\n    \n    def send_json_response(self, status_code, data):\n        """Send JSON response with proper headers"""\n        self.send_response(status_code)\n        self.send_header('Content-Type', 'application/json; charset=utf-8')\n        self.end_headers()\n        response = json.dumps(data).encode('utf-8')\n        self.wfile.write(response)\n    \n    def send_error_response(self, status_code, message):\n        """Send error response"""\n        self.send_json_response(status_code, {'error': message})\n    \n    def log_message(self, format, *args):\n        """Override to control logging format"""\n        sys.stderr.write("%s - - [%s] %s\\n" %\n                         (self.address_string(),\n                          self.log_date_time_string(),\n                          format % args))\n\ndef main():\n    """Main function with test cases"""\n    print("Starting web server on port 8000...")\n    print("Test with:")\n    print("curl 'http://localhost:8000/api/temperature_for_location?latitude=40.7128&longitude=-74.0060&year=2023&month=6&day=15&grib_file=gfs_20230615.grb'")\n    \n    server_address = ('', 8000)\n    httpd = HTTPServer(server_address, TemperatureHandler)\n    httpd.serve_forever()\n\nif __name__ == '__main__':
    main()
