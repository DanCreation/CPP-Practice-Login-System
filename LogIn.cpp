#include <iostream>
#include <string>
#include <format>
#include <unordered_map>
#include <conio.h>
#include <fstream>
#include <filesystem>

using namespace std;
namespace fs = filesystem;

enum class journalAction { open, remove };

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

class account
{
private:
    string username;
    string password;
public:
    account(string a, string b) : username(a), password(b) {}

    string getUsername() const { return username; }
    string getPassword() const { return password; }
   
    void setPassword(const string& newPass)
    {
        password = newPass;
    }

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
        outfile << pair.second.getUsername() << " " << pair.second.getPassword() << endl;
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

    string username{}, password{};

    while (infile >> username >> password)
    {
        accounts.emplace(username, account(username, password));
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
    string filename{}, line{};
    fs::path userPath = (format("{}'s folder", username));

    cin.ignore();

    cout << "Entry Title: ";
    getline(cin, filename);

    sanitiseFilename(filename, true);

    cout << "Journal Entry (Type 'END' on a new line to finish):\n";
    
    ofstream outfile(userPath / (format("{}.txt", filename)));
    while (true)
    {
        getline(cin, line);
        if (line == "END") break;
        outfile << line << endl;
    }

    outfile.close();
    cout << "Entry added\n" << endl;
}

void getJournalEntries(const string& username, journalAction action)
{
    fs::path userPath = format("{}'s folder", username);
    vector<string> entries{};

    for (const auto& entry : fs::directory_iterator(userPath))
    {
        if (entry.path().extension() == ".txt")
        {
            entries.push_back(entry.path().stem().string());
        }
    }

    for (size_t i = 0; i < entries.size(); ++i)
    {
        sanitiseFilename(entries[i], false);

        cout << i + 1 << ". " << entries[i] << endl;
    }

    int choice{};
    cout << ((action == journalAction::open)
        ? "Select an entry to open (0 to cancel): "
        : "Select an entry to delete (0 to cancel): ");
    cin >> choice;

    if (choice <= 0 || choice > entries.size()) return;

    string filename = entries[choice - 1];

    sanitiseFilename(filename, true);

    fs::path filePath = userPath / format("{}.txt", filename);

    if (action == journalAction::open)
    {
        ifstream infile(filePath);
        cout << "\nCurrent content:\n";
        string line{};
        while (getline(infile, line))
        {
            cout << line << endl;
        }
        infile.close();

        cout << "\nWould you like to edit this entry? (y/n): ";
        char editChoice{};
        cin >> editChoice;
        cin.ignore();

        if (editChoice == 'y' || editChoice == 'Y')
        {
            ofstream outfile(filePath);
            cout << "Enter new content (Type 'END' to finish):\n";
            while (true)
            {
                getline(cin, line);
                if (line == "END") break;
                outfile << line << endl;
            }
            outfile.close();
            cout << "Entry updated.\n" << endl;
        }
    }
    else if (action == journalAction::remove)
    {
        cout << "\nAre you sure you want to delete this entry? (y/n): ";
        char confirm{};
        cin >> confirm;
        cin.ignore();

        if (confirm == 'y' || confirm == 'Y')
        {
            fs::remove(filePath);
            cout << "Entry deleted.\n" << endl;
        }
    }
}

void viewJournal(const string& username)
{
    getJournalEntries(username, journalAction::open);
}

void deleteJournal(const string& username)
{
    getJournalEntries(username, journalAction::remove);
}

void loggedIn(string name)
{
    while (true)
    {
        string username{ name };

        cout << format("Welcome {}", name) << endl;
        cout << "1. Add Journal Entry\n2. View Journal Entry\n3. Delete Journal Entry\n"
            "4. Change Password\n5. Log Out\nInput: ";

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
            while (true)
            {
                cout << "To cancel type 'q'" << endl;
                cout << "\nOld Password: ";
                auto it = accounts.find(username);
                string oldPassword = getPassword();
                if (oldPassword == "q" || oldPassword == "Q")
                {
                    break;
                }
                else if (oldPassword == it->second.getPassword())
                {
                    while (true)
                    {
                        cout << "New Password: ";
                        string newPassword1 = getPassword();
                        cout << "Confirm New Password: ";
                        string newPassword2 = getPassword();
                        if (newPassword1 == newPassword2)
                        {
                            it->second.setPassword(newPassword1);
                            cout << "\nYour Password Has Been Changed\n" << endl;
                            saveAccounts();
                            break;
                        }
                        else
                        {
                            cout << "Passwords do not match" << endl;
                        }
                    }
                    break;
                }
                else
                {
                    cout << "Passwords do not match" << endl;
                }
            }
            break;
        case 5:
            cout << "Logged Out\n" << endl;
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
        cout << "\nCREATE ACCOUNT: " << endl;
        cout << "Username: ";
        cin >> username;

        auto it = accounts.find(username);

        if (it == accounts.end())
        {
            cout << "Password: ";
            password1 = getPassword();
            cout << "Confirm Password: ";
            password2 = getPassword();
            if (password1 == password2)
            {
                accounts.emplace(username, account(username, password1));
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
        cout << "\nLOG IN: \nTo exit the log in page type 'q' in Username" << endl;
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
            if (it->second.getPassword() == password)
            {
                cout << endl;
                loggedIn(username);
                return;
            }
            else
            {
                cout << "Incorrect password" << endl;
            }
        }
        else 
        {
            cout << "Incorrect Username" << endl;
        }

        counter++;
        if (counter >= 3)
        {
            cout << "Too many failed attempts\n" << endl;
            break;
        }
    }
}

void welcome()
{
    loadAccounts();

    while (true)
    {
        cout << "Welcome to the beta test!" << endl;
        cout << "1. Create An Account\n2. Log In\n3. Exit\nInput: ";

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
    welcome();
}