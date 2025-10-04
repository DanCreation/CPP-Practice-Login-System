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

        string getUsername() const {return username;}
        string getPassword() const {return password;}

        void display() const
        {
            cout << format("Username: {}\nPassword: {}", username, password) << endl;
        }
};

unordered_map<string, account> accounts{};

void createAccount()
{
    string username{}, password1{}, password2{};
    while (true)
    {
        cout << "\nCREATE ACCOUNT: " << endl;
        cout << "Username: ";
        cin >> username;
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
                cout << "\nLog in successful\n" << endl;
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
    bool online{true};

    while (online)
    {
        cout << "Welcome to the beta test!" << endl;
        cout << "1. Create an account\n2. Log In\n3. Exit\nInput: ";

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
            online = false;
            break;
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