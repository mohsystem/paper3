
import random
import string
from datetime import datetime, timedelta

class Task54:
    def __init__(self):
        self.users = {}
        self.otp_store = {}
        self.OTP_EXPIRY_MINUTES = 5
        self.OTP_LENGTH = 6
    
    class User:
        def __init__(self, username, password, email):
            self.username = username
            self.password = password
            self.email = email
    
    class OTP:
        def __init__(self, code, generated_at):
            self.code = code
            self.generated_at = generated_at
        
        def is_expired(self, expiry_minutes):
            return (datetime.now() - self.generated_at).total_seconds() / 60 >= expiry_minutes
    
    def generate_otp(self):
        return ''.join([str(random.randint(0, 9)) for _ in range(self.OTP_LENGTH)])
    
    def register_user(self, username, password, email):
        if username in self.users:
            return False
        self.users[username] = self.User(username, password, email)
        return True
    
    def authenticate_credentials(self, username, password):
        if username not in self.users or self.users[username].password != password:
            return None
        
        otp = self.generate_otp()
        self.otp_store[username] = self.OTP(otp, datetime.now())
        return otp
    
    def verify_otp(self, username, entered_otp):
        if username not in self.otp_store:
            return False
        
        otp = self.otp_store[username]
        
        if otp.is_expired(self.OTP_EXPIRY_MINUTES):
            del self.otp_store[username]
            return False
        
        if otp.code == entered_otp:
            del self.otp_store[username]
            return True
        
        return False
    
    def perform_login(self, username, password, entered_otp):
        generated_otp = self.authenticate_credentials(username, password)
        if generated_otp is None:
            return "Login Failed: Invalid credentials"
        
        if self.verify_otp(username, entered_otp):
            return "Login Successful"
        else:
            return "Login Failed: Invalid or expired OTP"


if __name__ == "__main__":
    auth_system = Task54()
    
    print("=== Two-Factor Authentication System ===\\n")
    
    # Test Case 1: Successful registration and login
    print("Test Case 1: Successful registration and login")
    auth_system.register_user("user1", "pass123", "user1@example.com")
    otp1 = auth_system.authenticate_credentials("user1", "pass123")
    print(f"Generated OTP: {otp1}")
    result1 = auth_system.perform_login("user1", "pass123", otp1)
    print(f"Result: {result1}\\n")
    
    # Test Case 2: Wrong password
    print("Test Case 2: Wrong password")
    auth_system.register_user("user2", "pass456", "user2@example.com")
    result2 = auth_system.perform_login("user2", "wrongpass", "123456")
    print(f"Result: {result2}\\n")
    
    # Test Case 3: Wrong OTP
    print("Test Case 3: Wrong OTP")
    auth_system.register_user("user3", "pass789", "user3@example.com")
    otp3 = auth_system.authenticate_credentials("user3", "pass789")
    print(f"Generated OTP: {otp3}")
    result3 = auth_system.perform_login("user3", "pass789", "000000")
    print(f"Result: {result3}\\n")
    
    # Test Case 4: User doesn't exist\n    print("Test Case 4: User doesn't exist")\n    result4 = auth_system.perform_login("nonexistent", "pass000", "123456")\n    print(f"Result: {result4}\
")\n    \n    # Test Case 5: Multiple users with successful login\n    print("Test Case 5: Multiple users with successful login")\n    auth_system.register_user("user4", "secure123", "user4@example.com")\n    auth_system.register_user("user5", "secure456", "user5@example.com")\n    otp4 = auth_system.authenticate_credentials("user4", "secure123")\n    otp5 = auth_system.authenticate_credentials("user5", "secure456")\n    print(f"User4 OTP: {otp4}")\n    print(f"User5 OTP: {otp5}")\n    result5a = auth_system.perform_login("user4", "secure123", otp4)\n    result5b = auth_system.perform_login("user5", "secure456", otp5)\n    print(f"User4 Result: {result5a}")\n    print(f"User5 Result: {result5b}")
