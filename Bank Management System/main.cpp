#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <mysql.h>
#include <ctime>
#include <iomanip>
#include <functional>
#include <thread>
#include <chrono>

// Configuration for database connection
struct DBConfig {
    const char* host;
    const char* user;
    const char* password;
    const char* database;
    unsigned int port;

    DBConfig() : host("localhost"), user("root"), password("030910"), 
                 database("bank"), port(3306) {}
};

// Interface for database operations - follows Interface Segregation Principle
class IDatabase {
public:
    virtual ~IDatabase() {}
    virtual bool connect() = 0;
    virtual bool disconnect() = 0;
    virtual bool executeQuery(const std::string& query) = 0;
    virtual bool executeQuery(const std::string& query, std::vector<std::vector<std::string>>& results) = 0;
};

// MySQL database implementation - follows Single Responsibility Principle
class MySQLDatabase : public IDatabase {
private:
    MYSQL* connection;
    DBConfig config;
    
public:
    MySQLDatabase(const DBConfig& cfg) : connection(nullptr), config(cfg) {}
    
    ~MySQLDatabase() {
        disconnect();
    }
    
    bool connect() override {
        connection = mysql_init(nullptr);
        
        if (!connection) {
            std::cerr << "MySQL initialization failed" << std::endl;
            return false;
        }
        
        // Establish connection to MySQL server
        if (!mysql_real_connect(connection, 
                               config.host, 
                               config.user, 
                               config.password, 
                               config.database, 
                               config.port, 
                               nullptr, 0)) {
            std::cerr << "Connection error: " << mysql_error(connection) << std::endl;
            mysql_close(connection);
            connection = nullptr;
            return false;
        }
        
        return true;
    }
    
    bool disconnect() override {
        if (connection) {
            mysql_close(connection);
            connection = nullptr;
        }
        return true;
    }
    
    bool executeQuery(const std::string& query) override {
        if (!connection) {
            std::cerr << "Not connected to database" << std::endl;
            return false;
        }
        
        if (mysql_query(connection, query.c_str())) {
            std::cerr << "Query execution error: " << mysql_error(connection) << std::endl;
            return false;
        }
        
        return true;
    }
    
    bool executeQuery(const std::string& query, std::vector<std::vector<std::string>>& results) override {
        results.clear();
        
        if (!connection) {
            std::cerr << "Not connected to database" << std::endl;
            return false;
        }
        
        if (mysql_query(connection, query.c_str())) {
            std::cerr << "Query execution error: " << mysql_error(connection) << std::endl;
            return false;
        }
        
        MYSQL_RES* result = mysql_store_result(connection);
        
        if (!result) {
            if (mysql_field_count(connection) == 0) {
                // Query does not return data (e.g., INSERT, UPDATE, DELETE)
                return true;
            } else {
                std::cerr << "Failed to retrieve result set: " << mysql_error(connection) << std::endl;
                return false;
            }
        }
        
        // Get number of fields
        int numFields = mysql_num_fields(result);
        
        // Fetch all rows
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result))) {
            std::vector<std::string> rowData;
            
            for (int i = 0; i < numFields; i++) {
                rowData.push_back(row[i] ? row[i] : "NULL");
            }
            
            results.push_back(rowData);
        }
        
        mysql_free_result(result);
        return true;
    }
};

// Base entity class for all bank entities
class Entity {
protected:
    int id;
    
public:
    Entity(int id = 0) : id(id) {}
    virtual ~Entity() {}
    
    int getId() const { return id; }
    void setId(int id) { this->id = id; }
    
    virtual void display() const = 0;
};

// Customer entity
class Customer : public Entity {
private:
    std::string name;
    std::string address;
    std::string phone;
    std::string email;
    
public:
    Customer(int id = 0, const std::string& name = "", const std::string& address = "",
             const std::string& phone = "", const std::string& email = "")
        : Entity(id), name(name), address(address), phone(phone), email(email) {}
    
    std::string getName() const { return name; }
    void setName(const std::string& name) { this->name = name; }
    
    std::string getAddress() const { return address; }
    void setAddress(const std::string& address) { this->address = address; }
    
    std::string getPhone() const { return phone; }
    void setPhone(const std::string& phone) { this->phone = phone; }
    
    std::string getEmail() const { return email; }
    void setEmail(const std::string& email) { this->email = email; }
    
    void display() const override {
        std::cout << "Customer ID: " << id << std::endl;
        std::cout << "Name: " << name << std::endl;
        std::cout << "Address: " << address << std::endl;
        std::cout << "Phone: " << phone << std::endl;
        std::cout << "Email: " << email << std::endl;
    }
};

// Account entity
class Account : public Entity {
protected:
    int customerId;
    double balance;
    std::string accountNumber;
    std::string accountType;
    std::string dateOpened;
    
public:
    Account(int id = 0, int customerId = 0, double balance = 0.0,
            const std::string& accountNumber = "", const std::string& accountType = "",
            const std::string& dateOpened = "")
        : Entity(id), customerId(customerId), balance(balance), 
          accountNumber(accountNumber), accountType(accountType), dateOpened(dateOpened) {}
    
    virtual ~Account() {}
    
    int getCustomerId() const { return customerId; }
    void setCustomerId(int customerId) { this->customerId = customerId; }
    
    double getBalance() const { return balance; }
    void setBalance(double balance) { this->balance = balance; }
    
    std::string getAccountNumber() const { return accountNumber; }
    void setAccountNumber(const std::string& accountNumber) { this->accountNumber = accountNumber; }
    
    std::string getAccountType() const { return accountType; }
    void setAccountType(const std::string& accountType) { this->accountType = accountType; }
    
    std::string getDateOpened() const { return dateOpened; }
    void setDateOpened(const std::string& dateOpened) { this->dateOpened = dateOpened; }
    
    void display() const override {
        std::cout << "Account ID: " << id << std::endl;
        std::cout << "Customer ID: " << customerId << std::endl;
        std::cout << "Account Number: " << accountNumber << std::endl;
        std::cout << "Account Type: " << accountType << std::endl;
        std::cout << "Balance: $" << std::fixed << std::setprecision(2) << balance << std::endl;
        std::cout << "Date Opened: " << dateOpened << std::endl;
    }
    
    virtual bool deposit(double amount) {
        if (amount <= 0) {
            std::cerr << "Invalid deposit amount" << std::endl;
            return false;
        }
        
        balance += amount;
        return true;
    }
    
    virtual bool withdraw(double amount) {
        if (amount <= 0) {
            std::cerr << "Invalid withdrawal amount" << std::endl;
            return false;
        }
        
        if (amount > balance) {
            std::cerr << "Insufficient funds" << std::endl;
            return false;
        }
        
        balance -= amount;
        return true;
    }
};

