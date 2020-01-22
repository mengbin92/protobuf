#include <iostream>
#include <fstream>
#include <string>

#include "addressbook.pb.h"

using namespace std;

//从用户输入解析通讯录
void PromptFromAddress(tutorial::Person *person)
{
    cout << "Enter person ID number: ";
    int id;
    cin >> id;
    person->set_id(id);
    cin.ignore(256, '\n');

    cout << "Enter email address(blank for none): ";
    string email;
    getline(cin, email);
    if (!email.empty())
        person->set_email(email);

    while (true)
    {
        cout << "Enter a phone number(or leave blank to finish): ";
        string number;
        getline(cin, number);
        if (number.empty())
            break;

        tutorial::Person::PhoneNumber *phone_number = person->add_phones();
        phone_number->set_number(number);

        cout << "Is this a mobile, home, or work phone? ";
        string type;
        getline(cin, type);
        if (type == "mobile")
            phone_number->set_type(tutorial::Person::MOBILE);
        else if (type == "home")
            phone_number->set_type(tutorial::Person::HOME);
        else if (type == "work")
            phone_number->set_type(tutorial::Person::WORK);
        else
        {
            cout << "Unknow phone type, Use default: home. " << endl;
            phone_number->set_type(tutorial::Person::HOME);
        }
    }
}

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        cerr << "Usage: " << argv[0] << " ADDRESS_BOOK_FILE" << endl;
        return -1;
    }

    tutorial::AddressBook address_book;

    fstream input(argv[1], ios::in | ios::binary);
    if (!input)
        cout << argv[1] << ": File not found. Create a new file." << endl;
    else if (!address_book.ParseFromIstream(&input))
    {
        cerr << "Failed to parse address book." << endl;
        return -2;
    }
    else
    {
        PromptFromAddress(address_book.add_people());
        fstream output(argv[1], ios::out | ios::binary);
        if (!address_book.SerializeToOstream(&output))
        {
            cerr << "Failed to write address book." << endl;
            return -3;
        }
    }

    //可选操作，用于清除libprotobuf申请的所有全局对象
    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}
