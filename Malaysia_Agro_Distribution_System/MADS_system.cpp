#include <iostream>  // Standard input-output stream library
#include <vector>    // For using dynamic arrays
#include <string>    // For string manipulations
#include <ctime>     // For handling timestamps
#include <fstream>   // For file operations (saving/loading blockchain data)
#include <sstream>   // For parsing and serializing data
#include <iomanip>   // For formatting output
#include <algorithm> // For algorithms like sorting, searching, etc.
#include <cstdlib>   // For random number generation
#include <cmath>     // For mathematical calculations
#include <limits>    // For handling numeric limits
#include <unordered_map> // For hash table-based maps

// Forward declarations of classes
class Block;
class Blockchain;
class Product;
class Supplier;
class Retailer;
class Transporter;
class Transaction;
class SmartContract;

// Utility function to get the current timestamp
std::string getCurrentTimestamp() {
    time_t now = time(0);                 // Get current time
    std::tm* timeinfo = localtime(&now);  // Convert to local time format
    char buffer[80];                      // Buffer to store formatted time
    strftime(buffer, sizeof(buffer), "%Y%m%d:%H:%M", timeinfo); // Format timestamp
    return std::string(buffer);           // Convert buffer to string and return
}

// Utility function to generate a random hash (dummy function, not cryptographically secure)
std::string generateHash() {
    const char alphanum[] = "0123456789abcdefghijklmnopqrstuvwxyz"; // Character set for hash
    std::string hash = "";  // Initialize empty hash string
    for (int i = 0; i < 10; i++) { // Generate 10-character hash
        hash += alphanum[rand() % (sizeof(alphanum) - 1)]; // Append random character
    }
    return hash; // Return generated hash
}

// Function to calculate the approximate distance between two geographical points
// This uses Euclidean distance; a real-world implementation should use the Haversine formula
double calculateDistance(double lat1, double lon1, double lat2, double lon2) {
    return sqrt(pow(lat2 - lat1, 2) + pow(lon2 - lon1, 2)) * 111.0; // Convert degrees to km
}

// Class representing a block in the blockchain
class Block {
private:
    int blockNumber;         // Block index in the chain
    std::string currentHash; // Hash of the current block
    std::string previousHash;// Hash of the previous block
    std::string timestamp;   // Time when the block was created
    std::string data;        // Data stored in the block

public:
    // Constructor to initialize a block
    Block(int blockNum, const std::string& prevHash, const std::string& blockData) 
        : blockNumber(blockNum), previousHash(prevHash), data(blockData) {
        timestamp = getCurrentTimestamp(); // Get and store current timestamp
        currentHash = generateHash();      // Generate and store block hash
    }

    // Getters for block attributes
    std::string getCurrentHash() const { return currentHash; }
    std::string getPreviousHash() const { return previousHash; }
    int getBlockNumber() const { return blockNumber; }
    std::string getTimestamp() const { return timestamp; }
    std::string getData() const { return data; }

    // Overloading the output stream operator to display block information
    friend std::ostream& operator<<(std::ostream& os, const Block& block) {
        os << "Block " << block.blockNumber << " | " 
           << block.currentHash << " | " 
           << block.previousHash << " | " 
           << block.timestamp << " | " 
           << block.data;
        return os;
    }

    // Serialize block data into a string format for file storage
    std::string serialize() const {
        std::stringstream ss;
        ss << blockNumber << "|" << currentHash << "|" << previousHash << "|" << timestamp << "|" << data;
        return ss.str();
    }

    // Deserialize a block from a stored string format
    static Block deserialize(const std::string& str) {
        std::stringstream ss(str);
        std::string item;
        std::vector<std::string> parts;
        
        while (std::getline(ss, item, '|')) { // Split string by '|'
            parts.push_back(item);
        }

        if (parts.size() < 5) { // Validate correct format
            throw std::runtime_error("Invalid block data format");
        }

        int blockNum = std::stoi(parts[0]); // Convert block number to integer
        Block block(blockNum, parts[2], parts[4]); // Create block instance
        block.currentHash = parts[1]; // Restore hash
        block.timestamp = parts[3];   // Restore timestamp
        
        return block;
    }
};

// Blockchain class managing a chain of blocks
class Blockchain {
private:
    std::vector<Block> chain; // Vector storing blocks in the chain
    
public:
    // Constructor to initialize blockchain with a genesis block
    Blockchain() {
        createGenesisBlock(); // Create the first block in the blockchain
    }

    // Function to create the genesis block (first block in the chain)
    void createGenesisBlock() {
        std::string genesisHash = generateHash(); // Generate a unique hash
        Block genesisBlock(0, genesisHash, "Genesis Block"); // Create genesis block
        chain.push_back(genesisBlock); // Add genesis block to chain
    }

    // Function to get the latest block in the chain
    Block getLatestBlock() const {
        if (chain.empty()) { // Check if blockchain is empty
            throw std::runtime_error("Blockchain is empty");
        }
        return chain.back(); // Return the last block in the chain
    }

    // Function to add a new block to the chain
    void addBlock(const std::string& data) {
        try {
            Block latestBlock = getLatestBlock(); // Get last block
            Block newBlock(latestBlock.getBlockNumber() + 1, latestBlock.getCurrentHash(), data); // Create new block
            chain.push_back(newBlock); // Add block to chain
        }
        catch (const std::exception& e) {
            std::cerr << "Error adding block: " << e.what() << std::endl;
            throw;
        }
    }

    // Function to check blockchain integrity
    bool isChainValid() const {
        for (size_t i = 1; i < chain.size(); i++) {
            if (chain[i].getPreviousHash() != chain[i-1].getCurrentHash()) { // Verify hash linkage
                return false;
            }
        }
        return true;
    }

    // Display all blocks in the blockchain
    void displayChain() const {
        for (const auto& block : chain) {
            std::cout << block << std::endl;
            std::cout << "--------------------------------------" << std::endl;
        }
    }

    // Save blockchain to file
    bool saveToFile(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Could not open file for writing: " << filename << std::endl;
            return false;
        }

        for (const auto& block : chain) {
            file << block.serialize() << std::endl;
        }
        
        file.close();
        return true;
    }

    // Load blockchain from file
    bool loadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Could not open file for reading: " << filename << std::endl;
            return false;
        }

        chain.clear(); // Clear existing chain
        std::string line;
        
        while (std::getline(file, line)) {
            try {
                Block block = Block::deserialize(line);
                chain.push_back(block);
            }
            catch (const std::exception& e) {
                return false;
            }
        }
        
        if (chain.empty()) {
            createGenesisBlock();
        }
        return true;
    }
    size_t size() const {
        return chain.size();
    }
};

// Product class
class Product {
    private:
        int id; // Unique identifier for the product
        std::string name; // Name of the product
        double price; // Price of the product
        int stock; // Stock quantity of the product
    
    public:
        // Constructor to initialize a Product object
        Product(int productId, const std::string& productName, double productPrice, int productStock)
            : id(productId), name(productName), price(productPrice), stock(productStock) {}
    
        // Getter function to return the product ID
        int getId() const { return id; }
    
        // Getter function to return the product name
        std::string getName() const { return name; }
    
        // Getter function to return the product price
        double getPrice() const { return price; }
    
        // Getter function to return the stock quantity
        int getStock() const { return stock; }
    
        // Function to update the stock quantity by adding or subtracting a given amount
        void updateStock(int quantity) {
            stock += quantity;
        }
    
        // Function to check if the stock is sufficient for a given quantity
        bool hasEnoughStock(int quantity) const {
            return stock >= quantity;
        }
    
        // Function to generate a string representation of the product details
        std::string toString() const {
            std::stringstream ss;
            ss << "Product ID: " << id << " | Name: " << name 
               << " | Price: RM" << std::fixed << std::setprecision(2) << price 
               << " | Stock: " << stock;
            return ss.str();
        }
    
        // Function to serialize product data into a string format for storage
        std::string serialize() const {
            std::stringstream ss;
            ss << id << "|" << name << "|" << price << "|" << stock;
            return ss.str();
        }
    
