#include <iostream>
#include <fstream>
#include <string>

#include "addressbook.pb.h"

using namespace std;

void ListPeople(const tutorial::AddressBook &address_book)
{
    for (int i = 0; i < address_book.people_size(); i++)
    {
        const tutorial::Person &person = address_book.people(i);

        cout << "Person ID: " << person.id() << endl;
        cout << "\t Name: " << person.name() << endl;
        if (!person.email().empty())
            cout << "\t Email: " << person.email() << endl;

        for (int j = 0; j < person.phones_size(); j++)
        {
            const tutorial::Person::PhoneNumber &phone_number = person.phones(j);

            switch (phone_number.type())
            {
            case tutorial::Person::MOBILE:
                cout << "\t\t Mobile phone: ";
                break;
            case tutorial::Person::HOME:
                cout << "\t\t Home phone: ";
                break;
            case tutorial::Person::WORK:
                cout << "\t\t Work phone: ";
                break;
            default:
                break;
            }
            cout << phone_number.number() << endl;
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
    if (!address_book.ParseFromIstream(&input))
    {
        cerr << "Failed to parse address book." << endl;
        return -2;
    }

    ListPeople(address_book);

    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}
