// WARNING: This code is for educational purposes only and is NOT secure.
// It demonstrates a basic in-memory data storage concept.
// Do NOT use this for storing real sensitive data.
// Real-world applications require robust encryption, secure storage, and compliance with standards like PCI DSS.

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_USERS 10
#define MAX_NAME_LEN 50
#define MAX_EMAIL_LEN 100
#define MAX_CC_LEN 25

typedef struct {
    char name[MAX_NAME_LEN];
    char email[MAX_EMAIL_LEN];
    char creditCardNumber[MAX_CC_LEN];
} UserProfile;

// This struct acts as our simple in-memory, non-persistent data store.
typedef struct {
    UserProfile users[MAX_USERS];
    int count;
} UserDatabase;

/**
 * Stores a user profile in the in-memory database.
 * @param db Pointer to the UserDatabase struct.
 * @param user The UserProfile object to store.
 */
void storeUserProfile(UserDatabase* db, UserProfile user) {
    if (db->count < MAX_USERS) {
        db->users[db->count] = user;
        db->count++;
    } else {
        printf("Error: Database is full. Cannot store user %s.\n", user.name);
    }
}

/**
 * Retrieves a user profile from the in-memory database by name.
 * @param db Pointer to the UserDatabase struct.
 * @param username The name of the user to retrieve.
 * @return A pointer to the found UserProfile, or NULL if not found.
 */
UserProfile* retrieveUserProfile(UserDatabase* db, const char* username) {
    for (int i = 0; i < db->count; i++) {
        if (strcmp(db->users[i].name, username) == 0) {
            return &db->users[i];
        }
    }
    return NULL;
}

/**
 * Creates a masked version of the credit card number.
 * @param ccNumber The raw credit card number string.
 * @param maskedOutput The character buffer to store the masked string.
 */
void getMaskedCreditCardNumber(const char* ccNumber, char* maskedOutput) {
    size_t len = strlen(ccNumber);
    if (len <= 4) {
        strcpy(maskedOutput, ccNumber);
        return;
    }
    size_t i = 0;
    for (i = 0; i < len - 4; ++i) {
        if (isdigit(ccNumber[i])) {
            maskedOutput[i] = '*';
        } else {
            maskedOutput[i] = ccNumber[i];
        }
    }
    strcpy(maskedOutput + i, ccNumber + len - 4);
}

void displayUserProfile(const UserProfile* user) {
    if (user == NULL) {
        printf("Cannot display a NULL user.\n");
        return;
    }
    char maskedCC[MAX_CC_LEN];
    getMaskedCreditCardNumber(user->creditCardNumber, maskedCC);
    printf("UserProfile{name='%s', email='%s', creditCardNumber='%s'}\n", 
           user->name, user->email, maskedCC);
}

int main() {
    UserDatabase db = { .count = 0 };

    printf("--- Storing 5 User Profiles ---\n");

    // Test Case 1
    UserProfile user1;
    strcpy(user1.name, "Alice");
    strcpy(user1.email, "alice@example.com");
    strcpy(user1.creditCardNumber, "1111-2222-3333-4444");
    storeUserProfile(&db, user1);
    printf("Stored: %s\n", user1.name);

    // Test Case 2
    UserProfile user2;
    strcpy(user2.name, "Bob");
    strcpy(user2.email, "bob@example.com");
    strcpy(user2.creditCardNumber, "5555-6666-7777-8888");
    storeUserProfile(&db, user2);
    printf("Stored: %s\n", user2.name);

    // Test Case 3
    UserProfile user3;
    strcpy(user3.name, "Charlie");
    strcpy(user3.email, "charlie@example.com");
    strcpy(user3.creditCardNumber, "9999-8888-7777-6666");
    storeUserProfile(&db, user3);
    printf("Stored: %s\n", user3.name);

    // Test Case 4
    UserProfile user4;
    strcpy(user4.name, "Diana");
    strcpy(user4.email, "diana@example.com");
    strcpy(user4.creditCardNumber, "1234-5678-9012-3456");
    storeUserProfile(&db, user4);
    printf("Stored: %s\n", user4.name);

    // Test Case 5
    UserProfile user5;
    strcpy(user5.name, "Eve");
    strcpy(user5.email, "eve@example.com");
    strcpy(user5.creditCardNumber, "0000-1111-2222-3333");
    storeUserProfile(&db, user5);
    printf("Stored: %s\n", user5.name);

    printf("\n--- Retrieving User Profiles ---\n");

    // Retrieval Test 1
    printf("Retrieving 'Bob':\n");
    UserProfile* retrievedUser = retrieveUserProfile(&db, "Bob");
    if (retrievedUser != NULL) {
        printf("Found: ");
        displayUserProfile(retrievedUser);
    } else {
        printf("User 'Bob' not found.\n");
    }

    // Retrieval Test 2
    printf("\nRetrieving 'Diana':\n");
    retrievedUser = retrieveUserProfile(&db, "Diana");
    if (retrievedUser != NULL) {
        printf("Found: ");
        displayUserProfile(retrievedUser);
    } else {
        printf("User 'Diana' not found.\n");
    }

    // Retrieval Test 3 (User not found)
    printf("\nRetrieving 'Frank':\n");
    retrievedUser = retrieveUserProfile(&db, "Frank");
    if (retrievedUser != NULL) {
        printf("Found: ");
        displayUserProfile(retrievedUser);
    } else {
        printf("User 'Frank' not found.\n");
    }

    return 0;
}