        // Static function to deserialize a product from a serialized string
        static Product deserialize(const std::string& str) {
            std::stringstream ss(str);
            std::string item;
            std::vector<std::string> parts;
            
            // Splitting the input string by '|' delimiter
            while (std::getline(ss, item, '|')) {
                parts.push_back(item);
            }
    
            // Ensure the correct number of fields is present
            if (parts.size() < 4) {
                throw std::runtime_error("Invalid product data format");
            }
    
            // Extract and convert data from string format
            int id = std::stoi(parts[0]);
            std::string name = parts[1];
            double price = std::stod(parts[2]);
            int stock = std::stoi(parts[3]);
            
            return Product(id, name, price, stock);
        }
    };
    
    // Supplier class
    class Supplier {
    private:
        int id; // Unique identifier for the supplier
        std::string name; // Supplier name
        std::string location; // Location of the supplier
        std::string branch; // Branch name of the supplier
        double latitude; // Latitude coordinate of the supplier location
        double longitude; // Longitude coordinate of the supplier location
        std::vector<int> productIds; // List of product IDs supplied
    
    public:
        // Constructor to initialize a Supplier object
        Supplier(int supplierId, const std::string& supplierName, const std::string& supplierLocation, 
                 const std::string& supplierBranch, double lat, double lon)
            : id(supplierId), name(supplierName), location(supplierLocation), 
              branch(supplierBranch), latitude(lat), longitude(lon) {}
    
        // Getter function to return the supplier ID
        int getId() const { return id; }
    
        // Getter function to return the supplier name
        std::string getName() const { return name; }
    
        // Getter function to return the supplier location
        std::string getLocation() const { return location; }
    
        // Getter function to return the supplier branch
        std::string getBranch() const { return branch; }
    
        // Getter function to return the latitude of the supplier location
        double getLatitude() const { return latitude; }
    
        // Getter function to return the longitude of the supplier location
        double getLongitude() const { return longitude; }
        
        // Function to add a product ID to the supplier's list of products
        void addProduct(int productId) {
            productIds.push_back(productId);
        }
    
        // Getter function to retrieve the list of product IDs
        const std::vector<int>& getProductIds() const {
            return productIds;
        }
    
        // Function to generate a string representation of supplier details
        std::string toString() const {
            std::stringstream ss;
            ss << "Supplier ID: " << id << " | Name: " << name 
               << " | Location: " << location << " | Branch: " << branch;
            return ss.str();
        }
    
        // Function to generate data to be stored in a blockchain
        std::string getBlockData() const {
            std::stringstream ss;
            ss << "Supplier ID: " << id << " | Supplier Name: " << name
               << " | Location: " << location << " | Branch: " << branch;
            return ss.str();
        }
    
        // Function to serialize supplier data into a string format for storage
        std::string serialize() const {
            std::stringstream ss;
            ss << id << "|" << name << "|" << location << "|" << branch << "|" << latitude << "|" << longitude;
            
            // Add product IDs to the serialized string
            ss << "|";
            for (size_t i = 0; i < productIds.size(); ++i) {
                ss << productIds[i];
                if (i < productIds.size() - 1) {
                    ss << ",";
                }
            }
            
            return ss.str();
        }
    
        // Static function to deserialize a supplier from a serialized string
        static Supplier deserialize(const std::string& str) {
            std::stringstream ss(str);
            std::string item;
            std::vector<std::string> parts;
            
            // Splitting the input string by '|' delimiter
            while (std::getline(ss, item, '|')) {
                parts.push_back(item);
            }
    
            // Ensure the correct number of fields is present
            if (parts.size() < 6) {
                throw std::runtime_error("Invalid supplier data format");
            }
    
            // Extract and convert data from string format
            int id = std::stoi(parts[0]);
            std::string name = parts[1];
            std::string location = parts[2];
            std::string branch = parts[3];
            double latitude = std::stod(parts[4]);
            double longitude = std::stod(parts[5]);
            
            Supplier supplier(id, name, location, branch, latitude, longitude);
            
            // Add product IDs if present
            if (parts.size() > 6) {
                std::stringstream productStream(parts[6]);
                std::string productId;
                
                while (std::getline(productStream, productId, ',')) {
                    if (!productId.empty()) {
                        supplier.addProduct(std::stoi(productId));
                    }
                }
            }
            
            return supplier;
        }
    };
    
// Retailer class
class Retailer {
private:
    int id; // Unique identifier for the retailer
    std::string name; // Name of the retailer
    std::string location; // Location of the retailer
    double latitude; // Latitude coordinate of the retailer's location
    double longitude; // Longitude coordinate of the retailer's location
    double creditBalance; // Current credit balance of the retailer
    double annualCreditBalance; // Annual credit balance of the retailer
    std::vector<int> productIds; // List of product IDs associated with the retailer

public:
    // Constructor to initialize a Retailer object
    Retailer(int retailerId, const std::string& retailerName, const std::string& retailerLocation, 
             double lat, double lon, double initialCredit, double annualCredit)
        : id(retailerId), name(retailerName), location(retailerLocation), 
          latitude(lat), longitude(lon), creditBalance(initialCredit), annualCreditBalance(annualCredit) {}

    // Getter function to return the retailer ID
    int getId() const { return id; }
    
    // Getter function to return the retailer name
    std::string getName() const { return name; }
    
    // Getter function to return the retailer location
    std::string getLocation() const { return location; }
    
    // Getter function to return the latitude of the retailer's location
    double getLatitude() const { return latitude; }
    
    // Getter function to return the longitude of the retailer's location
    double getLongitude() const { return longitude; }
    
    // Getter function to return the current credit balance of the retailer
    double getCreditBalance() const { return creditBalance; }
    
    // Getter function to return the annual credit balance of the retailer
    double getAnnualCreditBalance() const { return annualCreditBalance; }
    
    // Function to add a product ID to the retailer's list of products
    void addProduct(int productId) {
        productIds.push_back(productId);
    }

    // Getter function to retrieve the list of product IDs
    const std::vector<int>& getProductIds() const {
        return productIds;
    }

    // Function to deduct a specified amount from the retailer's credit balance
    bool deductCredit(double amount) {
        if (creditBalance >= amount) { // Check if sufficient credit is available
            creditBalance -= amount; // Deduct the amount from the current credit balance
            annualCreditBalance -= amount; // Deduct the amount from the annual credit balance
            return true; // Return true if deduction is successful
        }
        return false; // Return false if insufficient credit
    }

    // Function to add a specified amount to the retailer's credit balance
    void addCredit(double amount) {
        creditBalance += amount; // Add the amount to the current credit balance
        annualCreditBalance += amount; // Add the amount to the annual credit balance
    }

    // Function to generate a string representation of the retailer details
    std::string toString() const {
        std::stringstream ss;
        ss << "Retailer ID: " << id << " | Name: " << name 
           << " | Location: " << location 
           << " | Credit Balance: RM" << std::fixed << std::setprecision(2) << creditBalance
           << " | Annual Credit Balance: RM" << std::fixed << std::setprecision(2) << annualCreditBalance;
        return ss.str();
    }

    // Function to generate data to be stored in a blockchain
    std::string getBlockData() const {
        std::stringstream ss;
        ss << "Retailer ID: " << id << " | Retailer Name: " << name
           << " | Location: " << location
           << " | Credit Balance: RM" << std::fixed << std::setprecision(2) << creditBalance
           << " | Annual Credit Balance: RM" << std::fixed << std::setprecision(2) << annualCreditBalance;
        return ss.str();
    }

    // Function to serialize retailer data into a string format for storage
    std::string serialize() const {
        std::stringstream ss;
        ss << id << "|" << name << "|" << location << "|" << latitude << "|" << longitude << "|" 
           << creditBalance << "|" << annualCreditBalance;
        
        // Add product IDs to the serialized string
        ss << "|";
        for (size_t i = 0; i < productIds.size(); ++i) {
            ss << productIds[i];
            if (i < productIds.size() - 1) {
                ss << ",";
            }
        }
        
        return ss.str();
    }

    // Static function to deserialize a retailer from a serialized string
    static Retailer deserialize(const std::string& str) {
        std::stringstream ss(str);
        std::string item;
        std::vector<std::string> parts;
        
        // Splitting the input string by '|' delimiter
        while (std::getline(ss, item, '|')) {
            parts.push_back(item);
        }

        // Ensure the correct number of fields is present
        if (parts.size() < 7) {
            throw std::runtime_error("Invalid retailer data format");
        }

        // Extract and convert data from string format
        int id = std::stoi(parts[0]);
        std::string name = parts[1];
        std::string location = parts[2];
        double latitude = std::stod(parts[3]);
        double longitude = std::stod(parts[4]);
        double creditBalance = std::stod(parts[5]);
        double annualCreditBalance = std::stod(parts[6]);
        
        Retailer retailer(id, name, location, latitude, longitude, creditBalance, annualCreditBalance);
        
        // Add product IDs if present
        if (parts.size() > 7) {
            std::stringstream productStream(parts[7]);
            std::string productId;
            
            while (std::getline(productStream, productId, ',')) {
                if (!productId.empty()) {
                    retailer.addProduct(std::stoi(productId));
                }
            }
        }
        
        return retailer;
    }
};

// Transporter class
class Transporter {
private:
    int id; // Unique identifier for the transporter
    std::string name; // Name of the transporter
    std::string transportType; // Type of transportation (e.g., ground, air)
    double costPerKm; // Cost per kilometer for transportation
    double maxCapacity;  // Maximum capacity of the transporter in kilograms

public:
    // Constructor to initialize a Transporter object
    Transporter(int transporterId, const std::string& transporterName, const std::string& type,
                double perKmCost, double capacity)
        : id(transporterId), name(transporterName), transportType(type), 
          costPerKm(perKmCost), maxCapacity(capacity) {}

