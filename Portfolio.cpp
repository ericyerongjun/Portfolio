#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <ctime>   
#include <cstdlib> 
using namespace std;

enum TransactionType
{
    BUY,
    SELL,
    PRICE_UPDATE
};

// Class to represent a transaction
class Transaction
{
private:
    TransactionType type;
    string ticker;
    double price;
    int quantity;
    time_t timestamp;

public:
    Transaction(TransactionType t, string tick, double p, int q)
    {
        type = t;
        ticker = tick;
        price = p;
        quantity = q;
        timestamp = time(NULL); // Current time
    }

    void print() const
    {
        string typeStr = (type == BUY ? "BUY" : (type == SELL ? "SELL" : "PRICE_UPDATE"));
        cout << typeStr << " " << ticker << " - Price: " << price
             << ", Shares: " << quantity << ", Time: " << ctime(&timestamp);
    }

    double getValue() const { return price * quantity; }
    TransactionType getType() const { return type; }
};

class Stock
{
private:
    string ticker;
    double price;
    int quantity;
    double volatility;           
    vector<Transaction> history; // Transaction history for this stock

public:
    Stock(string t, double p, int q, double vol = 0.1)
    {
        ticker = t;
        price = p;
        quantity = q;
        volatility = vol;
        if (p < 0 || q < 0 || vol < 0)
        {
            throw invalid_argument("Price, quantity, and volatility must be non-negative.");
        }
        history.push_back(Transaction(BUY, ticker, price, quantity)); // Initial buy
    }

    string getTicker() const { return ticker; }
    double getPrice() const { return price; }
    int getQuantity() const { return quantity; }
    double getVolatility() const { return volatility; }
    double getValue() const { return price * quantity; }

    void updatePrice(double newPrice)
    {
        if (newPrice >= 0)
        {
            history.push_back(Transaction(PRICE_UPDATE, ticker, newPrice, quantity));
            price = newPrice;
        }
        else
        {
            throw invalid_argument("Price cannot be negative.");
        }
    }

    void addShares(int additionalQty, double buyPrice)
    {
        if (additionalQty > 0 && buyPrice >= 0)
        {
            quantity += additionalQty;
            history.push_back(Transaction(BUY, ticker, buyPrice, additionalQty));
        }
        else
        {
            throw invalid_argument("Additional quantity and buy price must be positive.");
        }
    }

    bool sellShares(int sellQty, double sellPrice)
    {
        if (sellQty > 0 && sellQty <= quantity && sellPrice >= 0)
        {
            quantity -= sellQty;
            history.push_back(Transaction(SELL, ticker, sellPrice, sellQty));
            return true;
        }
        return false;
    }

    void printHistory() const
    {
        cout << "Transaction History for " << ticker << ":" << endl;
        for (vector<Transaction>::const_iterator it = history.begin(); it != history.end(); ++it)
        {
            it->print();
        }
    }

    // Simulate price fluctuation based on volatility
    void simulatePriceUpdate()
    {
        double change = (rand() % 200 - 100) / 100.0 * volatility * price; // -100 to +100% of volatility
        updatePrice(price + change);
    }
};

class Portfolio
{
private:
    vector<Stock> stocks;
    string portfolioName;
    double cashBalance; // Cash available for trading

public:
    Portfolio(string name, double initialCash = 10000.0)
    {
        portfolioName = (name.empty() ? "Unnamed Portfolio" : name);
        cashBalance = initialCash;
    }

    void addStock(const Stock &stock)
    {
        stocks.push_back(stock);
        cashBalance -= stock.getValue(); // Deduct cost from cash
    }

    bool removeStock(string ticker)
    {
        for (vector<Stock>::iterator it = stocks.begin(); it != stocks.end(); ++it)
        {
            if (it->getTicker() == ticker)
            {
                cashBalance += it->getValue(); // Add sale proceeds to cash
                stocks.erase(it);
                return true;
            }
        }
        return false;
    }

    double getTotalValue() const
    {
        double total = cashBalance;
        for (vector<Stock>::const_iterator it = stocks.begin(); it != stocks.end(); ++it)
        {
            total += it->getValue();
        }
        return total;
    }

    void printPortfolio() const
    {
        if (stocks.empty())
        {
            cout << "Portfolio is empty" << endl;
        }
        else
        {
            cout << "Portfolio: " << portfolioName << endl;
            for (vector<Stock>::const_iterator it = stocks.begin(); it != stocks.end(); ++it)
            {
                cout << it->getTicker() << ", Price: " << it->getPrice()
                     << ", Shares: " << it->getQuantity()
                     << ", Value: " << it->getValue() << endl;
            }
        }
        cout << "Cash Balance: " << cashBalance << endl;
        cout << "Total portfolio value: " << getTotalValue() << endl;
    }

