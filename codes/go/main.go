package main

import (
	"fmt"
	"strconv"

	uuid "github.com/satori/go.uuid"

	"github.com/golang/protobuf/proto"
	"github.com/zj/tutorial"
)

// Write2AB return []byte from protobuf
func Write2AB(ab *tutorial.AddressBook) ([]byte, error) {
	for i := 0; i < 10; i++ {
		person := &tutorial.Person{}
		person.Id = int32(i)
		person.Name = "name" + strconv.Itoa(i)
		person.Email = strconv.Itoa(i) + "@email.com"

		phone := &tutorial.Person_PhoneNumber{}
		switch i % 3 {
		case 0:
			phone.Type = tutorial.Person_MOBILE
		case 2:
			phone.Type = tutorial.Person_WORK
		default:
			phone.Type = tutorial.Person_HOME
		}
		phone.Number = uuid.Must(uuid.NewV4()).String()

		person.Phones = append(person.Phones, phone)

		ab.People = append(ab.People, person)
	}

	return proto.Marshal(ab)

}

// ReadFromBytes read addressbook from []byte
func ReadFromBytes(abByte []byte) (*tutorial.AddressBook, error) {
	ab := &tutorial.AddressBook{}
	err := proto.Unmarshal(abByte, ab)
	if err != nil {
		return nil, err
	}
	return ab, nil
}

func main() {
	ab := &tutorial.AddressBook{}

	abBytes, err := Write2AB(ab)
	if err != nil {
		fmt.Println(err)
		return
	}

	ab, err = ReadFromBytes(abBytes)
	if err != nil {
		fmt.Println(err)
		return
	}
	fmt.Println(ab)
}
