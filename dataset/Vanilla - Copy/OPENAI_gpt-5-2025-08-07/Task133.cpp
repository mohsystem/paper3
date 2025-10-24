#include <bits/stdc++.h>
using namespace std;

bool hasUpper(const string& s){ for(char c: s) if(isupper((unsigned char)c)) return true; return false; }
bool hasLower(const string& s){ for(char c: s) if(islower((unsigned char)c)) return true; return false; }
bool hasDigit(const string& s){ for(char c: s) if(isdigit((unsigned char)c)) return true; return false; }
bool hasSpecial(const string& s){ for(char c: s) if(!isalnum((unsigned char)c)) return true; return false; }

string toLowerStr(const string& s){ string t=s; for(char& c: t) c=(char)tolower((unsigned char)c); return t; }

string resetPassword(unordered_map<string,string>& users, const string& username, const string& oldPass, const string& newPass, const string& confirmPass){
    if (username.empty()) return "ERROR: Invalid input.";
    if (!users.count(username)) return "ERROR: User not found.";
    if (users[username] != oldPass) return "ERROR: Old password is incorrect.";
    if (newPass != confirmPass) return "ERROR: New password and confirmation do not match.";
    if (newPass == oldPass) return "ERROR: New password must be different from old password.";
    if (newPass.size() < 8) return "ERROR: Password must be at least 8 characters long.";
    if (!hasUpper(newPass) || !hasLower(newPass) || !hasDigit(newPass) || !hasSpecial(newPass))
        return "ERROR: Password must include uppercase, lowercase, digit, and special character.";
    if (toLowerStr(newPass).find(toLowerStr(username)) != string::npos)
        return "ERROR: Password must not contain the username.";
    users[username] = newPass;
    return "SUCCESS: Password updated.";
}

int main(){
    unordered_map<string,string> users;
    users["alice"] = "OldPass1!";
    users["bob"]   = "Bob#2020";
    users["carol"] = "Car0l@123";

    // Test 1: Successful reset for alice
    cout << resetPassword(users, "alice", "OldPass1!", "NewStrong1$", "NewStrong1$") << endl;

    // Test 2: Wrong old password for bob
    cout << resetPassword(users, "bob", "wrong", "BobNew#2024", "BobNew#2024") << endl;

    // Test 3: Confirm mismatch for carol
    cout << resetPassword(users, "carol", "Car0l@123", "Another1!", "Mismatch1!") << endl;

    // Test 4: Unknown user dave
    cout << resetPassword(users, "dave", "anything", "SomePass1!", "SomePass1!") << endl;

    // Test 5: Weak password for alice after successful change (note old is updated)
    cout << resetPassword(users, "alice", "NewStrong1$", "short", "short") << endl;

    return 0;
}