// Savings Account - follows Open/Closed Principle
class SavingsAccount : public Account {
private:
    double interestRate;
    
public:
    SavingsAccount(int id = 0, int customerId = 0, double balance = 0.0,
                  const std::string& accountNumber = "", const std::string& dateOpened = "",
                  double interestRate = 0.0)
        : Account(id, customerId, balance, accountNumber, "Savings", dateOpened), 
          interestRate(interestRate) {}
    
    double getInterestRate() const { return interestRate; }
    void setInterestRate(double rate) { interestRate = rate; }
    
    void calculateInterest() {
        double interest = balance * interestRate / 100;
        balance += interest;
    }
    
    void display() const override {
        Account::display();
        std::cout << "Interest Rate: " << interestRate << "%" << std::endl;
    }
};

// Checking Account - follows Open/Closed Principle
class CheckingAccount : public Account {
private:
    double overdraftLimit;
    
public:
    CheckingAccount(int id = 0, int customerId = 0, double balance = 0.0,
                   const std::string& accountNumber = "", const std::string& dateOpened = "",
                   double overdraftLimit = 0.0)
        : Account(id, customerId, balance, accountNumber, "Checking", dateOpened), 
          overdraftLimit(overdraftLimit) {}
    
    double getOverdraftLimit() const { return overdraftLimit; }
    void setOverdraftLimit(double limit) { overdraftLimit = limit; }
    
    bool withdraw(double amount) override {
        if (amount <= 0) {
            std::cerr << "Invalid withdrawal amount" << std::endl;
            return false;
        }
        
        if (amount > balance + overdraftLimit) {
            std::cerr << "Exceeds overdraft limit" << std::endl;
            return false;
        }
        
        balance -= amount;
        return true;
    }
    
    void display() const override {
        Account::display();
        std::cout << "Overdraft Limit: $" << std::fixed << std::setprecision(2) << overdraftLimit << std::endl;
    }
};

// Transaction entity
class Transaction : public Entity {
private:
    int accountId;
    std::string type;
    double amount;
    std::string dateTime;
    std::string description;
    
public:
    Transaction(int id = 0, int accountId = 0, const std::string& type = "",
               double amount = 0.0, const std::string& dateTime = "",
               const std::string& description = "")
        : Entity(id), accountId(accountId), type(type), amount(amount),
          dateTime(dateTime), description(description) {}
    
    int getAccountId() const { return accountId; }
    void setAccountId(int accountId) { this->accountId = accountId; }
    
    std::string getType() const { return type; }
    void setType(const std::string& type) { this->type = type; }
    
    double getAmount() const { return amount; }
    void setAmount(double amount) { this->amount = amount; }
    
    std::string getDateTime() const { return dateTime; }
    void setDateTime(const std::string& dateTime) { this->dateTime = dateTime; }
    
    std::string getDescription() const { return description; }
    void setDescription(const std::string& description) { this->description = description; }
    
    void display() const override {
        std::cout << "Transaction ID: " << id << std::endl;
        std::cout << "Account ID: " << accountId << std::endl;
        std::cout << "Type: " << type << std::endl;
        std::cout << "Amount: $" << std::fixed << std::setprecision(2) << amount << std::endl;
        std::cout << "Date/Time: " << dateTime << std::endl;
        std::cout << "Description: " << description << std::endl;
    }
};

// Repository interface - Dependency Inversion Principle
template <typename T>
class IRepository {
public:
    virtual ~IRepository() {}
    virtual bool add(const T& entity) = 0;
    virtual bool update(const T& entity) = 0;
    virtual bool remove(int id) = 0;
    virtual std::unique_ptr<T> getById(int id) = 0;
    virtual std::vector<std::unique_ptr<T>> getAll() = 0;
};

// Customer Repository
class CustomerRepository : public IRepository<Customer> {
private:
    std::shared_ptr<IDatabase> db;
    
public:
    CustomerRepository(std::shared_ptr<IDatabase> db) : db(db) {}
    
    bool add(const Customer& customer) override {
        std::string query = "INSERT INTO customers (name, address, phone, email) VALUES ('" +
            customer.getName() + "', '" + customer.getAddress() + "', '" +
            customer.getPhone() + "', '" + customer.getEmail() + "')";
        
        return db->executeQuery(query);
    }
    
    bool update(const Customer& customer) override {
        std::string query = "UPDATE customers SET name='" + customer.getName() +
            "', address='" + customer.getAddress() + "', phone='" + customer.getPhone() +
            "', email='" + customer.getEmail() + "' WHERE customer_id=" + 
            std::to_string(customer.getId());
        
        return db->executeQuery(query);
    }
    
    bool remove(int id) override {
        std::string query = "DELETE FROM customers WHERE customer_id=" + std::to_string(id);
        return db->executeQuery(query);
    }
    
    std::unique_ptr<Customer> getById(int id) override {
        std::string query = "SELECT * FROM customers WHERE customer_id=" + std::to_string(id);
        std::vector<std::vector<std::string>> results;
        
        if (db->executeQuery(query, results) && !results.empty()) {
            const auto& row = results[0];
            return std::make_unique<Customer>(
                std::stoi(row[0]),  // id
                row[1],             // name
                row[2],             // address
                row[3],             // phone
                row[4]              // email
            );
        }
        
        return nullptr;
    }
    
    std::vector<std::unique_ptr<Customer>> getAll() override {
        std::string query = "SELECT * FROM customers";
        std::vector<std::vector<std::string>> results;
        std::vector<std::unique_ptr<Customer>> customers;
        
        if (db->executeQuery(query, results)) {
            for (const auto& row : results) {
                customers.push_back(std::make_unique<Customer>(
                    std::stoi(row[0]),  // id
                    row[1],             // name
                    row[2],             // address
                    row[3],             // phone
                    row[4]              // email
                ));
            }
        }
        
        return customers;
    }
};

// Account Repository
class AccountRepository : public IRepository<Account> {
private:
    std::shared_ptr<IDatabase> db;
    
public:
    AccountRepository(std::shared_ptr<IDatabase> db) : db(db) {}
    
    bool add(const Account& account) override {
        std::string query = "INSERT INTO accounts (customer_id, balance, account_number, account_type, date_opened) VALUES (" +
            std::to_string(account.getCustomerId()) + ", " + 
            std::to_string(account.getBalance()) + ", '" +
            account.getAccountNumber() + "', '" + 
            account.getAccountType() + "', '" +
            account.getDateOpened() + "')";
        
        return db->executeQuery(query);
    }
    
