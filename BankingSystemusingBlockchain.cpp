#include <bits/stdc++.h>
using namespace std;

#define TABLE_SIZE 10 // Hash table size

struct Block {
    int index{};
    string transactionID;
    string previousHash;
    time_t timestamp{};
    string data;
    string hash;
    Block* next{nullptr};
};

struct BSTNode {
    string transactionID;
    BSTNode *left{nullptr}, *right{nullptr};
};

struct Blockchain {
    Block* head{nullptr};
    int length{0};
    BSTNode* root{nullptr}; // BST of transaction IDs (not used elsewhere, retained)
} blockchain;

struct User {
    string accountNumber;
    string name;
    string mobile;
    string password;
    float balance{};
    User* next{nullptr}; // used both for the users list and hash buckets as in original
};

struct BankDatabase {
    User* users{nullptr};
    int nextAccountNumber{1};
    User* hashTable[TABLE_SIZE]{};
};

// ---------- Helpers ----------
static int hashFunction(const string& s) {
    unsigned long h = 5381;
    for (unsigned char c : s) h = h * 33 + c;
    return static_cast<int>(h % TABLE_SIZE);
}

static string computeHash(const string& s) {
    unsigned long h = 5381;
    for (unsigned char c : s) h = h * 33 + c;
    return to_string(h);
}

static BSTNode* insertBST(BSTNode* node, const string& transactionID) {
    if (!node) {
        auto* temp = new BSTNode();
        temp->transactionID = transactionID;
        return temp;
    }
    if (transactionID < node->transactionID) node->left = insertBST(node->left, transactionID);
    else node->right = insertBST(node->right, transactionID);
    return node;
}

static void addBlock(const string& data) {
    auto* newBlock = new Block();
    newBlock->index = blockchain.length++;
    newBlock->timestamp = time(nullptr);
    newBlock->data = data;
    newBlock->transactionID = string("TRX-") + to_string(newBlock->index);
    newBlock->hash = computeHash(data);

    if (!blockchain.head) {
        newBlock->previousHash = "0";
        blockchain.head = newBlock;
    } else {
        Block* cur = blockchain.head;
        while (cur->next) cur = cur->next;
        newBlock->previousHash = cur->hash;
        cur->next = newBlock;
    }
    blockchain.root = insertBST(blockchain.root, newBlock->transactionID);
}

static void initBankDatabase(BankDatabase* db) {
    db->users = nullptr;
    db->nextAccountNumber = 1;
    for (int i = 0; i < TABLE_SIZE; ++i) db->hashTable[i] = nullptr;
}

static User* createUser(BankDatabase* db, const string& name, const string& mobile,
                        const string& password, float initialDeposit) {
    auto* newUser = new (nothrow) User();
    if (!newUser) return nullptr;

    ostringstream acc;
    acc << "CSAGRP6A" << setw(3) << setfill('0') << db->nextAccountNumber++;
    newUser->accountNumber = acc.str();
    newUser->name = name;
    newUser->mobile = mobile;
    newUser->password = password;
    newUser->balance = initialDeposit;
    newUser->next = nullptr;

    // Append to users list (tail)
    if (!db->users) db->users = newUser;
    else {
        User* cur = db->users;
        while (cur->next) cur = cur->next;
        cur->next = newUser;
    }

    // Add to hash bucket (NOTE: uses same `next` pointer as original C code)
    int idx = hashFunction(newUser->accountNumber);
    newUser->next = db->hashTable[idx];
    db->hashTable[idx] = newUser;

    return newUser;
}

static User* findUser(BankDatabase* db, const string& accountNumber) {
    int idx = hashFunction(accountNumber);
    User* cur = db->hashTable[idx];
    while (cur) {
        if (cur->accountNumber == accountNumber) return cur;
        cur = cur->next;
    }
    return nullptr;
}