    // Getter function to return the transporter ID
    int getId() const { return id; }
    
    // Getter function to return the transporter name
    std::string getName() const { return name; }
    
    // Getter function to return the type of transportation
    std::string getTransportType() const { return transportType; }
    
    // Getter function to return the cost per kilometer for transportation
    double getCostPerKm() const { return costPerKm; }
    
    // Getter function to return the maximum capacity of the transporter
    double getMaxCapacity() const { return maxCapacity; }

    // Function to check if the transporter can handle a given weight
    bool canTransport(double weight) const {
        return weight <= maxCapacity;
    }

    // Function to calculate the transportation cost for a given distance
    double calculateTransportCost(double distance) const {
        return distance * costPerKm;
    }

    // Function to generate a string representation of the transporter details
    std::string toString() const {
        std::stringstream ss;
        ss << "Transporter ID: " << id << " | Name: " << name 
           << " | Type: " << transportType
           << " | Cost/km: RM" << std::fixed << std::setprecision(2) << costPerKm
           << " | Max Capacity: " << maxCapacity << "kg";
        return ss.str();
    }

    // Function to generate data to be stored in a blockchain
    std::string getBlockData() const {
        std::stringstream ss;
        ss << "Transporter ID: " << id << " | Transporter Name: " << name
           << " | Transportation Type: " << transportType
           << " | Cost/km: RM" << std::fixed << std::setprecision(2) << costPerKm
           << " | Max Capacity: " << maxCapacity << "kg";
        return ss.str();
    }

    // Function to serialize transporter data into a string format for storage
    std::string serialize() const {
        std::stringstream ss;
        ss << id << "|" << name << "|" << transportType << "|" << costPerKm << "|" << maxCapacity;
        return ss.str();
    }

    // Static function to deserialize a transporter from a serialized string
    static Transporter deserialize(const std::string& str) {
        std::stringstream ss(str);
        std::string item;
        std::vector<std::string> parts;
        
        // Splitting the input string by '|' delimiter
        while (std::getline(ss, item, '|')) {
            parts.push_back(item);
        }

        // Ensure the correct number of fields is present
        if (parts.size() < 5) {
            throw std::runtime_error("Invalid transporter data format");
        }

        // Extract and convert data from string format
        int id = std::stoi(parts[0]);
        std::string name = parts[1];
        std::string transportType = parts[2];
        double costPerKm = std::stod(parts[3]);
        double maxCapacity = std::stod(parts[4]);
        
        return Transporter(id, name, transportType, costPerKm, maxCapacity);
    }
};

// SmartContract class (abstract base class)
class SmartContract {
public:
    virtual ~SmartContract() = default; // Virtual destructor to allow proper cleanup of derived classes
    virtual bool validate(const Transaction& transaction) const = 0; // Pure virtual function to validate a transaction
    virtual std::string getDescription() const = 0; // Pure virtual function to get a description of the contract
};

// Transaction class
class Transaction {
private:
    int id; // Unique identifier for the transaction
    int supplierId; // ID of the supplier involved in the transaction
    int retailerId; // ID of the retailer involved in the transaction
    int productId; // ID of the product being transacted
    int transporterId; // ID of the transporter handling the delivery
    int quantity; // Quantity of the product being transacted
    double productCost; // Cost of the product
    double transportCost; // Cost of transportation
    double totalCost; // Total cost of the transaction (product cost + transport cost)
    std::string timestamp; // Timestamp of when the transaction was created
    std::string status; // Status of the transaction ("Pending", "Completed", "Failed")
    std::string orderType; // Type of order ("Seasonal", "Regular")

public:
    // Constructor to initialize a Transaction object
    Transaction(int transId, int sId, int rId, int pId, int tId, int qty)
        : id(transId), supplierId(sId), retailerId(rId), productId(pId), 
          transporterId(tId), quantity(qty), productCost(0), transportCost(0), totalCost(0) {
        timestamp = getCurrentTimestamp(); // Set the current timestamp
        status = "Pending"; // Set the initial status to "Pending"
        orderType = "Regular"; // Set the default order type to "Regular"
    }

    // Getter function to return the transaction ID
    int getId() const { return id; }
    // Getter function to return the supplier ID
    int getSupplierId() const { return supplierId; }
    // Getter function to return the retailer ID
    int getRetailerId() const { return retailerId; }
    // Getter function to return the product ID
    int getProductId() const { return productId; }
    // Getter function to return the transporter ID
    int getTransporterId() const { return transporterId; }
    // Getter function to return the quantity of the product
    int getQuantity() const { return quantity; }
    // Getter function to return the product cost
    double getProductCost() const { return productCost; }
    // Getter function to return the transport cost
    double getTransportCost() const { return transportCost; }
    // Getter function to return the total cost of the transaction
    double getTotalCost() const { return totalCost; }
    // Getter function to return the timestamp of the transaction
    std::string getTimestamp() const { return timestamp; }
    // Getter function to return the status of the transaction
    std::string getStatus() const { return status; }
    // Getter function to return the order type
    std::string getOrderType() const { return orderType; }

    // Setter function to set the product cost
    void setProductCost(double cost) { productCost = cost; }
    // Setter function to set the transport cost
    void setTransportCost(double cost) { transportCost = cost; }
    // Function to calculate the total cost of the transaction
    void calculateTotalCost() { totalCost = productCost + transportCost; }
    // Setter function to set the status of the transaction
    void setStatus(const std::string& newStatus) { status = newStatus; }
    // Setter function to set the order type
    void setOrderType(const std::string& type) { orderType = type; }

    // Function to generate a string representation of the transaction details
    std::string toString() const {
        std::stringstream ss;
        ss << "Transaction ID: " << id 
           << " | Supplier ID: " << supplierId
           << " | Retailer ID: " << retailerId
           << " | Product ID: " << productId
           << " | Transporter ID: " << transporterId
           << " | Quantity: " << quantity
           << " | Product Cost: RM" << std::fixed << std::setprecision(2) << productCost
           << " | Transport Cost: RM" << std::fixed << std::setprecision(2) << transportCost
           << " | Total Cost: RM" << std::fixed << std::setprecision(2) << totalCost
           << " | Timestamp: " << timestamp
           << " | Status: " << status
           << " | Order Type: " << orderType;
        return ss.str();
    }

    // Function to generate data to be stored in a blockchain
    std::string getBlockData() const {
        std::stringstream ss;
        ss << "Transaction ID: " << id 
           << " | Supplier ID: " << supplierId
           << " | Retailer ID: " << retailerId
           << " | Product ID: " << productId
           << " | Quantity: " << quantity
           << " | Total Cost: RM" << std::fixed << std::setprecision(2) << totalCost
           << " | Timestamp: " << timestamp
           << " | Status: " << status
           << " | Order Type: " << orderType;
        return ss.str();
    }

    // Function to serialize transaction data into a string format for storage
    std::string serialize() const {
        std::stringstream ss;
        ss << id << "|" << supplierId << "|" << retailerId << "|" << productId << "|" 
           << transporterId << "|" << quantity << "|" << productCost << "|" 
           << transportCost << "|" << totalCost << "|" << timestamp << "|" 
           << status << "|" << orderType;
        return ss.str();
    }

    // Static function to deserialize a transaction from a serialized string
    static Transaction deserialize(const std::string& str) {
        std::stringstream ss(str);
        std::string item;
        std::vector<std::string> parts;
        
        // Splitting the input string by '|' delimiter
        while (std::getline(ss, item, '|')) {
            parts.push_back(item);
        }

        // Ensure the correct number of fields is present
        if (parts.size() < 12) {
            throw std::runtime_error("Invalid transaction data format");
        }

        // Extract and convert data from string format
        int id = std::stoi(parts[0]);
        int supplierId = std::stoi(parts[1]);
        int retailerId = std::stoi(parts[2]);
        int productId = std::stoi(parts[3]);
        int transporterId = std::stoi(parts[4]);
        int quantity = std::stoi(parts[5]);
        
        Transaction transaction(id, supplierId, retailerId, productId, transporterId, quantity);
        
        transaction.setProductCost(std::stod(parts[6]));
        transaction.setTransportCost(std::stod(parts[7]));
        transaction.totalCost = std::stod(parts[8]);
        transaction.timestamp = parts[9];
        transaction.status = parts[10];
        transaction.orderType = parts[11];
        
        return transaction;
    }
};

// PriceThresholdContract class
class PriceThresholdContract : public SmartContract {
    private:
        double maxAllowedCost; // Maximum allowed cost for a transaction
    
    public:
        // Constructor to initialize the contract with a cost threshold
        PriceThresholdContract(double threshold) : maxAllowedCost(threshold) {}
    
        // Override the validate function to check if the transaction cost is within the allowed limit
        bool validate(const Transaction& transaction) const override {
            return transaction.getTotalCost() <= maxAllowedCost; // Return true if the transaction cost is within the limit
        }
    