    bool update(const Account& account) override {
        std::string query = "UPDATE accounts SET customer_id=" + 
            std::to_string(account.getCustomerId()) +
            ", balance=" + std::to_string(account.getBalance()) +
            ", account_number='" + account.getAccountNumber() +
            "', account_type='" + account.getAccountType() +
            "', date_opened='" + account.getDateOpened() +
            "' WHERE account_id=" + std::to_string(account.getId());
        
        return db->executeQuery(query);
    }
    
    bool remove(int id) override {
        std::string query = "DELETE FROM accounts WHERE account_id=" + std::to_string(id);
        return db->executeQuery(query);
    }
    
    std::unique_ptr<Account> getById(int id) override {
        std::string query = "SELECT * FROM accounts WHERE account_id=" + std::to_string(id);
        std::vector<std::vector<std::string>> results;
        
        if (db->executeQuery(query, results) && !results.empty()) {
            const auto& row = results[0];
            std::string accountType = row[4];
            
            if (accountType == "Savings") {
                // Get interest rate for savings account
                std::string savingsQuery = "SELECT interest_rate FROM savings_accounts WHERE account_id=" + row[0];
                std::vector<std::vector<std::string>> savingsResults;
                double interestRate = 0.0;
                
                if (db->executeQuery(savingsQuery, savingsResults) && !savingsResults.empty()) {
                    interestRate = std::stod(savingsResults[0][0]);
                }
                
                return std::make_unique<SavingsAccount>(
                    std::stoi(row[0]),         // id
                    std::stoi(row[1]),         // customer_id
                    std::stod(row[2]),         // balance
                    row[3],                    // account_number
                    row[5],                    // date_opened
                    interestRate               // interest_rate
                );
            } else if (accountType == "Checking") {
                // Get overdraft limit for checking account
                std::string checkingQuery = "SELECT overdraft_limit FROM checking_accounts WHERE account_id=" + row[0];
                std::vector<std::vector<std::string>> checkingResults;
                double overdraftLimit = 0.0;
                
                if (db->executeQuery(checkingQuery, checkingResults) && !checkingResults.empty()) {
                    overdraftLimit = std::stod(checkingResults[0][0]);
                }
                
                return std::make_unique<CheckingAccount>(
                    std::stoi(row[0]),         // id
                    std::stoi(row[1]),         // customer_id
                    std::stod(row[2]),         // balance
                    row[3],                    // account_number
                    row[5],                    // date_opened
                    overdraftLimit             // overdraft_limit
                );
            } else {
                return std::make_unique<Account>(
                    std::stoi(row[0]),         // id
                    std::stoi(row[1]),         // customer_id
                    std::stod(row[2]),         // balance
                    row[3],                    // account_number
                    row[4],                    // account_type
                    row[5]                     // date_opened
                );
            }
        }
        
        return nullptr;
    }
    
    std::vector<std::unique_ptr<Account>> getAll() override {
        std::string query = "SELECT * FROM accounts";
        std::vector<std::vector<std::string>> results;
        std::vector<std::unique_ptr<Account>> accounts;
        
        if (db->executeQuery(query, results)) {
            for (const auto& row : results) {
                std::string accountType = row[4];
                
                if (accountType == "Savings") {
                    // Get interest rate for savings account
                    std::string savingsQuery = "SELECT interest_rate FROM savings_accounts WHERE account_id=" + row[0];
                    std::vector<std::vector<std::string>> savingsResults;
                    double interestRate = 0.0;
                    
                    if (db->executeQuery(savingsQuery, savingsResults) && !savingsResults.empty()) {
                        interestRate = std::stod(savingsResults[0][0]);
                    }
                    
                    accounts.push_back(std::make_unique<SavingsAccount>(
                        std::stoi(row[0]),         // id
                        std::stoi(row[1]),         // customer_id
                        std::stod(row[2]),         // balance
                        row[3],                    // account_number
                        row[5],                    // date_opened
                        interestRate               // interest_rate
                    ));
                } else if (accountType == "Checking") {
                    // Get overdraft limit for checking account
                    std::string checkingQuery = "SELECT overdraft_limit FROM checking_accounts WHERE account_id=" + row[0];
                    std::vector<std::vector<std::string>> checkingResults;
                    double overdraftLimit = 0.0;
                    
                    if (db->executeQuery(checkingQuery, checkingResults) && !checkingResults.empty()) {
                        overdraftLimit = std::stod(checkingResults[0][0]);
                    }
                    
                    accounts.push_back(std::make_unique<CheckingAccount>(
                        std::stoi(row[0]),         // id
                        std::stoi(row[1]),         // customer_id
                        std::stod(row[2]),         // balance
                        row[3],                    // account_number
                        row[5],                    // date_opened
                        overdraftLimit             // overdraft_limit
                    ));
                } else {
                    accounts.push_back(std::make_unique<Account>(
                        std::stoi(row[0]),         // id
                        std::stoi(row[1]),         // customer_id
                        std::stod(row[2]),         // balance
                        row[3],                    // account_number
                        row[4],                    // account_type
                        row[5]                     // date_opened
                    ));
                }
            }
        }
        
        return accounts;
    }
    
    std::vector<std::unique_ptr<Account>> getByCustomerId(int customerId) {
        std::string query = "SELECT * FROM accounts WHERE customer_id=" + std::to_string(customerId);
        std::vector<std::vector<std::string>> results;
        std::vector<std::unique_ptr<Account>> accounts;
        
        if (db->executeQuery(query, results)) {
            for (const auto& row : results) {
                std::string accountType = row[4];
                
                if (accountType == "Savings") {
                    // Get interest rate for savings account
                    std::string savingsQuery = "SELECT interest_rate FROM savings_accounts WHERE account_id=" + row[0];
                    std::vector<std::vector<std::string>> savingsResults;
                    double interestRate = 0.0;
                    
                    if (db->executeQuery(savingsQuery, savingsResults) && !savingsResults.empty()) {
                        interestRate = std::stod(savingsResults[0][0]);
                    }
                    
                    accounts.push_back(std::make_unique<SavingsAccount>(
                        std::stoi(row[0]),         // id
                        std::stoi(row[1]),         // customer_id
                        std::stod(row[2]),         // balance
                        row[3],                    // account_number
                        row[5],                    // date_opened
                        interestRate               // interest_rate
                    ));
                } else if (accountType == "Checking") {
                    // Get overdraft limit for checking account
                    std::string checkingQuery = "SELECT overdraft_limit FROM checking_accounts WHERE account_id=" + row[0];
                    std::vector<std::vector<std::string>> checkingResults;
                    double overdraftLimit = 0.0;
                    
                    if (db->executeQuery(checkingQuery, checkingResults) && !checkingResults.empty()) {
                        overdraftLimit = std::stod(checkingResults[0][0]);
                    }
                    
                    accounts.push_back(std::make_unique<CheckingAccount>(
                        std::stoi(row[0]),         // id
                        std::stoi(row[1]),         // customer_id
                        std::stod(row[2]),         // balance
                        row[3],                    // account_number
                        row[5],                    // date_opened
                        overdraftLimit             // overdraft_limit
                    ));
                } else {
                    accounts.push_back(std::make_unique<Account>(
                        std::stoi(row[0]),         // id
                        std::stoi(row[1]),         // customer_id
                        std::stod(row[2]),         // balance
                        row[3],                    // account_number
                        row[4],                    // account_type
                        row[5]                     // date_opened
                    ));
                }
            }
        }
        
        return accounts;
    }
};

