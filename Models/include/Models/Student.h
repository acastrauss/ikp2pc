#pragma once

#include <string>
#include <ctime>
#include <map>
#include <Windows.h>

struct Student {
	std::string FirstName;
	std::string LastName;
	std::string Index;

	/// <summary>
	/// Key: subject name, Value: grade
	/// </summary>
	std::map<std::string, USHORT> PassedClasses;

	Student();
	
	Student(
		std::string firstName,
		std::string lastName,
		std::string index,
		const std::map<std::string, USHORT>& passedClasses
	);

	Student(
		const Student& ref
	);

	size_t GetBufferSize() const;

	Student& operator=(
		const Student& rhs
	);

	bool operator==(
		const Student& rhs
	);

	bool operator!=(
		const Student& rhs
	);

	friend std::ostream& operator<<(std::ostream& os, Student student);
};