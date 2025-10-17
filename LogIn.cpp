#include <iostream>
#include <string>
#include <format>
#include <unordered_map>
#include <conio.h>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <ctime>
#include <thread>

using namespace std;
namespace fs = filesystem;
namespace colour
{
    const string RESET = "\033[0m";
    const string RED = "\033[31m";
    const string GREEN = "\033[32m";
    const string YELLOW = "\033[33m";
    const string BLUE = "\033[34m";
    const string CYAN = "\033[36m";
    const string BOLD = "\033[1m";
}

enum class JournalAction { Open, Remove };
enum class DeleteResult {Cancelled, Failed, Success};

const int screenDelay = 2000;


void clearScreen(int delayMs = 0)
{
    if (delayMs > 0)
        this_thread::sleep_for(chrono::milliseconds(delayMs));

#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}


string getPassword()
{
    string password{};
    char ch{};

    while ((ch = _getch()) != '\r')
    {
        if (ch == '\b')
        {
            if (!password.empty())
            {
                password.pop_back();
                cout << "\b \b";
            }
        }
        else
        {
            password.push_back(ch);
            cout << '*';
        }
    }
    cout << endl;
    return password;
}


string simpleHashPassword(const string& password)
{
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < password.size(); ++i)
    {
        uint8_t c = static_cast<uint8_t>(password[i]);
        hash ^= (c + static_cast<uint8_t>(i));
        hash *= 16777619u;
    }

    ostringstream oss{};
    oss << hex << setw(8) << setfill('0') << hash;
    return oss.str();
}


string generateSalt()
{
    string salt{};
    for (int i = 0; i < 8; ++i)
    {
        salt += static_cast<char>('a' + rand() % 26);
    }
    return salt;
}


class account
{
private:
    string username;
    string password;
    string salt;
public:
    account(string a, string b, string c = "") : username(a), password(b), salt(c) {}

    string getUsername() const { return username; }
    string getPassword() const { return password; }
    string getSalt() const { return salt; }
   
    void setPassword(const string& newPass) { password = newPass; }
    void setSalt(const string& newSalt) { salt = newSalt; }

    void display() const
    {
        cout << format("Username: {}\nPassword: {}", username, password) << endl;
    }
};


unordered_map<string, account> accounts{};


void saveAccounts()
{
    ofstream outfile("accounts.txt");

    for (const auto& pair : accounts)
    {
        outfile << pair.second.getUsername() << " "
                << pair.second.getPassword() << " "
                << pair.second.getSalt() << endl;
    }

    outfile.close();
}


void loadAccounts()
{
    ifstream infile("accounts.txt");
    if (!infile)
    {
        cout << colour::RED << "No saved accounts found" << endl;
        return;
    }

    string username{}, password{}, salt{};

    while (infile >> username >> password >> salt)
    {
        accounts.emplace(username, account(username, password, salt));
    }

    infile.close();
}


void newDirectory(const string& directoryName)
{
    fs::path directoryPath = format("{}'s folder", directoryName);

    if (!fs::exists(directoryPath))
    {
        fs::create_directory(directoryPath);
    }
}


void sanitiseFilename(string& name, bool toFile)
{
    for (auto& c : name)
        if (toFile ? c == ' ' : c == '_')
            c = toFile ? '_' : ' ';
}


void newJournal(const string& username)
{
    cout << endl;
    string filename{}, line{};
    fs::path userPath = (format("{}'s folder", username));

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << colour::YELLOW << "To cancel type 'q' in Entry Title" 
         << colour::RESET << endl;
    cout << "Entry Title: ";
    getline(cin, filename);

    if (filename == "q" || filename == "Q")
    {
        cout << endl;
        return;
    }

    sanitiseFilename(filename, true);

    cout << "Journal Entry (" << colour::YELLOW 
         << "Type 'END' on a new line to finish" 
         << colour::RESET <<"):\n";
    
    ofstream outfile(userPath / (format("{}.txt", filename)));

    auto now = chrono::system_clock::now();
    auto time = chrono::system_clock::to_time_t(now);
    tm localTime{};
    localtime_s(&localTime, &time);
    outfile << "Date created: " << put_time(&localTime, "%Y-%m-%d %H:%M:%S") << "\n\n";

    while (true)
    {
        getline(cin, line);
        if (line == "END") break;
        outfile << line << endl;
    }

    outfile.close();
    cout << colour::GREEN << "\nEntry added\n" << endl;
    clearScreen(screenDelay);
}