// Transaction Repository
class TransactionRepository : public IRepository<Transaction> {
private:
    std::shared_ptr<IDatabase> db;
    
public:
    TransactionRepository(std::shared_ptr<IDatabase> db) : db(db) {}
    
    bool add(const Transaction& transaction) override {
        std::string query = "INSERT INTO transactions (account_id, type, amount, date_time, description) VALUES (" +
            std::to_string(transaction.getAccountId()) + ", '" + 
            transaction.getType() + "', " +
            std::to_string(transaction.getAmount()) + ", '" +
            transaction.getDateTime() + "', '" +
            transaction.getDescription() + "')";
        
        return db->executeQuery(query);
    }
    
    bool update(const Transaction& transaction) override {
        std::string query = "UPDATE transactions SET account_id=" + 
            std::to_string(transaction.getAccountId()) +
            ", type='" + transaction.getType() +
            "', amount=" + std::to_string(transaction.getAmount()) +
            ", date_time='" + transaction.getDateTime() +
            "', description='" + transaction.getDescription() +
            "' WHERE transaction_id=" + std::to_string(transaction.getId());
        
        return db->executeQuery(query);
    }
    
    bool remove(int id) override {
        std::string query = "DELETE FROM transactions WHERE transaction_id=" + std::to_string(id);
        return db->executeQuery(query);
    }
    
    std::unique_ptr<Transaction> getById(int id) override {
        std::string query = "SELECT * FROM transactions WHERE transaction_id=" + std::to_string(id);
        std::vector<std::vector<std::string>> results;
        
        if (db->executeQuery(query, results) && !results.empty()) {
            const auto& row = results[0];
            return std::make_unique<Transaction>(
                std::stoi(row[0]),         // id
                std::stoi(row[1]),         // account_id
                row[2],                    // type
                std::stod(row[3]),         // amount
                row[4],                    // date_time
                row[5]                     // description
            );
        }
        
        return nullptr;
    }
    
    std::vector<std::unique_ptr<Transaction>> getAll() override {
        std::string query = "SELECT * FROM transactions";
        std::vector<std::vector<std::string>> results;
        std::vector<std::unique_ptr<Transaction>> transactions;
        
        if (db->executeQuery(query, results)) {
            for (const auto& row : results) {
                transactions.push_back(std::make_unique<Transaction>(
                    std::stoi(row[0]),         // id
                    std::stoi(row[1]),         // account_id
                    row[2],                    // type
                    std::stod(row[3]),         // amount
                    row[4],                    // date_time
                    row[5]                     // description
                ));
            }
        }
        
        return transactions;
    }
    
    std::vector<std::unique_ptr<Transaction>> getByAccountId(int accountId) {
        std::string query = "SELECT * FROM transactions WHERE account_id=" + std::to_string(accountId);
        std::vector<std::vector<std::string>> results;
        std::vector<std::unique_ptr<Transaction>> transactions;
        
        if (db->executeQuery(query, results)) {
            for (const auto& row : results) {
                transactions.push_back(std::make_unique<Transaction>(
                    std::stoi(row[0]),         // id
                    std::stoi(row[1]),         // account_id
                    row[2],                    // type
                    std::stod(row[3]),         // amount
                    row[4],                    // date_time
                    row[5]                     // description
                ));
            }
        }
        
        return transactions;
    }
};

// Service interfaces - Service Layer Pattern & Single Responsibility Principle
class ICustomerService {
public:
    virtual ~ICustomerService() {}
    virtual bool addCustomer(const Customer& customer) = 0;
    virtual bool updateCustomer(const Customer& customer) = 0;
    virtual bool removeCustomer(int customerId) = 0;
    virtual std::unique_ptr<Customer> getCustomer(int customerId) = 0;
    virtual std::vector<std::unique_ptr<Customer>> getAllCustomers() = 0;
};

class IAccountService {
public:
    virtual ~IAccountService() {}
    virtual bool openAccount(const Account& account) = 0;
    virtual bool closeAccount(int accountId) = 0;
    virtual bool deposit(int accountId, double amount) = 0;
    virtual bool withdraw(int accountId, double amount) = 0;
    virtual bool transfer(int fromAccountId, int toAccountId, double amount) = 0;
    virtual std::unique_ptr<Account> getAccount(int accountId) = 0;
    virtual std::vector<std::unique_ptr<Account>> getCustomerAccounts(int customerId) = 0;
    virtual double getBalance(int accountId) = 0;
};

class ITransactionService {
public:
    virtual ~ITransactionService() {}
    virtual bool recordTransaction(const Transaction& transaction) = 0;
    virtual std::vector<std::unique_ptr<Transaction>> getAccountTransactions(int accountId) = 0;
    virtual std::unique_ptr<Transaction> getTransaction(int transactionId) = 0;
};

// Service implementations
class CustomerService : public ICustomerService {
private:
    std::shared_ptr<CustomerRepository> repository;
public:
    CustomerService(std::shared_ptr<CustomerRepository> repository) : repository(repository) {}
    
    bool addCustomer(const Customer& customer) override {
        return repository->add(customer);
    }
    
    bool updateCustomer(const Customer& customer) override {
        return repository->update(customer);
    }
    
    bool removeCustomer(int customerId) override {
        return repository->remove(customerId);
    }
    
    std::unique_ptr<Customer> getCustomer(int customerId) override {
        return repository->getById(customerId);
    }
    
    std::vector<std::unique_ptr<Customer>> getAllCustomers() override {
        return repository->getAll();
    }
};

