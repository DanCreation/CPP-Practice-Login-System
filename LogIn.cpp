#include <iostream>
#include <string>
#include <format>
#include <unordered_map>
#include <conio.h>

using namespace std;

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

    void display() const
    {
        cout << format("Username: {}\nPassword: {}", username, password) << endl;
    }
};

unordered_map<string, account> accounts{};

void loggedIn(string name)
{
    while (true)
    {
        string username{ name };

        cout << format("Welcome {}", name) << endl;
        cout << "1. Add Journal Entry\n2. Update Journal Entry\n3. Delete Journal Entry\n"
            "4. Change Password\n5. Log Out\nInput: ";

        int input{};
        cin >> input;

        switch (input)
        {
        case 1:
            cout << "Added Entry\n" << endl;
            break;
        case 2:
            cout << "Updated Entry\n" << endl;
            break;
        case 3:
            cout << "Deleted Entry\n" << endl;
            break;
        case 4:
            while (true)
            {
                cout << "\nOld Password: ";
                auto it = accounts.find(username);
                string oldPassword = getPassword();
                if (oldPassword == it->second.getPassword())
                {
                    while (true)
                    {
                        cout << "New Password: ";
                        string newPassword1 = getPassword();
                        cout << "Confirm New Password: ";
                        string newPassword2 = getPassword();
                        if (newPassword1 == newPassword2)
                        {
                            it->second = account(username, newPassword1);
                            cout << "\nYour Password Has Been Changed\n" << endl;
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