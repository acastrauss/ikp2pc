#include "pch.h"
#include "..\include\Serializator\Serialization.h"

#pragma warning(disable:6386)

char* SerializeString(const std::string& str)
{
	size_t bufferSize = str.size() + sizeof(str.size());
	char* buffer = new char[bufferSize];
	size_t offset = 0;
	size_t strLength = str.size();

	memcpy(
		buffer + offset,
		&strLength,
		sizeof(strLength)
	);

	offset += sizeof(strLength);

	memcpy(
		buffer + offset,
		str.c_str(),
		str.size()
	);

	return buffer;
}

char* SerializeStudent(const Student& ref)
{
	size_t bufferSize = ref.GetBufferSize();
	char* buffer = new char[bufferSize];
	size_t offset = 0;

	char* firstNameBuffer = SerializeString(ref.FirstName);
	size_t firstNameBufferSize = ref.FirstName.size() + sizeof(ref.FirstName.size());

	memcpy(
		buffer + offset,
		firstNameBuffer,
		firstNameBufferSize	
	);

	offset += firstNameBufferSize;

	char* lastNameBuffer = SerializeString(ref.LastName);
	size_t lastNameBufferSize = ref.LastName.size() + sizeof(ref.LastName.size());

	memcpy(
		buffer + offset,
		lastNameBuffer,
		lastNameBufferSize
	);

	offset += lastNameBufferSize;
	
	char* indexBuffer = SerializeString(ref.Index);
	size_t indexBufferSize = ref.Index.size() + sizeof(ref.Index.size());

	memcpy(
		buffer + offset,
		indexBuffer,
		indexBufferSize
	);

	offset += indexBufferSize;

	size_t passedClassesSize = ref.PassedClasses.size();

	memcpy(
		buffer + offset,
		&passedClassesSize,
		sizeof(passedClassesSize)
	);

	offset += sizeof(passedClassesSize);

	for (auto it = ref.PassedClasses.begin(); it != ref.PassedClasses.end(); it++)
	{
		char* classNameBuffer = SerializeString(it->first);
		size_t classNameBufferSize = it->first.size() + sizeof(it->first.size());

		memcpy(
			buffer + offset,
			classNameBuffer,
			classNameBufferSize
		);

		offset += classNameBufferSize;

		memcpy(
			buffer + offset,
			&(it->second),
			sizeof(it->second)
		);

		offset += sizeof(it->second);
	}

	return buffer;
}

char* SerializeStudentDB(const StudentDB& ref)
{
	size_t bufferSize = ref.GetBufferSize();
	char* buffer = new char[bufferSize];
	size_t offset = 0;

	std::vector<Student> students = ref.GetStudents();

	size_t studentsSize = students.size();

	memcpy(
		buffer + offset,
		&studentsSize,
		sizeof(studentsSize)
	);

	offset += sizeof(studentsSize);

	for (auto it = students.begin(); it != students.end(); it++)
	{
		char* studentBuffer = SerializeStudent(*it);

		memcpy(
			buffer + offset,
			studentBuffer,
			it->GetBufferSize()
		);

		delete[] studentBuffer;

		offset += it->GetBufferSize();
	}

	return buffer;
}

char* SerializePassedClass(const PassedClassEntry& ref)
{
	size_t bufferSize = ref.GetBufferSize();
	char* buffer = new char[bufferSize];
	size_t offset = 0;

	char* studentIndexBuffer = SerializeString(ref.StudentIndex);

	memcpy(
		buffer + offset,
		studentIndexBuffer,
		sizeof(ref.StudentIndex.size()) + ref.StudentIndex.size()
	);

	delete[] studentIndexBuffer;

	offset += sizeof(ref.StudentIndex.size()) + ref.StudentIndex.size();

	char* classNameBuffer = SerializeString(ref.ClassName);

	memcpy(
		buffer + offset,
		classNameBuffer,
		sizeof(ref.ClassName.size()) + ref.ClassName.size()
	);

	delete[] classNameBuffer;

	offset += sizeof(ref.ClassName.size()) + ref.ClassName.size();

	memcpy(
		buffer + offset,
		&ref.Grade,
		sizeof(ref.Grade)
	);

	return buffer;
}

std::string DeserializeString(char* buffer)
{
	size_t offset = 0;
	size_t strSize = *((size_t*)(buffer + offset));

	offset += sizeof(strSize);

	char* strContent = new char[strSize + 1];

	
	memcpy(
		strContent,
		buffer + offset,
		strSize
	);

	strContent[strSize] = '\0';

	std::string retVal(strContent);

	delete[] strContent;

	return retVal;
}

Student DeserializeStudent(char* buffer)
{
	size_t offset = 0;

	Student student;

	student.FirstName = DeserializeString(buffer + offset);
	offset += student.FirstName.size() + sizeof(student.FirstName.size());

	student.LastName = DeserializeString(buffer + offset);
	offset += student.LastName.size() + sizeof(student.LastName.size());
	
	student.Index = DeserializeString(buffer + offset);
	offset += student.Index.size() + sizeof(student.Index.size());
	
	size_t passedClassesSize = *((size_t*)(buffer + offset));
	offset += sizeof(passedClassesSize);


	for (size_t i = 0; i < passedClassesSize; i++)
	{
		std::string className = DeserializeString(buffer + offset);

		offset += className.size() + sizeof(className.size());

		USHORT classGrade = *((USHORT*)(buffer + offset));
		offset += sizeof(classGrade);
		
		student.PassedClasses.insert({
			className, classGrade
		});
	}

	return student;
}

StudentDB DeserializeStudentDB(char* buffer)
{
	size_t offset = 0;
	StudentDB studentDb;

	size_t studentsSize = *((size_t*)(buffer + offset));
	offset += sizeof(studentsSize);

	for (size_t i = 0; i < studentsSize; i++)
	{
		Student student = DeserializeStudent(buffer + offset);
	
		studentDb.AddStudentPrepare(student);

		offset += student.GetBufferSize();
	}

	studentDb.SavePermanentChanges();

	return studentDb;
}

PassedClassEntry DeserializePassedClass(char* buffer)
{
	size_t offset = 0;

	PassedClassEntry pce;

	pce.StudentIndex = DeserializeString(buffer + offset);

	offset += pce.StudentIndex.size() + sizeof(pce.StudentIndex.size());

	pce.ClassName = DeserializeString(buffer + offset);

	offset += pce.ClassName.size() + sizeof(pce.ClassName.size());

	pce.Grade = *((USHORT*)(buffer + offset));

	return pce;
}
