#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <mysql.h> // Include MySQL library

// Function to establish a database connection
MYSQL* connectToDatabase() {
    MYSQL* connection = mysql_init(nullptr);
    if (connection == nullptr) {
        std::cerr << "mysql_init() failed" << std::endl;
        return nullptr;
    }

    // Replace with your actual MySQL server details
    const char* server = "localhost";
    const char* user = "your_user";        // e.g., "root"
    const char* password = "your_password";  // Your MySQL password
    const char* database = "book_keeping"; // The database name you'll create

    if (!mysql_real_connect(connection, server, user, password, database, 0, nullptr, 0)) {
        std::cerr << "mysql_real_connect() failed: " << mysql_error(connection) << std::endl;
        mysql_close(connection);
        return nullptr;
    }
    return connection;
}

// Function to close the database connection
void closeDatabase(MYSQL* connection) {
    if (connection) {
        mysql_close(connection);
    }
}

// Function to execute a SQL query
int executeSQL(MYSQL* connection, const std::string& sql) {
    if (mysql_query(connection, sql.c_str())) {
        std::cerr << "MySQL query error: " << mysql_error(connection) << " (" << sql << ")" << std::endl;
        return 1; // Return 1 to indicate error
    }
    return 0; // Return 0 to indicate success
}

// Function to create the books table if it doesn't exist
void createBooksTable(MYSQL* connection) {
    std::string sql = "CREATE TABLE IF NOT EXISTS books ("
                      "id INT AUTO_INCREMENT PRIMARY KEY, "
                      "title VARCHAR(255) NOT NULL, "
                      "author VARCHAR(255) NOT NULL, "
                      "isbn VARCHAR(20) NOT NULL UNIQUE, "
                      "published_date DATE NOT NULL);";
    if (executeSQL(connection, sql) != 0) {
        std::cerr << "Error creating books table." << std::endl;
        //  No return here, the program can still run.  We'll check for valid db later.
    }
}

// Function to add a new book to the database
int addBook(MYSQL* connection, const std::string& title, const std::string& author, const std::string& isbn, const std::string& publishedDate) {
    if (!connection) return 1; // Check for a valid database connection
    std::string sql = "INSERT INTO books (title, author, isbn, published_date) VALUES ('"
                      + title + "', '" + author + "', '" + isbn + "', '" + publishedDate + "');";
    if (executeSQL(connection, sql) != 0) {
        return 1; // Error occurred
    }
    std::cout << "Book added successfully." << std::endl;
    return 0;
}

// Function to display all books from the database
void displayAllBooks(MYSQL* connection) {
    if (!connection) {
        std::cerr << "Invalid database connection" << std::endl;
        return;
    }
    std::string sql = "SELECT id, title, author, isbn, published_date FROM books;";
    if (executeSQL(connection, sql) != 0) {
        return;
    }

    MYSQL_RES* result = mysql_store_result(connection);
    if (result == nullptr) {
        std::cerr << "MySQL store result error: " << mysql_error(connection) << std::endl;
        return;
    }

    int numFields = mysql_num_fields(result);
    MYSQL_FIELD* fields = mysql_fetch_fields(result);

    std::cout << std::left;
    for (int i = 0; i < numFields; ++i) {
        std::cout << std::setw(20) << fields[i].name;
    }
    std::cout << std::endl;
    for (int i = 0; i < numFields; ++i) {
        std::cout << std::setw(20) << std::string(20, '-');
    }
    std::cout << std::endl;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        for (int i = 0; i < numFields; ++i) {
            std::cout << std::setw(20) << (row[i] ? row[i] : "NULL");
        }
        std::cout << std::endl;
    }
    mysql_free_result(result);
}

// Function to get a single book's details by ID
void getBookDetails(MYSQL* connection, int id) {
    if (!connection) {
        std::cerr << "Invalid database connection" << std::endl;
        return;
    }
    std::string sql = "SELECT title, author, isbn, published_date FROM books WHERE id = " + std::to_string(id) + ";";
    if (executeSQL(connection, sql) != 0) {
        return;
    }

    MYSQL_RES* result = mysql_store_result(connection);
    if (result == nullptr) {
        std::cerr << "MySQL store result error: " << mysql_error(connection) << std::endl;
        return;
    }

    if (mysql_num_rows(result) == 0) {
        std::cout << "Book with ID " << id << " not found." << std::endl;
    } else {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row) {
            std::cout << "Title: " << (row[0] ? row[0] : "NULL") << std::endl;
            std::cout << "Author: " << (row[1] ? row[1] : "NULL") << std::endl;
            std::cout << "ISBN: " << (row[2] ? row[2] : "NULL") << std::endl;
            std::cout << "Published Date: " << (row[3] ? row[3] : "NULL") << std::endl;
        }
    }
    mysql_free_result(result);
}