class AccountService : public IAccountService {
private:
    std::shared_ptr<AccountRepository> accountRepository;
    std::shared_ptr<TransactionRepository> transactionRepository;
    std::string getCurrentDateTime() {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
    
public:
    AccountService(std::shared_ptr<AccountRepository> accountRepo, 
                  std::shared_ptr<TransactionRepository> transactionRepo)
        : accountRepository(accountRepo), transactionRepository(transactionRepo) {}
    
    bool openAccount(const Account& account) override {
        return accountRepository->add(account);
    }
    
    bool closeAccount(int accountId) override {
        auto account = accountRepository->getById(accountId);
        if (!account || account->getBalance() != 0) {
            return false;
        }
        return accountRepository->remove(accountId);
    }
    
    bool deposit(int accountId, double amount) override {
        auto account = accountRepository->getById(accountId);
        if (!account) {
            return false;
        }
        
        if (account->deposit(amount)) {
            // Record transaction
            Transaction transaction(0, accountId, "Deposit", amount, 
                                   getCurrentDateTime(), "Deposit to account");
            transactionRepository->add(transaction);
            
            // Update account balance
            return accountRepository->update(*account);
        }
        
        return false;
    }
    
    bool withdraw(int accountId, double amount) override {
        auto account = accountRepository->getById(accountId);
        if (!account) {
            return false;
        }
        
        if (account->withdraw(amount)) {
            // Record transaction
            Transaction transaction(0, accountId, "Withdrawal", amount, 
                                   getCurrentDateTime(), "Withdrawal from account");
            transactionRepository->add(transaction);
            
            // Update account balance
            return accountRepository->update(*account);
        }
        
        return false;
    }
    
    bool transfer(int fromAccountId, int toAccountId, double amount) override {
        auto fromAccount = accountRepository->getById(fromAccountId);
        auto toAccount = accountRepository->getById(toAccountId);
        
        if (!fromAccount || !toAccount) {
            return false;
        }
        
        if (fromAccount->withdraw(amount)) {
            toAccount->deposit(amount);
            
            // Record transactions
            std::string dateTime = getCurrentDateTime();
            std::string description = "Transfer from account " + std::to_string(fromAccountId) + 
                                     " to account " + std::to_string(toAccountId);
            
            Transaction fromTransaction(0, fromAccountId, "Transfer Out", amount, 
                                      dateTime, description);
            Transaction toTransaction(0, toAccountId, "Transfer In", amount, 
                                    dateTime, description);
            
            transactionRepository->add(fromTransaction);
            transactionRepository->add(toTransaction);
            
            // Update account balances
            accountRepository->update(*fromAccount);
            accountRepository->update(*toAccount);
            
            return true;
        }
        
        return false;
    }
    
    std::unique_ptr<Account> getAccount(int accountId) override {
        return accountRepository->getById(accountId);
    }
    
    std::vector<std::unique_ptr<Account>> getCustomerAccounts(int customerId) override {
        return accountRepository->getByCustomerId(customerId);
    }
    
    double getBalance(int accountId) override {
        auto account = accountRepository->getById(accountId);
        if (account) {
            return account->getBalance();
        }
        return -1; // Indicates error
    }
};

class TransactionService : public ITransactionService {
private:
    std::shared_ptr<TransactionRepository> repository;
    
public:
    TransactionService(std::shared_ptr<TransactionRepository> repository) 
        : repository(repository) {}
    
    bool recordTransaction(const Transaction& transaction) override {
        return repository->add(transaction);
    }
    
    std::vector<std::unique_ptr<Transaction>> getAccountTransactions(int accountId) override {
        return repository->getByAccountId(accountId);
    }
    
    std::unique_ptr<Transaction> getTransaction(int transactionId) override {
        return repository->getById(transactionId);
    }
};

// Helper for creating database schema
class DatabaseSetup {
private:
    std::shared_ptr<IDatabase> db;
    
public:
    DatabaseSetup(std::shared_ptr<IDatabase> db) : db(db) {}
    
    bool createSchema() {
        // Create customers table
        std::string createCustomersTable = 
            "CREATE TABLE IF NOT EXISTS customers ("
            "customer_id INT AUTO_INCREMENT PRIMARY KEY, "
            "name VARCHAR(100) NOT NULL, "
            "address VARCHAR(200), "
            "phone VARCHAR(20), "
            "email VARCHAR(100) UNIQUE"
            ")";
        
        if (!db->executeQuery(createCustomersTable)) {
            return false;
        }
        
        // Create accounts table
        std::string createAccountsTable = 
            "CREATE TABLE IF NOT EXISTS accounts ("
            "account_id INT AUTO_INCREMENT PRIMARY KEY, "
            "customer_id INT NOT NULL, "
            "balance DECIMAL(15,2) DEFAULT 0.00, "
            "account_number VARCHAR(20) UNIQUE NOT NULL, "
            "account_type VARCHAR(20) NOT NULL, "
            "date_opened VARCHAR(20) NOT NULL, "
            "FOREIGN KEY (customer_id) REFERENCES customers(customer_id) ON DELETE CASCADE"
            ")";
        
        if (!db->executeQuery(createAccountsTable)) {
            return false;
        }
        
        // Create savings_accounts table
        std::string createSavingsAccountsTable = 
            "CREATE TABLE IF NOT EXISTS savings_accounts ("
            "savings_id INT AUTO_INCREMENT PRIMARY KEY, "
            "account_id INT NOT NULL, "
            "interest_rate DECIMAL(5,2) DEFAULT 0.00, "
            "FOREIGN KEY (account_id) REFERENCES accounts(account_id) ON DELETE CASCADE"
            ")";
        
        if (!db->executeQuery(createSavingsAccountsTable)) {
            return false;
        }
        
        // Create checking_accounts table
        std::string createCheckingAccountsTable = 
            "CREATE TABLE IF NOT EXISTS checking_accounts ("
            "checking_id INT AUTO_INCREMENT PRIMARY KEY, "
            "account_id INT NOT NULL, "
            "overdraft_limit DECIMAL(15,2) DEFAULT 0.00, "
            "FOREIGN KEY (account_id) REFERENCES accounts(account_id) ON DELETE CASCADE"
            ")";
        
        if (!db->executeQuery(createCheckingAccountsTable)) {
            return false;
        }
        
        // Create transactions table
        std::string createTransactionsTable = 
            "CREATE TABLE IF NOT EXISTS transactions ("
            "transaction_id INT AUTO_INCREMENT PRIMARY KEY, "
            "account_id INT NOT NULL, "
            "type VARCHAR(50) NOT NULL, "
            "amount DECIMAL(15,2) NOT NULL, "
            "date_time VARCHAR(20) NOT NULL, "
            "description VARCHAR(200), "
            "FOREIGN KEY (account_id) REFERENCES accounts(account_id) ON DELETE CASCADE"
            ")";
        
        return db->executeQuery(createTransactionsTable);
    }
};

// UI interface - follows Interface Segregation Principle
class IUserInterface {
public:
    virtual ~IUserInterface() {}
    virtual void start() = 0;
};

// User class for authentication
class User {
private:
    std::string username;
    std::string password;

public:
    User(const std::string& user, const std::string& pass) : username(user), password(pass) {}

    bool authenticate(const std::string& user, const std::string& pass) {
        return username == user && password == pass;
    }
};

// Console UI implementation - follows Single Responsibility Principle
class ConsoleUI : public IUserInterface {
private:
    std::shared_ptr<ICustomerService> customerService;
    std::shared_ptr<IAccountService> accountService;
    std::shared_ptr<ITransactionService> transactionService;
    std::shared_ptr<User> currentUser;
    
