#include "pch.h"
#include "Models/StudentDB.h"
#include <iostream>
#include <algorithm>

StudentDB::StudentDB()
	:m_Students(), m_StudentsPrepared(), m_Changed(false)
{
	InitializeCriticalSection(&cs);
}

StudentDB::StudentDB(
	std::vector<Student> students,
	std::vector<Student> studentsPrepared,
	bool changed
)
	:m_Students(students), m_StudentsPrepared(studentsPrepared), m_Changed(changed)
{
	InitializeCriticalSection(&cs);
}

StudentDB::StudentDB(const StudentDB& ref)
	:m_Students(ref.m_Students), m_StudentsPrepared(ref.m_StudentsPrepared), m_Changed(ref.m_Changed)
{
	InitializeCriticalSection(&cs);
}

StudentDB::~StudentDB()
{
	DeleteCriticalSection(&cs);
}

size_t StudentDB::GetBufferSize() const
{
	size_t bufferSize = sizeof(m_Students.size());

	for (auto it = m_Students.begin(); it != m_Students.end(); it++)
	{
		bufferSize += it->GetBufferSize();
	}

	return bufferSize;
}

const std::vector<Student>& StudentDB::GetStudents() const
{
	return m_Students;
}

void StudentDB::AddStudentPrepare(const Student& student)
{
	EnterCriticalSection(&cs);

	m_StudentsPrepared.push_back(student);
	m_Changed = true;

	LeaveCriticalSection(&cs);
}

void StudentDB::AddPassedSubjectToStudentPrepare(
	std::string studentIndex,
	std::pair<std::string, USHORT> subject
)
{
	EnterCriticalSection(&cs);

	auto studentIt = std::find_if(
		m_StudentsPrepared.begin(),
		m_StudentsPrepared.end(),
		[=](const Student& s) {
			return s.Index == studentIndex;
		}
	);

	if (studentIt != m_StudentsPrepared.end()) {
		studentIt->PassedClasses.insert(subject);
		m_Changed = true;
	}

	LeaveCriticalSection(&cs);
}

std::vector<Student> StudentDB::GetStudentsPrepare()
{
	EnterCriticalSection(&cs);
	auto retVal = m_StudentsPrepared;

	LeaveCriticalSection(&cs);

	return retVal;
}


void StudentDB::SavePermanentChanges()
{
	EnterCriticalSection(&cs);

	m_Students = m_StudentsPrepared;
	m_Changed = false;

	LeaveCriticalSection(&cs);
}

void StudentDB::DiscardNewChanges()
{
	EnterCriticalSection(&cs);
	
	m_StudentsPrepared = m_Students;
	m_Changed = false;

	LeaveCriticalSection(&cs);
}

void StudentDB::RemoveStudent(std::string index)
{
	EnterCriticalSection(&cs);

	auto it = std::find_if(
		m_Students.begin(),
		m_Students.end(),
		[&](const Student& s) {
			return s.Index == index;
		}
	);

	if (it != m_Students.end()) {
		m_Students.erase(it);
	}
	
	LeaveCriticalSection(&cs);
}

void StudentDB::SetCurrentDb(const std::vector<Student>& students)
{
	EnterCriticalSection(&cs);
	
	m_Students.clear();
	m_Students = students;

	LeaveCriticalSection(&cs);
}

std::ostream& operator<<(std::ostream& os, StudentDB studentDb)
{
	os << "Students:" << std::endl;

	auto students = studentDb.GetStudents();

	std::for_each(
		students.begin(),
		students.end(),
		[&](const Student& s) {
			os << s << std::endl;
		}
	);

	return os;
}