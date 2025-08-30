# Blockchain-based-Banking-System-in-CPP

📌 Overview

The Blockchain-Based Banking System is a C/C++ project that demonstrates how blockchain concepts can be applied to a banking system.
It simulates a secure, transparent, and immutable ledger for banking transactions (account creation, deposits, withdrawals, and fund transfers).

The project leverages blockchain data structures, hashing, cryptography, and file handling to ensure:

Data integrity

User authentication

Secure transaction recording

Persistent storage in CSV files

🚀 Features

Account Management: Create and manage multiple user accounts.

Transaction Processing: Deposit, withdraw, and transfer funds securely.

Blockchain Integration: Every transaction is stored as a block with a cryptographic hash.

User Authentication: Password-protected accounts.

Data Persistence: Accounts and transactions are saved in CSV files.

Security & Integrity: Tamper-proof ledger using cryptographic hashing.

Scalability & Error Handling: Handles multiple accounts with validation checks.

🛠️ Concepts Used

Blockchain Technology: Immutable ledger with linked blocks.

Data Structures: Linked Lists, Binary Search Trees, Hash Tables.

File Handling: Save/Load users and transactions (users.csv, transactions.csv).

Cryptography: Hashing for transaction integrity (computeHash).

Dynamic Memory Allocation: Efficient resource usage (malloc, free).

Input Validation & Error Handling: Ensures safe and reliable operations.

Time Handling: Records timestamps for each transaction.


# Steps to Run

# Clone this repository
git clone https://github.com/<your-username>/Blockchain-Banking-System.git

# Navigate to project folder
cd Blockchain-Banking-System

# Compile the code (C version)
gcc main.c -o banking

# OR for C++ version
g++ main.cpp -o banking

# Run the program
./banking