void getJournalEntries(const string& username, JournalAction action)
{
    cout << endl;
    fs::path userPath = format("{}'s folder", username);
    vector<string> entries{};

    for (const auto& entry : fs::directory_iterator(userPath))
    {
        if (entry.path().extension() == ".txt")
        {
            entries.push_back(entry.path().stem().string());
        }
    }

    if (entries.empty())
    {
        cout << colour::RED << "No journal entries found\n" << endl;
        clearScreen(screenDelay);
        return;
    }

    cout << colour::YELLOW << ((action == JournalAction::Open)
        ? "Select an entry to open (0 to cancel): \n"
        : "Select an entry to delete (0 to cancel): \n");

    for (size_t i = 0; i < entries.size(); ++i)
    {
        sanitiseFilename(entries[i], false);

        cout << colour::CYAN << i + 1 << ". " << entries[i] << endl;
    }

    int choice{};
    cout << colour::RESET << "Input: ";
    cin >> choice;
    cout << endl;

    if (choice <= 0 || choice > entries.size()) return;

    string displayName = entries[choice - 1];
    string filename = displayName;
    sanitiseFilename(filename, true);

    fs::path filePath = userPath / format("{}.txt", filename);

    if (action == JournalAction::Open)
    {
        ifstream infile(filePath);
        cout << format("{}:\n", displayName);
        string line{};
        while (getline(infile, line))
        {
            cout << line << endl;
        }
        infile.close();

        cout << colour::YELLOW 
             << "\nWould you like to edit this entry? (y/n): "
             << colour::RESET;
        char editChoice{};
        cin >> editChoice;
        cout << endl;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (editChoice == 'y' || editChoice == 'Y')
        {
            ifstream infile(filePath);
            string originalDate{};
            getline(infile, originalDate);
            infile.close();

            ofstream outfile(filePath);
            outfile << originalDate << '\n';

            auto now = chrono::system_clock::now();
            auto time = chrono::system_clock::to_time_t(now);
            tm localTime{};
            localtime_s(&localTime, &time);
            outfile << "Date last edited: " 
                    << put_time(&localTime, "%Y-%m-%d %H:%M:%S") << "\n\n";

            cout << "Enter new content ("
                 << colour::YELLOW << "Type 'END' to finish" 
                 << colour::RESET << "):\n";
            while (true)
            {
                getline(cin, line);
                if (line == "END") break;
                outfile << line << endl;
            }
            
            outfile.close();
            cout << colour::GREEN << "\nEntry updated.\n" << endl;
            clearScreen(screenDelay);
        }
    }
    else if (action == JournalAction::Remove)
    {
        cout << colour::YELLOW
             << "Are you sure you want to delete this entry? (y/n): "
             << colour::RESET;
        char confirm{};
        cin >> confirm;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (confirm == 'y' || confirm == 'Y')
        {
            fs::remove(filePath);
            cout << colour::GREEN << "\nEntry deleted.\n" << endl;
            clearScreen(screenDelay);
        }
    }
}


void viewJournal(const string& username)
{
    getJournalEntries(username, JournalAction::Open);
}


void deleteJournal(const string& username)
{
    getJournalEntries(username, JournalAction::Remove);
}


bool isValidPassword(const string& password)
{
    return !password.empty() && password.length() >= 6;
}