        // Override the getDescription function to provide a description of the contract
        std::string getDescription() const override {
            std::stringstream ss;
            ss << "Price Threshold Contract: Maximum allowed cost is RM" 
               << std::fixed << std::setprecision(2) << maxAllowedCost; // Format the description with the cost threshold
            return ss.str(); // Return the description as a string
        }
    };

// ProductionPlanningSystem class
class ProductionPlanningSystem {
    private:
        std::vector<Product> products; // List of products in the system
        std::vector<Supplier> suppliers; // List of suppliers in the system
        std::vector<Retailer> retailers; // List of retailers in the system
        std::vector<Transporter> transporters; // List of transporters in the system
        std::vector<Transaction> transactions; // List of transactions in the system
        std::vector<SmartContract*> contracts; // List of smart contracts in the system
        Blockchain blockchain; // Blockchain instance to store transaction data
        
        int nextProductId; // Next available product ID
        int nextSupplierId; // Next available supplier ID
        int nextRetailerId; // Next available retailer ID
        int nextTransporterId; // Next available transporter ID
        int nextTransactionId; // Next available transaction ID

    // Helper function to find entities by ID
Product* findProduct(int id) {
    for (auto& product : products) {
        if (product.getId() == id) {
            return &product; // Return pointer to the product if found
        }
    }
    return nullptr; // Return nullptr if product not found
}

Supplier* findSupplier(int id) {
    for (auto& supplier : suppliers) {
        if (supplier.getId() == id) {
            return &supplier; // Return pointer to the supplier if found
        }
    }
    return nullptr; // Return nullptr if supplier not found
}

Retailer* findRetailer(int id) {
    for (auto& retailer : retailers) {
        if (retailer.getId() == id) {
            return &retailer; // Return pointer to the retailer if found
        }
    }
    return nullptr; // Return nullptr if retailer not found
}

   // Helper function to find a transporter by ID
Transporter* findTransporter(int id) {
    for (auto& transporter : transporters) {
        if (transporter.getId() == id) {
            return &transporter; // Return pointer to the transporter if found
        }
    }
    return nullptr; // Return nullptr if transporter not found
}

// Helper function to find a transaction by ID
Transaction* findTransaction(int id) {
    for (auto& transaction : transactions) {
        if (transaction.getId() == id) {
            return &transaction; // Return pointer to the transaction if found
        }
    }
    return nullptr; // Return nullptr if transaction not found
}

public:
    // Constructor to initialize the production planning system
    ProductionPlanningSystem() 
        : nextProductId(1), nextSupplierId(1), nextRetailerId(1), 
          nextTransporterId(1), nextTransactionId(1) {
        // Add default smart contract
        contracts.push_back(new PriceThresholdContract(4000.0));
        
        // Initialize with sample data
        loadSampleData();
    }
    ~ProductionPlanningSystem() {
        // Destructor to clean up dynamically allocated smart contracts
        for (auto contract : contracts) {
            delete contract; // Delete each smart contract to free memory
        }
    }
    
    void loadSampleData() {
        // Sample products
        addProduct("Rice", 5.50, 1000); // Add product "Rice" with price 5.50 and stock 1000
        addProduct("Vegetables", 3.20, 800); // Add product "Vegetables" with price 3.20 and stock 800
        addProduct("Fruits", 4.75, 600); // Add product "Fruits" with price 4.75 and stock 600
        
        // Sample suppliers
        addSupplier("Malayan Agro", "Lot 348, Kampung Datuk Keramat, 50400 Kuala Lumpur", 
                   "Federal Territory of Kuala Lumpur", 3.168, 101.708); // Add supplier "Malayan Agro"
        addSupplier("Farm Fresh Produce", "Jalan Tun Razak, 55000 Kuala Lumpur", 
                   "Federal Territory of Kuala Lumpur", 3.161, 101.720); // Add supplier "Farm Fresh Produce"
        
        // Link products to suppliers
        Supplier* s1 = findSupplier(1); // Find supplier with ID 1
        Supplier* s2 = findSupplier(2); // Find supplier with ID 2
        
        if (s1) {
            s1->addProduct(1); // Add product ID 1 to supplier 1
            s1->addProduct(2); // Add product ID 2 to supplier 1
        }
        
        if (s2) {
            s2->addProduct(2); // Add product ID 2 to supplier 2
            s2->addProduct(3); // Add product ID 3 to supplier 2
        }
        
        // Sample retailers
        addRetailer("SuperMart", "Bukit Bintang, 55100 Kuala Lumpur", 
                   3.148, 101.698, 10000.0, 100000.0); // Add retailer "SuperMart"
        addRetailer("FreshMart", "Petaling Jaya, 47800 Selangor", 
                   3.107, 101.607, 8000.0, 80000.0); // Add retailer "FreshMart"
        addRetailer("QuickMart", "Shah Alam, 40000 Selangor", 
                   3.073, 101.518, 5000.0, 50000.0); // Add retailer "QuickMart"
        
        // Sample transporters
        addTransporter("FastTruck", "Ordinary Ground Transfer", 2.50, 2000.0); // Add transporter "FastTruck"
        addTransporter("SpeedyDel", "Express Delivery", 3.75, 1500.0); // Add transporter "SpeedyDel"
        
        // Create blockchain with genesis block
        // Additional blocks will be added when transactions are created
    }

