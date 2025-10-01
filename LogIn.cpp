#include <iostream>
#include <string>
#include <vector>
#include <format>
#include <unordered_map>

using namespace std;

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

unordered_map<string, string> accounts{};
//vector<account> accounts{};

void createAccount()
{
    string username{}, password{};

    cout << "\nCREATE ACCOUNT: " << endl;
    cout << "Username: ";
    cin >> username;
    cout << "Password: ";
    cin >> password;

    accounts[username] = password;
    //accounts.push_back(account(username, password));
    cout << "\nAccount Created" << endl;
    //accounts.back().display();
    cout << endl;
}

void logIn()
{
    bool loggingIn{ true };
    while (loggingIn)
    {
        cout << "\nLOG IN: " << endl;
        string username{}, password{};
        cout << "Username: ";
        cin >> username;

        auto it = accounts.find(username);

        if (it != accounts.end())
        {
            cout << "Password: ";
            cin >> password;
            if (it->second == password)
            {
                cout << "Log in successful\n" << endl;
                break;
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

        //bool nameExists{ false };
        /*for (int i{}; i < accounts.size(); i++)
        {
            if (username == accounts[i].getUsername())
            {
                nameExists = true;
                cout << "Password: ";
                cin >> password;
                if (password == accounts[i].getPassword())
                {
                    cout << "Log in successful\n" << endl;
                    loggingIn = false;
                }
                else
                {
                    cout << "Log in failed\n" << endl;
                }
                break;
            }
        }*/
        /*if (nameExists == false)
        {
            cout << "Incorrect username\n" << endl;
        }*/
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
            cout << "Exited" << endl;
            online = false;
            break;
        default:
            cout << "Nothing" << endl;
            break;
        }
    }
   
        
}

int main()
{
    welcome();
}