    void displayMainMenu() {
        std::cout << "\n========= BANK MANAGEMENT SYSTEM =========\n";
        std::cout << "1. Customer Management\n";
        std::cout << "2. Account Management\n";
        std::cout << "3. Transaction Management\n";
        std::cout << "0. Exit\n";
        std::cout << "Enter your choice: ";
    }
    
    void displayCustomerMenu() {
        std::cout << "\n========= CUSTOMER MANAGEMENT =========\n";
        std::cout << "1. Add New Customer\n";
        std::cout << "2. Update Customer Information\n";
        std::cout << "3. Remove Customer\n";
        std::cout << "4. View Customer Details\n";
        std::cout << "5. List All Customers\n";
        std::cout << "0. Back to Main Menu\n";
        std::cout << "Enter your choice: ";
    }
    
    void displayAccountMenu() {
        std::cout << "\n========= ACCOUNT MANAGEMENT =========\n";
        std::cout << "1. Open New Account\n";
        std::cout << "2. Close Account\n";
        std::cout << "3. Deposit\n";
        std::cout << "4. Withdraw\n";
        std::cout << "5. Transfer\n";
        std::cout << "6. View Account Details\n";
        std::cout << "7. List Customer Accounts\n";
        std::cout << "0. Back to Main Menu\n";
        std::cout << "Enter your choice: ";
    }
    
    void displayTransactionMenu() {
        std::cout << "\n========= TRANSACTION MANAGEMENT =========\n";
        std::cout << "1. View Transaction Details\n";
        std::cout << "2. View Account Transactions\n";
        std::cout << "0. Back to Main Menu\n";
        std::cout << "Enter your choice: ";
    }
    
    void handleCustomerManagement() {
        int choice = -1;
        
        while (choice != 0) {
            displayCustomerMenu();
            std::cin >> choice;
            
            switch (choice) {
                case 1:
                    addCustomer();
                    break;
                case 2:
                    updateCustomer();
                    break;
                case 3:
                    removeCustomer();
                    break;
                case 4:
                    viewCustomerDetails();
                    break;
                case 5:
                    listAllCustomers();
                    break;
                case 0:
                    std::cout << "Returning to main menu...\n";
                    break;
                default:
                    std::cout << "Invalid choice. Please try again.\n";
            }
        }
    }
    
    void handleAccountManagement() {
        int choice = -1;
        
        while (choice != 0) {
            displayAccountMenu();
            std::cin >> choice;
            
            switch (choice) {
                case 1:
                    openAccount();
                    break;
                case 2:
                    closeAccount();
                    break;
                case 3:
                    deposit();
                    break;
                case 4:
                    withdraw();
                    break;
                case 5:
                    transfer();
                    break;
                case 6:
                    viewAccountDetails();
                    break;
                case 7:
                    listCustomerAccounts();
                    break;
                case 0:
                    std::cout << "Returning to main menu...\n";
                    break;
                default:
                    std::cout << "Invalid choice. Please try again.\n";
            }
        }
    }
    
    void handleTransactionManagement() {
        int choice = -1;
        
        while (choice != 0) {
            displayTransactionMenu();
            std::cin >> choice;
            
            switch (choice) {
                case 1:
                    viewTransactionDetails();
                    break;
                case 2:
                    viewAccountTransactions();
                    break;
                case 0:
                    std::cout << "Returning to main menu...\n";
                    break;
                default:
                    std::cout << "Invalid choice. Please try again.\n";
            }
        }
    }
    
    // Customer management functions
    void addCustomer() {
        std::string name, address, phone, email;
        
        std::cin.ignore();
        std::cout << "Enter customer name: ";
        std::getline(std::cin, name);
        
        std::cout << "Enter address: ";
        std::getline(std::cin, address);
        
        std::cout << "Enter phone number: ";
        std::getline(std::cin, phone);
        
        std::cout << "Enter email: ";
        std::getline(std::cin, email);
        
        Customer customer(0, name, address, phone, email);
        
        if (customerService->addCustomer(customer)) {
            std::cout << "Customer added successfully.\n";
        } else {
            std::cout << "Failed to add customer.\n";
        }
    }
    
    void updateCustomer() {
        int customerId;
        std::cout << "Enter customer ID: ";
        std::cin >> customerId;
        
        auto customer = customerService->getCustomer(customerId);
        
        if (!customer) {
            std::cout << "Customer not found.\n";
            return;
        }
        
        std::string name, address, phone, email;
        
        std::cin.ignore();
        std::cout << "Enter new name (current: " << customer->getName() << "): ";
        std::getline(std::cin, name);
        if (!name.empty()) customer->setName(name);
        
        std::cout << "Enter new address (current: " << customer->getAddress() << "): ";
        std::getline(std::cin, address);
        if (!address.empty()) customer->setAddress(address);
        
        std::cout << "Enter new phone (current: " << customer->getPhone() << "): ";
        std::getline(std::cin, phone);
        if (!phone.empty()) customer->setPhone(phone);
        
        std::cout << "Enter new email (current: " << customer->getEmail() << "): ";
        std::getline(std::cin, email);
        if (!email.empty()) customer->setEmail(email);
        
        if (customerService->updateCustomer(*customer)) {
            std::cout << "Customer updated successfully.\n";
        } else {
            std::cout << "Failed to update customer.\n";
        }
    }
    
    void removeCustomer() {
        int customerId;
        std::cout << "Enter customer ID: ";
        std::cin >> customerId;
        
        if (customerService->removeCustomer(customerId)) {
            std::cout << "Customer removed successfully.\n";
        } else {
            std::cout << "Failed to remove customer.\n";
        }
    }
    
    void viewCustomerDetails() {
        int customerId;
        std::cout << "Enter customer ID: ";
        std::cin >> customerId;
        
        auto customer = customerService->getCustomer(customerId);
        
        if (customer) {
            std::cout << "\n------------ Customer Details ------------\n";
            customer->display();
            
            // Display customer accounts
            auto accounts = accountService->getCustomerAccounts(customerId);
            
            if (!accounts.empty()) {
                std::cout << "\nCustomer Accounts:\n";
                for (const auto& account : accounts) {
                    std::cout << "Account Number: " << account->getAccountNumber()
                              << ", Type: " << account->getAccountType()
                              << ", Balance: $" << std::fixed << std::setprecision(2) 
                              << account->getBalance() << std::endl;
                }
            } else {
                std::cout << "No accounts found for this customer.\n";
            }
        } else {
            std::cout << "Customer not found.\n";
        }
    }
    