// Function to update a book's details
int updateBook(MYSQL* connection, int id, const std::string& title, const std::string& author, const std::string& isbn, const std::string& publishedDate) {
    if (!connection) return 1;
    std::string sql = "UPDATE books SET title = '" + title + "', author = '" + author + "', isbn = '" + isbn + "', published_date = '" + publishedDate + "' WHERE id = " + std::to_string(id) + ";";
    if (executeSQL(connection, sql) != 0) {
        return 1;
    }
    std::cout << "Book updated successfully." << std::endl;
    return 0;
}

// Function to delete a book
int deleteBook(MYSQL* connection, int id) {
    if (!connection) return 1;
    std::string sql = "DELETE FROM books WHERE id = " + std::to_string(id) + ";";
    if (executeSQL(connection, sql) != 0) {
        return 1;
    }
    std::cout << "Book deleted successfully." << std::endl;
    return 0;
}

// Function to run the book keeping application
void runBookKeepingApp() {
    MYSQL* connection = connectToDatabase();
    if (!connection) {
        std::cerr << "Failed to connect to the database. Exiting." << std::endl;
        return; // Exit if the database connection fails
    }

    createBooksTable(connection);

    int choice;
    int id;
    std::string title, author, isbn, publishedDate;

    while (true) {
        std::cout << "\n--- Book Keeping Application ---" << std::endl;
        std::cout << "1. Add Book" << std::endl;
        std::cout << "2. Display All Books" << std::endl;
        std::cout << "3. Get Book Details" << std::endl;
        std::cout << "4. Update Book" << std::endl;
        std::cout << "5. Delete Book" << std::endl;
        std::cout << "6. Exit" << std::endl;
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        // added to handle non int inputs
        if (std::cin.fail()) {
            std::cout << "Invalid input. Please enter a number." << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear input buffer

        switch (choice) {
            case 1:
                std::cout << "Enter title: ";
                std::getline(std::cin, title);
                std::cout << "Enter author: ";
                std::getline(std::cin, author);
                std::cout << "Enter ISBN: ";
                std::getline(std::cin, isbn);
                std::cout << "Enter published date (YYYY-MM-DD): ";
                std::getline(std::cin, publishedDate);
                addBook(connection, title, author, isbn, publishedDate);
                break;
            case 2:
                displayAllBooks(connection);
                break;
            case 3:
                std::cout << "Enter book ID: ";
                if (!(std::cin >> id)) {
                    std::cerr << "Invalid input for book ID." << std::endl;
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    break;
                }
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                getBookDetails(connection, id);
                break;
            case 4:
                std::cout << "Enter book ID: ";
                 if (!(std::cin >> id)) {
                    std::cerr << "Invalid input for book ID." << std::endl;
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    break;
                }
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Enter title: ";
                std::getline(std::cin, title);
                std::cout << "Enter author: ";
                std::getline(std::cin, author);
                std::cout << "Enter ISBN: ";
                std::getline(std::cin, isbn);
                std::cout << "Enter published date (YYYY-MM-DD): ";
                std::getline(std::cin, publishedDate);
                updateBook(connection, id, title, author, isbn, publishedDate);
                break;
            case 5:
                std::cout << "Enter book ID: ";
                if (!(std::cin >> id)) {
                    std::cerr << "Invalid input for book ID." << std::endl;
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    break;
                }
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                deleteBook(connection, id);
                break;
            case 6:
                std::cout << "Exiting application." << std::endl;
                closeDatabase(connection);
                return;
            default:
                std::cout << "Invalid choice. Please try again." << std::endl;
        }
    }
    closeDatabase(connection);
}

int main() {
    runBookKeepingApp();
    return 0;
}
