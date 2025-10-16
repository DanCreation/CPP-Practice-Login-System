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

using namespace std;
namespace fs = filesystem;

enum class JournalAction { Open, Remove };
enum class DeleteResult {Cancelled, Failed, Success};

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
        cout << "No saved accounts found" << endl;
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

    cout << "To cancel type 'q' in Entry Title" << endl;
    cout << "Entry Title: ";
    getline(cin, filename);

    if (filename == "q" || filename == "Q")
    {
        cout << endl;
        return;
    }

    sanitiseFilename(filename, true);

    cout << "Journal Entry (Type 'END' on a new line to finish):\n";
    
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
    cout << "\nEntry added\n" << endl;
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
        cout << "No journal entries found\n" << endl;
        return;
    }

    cout << ((action == JournalAction::Open)
        ? "Select an entry to open (0 to cancel): \n"
        : "Select an entry to delete (0 to cancel): \n");

    for (size_t i = 0; i < entries.size(); ++i)
    {
        sanitiseFilename(entries[i], false);

        cout << i + 1 << ". " << entries[i] << endl;
    }

    int choice{};
    cout << "Input: ";
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

        cout << "\nWould you like to edit this entry? (y/n): ";
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
            outfile << "Date last edited: " << put_time(&localTime, "%Y-%m-%d %H:%M:%S") << "\n\n";

            cout << "Enter new content (Type 'END' to finish):\n";
            while (true)
            {
                getline(cin, line);
                if (line == "END") break;
                outfile << line << endl;
            }
            
            outfile.close();
            cout << "\nEntry updated.\n" << endl;
        }
    }
    else if (action == JournalAction::Remove)
    {
        cout << "Are you sure you want to delete this entry? (y/n): ";
        char confirm{};
        cin >> confirm;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (confirm == 'y' || confirm == 'Y')
        {
            fs::remove(filePath);
            cout << "\nEntry deleted.\n" << endl;
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
        cout << "\nTo cancel type 'q'";
        cout << "\nOld Password: ";
        auto it = accounts.find(username);
        string oldPassword = getPassword();
        if (oldPassword == "q" || oldPassword == "Q") return;

        string hashedOld = simpleHashPassword(it->second.getSalt() + oldPassword);

        if (hashedOld != it->second.getPassword())
        {
            cout << "Incorrect password" << endl;
            continue;
        }
            
        while (true)
        {
            cout << "New Password: ";
            string newPassword = getPassword();

            if (newPassword == "q" || newPassword == "Q") return;

            if (!isValidPassword(newPassword))
            {
                cout << "\nPassword does not meet requirements\n"
                    << "Password cannot be empty\n"
                    << "Password must be at least 6 characters long\n" << endl;
                continue;
            }

            cout << "Confirm New Password: ";
            string confirmPassword = getPassword();
            if (newPassword != confirmPassword)
            {
                cout << "Passwords do not match\n" << endl;
                continue;
            }
                
            string newSalt = generateSalt();
            string newHash = simpleHashPassword(newSalt + newPassword);

            it->second.setSalt(newSalt);
            it->second.setPassword(newHash);

            saveAccounts();
            cout << "\nYour Password Has Been Changed\n" << endl;
            return;
        }
    }
}


DeleteResult deleteAccount(const string& username)
{
    string choice{};
    cout << "\nAre you sure? (y/n): ";
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
        cout << "Account deleted\n" << endl;
        cout << "You have been logged out\n" << endl;
        return DeleteResult::Success;
    }

    cout << "Error: Account not found\n" << endl;
    return DeleteResult::Failed;
}


void loggedIn(const string& name)
{
    while (true)
    {
        string username{ name };

        cout << format("Welcome {}", name) << endl;
        cout << "1. Add Journal Entry\n"
                "2. View Journal Entry\n"
                "3. Delete Journal Entry\n"
                "4. Change Password\n"
                "5. Delete Account\n"
                "6. Log Out\n"
                "Input: ";


        int input{};
        cin >> input;

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
            cout << "\nLogged Out\n" << endl;
            return;
        default:
            cout << "Invalid Input\n" << endl;
            break;
        }
    }
}


void createAccount()
{
    string username{}, password1{}, password2{};
    while (true)
    {
        cout << "\nCREATE ACCOUNT:\nTo cancel type 'q' in Username" << endl;
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
                cout << "\nPassword does not meet requirements\n"
                     << "Password cannot be empty\n"
                     << "Password must be at least 6 characters long\n" << endl;
                continue;
            }

            if (password1 == password2)
            {
                string salt = generateSalt();
                string hashed = simpleHashPassword(salt + password1);
                accounts.emplace(username, account(username, hashed, salt));
                cout << "\nAccount Created" << endl;
                cout << endl;
                saveAccounts();
                newDirectory(username);
                return;
            }
            else
            {
                cout << "Passwords do not match" << endl;
            }
        }
        else
        {
            cout << "Username already exists" << endl;
        }
    }
}


void logIn()
{
    int counter{};

    while (true)
    {
        cout << "\nLOG IN:\nTo cancel type 'q' in Username" << endl;
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
                loggedIn(username);
                return;
            }
            else
            {
                cout << "\nIncorrect password" << endl;
            }
        }
        else 
        {
            cout << "\nIncorrect Username" << endl;
        }

        counter++;
        if (counter >= 3)
        {
            cout << "\nToo many failed attempts\n" << endl;
            return;
        }
    }
}


void welcome()
{
    loadAccounts();

    while (true)
    {
        cout << "Welcome to the beta test!" << endl;
        cout << "1. Create An Account\n"
                "2. Log In\n"
                "3. Exit\n"
                "Input: ";

        int input{};
        cin >> input;

        switch (input)
        {
        case 1:
            createAccount();
            break;
        case 2:
            logIn();
            break;
        case 3:
            cout << "\nExited" << endl;
            return;
        default:
            cout << "Invalid input\n" << endl;
            break;
        }
    }
}


int main()
{
    srand(static_cast<unsigned int>(time(nullptr)));
    welcome();
}