void changePassword(const string& username)
{
    auto it = accounts.find(username);
    if (it == accounts.end()) return;

    while (true)
    {
        cout << colour::YELLOW << "\nTo cancel type 'q'" << colour::RESET;
        cout << "\nOld Password: ";
        auto it = accounts.find(username);
        string oldPassword = getPassword();
        if (oldPassword == "q" || oldPassword == "Q") return;

        string hashedOld = simpleHashPassword(it->second.getSalt() + oldPassword);

        if (hashedOld != it->second.getPassword())
        {
            cout << colour::RED << "\nIncorrect password" << endl;
            continue;
        }
            
        while (true)
        {
            cout << "New Password: ";
            string newPassword = getPassword();

            if (newPassword == "q" || newPassword == "Q") return;

            if (!isValidPassword(newPassword))
            {
                cout << colour::RED
                     << "\nPassword does not meet requirements\n"
                        "Password cannot be empty\n"
                        "Password must be at least 6 characters long\n\n" 
                     << colour::RESET;
                continue;
            }

            cout << "Confirm New Password: ";
            string confirmPassword = getPassword();
            if (newPassword != confirmPassword)
            {
                cout << colour::RED << "\nPasswords do not match\n\n" 
                     << colour::RESET;
                continue;
            }
                
            string newSalt = generateSalt();
            string newHash = simpleHashPassword(newSalt + newPassword);

            it->second.setSalt(newSalt);
            it->second.setPassword(newHash);

            saveAccounts();
            cout << colour::GREEN << "\nYour Password Has Been Changed\n\n";
            clearScreen(screenDelay);
            return;
        }
    }
}


DeleteResult deleteAccount(const string& username)
{
    string choice{};
    cout << colour::YELLOW << "\nAre you sure? (y/n): ";
    cin >> choice;
    cout << endl;

    if (choice == "n" || choice == "N") return DeleteResult::Cancelled;

    fs::path userPath = format("{}'s folder", username);

    auto it = accounts.find(username);

    if (it != accounts.end())
    {
        fs::remove_all(userPath);
        accounts.erase(username);
        saveAccounts();
        cout << colour::GREEN << "Account deleted\n"
                                 "\nYou have been logged out\n" << endl;
        clearScreen(screenDelay);
        return DeleteResult::Success;
    }

    cout << colour::RED << "Error: Account not found\n" << endl;
    clearScreen(screenDelay);
    return DeleteResult::Failed;
}


void loggedIn(const string& name)
{
    while (true)
    {
        clearScreen();

        string username{ name };
        string welcomeMessage = format("Welcome {}", name);

        if (welcomeMessage.length() > 28)
            welcomeMessage = welcomeMessage.substr(0, 28);

        int totalWidth = 32;
        int padding = totalWidth - static_cast<int>(welcomeMessage.length()) - 1;


        cout << colour::CYAN << "|--------------------------------|\n";
        cout << "| " << colour::BOLD << welcomeMessage << colour::RESET;
        cout << string(padding, ' ') << colour::CYAN << "|\n"
                "|--------------------------------|\n"
                "| 1. Add Journal Entry           |\n"
                "| 2. View Journal Entry          |\n"
                "| 3. Delete Journal Entry        |\n"
                "| 4. Change Password             |\n"
                "| 5. Delete Account              |\n"
                "| 6. Log Out                     |\n"
                "|--------------------------------|\n" << colour::RESET;
        cout << "Input: ";


        int input{};
        if (!(cin >> input))
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << colour::RED << "\nInvalid input, please enter a number\n\n"
                << colour::RESET;
            clearScreen(screenDelay);
            continue;
        }

        switch (input)
        {
        case 1:
            newJournal(username);
            break;
        case 2:
            viewJournal(username);
            break;
        case 3:
            deleteJournal(username);
            break;
        case 4:
            changePassword(username);
            break;
        case 5:
        {
            DeleteResult result = deleteAccount(username);
            if (result == DeleteResult::Cancelled) break;
            if (result == DeleteResult::Success) return;
        }
            break;
        case 6:
            cout << colour::GREEN << "\nLogged Out\n" << endl;
            clearScreen(screenDelay);
            return;
        default:
            cout << colour::RED 
                 << "\nInvalid Input, plese choose 1, 2, 3, 4, 5, or 6\n\n";
            clearScreen(screenDelay);
            break;
        }
    }
}


