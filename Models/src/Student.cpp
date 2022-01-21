#include "pch.h"
#include "Models/Student.h"
#include <iostream>
#include <algorithm>

Student::Student():
	FirstName(),
	LastName(),
	Index(),
	PassedClasses()
{}

Student::Student(
	std::string firstName,
	std::string lastName,
	std::string index,
	const std::map<std::string, USHORT>& passedClasses
): 
	FirstName(firstName),
	LastName(lastName),
	Index(index),
	PassedClasses(passedClasses)
{}

Student::Student(const Student& ref)
	:
	FirstName(ref.FirstName),
	LastName(ref.LastName),
	Index(ref.Index),
	PassedClasses(ref.PassedClasses)
{}

size_t Student::GetBufferSize() const
{
	size_t bufferSize = 
		sizeof(FirstName.size()) + FirstName.size() +
		sizeof(LastName.size()) + LastName.size() +
		sizeof(Index.size()) + Index.size();

	bufferSize += sizeof(PassedClasses.size());

	for (auto it = PassedClasses.begin(); it != PassedClasses.end(); it++)
	{
		bufferSize += sizeof(it->first.size()) + it->first.size() + sizeof(it->second);
	}

	return bufferSize;
}

Student& Student::operator=(const Student& rhs)
{
	FirstName = rhs.FirstName;
	LastName = rhs.LastName;
	Index = rhs.Index;
	PassedClasses = rhs.PassedClasses;
	return *this;
}

bool Student::operator==(const Student& rhs)
{
	bool retVal = FirstName == rhs.FirstName &&
		LastName == rhs.LastName &&
		Index == rhs.Index;

	// if they have same subjects
	std::for_each(
		PassedClasses.begin(),
		PassedClasses.end(),
		[&](const std::pair<std::string, USHORT>& pc) {
			retVal &= rhs.PassedClasses.find(pc.first) != rhs.PassedClasses.end();
		}
	);


	return retVal;
		
}

bool Student::operator!=(const Student& rhs)
{
	return !this->operator==(rhs);
}

std::ostream& operator<<(std::ostream& os, Student student)
{
	os << "Index: " << student.Index << std::endl
		<< "First name: " << student.FirstName << std::endl
		<< "Last name: " << student.LastName << std::endl;

	os << std::endl;
	os << "Passed classes: " << std::endl;

	std::for_each(
		student.PassedClasses.begin(),
		student.PassedClasses.end(),
		[&](const std::pair<std::string, USHORT> pc) {
			os << pc.first << ", " << pc.second << std::endl;
		}
	);

	return os;
}