    // CRUD operations for products
int addProduct(const std::string& name, double price, int stock) {
    try {
        Product newProduct(nextProductId, name, price, stock); // Create a new product
        products.push_back(newProduct); // Add the new product to the products list
        
        // Add to blockchain
        std::string blockData = "Added Product | " + newProduct.toString(); // Prepare block data
        blockchain.addBlock(blockData); // Add a new block to the blockchain
        
        return nextProductId++; // Return the product ID and increment the next available product ID
    }
    catch (const std::exception& e) {
        std::cerr << "Error adding product: " << e.what() << std::endl; // Print error message
        return -1; // Return -1 to indicate failure
    }
}

void displayProducts() const {
    std::cout << "\n===== PRODUCTS =====" << std::endl; // Print header
    if (products.empty()) {
        std::cout << "No products available." << std::endl; // Print message if no products
        return;
    }
    
    for (const auto& product : products) {
        std::cout << product.toString() << std::endl; // Print each product's details
    }
}

// CRUD operations for suppliers
int addSupplier(const std::string& name, const std::string& location, 
               const std::string& branch, double latitude, double longitude) {
    try {
        Supplier newSupplier(nextSupplierId, name, location, branch, latitude, longitude); // Create a new supplier
        suppliers.push_back(newSupplier); // Add the new supplier to the suppliers list
        
        // Add to blockchain
        std::string blockData = "Added Supplier | " + newSupplier.toString(); // Prepare block data
        blockchain.addBlock(blockData); // Add a new block to the blockchain
        
        return nextSupplierId++; // Return the supplier ID and increment the next available supplier ID
    }
    catch (const std::exception& e) {
        std::cerr << "Error adding supplier: " << e.what() << std::endl; // Print error message
        return -1; // Return -1 to indicate failure
    }
}
void displaySuppliers() const {
    std::cout << "\n===== SUPPLIERS =====" << std::endl; // Print header
    if (suppliers.empty()) {
        std::cout << "No suppliers available." << std::endl; // Print message if no suppliers
        return;
    }
    
    for (const auto& supplier : suppliers) {
        std::cout << supplier.toString() << std::endl; // Print each supplier's details
        
        // Show linked products
        std::cout << "  Products: ";
        const auto& productIds = supplier.getProductIds();
        if (productIds.empty()) {
            std::cout << "None"; // Print message if no products linked to supplier
        } else {
            for (size_t i = 0; i < productIds.size(); ++i) {
                for (const auto& product : products) {
                    if (product.getId() == productIds[i]) {
                        std::cout << product.getName(); // Print product name
                        break;
                    }
                }
                if (i < productIds.size() - 1) {
                    std::cout << ", "; // Print comma separator between product names
                }
            }
        }
        std::cout << std::endl; // New line after listing products
    }
}

// CRUD operations for retailers
int addRetailer(const std::string& name, const std::string& location, 
               double latitude, double longitude, double initialCredit, double annualCredit) {
    try {
        Retailer newRetailer(nextRetailerId, name, location, latitude, longitude, initialCredit, annualCredit); // Create a new retailer
        retailers.push_back(newRetailer); // Add the new retailer to the retailers list
        
        // Add to blockchain
        std::string blockData = "Added Retailer | " + newRetailer.toString(); // Prepare block data
        blockchain.addBlock(blockData); // Add a new block to the blockchain
        
        return nextRetailerId++; // Return the retailer ID and increment the next available retailer ID
    }
    catch (const std::exception& e) {
        std::cerr << "Error adding retailer: " << e.what() << std::endl; // Print error message
        return -1; // Return -1 to indicate failure
    }
}
void displayRetailers() const {
    std::cout << "\n===== RETAILERS =====" << std::endl; // Print header
    if (retailers.empty()) {
        std::cout << "No retailers available." << std::endl; // Print message if no retailers
        return;
    }
    
    for (const auto& retailer : retailers) {
        std::cout << retailer.toString() << std::endl; // Print each retailer's details
    }
}

// CRUD operations for transporters
int addTransporter(const std::string& name, const std::string& type, 
                 double costPerKm, double capacity) {
    try {
        Transporter newTransporter(nextTransporterId, name, type, costPerKm, capacity); // Create a new transporter
        transporters.push_back(newTransporter); // Add the new transporter to the transporters list
        
        // Add to blockchain
        std::string blockData = "Added Transporter | " + newTransporter.toString(); // Prepare block data
        blockchain.addBlock(blockData); // Add a new block to the blockchain
        
        return nextTransporterId++; // Return the transporter ID and increment the next available transporter ID
    }
    catch (const std::exception& e) {
        std::cerr << "Error adding transporter: " << e.what() << std::endl; // Print error message
        return -1; // Return -1 to indicate failure
    }
}

void displayTransporters() const {
    std::cout << "\n===== TRANSPORTERS =====" << std::endl; // Print header
    if (transporters.empty()) {
        std::cout << "No transporters available." << std::endl; // Print message if no transporters
        return;
    }
    
    for (const auto& transporter : transporters) {
        std::cout << transporter.toString() << std::endl; // Print each transporter's details
    }
}
    // Transaction operations
int createTransaction(int supplierId, int retailerId, int productId, int transporterId, int quantity, const std::string& orderType = "Regular") {
    try {
        // Validate entities exist
        Supplier* supplier = findSupplier(supplierId);
        Retailer* retailer = findRetailer(retailerId);
        Product* product = findProduct(productId);
        Transporter* transporter = findTransporter(transporterId);
        
        if (!supplier) {
            throw std::runtime_error("Supplier ID not found");
        }
        if (!retailer) {
            throw std::runtime_error("Retailer ID not found");
        }
        if (!product) {
            throw std::runtime_error("Product ID not found");
        }
        if (!transporter) {
            throw std::runtime_error("Transporter ID not found");
        }
        
        // Check if supplier has this product
        bool supplierHasProduct = false;
        for (int id : supplier->getProductIds()) {
            if (id == productId) {
                supplierHasProduct = true;
                break;
            }
        }
        
        if (!supplierHasProduct) {
            throw std::runtime_error("This supplier does not supply this product");
        }
        
        // Check product stock
        if (!product->hasEnoughStock(quantity)) {
            throw std::runtime_error("Insufficient product stock");
        }
        
        // Calculate costs
        double productCost = product->getPrice() * quantity;
        
        // Calculate distance and transport cost
        double distance = calculateDistance(
            supplier->getLatitude(), supplier->getLongitude(),
            retailer->getLatitude(), retailer->getLongitude()
        );
        
        double transportCost = transporter->calculateTransportCost(distance);
        
        // Create transaction
        Transaction transaction(nextTransactionId, supplierId, retailerId, productId, transporterId, quantity);
        transaction.setProductCost(productCost);
        transaction.setTransportCost(transportCost);
        transaction.calculateTotalCost();
        transaction.setOrderType(orderType);
        
        // Validate with smart contracts
        bool isValid = true;
        for (const auto& contract : contracts) {
            if (!contract->validate(transaction)) {
                isValid = false;
                std::cout << "Transaction failed validation: " << contract->getDescription() << std::endl;
                break;
            }
        }
        
        if (!isValid) {
            transaction.setStatus("Failed");
            transactions.push_back(transaction);
            
            // Add to blockchain
            std::string blockData = "Failed Transaction | " + transaction.getBlockData();
            blockchain.addBlock(blockData);
            
            return nextTransactionId++;
        }
        
        // Check retailer credit
        double totalCost = transaction.getTotalCost();
        if (!retailer->deductCredit(totalCost)) {
            transaction.setStatus("Failed");
            std::cout << "Transaction failed: Insufficient retailer credit" << std::endl;
            transactions.push_back(transaction);
            
            // Add to blockchain
            std::string blockData = "Failed Transaction (Credit) | " + transaction.getBlockData();
            blockchain.addBlock(blockData);
            
            return nextTransactionId++;
        }
        
        // Update product stock
        product->updateStock(-quantity);
        
        // Complete transaction
        transaction.setStatus("Completed");
        transactions.push_back(transaction);
        
        // Add to blockchain
        std::string blockData = "Completed Transaction | " + transaction.getBlockData();
        blockchain.addBlock(blockData);
        
        return nextTransactionId++;
    }
    catch (const std::exception& e) {
        std::cerr << "Error creating transaction: " << e.what() << std::endl;
        return -1;
    }
}
void displayTransactions() const {
    std::cout << "\n===== TRANSACTIONS =====" << std::endl; // Print header
    if (transactions.empty()) {
        std::cout << "No transactions available." << std::endl; // Print message if no transactions
        return;
    }
    
    for (const auto& transaction : transactions) {
        std::cout << transaction.toString() << std::endl; // Print each transaction's details
        std::cout << "--------------------------------------" << std::endl; // Separator line
    }
}

// Blockchain operations
void displayBlockchain() const {
    std::cout << "\n===== BLOCKCHAIN =====" << std::endl; // Print header
    if (blockchain.size() <= 1) {
        std::cout << "Only genesis block available." << std::endl; // Print message if only genesis block is available
    }
    
    blockchain.displayChain(); // Display all blocks in the blockchain
    
    if (blockchain.isChainValid()) {
        std::cout << "Blockchain integrity: VALID" << std::endl; // Print message if blockchain is valid
    } else {
        std::cout << "Blockchain integrity: COMPROMISED" << std::endl; // Print message if blockchain is compromised
    }
}

// Data persistence
bool saveData() const {
    try {
        // Save blockchain
        if (!blockchain.saveToFile("blockchain.dat")) {
            return false; // Return false if saving blockchain fails
        }
        
        // Save products
        std::ofstream productFile("products.dat");
        if (!productFile.is_open()) {
            return false; // Return false if opening product file fails
        }
        for (const auto& product : products) {
            productFile << product.serialize() << std::endl; // Save each product to file
        }
        productFile.close();
        
        // Save suppliers
        std::ofstream supplierFile("suppliers.dat");
        if (!supplierFile.is_open()) {
            return false; // Return false if opening supplier file fails
        }
        for (const auto& supplier : suppliers) {
            supplierFile << supplier.serialize() << std::endl; // Save each supplier to file
        }
        supplierFile.close();
        
        // Save retailers
        std::ofstream retailerFile("retailers.dat");
        if (!retailerFile.is_open()) {
            return false; // Return false if opening retailer file fails
        }
        for (const auto& retailer : retailers) {
            retailerFile << retailer.serialize() << std::endl; // Save each retailer to file
        }
        retailerFile.close();
        
        // Save transporters
        std::ofstream transporterFile("transporters.dat");
        if (!transporterFile.is_open()) {
            return false; // Return false if opening transporter file fails
        }
        for (const auto& transporter : transporters) {
            transporterFile << transporter.serialize() << std::endl; // Save each transporter to file
        }
        transporterFile.close();
        
        // Save transactions
        std::ofstream transactionFile("transactions.dat");
        if (!transactionFile.is_open()) {
            return false; // Return false if opening transaction file fails
        }
        for (const auto& transaction : transactions) {
            transactionFile << transaction.serialize() << std::endl; // Save each transaction to file
        }
        transactionFile.close();
        
        // Save next IDs
        std::ofstream idFile("nextids.dat");
        if (!idFile.is_open()) {
            return false; // Return false if opening ID file fails
        }
        idFile << nextProductId << std::endl;
        idFile << nextSupplierId << std::endl;
        idFile << nextRetailerId << std::endl;
        idFile << nextTransporterId << std::endl;
        idFile << nextTransactionId << std::endl;
        idFile.close();
        
        return true; // Return true if all data is saved successfully
    }
    catch (const std::exception& e) {
        std::cerr << "Error saving data: " << e.what() << std::endl; // Print error message
        return false; // Return false if an exception occurs
    }
}
bool loadData() {
    try {
        // Clear existing data
        products.clear();
        suppliers.clear();
        retailers.clear();
        transporters.clear();
        transactions.clear();
        
        // Load blockchain
        if (!blockchain.loadFromFile("blockchain.dat")) {
            return false; // Return false if loading blockchain fails
        }
        
        // Load products
        std::ifstream productFile("products.dat");
        if (productFile.is_open()) {
            std::string line;
            while (std::getline(productFile, line)) {
                try {
                    Product product = Product::deserialize(line); // Deserialize product from file
                    products.push_back(product); // Add product to list
                }
                catch (const std::exception& e) {
                    std::cerr << "Error deserializing product: " << e.what() << std::endl; // Print error message
                }
            }
            productFile.close();
        }
        
        // Load suppliers
        std::ifstream supplierFile("suppliers.dat");
        if (supplierFile.is_open()) {
            std::string line;
            while (std::getline(supplierFile, line)) {
                try {
                    Supplier supplier = Supplier::deserialize(line); // Deserialize supplier from file
                    suppliers.push_back(supplier); // Add supplier to list
                }
                catch (const std::exception& e) {
                    std::cerr << "Error deserializing supplier: " << e.what() << std::endl; // Print error message
                }
            }
            supplierFile.close();
        }
        
        // Load retailers
        std::ifstream retailerFile("retailers.dat");
        if (retailerFile.is_open()) {
            std::string line;
            while (std::getline(retailerFile, line)) {
                try {
                    Retailer retailer = Retailer::deserialize(line); // Deserialize retailer from file
                    retailers.push_back(retailer); // Add retailer to list
                }
                catch (const std::exception& e) {
                    std::cerr << "Error deserializing retailer: " << e.what() << std::endl; // Print error message
                }
            }
            retailerFile.close();
        }
        
        // Load transporters
        std::ifstream transporterFile("transporters.dat");
        if (transporterFile.is_open()) {
            std::string line;
            while (std::getline(transporterFile, line)) {
                try {
                    Transporter transporter = Transporter::deserialize(line); // Deserialize transporter from file
                    transporters.push_back(transporter); // Add transporter to list
                }
                catch (const std::exception& e) {
                    std::cerr << "Error deserializing transporter: " << e.what() << std::endl; // Print error message
                }
            }
            transporterFile.close();
        }
        
        // Load transactions
        std::ifstream transactionFile("transactions.dat");
        if (transactionFile.is_open()) {
            std::string line;
            while (std::getline(transactionFile, line)) {
                try {
                    Transaction transaction = Transaction::deserialize(line); // Deserialize transaction from file
                    transactions.push_back(transaction); // Add transaction to list
                }
                catch (const std::exception& e) {
                    std::cerr << "Error deserializing transaction: " << e.what() << std::endl; // Print error message
                }
            }
            transactionFile.close();
        }
        
        // Load next IDs
        std::ifstream idFile("nextids.dat");
        if (idFile.is_open()) {
            idFile >> nextProductId;
            idFile >> nextSupplierId;
            idFile >> nextRetailerId;
            idFile >> nextTransporterId;
            idFile >> nextTransactionId;
            idFile.close();
        }
        
        return true; // Return true if all data is loaded successfully
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading data: " << e.what() << std::endl; // Print error message
        return false; // Return false if an exception occurs
    }
}

