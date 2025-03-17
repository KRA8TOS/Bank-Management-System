# Bank-Management-System
This is a Bank Management System implemented in C++ with relational database MYSQL. This project has multiple functionalities like customer management,Account management and transaction managment with data authentication and security. This project follows the SOLID principles of OOPs and ensure modularity concise clear and very efficient program.

## Project Overview
The Bank Management System is a console-based application developed in C++ that manages banking operations efficiently. It allows users to perform various functionalities such as account creation, balance inquiry, fund transfers, and transaction history management. This project aims to provide a user-friendly interface for managing banking tasks while ensuring data integrity and security.

## Features
- **Account Creation**: Users can create new bank accounts by providing necessary details such as name, initial deposit, and account type (savings or checking).
- **Balance Inquiry**: Users can check their account balance at any time.
- **Fund Transfer**: Users can securely transfer funds between accounts.
- **Transaction History**: Users can view their transaction history, including deposits, withdrawals, and transfers.
- **Account Closure**: Users can close their accounts when needed.

## Requirements
- C++ Compiler (e.g., g++, clang++)
- MySQL Server
- MySQL Connector/C++ (for database connectivity)
- CMake (optional, for building the project)

## How to Run the Project


1. **Clone the Repository**:
   ```bash
   git clone <repository-url>
   cd Bank-Management-System
   ```

2. **Set Up MySQL Database**:
   - Ensure you have MySQL installed and running.
   - Create a database named `bank`:
     ```sql
     CREATE DATABASE bank;
     ```
   - Run the provided SQL scripts to create the necessary tables (customers, accounts, transactions, etc.).

Process 1:

3. **Install MySQL Connector/C++**:
   - Follow the installation instructions for the MySQL Connector/C++ to enable database connectivity.

4. **Compile the Application**:
   If using CMake, create a build directory and run:
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

5. **Run the Application**:
   Execute the compiled binary:
   ```bash
   ./BankManagementSystem
   ```

6. **Follow On-Screen Instructions**:
   The application will guide you through the available functionalities. Follow the prompts to perform banking operations.


Process 2 :

1.Open Terminal :

 cd '.\Bank Management System\'


2.Run this command :

g++ -o main  main.cpp -I"C:\Program Files\MySQL\MySQL Server 8.0\include" -L"C:\Program Files\MySQL\MySQL Server 8.0\lib" -lmysql

Run this command if you are using mingw compiler

3. Execute the program :

.\main.exe

END!!!!

## Usage Example
- To create an account, select the option from the main menu and enter the required details.
- To transfer funds, select the transfer option, input the account numbers and amount, and confirm the transaction.

## Contributing
Contributions are welcome! Please feel free to submit a pull request or open an issue for any suggestions or improvements.

## Acknowledgments
- Devendra Kumar Dewangan
- Any resources or libraries used in the project.

##Contact :
Email :- ankit2004dewangan@gmail.com