// ---------- CSV I/O ----------
static void saveUsersToCSV(BankDatabase* db, const string& filename) {
    ofstream file(filename);
    if (!file) {
        cerr << "Failed to open file: " << filename << "\n";
        return;
    }
    file << "AccountNumber,Name,Mobile,Password,Balance\n";
    // Iterating over db->users may be unreliable due to shared `next` usage; write via buckets for safety
    vector<User*> seen;
    seen.reserve(128);
    for (int i = 0; i < TABLE_SIZE; ++i) {
        for (User* cur = db->hashTable[i]; cur; cur = cur->next) {
            // avoid duplicates
            if (find(seen.begin(), seen.end(), cur) != seen.end()) continue;
            seen.push_back(cur);
            file << cur->accountNumber << ','
                 << cur->name << ','
                 << cur->mobile << ','
                 << cur->password << ','
                 << fixed << setprecision(2) << cur->balance << '\n';
        }
    }
}

static void ensureUsersCSVExists(const string& filename) {
    ifstream fin(filename);
    if (fin.good()) return;
    ofstream fout(filename);
    if (!fout) {
        cerr << "Failed to create file: " << filename << "\n";
        return;
    }
    fout << "AccountNumber,Name,Mobile,Password,Balance\n";
    cerr << "New file created: " << filename << "\n";
}

static void loadUsersFromCSV(BankDatabase* db, const string& filename) {
    ensureUsersCSVExists(filename);
    ifstream file(filename);
    if (!file) {
        cerr << "Failed to open file: " << filename << "\n";
        return;
    }
    string line;
    getline(file, line); // skip header
    while (getline(file, line)) {
        if (line.empty()) continue;
        // naive CSV split (no quoted commas)
        stringstream ss(line);
        string accountNumber, name, mobile, password, balanceStr;
        if (!getline(ss, accountNumber, ',')) continue;
        if (!getline(ss, name, ',')) continue;
        if (!getline(ss, mobile, ',')) continue;
        if (!getline(ss, password, ',')) continue;
        if (!getline(ss, balanceStr, ',')) continue;

        float balance = stof(balanceStr);
        User* user = createUser(db, name, mobile, password, balance);
        if (user) {
            user->accountNumber = accountNumber; // restore saved account number
        }
    }
}

static void saveTransactionsToCSV(const string& filename) {
    ofstream file(filename);
    if (!file) {
        cerr << "Failed to open file: " << filename << "\n";
        return;
    }
    file << "Index,TransactionID,PreviousHash,Timestamp,Data,Hash\n";
    for (Block* cur = blockchain.head; cur; cur = cur->next) {
        file << cur->index << ','
             << cur->transactionID << ','
             << cur->previousHash << ','
             << static_cast<long long>(cur->timestamp) << ','
             << cur->data << ','
             << cur->hash << '\n';
    }
}

static void ensureTxCSVExists(const string& filename) {
    ifstream fin(filename);
    if (fin.good()) return;
    ofstream fout(filename);
    if (!fout) {
        cerr << "Failed to create file: " << filename << "\n";
        return;
    }
    fout << "Index,TransactionID,PreviousHash,Timestamp,Data,Hash\n";
    cerr << "New transactions file created: " << filename << "\n";
}

static void loadTransactionsFromCSV(const string& filename) {
    ensureTxCSVExists(filename);
    ifstream file(filename);
    if (!file) {
        cerr << "Failed to open file: " << filename << "\n";
        return;
    }
    string line;
    getline(file, line); // header
    // We will rebuild the linked list in the order found (assumed already chronological)
    Block* tail = nullptr;
    while (getline(file, line)) {
        if (line.empty()) continue;
        // naive split by comma; note: 'data' must not contain commas to be safe
        stringstream ss(line);
        string idxStr, txid, prev, tsStr, data, h;
        if (!getline(ss, idxStr, ',')) continue;
        if (!getline(ss, txid, ',')) continue;
        if (!getline(ss, prev, ',')) continue;
        if (!getline(ss, tsStr, ',')) continue;
        if (!getline(ss, data, ',')) continue;
        if (!getline(ss, h, ',')) continue;

        auto* b = new Block();
        b->index = stoi(idxStr);
        b->transactionID = txid;
        b->previousHash = prev;
        b->timestamp = static_cast<time_t>(stoll(tsStr));
        b->data = data;
        b->hash = h;
        b->next = nullptr;

        if (!blockchain.head) {
            blockchain.head = b;
            tail = b;
        } else {
            tail->next = b;
            tail = b;
        }
        blockchain.length = max(blockchain.length, b->index + 1);
        blockchain.root = insertBST(blockchain.root, b->transactionID);
    }
}

