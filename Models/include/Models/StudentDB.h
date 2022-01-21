#pragma once

#include "Student.h"
#include <vector>

class StudentDB
{
private:
	/// <summary>
	/// Final students db
	/// </summary>
	std::vector<Student> m_Students;

	/// <summary>
	/// To add prepared changes
	/// </summary>
	std::vector<Student> m_StudentsPrepared;

	bool m_Changed;

	CRITICAL_SECTION cs;

public:

	StudentDB();

	StudentDB(
		std::vector<Student> students,
		std::vector<Student> studentsPrepared,
		bool changed
	);

	StudentDB(
		const StudentDB& ref
	);

	size_t GetBufferSize() const;

	const std::vector<Student>& GetStudents() const;

	StudentDB& operator=(
		const StudentDB& ref
	) = delete;

	~StudentDB();

	/// <summary>
	/// Add student to prepared db
	/// </summary>
	/// <param name="student"></param>
	void AddStudentPrepare(
		const Student& student
	);

	void AddPassedSubjectToStudentPrepare(
		std::string studentIndex,
		std::pair<std::string, USHORT> subject
	);

	std::vector<Student> GetStudentsPrepare();

	void SavePermanentChanges();
	void DiscardNewChanges();

	void RemoveStudent(std::string index);

	void SetCurrentDb(const std::vector<Student>& students);

	friend std::ostream& operator<<(std::ostream& os, StudentDB studentDb);
};