    string getName() const { return portfolioName; }

    bool updateStockPrice(string ticker, double newPrice)
    {
        for (vector<Stock>::iterator it = stocks.begin(); it != stocks.end(); ++it)
        {
            if (it->getTicker() == ticker)
            {
                it->updatePrice(newPrice);
                return true;
            }
        }
        return false;
    }

    // Buy additional shares of an existing stock or add new stock
    void buyStock(string ticker, double price, int qty)
    {
        for (vector<Stock>::iterator it = stocks.begin(); it != stocks.end(); ++it)
        {
            if (it->getTicker() == ticker)
            {
                double cost = price * qty;
                if (cost <= cashBalance)
                {
                    it->addShares(qty, price);
                    cashBalance -= cost;
                }
                else
                {
                    cout << "Insufficient cash to buy " << qty << " shares of " << ticker << endl;
                }
                return;
            }
        }
        // If stock not found, add it as new
        double cost = price * qty;
        if (cost <= cashBalance)
        {
            addStock(Stock(ticker, price, qty, 0.15)); // Default volatility 15%
        }
        else
        {
            cout << "Insufficient cash to buy " << qty << " shares of " << ticker << endl;
        }
    }

    // Sell shares of a stock
    bool sellStock(string ticker, int qty, double sellPrice)
    {
        for (vector<Stock>::iterator it = stocks.begin(); it != stocks.end(); ++it)
        {
            if (it->getTicker() == ticker)
            {
                if (it->sellShares(qty, sellPrice))
                {
                    cashBalance += sellPrice * qty;
                    if (it->getQuantity() == 0)
                    {
                        stocks.erase(it); // Remove stock if no shares left
                    }
                    return true;
                }
                return false;
            }
        }
        return false;
    }

    // Calculate portfolio diversification (simple: % of total value per stock)
    void printDiversification() const
    {
        double totalValue = getTotalValue() - cashBalance; // Exclude cash
        if (totalValue <= 0)
        {
            cout << "No stock value to analyze diversification" << endl;
            return;
        }
        cout << "Portfolio Diversification:" << endl;
        for (vector<Stock>::const_iterator it = stocks.begin(); it != stocks.end(); ++it)
        {
            double percentage = (it->getValue() / totalValue) * 100;
            cout << it->getTicker() << ": " << percentage << "%" << endl;
        }
    }

    // Simulate market update for all stocks
    void simulateMarketUpdate()
    {
        for (vector<Stock>::iterator it = stocks.begin(); it != stocks.end(); ++it)
        {
            it->simulatePriceUpdate();
        }
    }

    // Print transaction history for all stocks
    void printAllHistories() const
    {
        for (vector<Stock>::const_iterator it = stocks.begin(); it != stocks.end(); ++it)
        {
            it->printHistory();
        }
    }
};

int main()
{
    srand(time(NULL));                            
    Portfolio retire("Retirement Fund", 50000.0); // Start with $50,000 cash

    // Initial investments
    retire.addStock(Stock("AAPL", 150.0, 50, 0.2));  // Apple, high volatility
    retire.addStock(Stock("GOOG", 2000.0, 10, 0.1)); // Google, moderate volatility
    retire.addStock(Stock("BND", 80.0, 100, 0.05));  // Bond ETF, low volatility
    cout << "Initial Portfolio:" << endl;
    retire.printPortfolio();
    retire.printDiversification();
    cout << endl;

    // Simulate buying more shares
    retire.buyStock("AAPL", 155.0, 20); // Buy 20 more AAPL shares
    cout << "After buying 20 more AAPL shares:" << endl;
    retire.printPortfolio();
    cout << endl;

    // Simulate selling shares
    if (retire.sellStock("GOOG", 5, 2050.0))
    {
        cout << "After selling 5 GOOG shares:" << endl;
        retire.printPortfolio();
    }
    else
    {
        cout << "Failed to sell GOOG shares" << endl;
    }
    cout << endl;

    // Simulate real-time market update
    retire.simulateMarketUpdate();
    cout << "After market price update:" << endl;
    retire.printPortfolio();
    retire.printDiversification();
    cout << endl;

    // Update price manually
    retire.updateStockPrice("BND", 82.0);
    cout << "After manual BND price update to 82.0:" << endl;
    retire.printPortfolio();
    cout << endl;

    // Remove a stock entirely
    if (retire.removeStock("AAPL"))
    {
        cout << "After removing AAPL:" << endl;
        retire.printPortfolio();
    }
    cout << endl;

    // Print full transaction history
    cout << "Full Transaction History:" << endl;
    retire.printAllHistories();

    return 0;
}