// ---------- Banking ops ----------
static void printUsers(BankDatabase* db) {
    cout << "List of Users:\n";
    // safer to iterate buckets to avoid `next` collision issues
    vector<User*> seen;
    seen.reserve(128);
    for (int i = 0; i < TABLE_SIZE; ++i) {
        for (User* cur = db->hashTable[i]; cur; cur = cur->next) {
            if (find(seen.begin(), seen.end(), cur) != seen.end()) continue;
            seen.push_back(cur);
            cout << "Account #" << cur->accountNumber
                 << ": " << cur->name
                 << ", Mobile: " << cur->mobile
                 << ", Balance: Rs." << fixed << setprecision(2) << cur->balance
                 << "\n";
        }
    }
}

static bool authenticateUser(BankDatabase* db, const string& accountNumber, const string& password) {
    User* user = findUser(db, accountNumber);
    return (user && user->password == password);
}

static void transaction(BankDatabase* db, const string& accountNumber, float amount, int type) {
    string password;
    cout << "Enter password for account " << accountNumber << ": ";
    cin >> password;

    if (!authenticateUser(db, accountNumber, password)) {
        cout << "Authentication failed. Transaction aborted.\n";
        return;
    }

    User* user = findUser(db, accountNumber);
    if (!user) {
        cout << "Account number " << accountNumber << " not found.\n";
        return;
    }

    string data;
    if (type == 1) { // Deposit
        user->balance += amount;
        {
            ostringstream oss;
            oss << "Deposited Rs." << fixed << setprecision(2) << amount
                << " to " << user->accountNumber
                << ". New Balance: Rs." << fixed << setprecision(2) << user->balance;
            data = oss.str();
        }
        cout << "Rs." << fixed << setprecision(2) << amount
             << " deposited to Account #" << user->accountNumber
             << ". New Balance: Rs." << fixed << setprecision(2) << user->balance << "\n";
    } else if (type == 2) { // Withdrawal
        if (user->balance >= amount) {
            user->balance -= amount;
            {
                ostringstream oss;
                oss << "Withdrawn Rs." << fixed << setprecision(2) << amount
                    << " from " << user->accountNumber
                    << ". New Balance: Rs." << fixed << setprecision(2) << user->balance;
                data = oss.str();
            }
            cout << "Rs." << fixed << setprecision(2) << amount
                 << " withdrawn from Account #" << user->accountNumber
                 << ". New Balance: Rs." << fixed << setprecision(2) << user->balance << "\n";
        } else {
            cout << "Insufficient funds for withdrawal.\n";
            return;
        }
    } else {
        cout << "Invalid transaction type.\n";
        return;
    }
    addBlock(data);
}