    // Simulation and Reports
    void runSeasonalSimulation() {
        std::cout << "\n===== RUNNING SEASONAL SIMULATION =====" << std::endl;
        
        // 1. Create seasonal demand
        int highDemandProductId = 1; // Rice
        int normalDemandProductId = 2; // Vegetables
        
        // 2. Process transactions for all retailers
        for (const auto& retailer : retailers) {
            // High demand product transaction (large quantity)
            std::cout << "Creating seasonal high-demand transaction for " << retailer.getName() << std::endl;
            createTransaction(1, retailer.getId(), highDemandProductId, 1, 100, "Seasonal");
            
            // Normal demand product transaction
            std::cout << "Creating seasonal normal-demand transaction for " << retailer.getName() << std::endl;
            createTransaction(2, retailer.getId(), normalDemandProductId, 2, 50, "Seasonal");
        }
        
        std::cout << "Seasonal simulation complete." << std::endl;
    }

    void generateDistributionReport() const {
        std::cout << "\n===== DISTRIBUTION REPORT =====" << std::endl;
        
        // Count transactions by status
        int completed = 0;
        int failed = 0;
        double totalRevenue = 0.0;
        std::unordered_map<int, int> productQuantities;
        
        for (const auto& transaction : transactions) {
            if (transaction.getStatus() == "Completed") {
                completed++;
                totalRevenue += transaction.getTotalCost();
                
                // Add to product quantities
                int productId = transaction.getProductId();
                int quantity = transaction.getQuantity();
                productQuantities[productId] += quantity;
            } else if (transaction.getStatus() == "Failed") {
                failed++;
            }
        }
        
        // Print summary
        std::cout << "Completed Transactions: " << completed << std::endl;
        std::cout << "Failed Transactions: " << failed << std::endl;
        std::cout << "Total Revenue: RM" << std::fixed << std::setprecision(2) << totalRevenue << std::endl;
        
        // Print product distribution
        std::cout << "\nProduct Distribution:" << std::endl;
        for (const auto& pair : productQuantities) {
            int productId = pair.first;
            int quantity = pair.second;
            
            for (const auto& product : products) {
                if (product.getId() == productId) {
                    std::cout << product.getName() << ": " << quantity << " units" << std::endl;
                    break;
                }
            }
        }
    }

