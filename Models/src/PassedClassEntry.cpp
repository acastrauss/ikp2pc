#include "pch.h"
#include "Models/PassedClassEntry.h"

PassedClassEntry::PassedClassEntry()
{
    StudentIndex = "";
    ClassName = "";
    Grade = 5;
}

PassedClassEntry::PassedClassEntry(
    std::string studentIndex,
    std::string className,
    USHORT grade
){
    StudentIndex = studentIndex;
    ClassName = className;
    Grade = grade;
}

PassedClassEntry::PassedClassEntry(const PassedClassEntry& ref)
{
    StudentIndex = ref.StudentIndex;
    ClassName = ref.ClassName;
    Grade = ref.Grade;
}

PassedClassEntry& PassedClassEntry::operator=(const PassedClassEntry& ref)
{
    StudentIndex = ref.StudentIndex;
    ClassName = ref.ClassName;
    Grade = ref.Grade;
    return *this;
}

size_t PassedClassEntry::GetBufferSize() const
{
    return 
        StudentIndex.size() + sizeof(StudentIndex.size()) +
        ClassName.size() + sizeof(ClassName.size()) +
        sizeof(Grade);
}