void createAccount()
{
    string username{}, password1{}, password2{};
    while (true)
    {
        cout << colour::BLUE << colour::BOLD 
             << "\nCREATE ACCOUNT:\n" << colour::RESET;
        cout << colour::YELLOW << "To cancel type 'q' in Username" 
             << colour::RESET<< endl;
        cout << "Username: ";
        cin >> username;

        if (username == "q" || username == "Q")
        {
            cout << endl;
            return;
        }

        auto it = accounts.find(username);

        if (it == accounts.end())
        {
            cout << "Password: ";
            password1 = getPassword();
            cout << "Confirm Password: ";
            password2 = getPassword();

            if (!isValidPassword(password1))
            {
                cout << colour::RED << "\nPassword does not meet requirements\n"
                     << "Password cannot be empty\n"
                     << "Password must be at least 6 characters long\n" << endl;
                continue;
            }

            if (password1 == password2)
            {
                string salt = generateSalt();
                string hashed = simpleHashPassword(salt + password1);
                accounts.emplace(username, account(username, hashed, salt));
                cout << colour::GREEN << "\nAccount Created" << endl;
                cout << endl;
                clearScreen(screenDelay);
                saveAccounts();
                newDirectory(username);
                return;
            }
            else
            {
                cout << colour::RED << "\nPasswords do not match" << endl;
            }
        }
        else
        {
            cout << colour::RED << "\nUsername already exists" << endl;
        }
    }
}


void logIn()
{
    int counter{};

    while (true)
    {
        cout << colour::BLUE << colour::BOLD << "\nLOG IN:\n" << colour::RESET;
        cout << colour::YELLOW << "To cancel type 'q' in Username" 
             << colour::RESET << endl;

        string username{}, password{};
        cout << "Username: ";
        cin >> username;

        if (username == "q" || username == "Q")
        {
            cout << endl;
            return;
        }

        auto it = accounts.find(username);

        if (it != accounts.end())
        {
            cout << "Password: ";
            password = getPassword();
            string hashedInput = simpleHashPassword(it->second.getSalt() + password);
            if (it->second.getPassword() == hashedInput)
            {
                cout << endl;
                clearScreen();
                loggedIn(username);
                return;
            }
            else
            {
                cout << colour::RED << "\nIncorrect password" << endl;
            }
        }
        else 
        {
            cout << colour::RED << "\nIncorrect Username" << endl;
        }

        counter++;
        if (counter >= 3)
        {
            cout << colour::RED << "\nToo many failed attempts\n" << endl;
            clearScreen(screenDelay);
            return;
        }
    }
}


void welcome()
{
    loadAccounts();

    while (true)
    {
        clearScreen();

        cout << colour::CYAN << "|--------------------------------|\n";
        cout << "| " << colour::BOLD << "Welcome to the beta test!" << colour::RESET << colour::CYAN << "      |\n";
        cout << "|--------------------------------|\n";
        cout << "| 1. Create An Account           |\n"
                "| 2. Log In                      |\n"
                "| 3. Exit                        |\n"
                "|--------------------------------|\n" << colour::RESET;
        cout << "Input: ";

        int input{};
        if (!(cin >> input))
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << colour::RED << "\nInvalid input, please enter a number\n\n"
                << colour::RESET;
            clearScreen(screenDelay);
            continue;
        }

        switch (input)
        {
        case 1:
            createAccount();
            break;
        case 2:
            logIn();
            break;
        case 3:
            cout << colour::GREEN << "\nExited" << colour::RESET << endl;
            return;
        default:
            cout << colour::RED 
                 << "\nInvalid input, please choose 1, 2, or 3\n\n";
            clearScreen(screenDelay);
            break;
        }
    }
}


int main()
{
    srand(static_cast<unsigned int>(time(nullptr)));
    welcome();
}