    void listAllCustomers() {
        auto customers = customerService->getAllCustomers();
        
        if (customers.empty()) {
            std::cout << "No customers found.\n";
            return;
        }
        
        std::cout << "\n------------ All Customers ------------\n";
        for (const auto& customer : customers) {
            std::cout << "ID: " << customer->getId()
                      << ", Name: " << customer->getName()
                      << ", Phone: " << customer->getPhone()
                      << ", Email: " << customer->getEmail() << std::endl;
        }
    }
    
    // Account management functions
    void openAccount() {
        int customerId;
        int accountType;
        double initialDeposit;
        
        std::cout << "Enter customer ID: ";
        std::cin >> customerId;
        
        auto customer = customerService->getCustomer(customerId);
        
        if (!customer) {
            std::cout << "Customer not found.\n";
            return;
        }
        
        std::cout << "Select account type:\n";
        std::cout << "1. Savings Account\n";
        std::cout << "2. Checking Account\n";
        std::cout << "Enter choice: ";
        std::cin >> accountType;
        
        std::cout << "Enter initial deposit amount: $";
        std::cin >> initialDeposit;
        
        if (initialDeposit <= 0) {
            std::cout << "Initial deposit must be greater than zero.\n";
            return;
        }
        
        // Generate account number (simple implementation for demonstration)
        auto now = std::time(nullptr);
        std::string accountNumber = std::to_string(customerId) + std::to_string(now);
        
        // Get current date
        auto tm = *std::localtime(&now);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d");
        std::string dateOpened = oss.str();
        
        if (accountType == 1) {
            // Savings account
            double interestRate;
            std::cout << "Enter interest rate (%): ";
            std::cin >> interestRate;
            
            SavingsAccount account(0, customerId, initialDeposit, accountNumber, dateOpened, interestRate);
            
            if (accountService->openAccount(account)) {
                std::cout << "Savings account opened successfully.\n";
                std::cout << "Account Number: " << accountNumber << std::endl;
            } else {
                std::cout << "Failed to open savings account.\n";
            }
        } else if (accountType == 2) {
            // Checking account
            double overdraftLimit;
            std::cout << "Enter overdraft limit: $";
            std::cin >> overdraftLimit;
            
            CheckingAccount account(0, customerId, initialDeposit, accountNumber, dateOpened, overdraftLimit);
            
            if (accountService->openAccount(account)) {
                std::cout << "Checking account opened successfully.\n";
                std::cout << "Account Number: " << accountNumber << std::endl;
            } else {
                std::cout << "Failed to open checking account.\n";
            }
        } else {
            std::cout << "Invalid account type.\n";
        }
    }
    
    void closeAccount() {
        int accountId;
        std::cout << "Enter account ID: ";
        std::cin >> accountId;
        
        auto account = accountService->getAccount(accountId);
        
        if (!account) {
            std::cout << "Account not found.\n";
            return;
        }
        
        if (account->getBalance() > 0) {
            std::cout << "Account has a balance of $" << std::fixed << std::setprecision(2) 
                     << account->getBalance() << ". Withdraw before closing.\n";
            return;
        }
        
        if (accountService->closeAccount(accountId)) {
            std::cout << "Account closed successfully.\n";
        } else {
            std::cout << "Failed to close account.\n";
        }
    }
    
    void deposit() {
        int accountId;
        double amount;
        
        std::cout << "Enter account ID: ";
        std::cin >> accountId;
        
        auto account = accountService->getAccount(accountId);
        
        if (!account) {
            std::cout << "Account not found.\n";
            return;
        }
        
        std::cout << "Enter deposit amount: $";
        std::cin >> amount;
        
        if (amount <= 0) {
            std::cout << "Deposit amount must be greater than zero.\n";
            return;
        }
        
        if (accountService->deposit(accountId, amount)) {
            std::cout << "Deposit successful.\n";
            std::cout << "New balance: $" << std::fixed << std::setprecision(2) 
                     << accountService->getBalance(accountId) << std::endl;
        } else {
            std::cout << "Deposit failed.\n";
        }
    }
    
    void withdraw() {
        int accountId;
        double amount;
        
        std::cout << "Enter account ID: ";
        std::cin >> accountId;
        
        auto account = accountService->getAccount(accountId);
        
        if (!account) {
            std::cout << "Account not found.\n";
            return;
        }
        
        std::cout << "Enter withdrawal amount: $";
        std::cin >> amount;
        
        if (amount <= 0) {
            std::cout << "Withdrawal amount must be greater than zero.\n";
            return;
        }
        
        if (accountService->withdraw(accountId, amount)) {
            std::cout << "Withdrawal successful.\n";
            std::cout << "New balance: $" << std::fixed << std::setprecision(2) 
                     << accountService->getBalance(accountId) << std::endl;
        } else {
            std::cout << "Withdrawal failed. Insufficient funds or exceeded overdraft limit.\n";
        }
    }
    
    void transfer() {
        int fromAccountId, toAccountId;
        double amount;
        
        std::cout << "Enter source account ID: ";
        std::cin >> fromAccountId;
        
        auto fromAccount = accountService->getAccount(fromAccountId);
        
        if (!fromAccount) {
            std::cout << "Source account not found.\n";
            return;
        }
        
        std::cout << "Enter destination account ID: ";
        std::cin >> toAccountId;
        
        auto toAccount = accountService->getAccount(toAccountId);
        
        if (!toAccount) {
            std::cout << "Destination account not found.\n";
            return;
        }
        
        std::cout << "Enter transfer amount: $";
        std::cin >> amount;
        
        if (amount <= 0) {
            std::cout << "Transfer amount must be greater than zero.\n";
            return;
        }
        
        if (accountService->transfer(fromAccountId, toAccountId, amount)) {
            std::cout << "Transfer successful.\n";
            std::cout << "Source account balance: $" << std::fixed << std::setprecision(2) 
                     << accountService->getBalance(fromAccountId) << std::endl;
            std::cout << "Destination account balance: $" << std::fixed << std::setprecision(2) 
                     << accountService->getBalance(toAccountId) << std::endl;
        } else {
            std::cout << "Transfer failed. Insufficient funds or exceeded overdraft limit.\n";
        }
    }
    
    void viewAccountDetails() {
        int accountId;
        std::cout << "Enter account ID: ";
        std::cin >> accountId;
        
        auto account = accountService->getAccount(accountId);
        
        if (account) {
            std::cout << "\n------------ Account Details ------------\n";
            account->display();
        } else {
            std::cout << "Account not found.\n";
        }
    }
    