    void beginRetailerTransaction() {
        try {
            // 1. Display available retailers
            displayRetailers();
            
            // 2. Select retailer
            int retailerId;
            std::cout << "\nEnter Retailer ID: ";
            std::cin >> retailerId;
            
            Retailer* retailer = findRetailer(retailerId);
            if (!retailer) {
                throw std::runtime_error("Retailer not found");
            }
            
            // 3. Display available suppliers
            displaySuppliers();
            
            // 4. Select supplier
            int supplierId;
            std::cout << "\nEnter Supplier ID: ";
            std::cin >> supplierId;
            
            Supplier* supplier = findSupplier(supplierId);
            if (!supplier) {
                throw std::runtime_error("Supplier not found");
            }
            
            // 5. Display supplier's products
            std::cout << "\nAvailable products from this supplier:" << std::endl;
            for (int prodId : supplier->getProductIds()) {
                Product* product = findProduct(prodId);
                if (product) {
                    std::cout << product->toString() << std::endl;
                }
            }
            
            // 6. Select product
            int productId;
            std::cout << "\nEnter Product ID: ";
            std::cin >> productId;
            
            // Check if supplier has this product
            bool hasProduct = false;
            for (int id : supplier->getProductIds()) {
                if (id == productId) {
                    hasProduct = true;
                    break;
                }
            }
            
            if (!hasProduct) {
                throw std::runtime_error("This supplier does not provide this product");
            }
            
            // 7. Enter quantity
            int quantity;
            std::cout << "Enter Quantity: ";
            std::cin >> quantity;
            
            if (quantity <= 0) {
                throw std::runtime_error("Quantity must be positive");
            }
            
            // 8. Display available transporters
            displayTransporters();
            
            // 9. Select transporter
            int transporterId;
            std::cout << "\nEnter Transporter ID: ";
            std::cin >> transporterId;
            
            // 10. Create transaction
            int transactionId = createTransaction(supplierId, retailerId, productId, transporterId, quantity);
            
            if (transactionId != -1) {
                std::cout << "\nTransaction " << transactionId << " created successfully." << std::endl;
            } else {
                std::cout << "\nTransaction failed." << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error creating transaction: " << e.what() << std::endl;
        }
    }

    void resetSystem() {
        try {
            // Clear all data structures
            products.clear();
            suppliers.clear();
            retailers.clear();
            transporters.clear();
            transactions.clear();
            
            // Reset ID counters
            nextProductId = 1;
            nextSupplierId = 1;
            nextRetailerId = 1;
            nextTransporterId = 1;
            nextTransactionId = 1;
            
            // Reinitialize blockchain (create new genesis block)
            blockchain = Blockchain();
            
            // Re-initialize with sample data
            loadSampleData();
            
            std::cout << "System reset successful. All data has been cleared and reinitialized." << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Error resetting system: " << e.what() << std::endl;
        }
    }

    void optimizeDistributionRoute() {
        try {
            // Check if we have enough retailers
            if (retailers.size() < 2) {
                throw std::runtime_error("Need at least 2 retailers to optimize routes");
            }

            std::cout << "\n===== OPTIMIZING DISTRIBUTION ROUTE =====" << std::endl;
            
            // First, select a starting supplier
            displaySuppliers();
            int supplierId;
            std::cout << "Enter starting Supplier ID: ";
            std::cin >> supplierId;
            
            Supplier* supplier = findSupplier(supplierId);
            if (!supplier) {
                throw std::runtime_error("Supplier not found");
            }
            
            // Create distance matrix between all retailers
            std::vector<std::vector<double>> distances(retailers.size(), std::vector<double>(retailers.size(), 0.0));
            for (size_t i = 0; i < retailers.size(); ++i) {
                for (size_t j = 0; j < retailers.size(); ++j) {
                    if (i != j) {
                        distances[i][j] = calculateDistance(
                            retailers[i].getLatitude(), retailers[i].getLongitude(),
                            retailers[j].getLatitude(), retailers[j].getLongitude()
                        );
                    }
                }
            }
            
            // Map to convert retailer index to ID
            std::vector<int> indexToId(retailers.size());
            std::unordered_map<int, size_t> idToIndex;
            for (size_t i = 0; i < retailers.size(); ++i) {
                indexToId[i] = retailers[i].getId();
                idToIndex[retailers[i].getId()] = i;
            }
            
            // Greedy TSP algorithm
            std::vector<int> route;
            std::vector<bool> visited(retailers.size(), false);
            
            // Calculate distance from supplier to each retailer
            std::vector<double> supplierDistances(retailers.size());
            for (size_t i = 0; i < retailers.size(); ++i) {
                supplierDistances[i] = calculateDistance(
                    supplier->getLatitude(), supplier->getLongitude(),
                    retailers[i].getLatitude(), retailers[i].getLongitude()
                );
            }
            
            // Start with nearest retailer to supplier
            size_t currentIndex = 0;
            double minDist = std::numeric_limits<double>::max();
            for (size_t i = 0; i < retailers.size(); ++i) {
                if (supplierDistances[i] < minDist) {
                    minDist = supplierDistances[i];
                    currentIndex = i;
                }
            }
            
            route.push_back(indexToId[currentIndex]);
            visited[currentIndex] = true;
            
            // Greedy selection of nearest unvisited retailer
            for (size_t i = 1; i < retailers.size(); ++i) {
                size_t nextIndex = 0;
                double minDist = std::numeric_limits<double>::max();
                
                for (size_t j = 0; j < retailers.size(); ++j) {
                    if (!visited[j] && distances[currentIndex][j] < minDist) {
                        minDist = distances[currentIndex][j];
                        nextIndex = j;
                    }
                }
                
                route.push_back(indexToId[nextIndex]);
                visited[nextIndex] = true;
                currentIndex = nextIndex;
            }
            
            // Calculate total route distance including return to supplier
            double totalDistance = supplierDistances[idToIndex[route[0]]]; // Supplier to first retailer
            
            for (size_t i = 0; i < route.size() - 1; ++i) {
                totalDistance += distances[idToIndex[route[i]]][idToIndex[route[i+1]]];
            }
            
            // Add distance from last retailer back to supplier
            totalDistance += calculateDistance(
                retailers[idToIndex[route.back()]].getLatitude(), 
                retailers[idToIndex[route.back()]].getLongitude(),
                supplier->getLatitude(), supplier->getLongitude()
            );
            
            // Display optimized route
            std::cout << "\nOptimized Distribution Route:" << std::endl;
            std::cout << "Starting at Supplier: " << supplier->getName() << " (ID: " << supplier->getId() << ")" << std::endl;
            
            for (size_t i = 0; i < route.size(); ++i) {
                for (const auto& retailer : retailers) {
                    if (retailer.getId() == route[i]) {
                        std::cout << (i+1) << ". " << retailer.getName() 
                                  << " (ID: " << retailer.getId() << ")" << std::endl;
                        break;
                    }
                }
            }
            
            std::cout << "Return to Supplier: " << supplier->getName() << std::endl;
            std::cout << "Total Distance: " << std::fixed << std::setprecision(2) 
                      << totalDistance << " km" << std::endl;
            
            // Create distribution plan with estimated costs
            std::cout << "\nDistribution Plan:" << std::endl;
            
            // Ask for product and transporter
            displayTransporters();
            int transporterId;
            std::cout << "Select Transporter ID for this route: ";
            std::cin >> transporterId;
            
            Transporter* transporter = findTransporter(transporterId);
            if (!transporter) {
                throw std::runtime_error("Transporter not found");
            }
            
            // Calculate transportation cost
            double transportCost = transporter->calculateTransportCost(totalDistance);
            
            std::cout << "Transportation Cost: RM" << std::fixed << std::setprecision(2) 
                      << transportCost << std::endl;
            
            // Add to blockchain for record-keeping
            std::stringstream ss;
            ss << "Optimized Route | Supplier: " << supplier->getId() 
               << " | Retailers: ";
            for (int id : route) {
                ss << id << ",";
            }
            ss << " | Distance: " << totalDistance 
               << " | Transporter: " << transporterId 
               << " | Cost: " << transportCost;
            
            blockchain.addBlock(ss.str());
            std::cout << "Route optimization recorded in blockchain." << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Error optimizing route: " << e.what() << std::endl;
        }
    }

    void optimizeInventory() {
        try {
            std::cout << "\n===== INVENTORY OPTIMIZATION =====" << std::endl;
            
            if (products.empty() || retailers.empty()) {
                throw std::runtime_error("Need products and retailers to optimize inventory");
            }
            
            // Display products to select for optimization
            displayProducts();
            
            int productId;
            std::cout << "Select Product ID to optimize inventory: ";
            std::cin >> productId;
            
            Product* product = findProduct(productId);
            if (!product) {
                throw std::runtime_error("Product not found");
            }
            
            // Analyze transaction history to determine demand patterns
            std::unordered_map<int, int> retailerDemand; // retailerId -> total quantity
            std::unordered_map<int, int> transactionCount; // retailerId -> transaction count
            
            for (const auto& transaction : transactions) {
                if (transaction.getProductId() == productId && transaction.getStatus() == "Completed") {
                    int retailerId = transaction.getRetailerId();
                    retailerDemand[retailerId] += transaction.getQuantity();
                    transactionCount[retailerId]++;
                }
            }
            
            // Calculate total available stock
            int totalStock = product->getStock();
            int totalDemand = 0;
            
            for (const auto& pair : retailerDemand) {
                totalDemand += pair.second;
            }
            
            std::cout << "\nCurrent Product Stock: " << totalStock << " units" << std::endl;
            std::cout << "Total Historical Demand: " << totalDemand << " units" << std::endl;
            
            // Calculate allocation based on historical demand percentage
            std::unordered_map<int, int> optimalAllocation;
            std::cout << "\nOptimal Inventory Allocation:" << std::endl;
            std::cout << "--------------------------------" << std::endl;
            std::cout << "Retailer\t\tHistorical Demand\tOptimal Allocation" << std::endl;
            
            for (const auto& retailer : retailers) {
                int retailerId = retailer.getId();
                int demand = retailerDemand[retailerId];
                int allocation = 0;
                
                // Calculate allocation - retailers with no history get minimum stock
                if (totalDemand > 0) {
                    double demandPercentage = static_cast<double>(demand) / totalDemand;
                    allocation = static_cast<int>(totalStock * demandPercentage);
                } else {
                    // Equal distribution if no historical data
                    allocation = totalStock / retailers.size();
                }
                
                // Ensure minimum allocation (safety stock)
                allocation = std::max(allocation, 10); // Minimum 10 units
                
                optimalAllocation[retailerId] = allocation;
                
                std::cout << retailer.getName() << " (ID: " << retailerId << ")\t" 
                          << demand << " units\t\t" 
                          << allocation << " units" << std::endl;
            }
            
            // Calculate efficiency metrics
            double totalAllocation = 0;
            for (const auto& pair : optimalAllocation) {
                totalAllocation += pair.second;
            }
            
            // Adjust if total allocation exceeds stock
            if (totalAllocation > totalStock) {
                double adjustmentFactor = totalStock / totalAllocation;
                
                std::cout << "\nAdjusting allocation to match available stock..." << std::endl;
                
                for (auto& pair : optimalAllocation) {
                    pair.second = static_cast<int>(pair.second * adjustmentFactor);
                }
                
                // Display adjusted allocation
                std::cout << "\nAdjusted Inventory Allocation:" << std::endl;
                std::cout << "--------------------------------" << std::endl;
                
                for (const auto& retailer : retailers) {
                    int retailerId = retailer.getId();
                    std::cout << retailer.getName() << " (ID: " << retailerId << ")\t" 
                              << optimalAllocation[retailerId] << " units" << std::endl;
                }
            }
            
            // Calculate potential savings
            double serviceLevel = 0.95; // 95% service level target
            double holdingCost = product->getPrice() * 0.2; // 20% of product price as annual holding cost
            double currentHoldingCost = 0;
            double optimizedHoldingCost = 0;
            
            // Estimate current inventory distribution (equally distributed)
            int equalAllocation = totalStock / retailers.size();
            
            for (const auto& retailer : retailers) {
                currentHoldingCost += equalAllocation * holdingCost;
                optimizedHoldingCost += optimalAllocation[retailer.getId()] * holdingCost;
            }
            
            double savings = currentHoldingCost - optimizedHoldingCost;
            
            std::cout << "\nInventory Cost Analysis:" << std::endl;
            std::cout << "Estimated Current Holding Cost: RM" << std::fixed << std::setprecision(2) << currentHoldingCost << std::endl;
            std::cout << "Optimized Holding Cost: RM" << std::fixed << std::setprecision(2) << optimizedHoldingCost << std::endl;
            std::cout << "Potential Annual Savings: RM" << std::fixed << std::setprecision(2) << savings << std::endl;
            
            // Ask user if they want to implement the optimization
            char implement;
            std::cout << "\nImplement this inventory optimization? (y/n): ";
            std::cin >> implement;
            
            if (implement == 'y' || implement == 'Y') {
                // Record in blockchain
                std::stringstream ss;
                ss << "Inventory Optimization | Product: " << product->getId() 
                   << " | Total Stock: " << totalStock
                   << " | Optimization Savings: " << savings;
                
                blockchain.addBlock(ss.str());
                
                std::cout << "Inventory optimization plan recorded in blockchain." << std::endl;
                std::cout << "To implement: Create transactions to distribute inventory according to the plan." << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error optimizing inventory: " << e.what() << std::endl;
        }
    }

    void addNewProduct() {
        try {
            std::string name;
            double price;
            int stock;

            std::cout << "Enter product name: ";
            std::cin.ignore();
            std::getline(std::cin, name);

            std::cout << "Enter price: ";
            if (!(std::cin >> price) || price < 0) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                throw std::runtime_error("Invalid price");
            }

            std::cout << "Enter initial stock: ";
            if (!(std::cin >> stock) || stock < 0) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                throw std::runtime_error("Invalid stock");
            }

            int id = addProduct(name, price, stock);
            if (id != -1) {
                std::cout << "Product added with ID: " << id << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error adding product: " << e.what() << std::endl;
        }
    }

    void addNewSupplier() {
        try {
            std::string name, location, branch;
            double latitude, longitude;

            std::cout << "Enter supplier name: ";
            std::cin.ignore();
            std::getline(std::cin, name);

            std::cout << "Enter location: ";
            std::getline(std::cin, location);

            std::cout << "Enter branch: ";
            std::getline(std::cin, branch);

            std::cout << "Enter latitude: ";
            if (!(std::cin >> latitude) || latitude < -90 || latitude > 90) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                throw std::runtime_error("Invalid latitude (-90 to 90)");
            }

            std::cout << "Enter longitude: ";
            if (!(std::cin >> longitude) || longitude < -180 || longitude > 180) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                throw std::runtime_error("Invalid longitude (-180 to 180)");
            }

            int id = addSupplier(name, location, branch, latitude, longitude);
            if (id != -1) {
                std::cout << "Supplier added with ID: " << id << std::endl;

                // Link products to supplier
                displayProducts();
                char addProduct;
                do {
                    int productId;
                    std::cout << "Enter product ID to link to supplier (0 to stop): ";
                    std::cin >> productId;

                    if (productId == 0) break;

                    if (findProduct(productId)) {
                        findSupplier(id)->addProduct(productId);
                        std::cout << "Product linked to supplier." << std::endl;
                    } else {
                        std::cout << "Product not found. Try again." << std::endl;
                    }

                    std::cout << "Add another product? (y/n): ";
                    std::cin >> addProduct;
                } while (addProduct == 'y' || addProduct == 'Y');
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error adding supplier: " << e.what() << std::endl;
        }
    }