static void transfer(BankDatabase* db, const string& fromAccount, const string& toAccount, float amount) {
    string password;
    cout << "Enter password for account " << fromAccount << ": ";
    cin >> password;

    if (!authenticateUser(db, fromAccount, password)) {
        cout << "Authentication failed. Transfer aborted.\n";
        return;
    }

    User* fromUser = findUser(db, fromAccount);
    User* toUser = findUser(db, toAccount);

    if (!fromUser || !toUser) {
        cout << "One or both account numbers not found.\n";
        return;
    }

    if (fromUser->balance >= amount) {
        fromUser->balance -= amount;
        toUser->balance += amount;

        ostringstream oss;
        oss << "Transferred Rs." << fixed << setprecision(2) << amount
            << " from " << fromUser->accountNumber
            << " to " << toUser->accountNumber;
        string data = oss.str();

        cout << "Rs." << fixed << setprecision(2) << amount
             << " transferred from Account #" << fromAccount
             << " to Account #" << toAccount << "\n";
        addBlock(data);
    } else {
        cout << "Insufficient funds in source account.\n";
    }
}

// ---------- Menu ----------
static void menu() {
    // Choose relative CSV paths for portability
    const string USERS_CSV = "users.csv";
    const string TX_CSV = "transactions.csv";

    BankDatabase db;
    initBankDatabase(&db);
    loadUsersFromCSV(&db, USERS_CSV);
    loadTransactionsFromCSV(TX_CSV);

    int choice;
    string accountNumber;
    float amount;
    string name, mobile, password, confirmPassword;

    do {
        cout << "\n--- Bank Menu ---\n";
        cout << "1. Create Account\n";
        cout << "2. Deposit Money\n";
        cout << "3. Withdraw Money\n";
        cout << "4. Transfer Money\n";
        cout << "5. View Accounts\n";
        cout << "6. Exit\n";
        cout << "Choose an option: ";
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input.\n";
            continue;
        }

        switch (choice) {
            case 1: {
                cout << "Enter name: ";
                cin >> name; // single-token like original
                cout << "Enter mobile number: ";
                cin >> mobile;
                if (mobile.size() != 10) {
                    cout << "Error: Mobile number must be exactly 10 digits long.\n";
                    break;
                }
                cout << "Create password: ";
                cin >> password;
                cout << "Confirm password: ";
                cin >> confirmPassword;
                if (password != confirmPassword) {
                    cout << "Passwords do not match. Account creation failed.\n";
                    break;
                }
                cout << "Initial deposit: ";
                cin >> amount;

                User* user = createUser(&db, name, mobile, password, amount);
                if (user) {
                    cout << "Account created successfully. Account Number: " << user->accountNumber << "\n";
                    ostringstream oss;
                    oss << "Created account for " << name
                        << " with initial deposit of Rs." << fixed << setprecision(2) << amount
                        << ". Account Number: " << user->accountNumber;
                    addBlock(oss.str());
                    saveUsersToCSV(&db, USERS_CSV);
                    saveTransactionsToCSV(TX_CSV);
                }
                break;
            }
            case 2: {
                cout << "Enter account number: ";
                cin >> accountNumber;
                cout << "Enter amount to deposit: ";
                cin >> amount;
                transaction(&db, accountNumber, amount, 1);
                break;
            }
            case 3: {
                cout << "Enter account number: ";
                cin >> accountNumber;
                cout << "Enter amount to withdraw: ";
                cin >> amount;
                transaction(&db, accountNumber, amount, 2);
                break;
            }
            case 4: {
                string toAccount;
                cout << "Enter from account number: ";
                cin >> accountNumber;
                cout << "Enter to account number: ";
                cin >> toAccount;
                cout << "Enter amount to transfer: ";
                cin >> amount;
                transfer(&db, accountNumber, toAccount, amount);
                saveUsersToCSV(&db, USERS_CSV);
                saveTransactionsToCSV(TX_CSV);
                break;
            }
            case 5:
                printUsers(&db);
                break;
            case 6:
                cout << "Exiting and saving data...\n";
                saveUsersToCSV(&db, USERS_CSV);
                saveTransactionsToCSV(TX_CSV);
                cout << "Data saved. Exiting program.\n";
                break;
            default:
                cout << "Invalid option.\n";
        }
    } while (choice != 6);
}

int main() {
    blockchain.head = nullptr;
    blockchain.length = 0;
    menu();
    return 0;
}