    void listCustomerAccounts() {
        int customerId;
        std::cout << "Enter customer ID: ";
        std::cin >> customerId;
        
        auto customer = customerService->getCustomer(customerId);
        
        if (!customer) {
            std::cout << "Customer not found.\n";
            return;
        }
        
        auto accounts = accountService->getCustomerAccounts(customerId);
        
        if (accounts.empty()) {
            std::cout << "No accounts found for this customer.\n";
            return;
        }
        
        std::cout << "\n------------ Customer Accounts ------------\n";
        std::cout << "Customer: " << customer->getName() << std::endl;
        
        for (const auto& account : accounts) {
            std::cout << "\nAccount ID: " << account->getId() << std::endl;
            std::cout << "Account Number: " << account->getAccountNumber() << std::endl;
            std::cout << "Account Type: " << account->getAccountType() << std::endl;
            std::cout << "Balance: $" << std::fixed << std::setprecision(2) << account->getBalance() << std::endl;
            std::cout << "Date Opened: " << account->getDateOpened() << std::endl;
            
            if (account->getAccountType() == "Savings") {
                auto savingsAccount = dynamic_cast<SavingsAccount*>(account.get());
                if (savingsAccount) {
                    std::cout << "Interest Rate: " << savingsAccount->getInterestRate() << "%" << std::endl;
                }
            } else if (account->getAccountType() == "Checking") {
                auto checkingAccount = dynamic_cast<CheckingAccount*>(account.get());
                if (checkingAccount) {
                    std::cout << "Overdraft Limit: $" << std::fixed << std::setprecision(2)
                             << checkingAccount->getOverdraftLimit() << std::endl;
                }
            }
        }
    }
    
    // Transaction management functions
    void viewTransactionDetails() {
        int transactionId;
        std::cout << "Enter transaction ID: ";
        std::cin >> transactionId;
        
        auto transaction = transactionService->getTransaction(transactionId);
        
        if (transaction) {
            std::cout << "\n------------ Transaction Details ------------\n";
            transaction->display();
        } else {
            std::cout << "Transaction not found.\n";
        }
    }
    
    void viewAccountTransactions() {
        int accountId;
        std::cout << "Enter account ID: ";
        std::cin >> accountId;
        
        auto account = accountService->getAccount(accountId);
        
        if (!account) {
            std::cout << "Account not found.\n";
            return;
        }
        
        auto transactions = transactionService->getAccountTransactions(accountId);
        
        if (transactions.empty()) {
            std::cout << "No transactions found for this account.\n";
            return;
        }
        
        std::cout << "\n------------ Account Transactions ------------\n";
        std::cout << "Account: " << account->getAccountNumber() << std::endl;
        
        for (const auto& transaction : transactions) {
            std::cout << "\nTransaction ID: " << transaction->getId() << std::endl;
            std::cout << "Type: " << transaction->getType() << std::endl;
            std::cout << "Amount: $" << std::fixed << std::setprecision(2) << transaction->getAmount() << std::endl;
            std::cout << "Date/Time: " << transaction->getDateTime() << std::endl;
            std::cout << "Description: " << transaction->getDescription() << std::endl;
        }
    }
    
    // Login method
    void login() {
        std::string username, password;
        int attempts = 0;
        const int maxAttempts = 3;
        const int lockoutDuration = 60; // Lockout duration in seconds

        while (attempts < maxAttempts) {
            std::cout << "Enter username: ";
            std::cin >> username;
            std::cout << "Enter password: ";
            std::cin >> password;

            // Simple hardcoded user for demonstration
            User user("admin", "password123");
            currentUser = std::make_shared<User>(user);

            if (currentUser->authenticate(username, password)) {
                std::cout << "Login successful!\n";
                return; // Exit the loop on successful login
            } else {
                attempts++;
                std::cout << "Invalid credentials. Attempts left: " << (maxAttempts - attempts) << "\n";
            }

            if (attempts == maxAttempts) {
                std::cout << "Too many failed attempts. Please wait " << lockoutDuration << " seconds before trying again.\n";
                std::this_thread::sleep_for(std::chrono::seconds(lockoutDuration)); // Lockout timer
                attempts = 0; // Reset attempts after lockout
            }
        }

        std::cout << "Access denied. Exiting...\n";
        exit(0); // Exit if authentication fails after max attempts
    }
    
public:
    ConsoleUI(std::shared_ptr<ICustomerService> customerSvc,
             std::shared_ptr<IAccountService> accountSvc,
             std::shared_ptr<ITransactionService> transactionSvc)
        : customerService(customerSvc), accountService(accountSvc), transactionService(transactionSvc) {}
    
    void start() override {
        login(); // Call login before showing the main menu
        int choice = -1;
        
        while (choice != 0) {
            displayMainMenu();
            std::cin >> choice;
            
            switch (choice) {
                case 1:
                    handleCustomerManagement();
                    break;
                case 2:
                    handleAccountManagement();
                    break;
                case 3:
                    handleTransactionManagement();
                    break;
                case 0:
                    std::cout << "Thank you for using the Bank Management System. Goodbye!\n";
                    break;
                default:
                    std::cout << "Invalid choice. Please try again.\n";
            }
        }
    }
};

// Application class to demonstrate Dependency Injection
class BankApplication {
private:
    std::shared_ptr<IUserInterface> ui;
    std::shared_ptr<IDatabase> db;
    
public:
    BankApplication(std::shared_ptr<IUserInterface> ui, std::shared_ptr<IDatabase> db)
        : ui(ui), db(db) {}
    
    bool initialize() {
        // Connect to database
        if (!db->connect()) {
            std::cerr << "Failed to connect to database\n";
            return false;
        }
        
        // Setup database schema
        DatabaseSetup setup(db);
        if (!setup.createSchema()) {
            std::cerr << "Failed to create database schema\n";
            return false;
        }
        
        std::cout << "Bank Management System initialized successfully\n";
        return true;
    }
    
    void run() {
        ui->start();
    }
    
    void shutdown() {
        db->disconnect();
        std::cout << "Bank Management System shut down\n";
    }
};

// Main function
int main() {
    // Create database connection
    DBConfig config;
    auto db = std::make_shared<MySQLDatabase>(config);
    
    // Create repositories
    auto customerRepo = std::make_shared<CustomerRepository>(db);
    auto accountRepo = std::make_shared<AccountRepository>(db);
    auto transactionRepo = std::make_shared<TransactionRepository>(db);
    
    // Create services
    auto customerService = std::make_shared<CustomerService>(customerRepo);
    auto accountService = std::make_shared<AccountService>(accountRepo, transactionRepo);
    auto transactionService = std::make_shared<TransactionService>(transactionRepo);
    
    // Create UI
    auto ui = std::make_shared<ConsoleUI>(customerService, accountService, transactionService);
    
    // Create and run the application
    BankApplication app(ui, db);
    
    if (app.initialize()) {
        app.run();
    }
    
    app.shutdown();
    
    return 0;
}