    void addNewRetailer() {
        try {
            std::string name, location;
            double latitude, longitude, initialCredit, annualCredit;

            std::cout << "Enter retailer name: ";
            std::cin.ignore();
            std::getline(std::cin, name);

            std::cout << "Enter location: ";
            std::getline(std::cin, location);

            std::cout << "Enter latitude: ";
            if (!(std::cin >> latitude) || latitude < -90 || latitude > 90) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                throw std::runtime_error("Invalid latitude (-90 to 90)");
            }

            std::cout << "Enter longitude: ";
            if (!(std::cin >> longitude) || longitude < -180 || longitude > 180) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                throw std::runtime_error("Invalid longitude (-180 to 180)");
            }

            std::cout << "Enter initial credit: ";
            if (!(std::cin >> initialCredit) || initialCredit < 0) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                throw std::runtime_error("Invalid credit amount");
            }

            std::cout << "Enter annual credit limit: ";
            if (!(std::cin >> annualCredit) || annualCredit < 0) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                throw std::runtime_error("Invalid credit amount");
            }

            int id = addRetailer(name, location, latitude, longitude, initialCredit, annualCredit);
            if (id != -1) {
                std::cout << "Retailer added with ID: " << id << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error adding retailer: " << e.what() << std::endl;
        }
    }

    void addNewTransporter() {
        try {
            std::string name, type;
            double costPerKm, capacity;

            std::cout << "Enter transporter name: ";
            std::cin.ignore();
            std::getline(std::cin, name);

            std::cout << "Enter type (e.g., Truck, Van): ";
            std::getline(std::cin, type);

            std::cout << "Enter cost per km: ";
            if (!(std::cin >> costPerKm) || costPerKm <= 0) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                throw std::runtime_error("Invalid cost");
            }

            std::cout << "Enter capacity (kg): ";
            if (!(std::cin >> capacity) || capacity <= 0) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                throw std::runtime_error("Invalid capacity");
            }

            int id = addTransporter(name, type, costPerKm, capacity);
            if (id != -1) {
                std::cout << "Transporter added with ID: " << id << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error adding transporter: " << e.what() << std::endl;
        }
    }
};

// Main function
int main() {
    // Seed for random number generation
    srand(static_cast<unsigned int>(time(nullptr)));
    
    try {
        ProductionPlanningSystem system; // Create an instance of the production planning system
        
        int choice;
        bool running = true;
        
        // Main menu loop
        while (running) {
            std::cout << "\n===== AGRO PRODUCTION PLANNING AND DISTRIBUTION MANAGEMENT SYSTEM =====" << std::endl;
            std::cout << "1. Display Products" << std::endl;
            std::cout << "2. Display Suppliers" << std::endl;
            std::cout << "3. Display Retailers" << std::endl;
            std::cout << "4. Display Transporters" << std::endl;
            std::cout << "5. Display Transactions" << std::endl;
            std::cout << "6. Create New Transaction" << std::endl;
            std::cout << "7. Run Seasonal Simulation" << std::endl;
            std::cout << "8. Generate Distribution Report" << std::endl;
            std::cout << "9. Display Blockchain" << std::endl;
            std::cout << "10. Save Data" << std::endl;
            std::cout << "11. Load Data" << std::endl;
            std::cout << "12. Reset System" << std::endl;
            std::cout << "13. Optimize Distribution Route" << std::endl;
            std::cout << "14. Optimize Inventory" << std::endl; // New option
            std::cout << "15. Add New Product" << std::endl; // New option
            std::cout << "16. Add New Supplier" << std::endl; // New option
            std::cout << "17. Add New Retailer" << std::endl; // New option
            std::cout << "18. Add New Transporter" << std::endl; // New option
            std::cout << "0. Exit" << std::endl;
            std::cout << "Enter your choice: ";
            
            if (!(std::cin >> choice)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid input. Please enter a number." << std::endl;
                continue;
            }
            
            switch (choice) {
                case 1:
                    system.displayProducts(); // Display all products
                    break;
                case 2:
                    system.displaySuppliers(); // Display all suppliers
                    break;
                case 3:
                    system.displayRetailers(); // Display all retailers
                    break;
                case 4:
                    system.displayTransporters(); // Display all transporters
                    break;
                case 5:
                    system.displayTransactions(); // Display all transactions
                    break;
                case 6:
                    system.beginRetailerTransaction(); // Create a new transaction
                    break;
                case 7:
                    system.runSeasonalSimulation(); // Run seasonal simulation
                    break;
                case 8:
                    system.generateDistributionReport(); // Generate distribution report
                    break;
                case 9:
                    system.displayBlockchain(); // Display blockchain
                    break;
                case 10:
                    if (system.saveData()) {
                        std::cout << "Data saved successfully." << std::endl;
                    } else {
                        std::cout << "Failed to save data." << std::endl;
                    }
                    break;
                case 11:
                    if (system.loadData()) {
                        std::cout << "Data loaded successfully." << std::endl;
                    } else {
                        std::cout << "Failed to load data or no saved data found." << std::endl;
                    }
                    break;
                case 12:
                    {
                        char confirm;
                        std::cout << "WARNING: This will erase all current data. Continue? (y/n): ";
                        std::cin >> confirm;
                        if (confirm == 'y' || confirm == 'Y') {
                            system.resetSystem(); // Reset the system
                        }
                    }
                    break;
                case 13:
                    system.optimizeDistributionRoute(); // Optimize distribution route
                    break;
                case 14:
                    system.optimizeInventory(); // Optimize inventory
                    break;
                case 15:
                    system.addNewProduct(); // Add new product
                    break;
                case 16:
                    system.addNewSupplier(); // Add new supplier
                    break;
                case 17:
                    system.addNewRetailer(); // Add new retailer
                    break;
                case 18:
                    system.addNewTransporter(); // Add new transporter
                    break;
                case 0:
                    running = false; // Exit the system
                    std::cout << "Exiting system. Thank you!" << std::endl;
                    break;
                default:
                    std::cout << "Invalid choice. Please try again." << std::endl;
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Critical error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}