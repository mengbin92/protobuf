import addressbook_pb2
import random
import uuid


def Home():
    return addressbook_pb2.Person.HOME


def Work():
    return addressbook_pb2.Person.WORK


def Mobile():
    return addressbook_pb2.Person.MOBILE


switch = {0: Mobile(), 1: Home(), 2: Work()}

# from protobuf to string


def Write2AB(addressbook):
    for i in range(20):
        person = addressbook.people.add()
        person.id = i
        person.name = 'name: ' + str(i)
        person.email = str(i) + '@eamil.com'

        phone = person.phones.add()
        phone.type = switch[random.randint(0, 999) % 3]
        phone.number = str(uuid.uuid1())

    return addressbook.SerializeToString()


def ReadFromAB(addressbook):
    i = 1
    for people in addressbook.people:
        print('\n======  {} ======'.format(i))
        print('Person ID: ', people.id)
        print('Person Name: ', people.name)
        print('Person Email: ', people.email)

        print('Person PhoneNumber:')
        for phone in people.phones:
            if phone.type == addressbook_pb2.Person.MOBILE:
                print('\t type:Mobile number:{}'.format(phone.number))
            elif phone.type == addressbook_pb2.Person.HOME:
                print('\t type:Home number:{}'.format(phone.number))
            if phone.type == addressbook_pb2.Person.WORK:
                print('\t type:work number:{}'.format(phone.number))
        i += 1


if __name__ == "__main__":
    ab = addressbook_pb2.AddressBook()
    abString = Write2AB(ab)
    print(abString)

    # from string to protobuf
    ab.ParseFromString(abString)
    ReadFromAB(ab)
