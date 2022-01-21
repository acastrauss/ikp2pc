#pragma once

#include "Models/StudentDB.h"
#include "Models/PassedClassEntry.h"

/// <summary>
/// str length
/// str content
/// </summary>
/// <param name="str"></param>
/// <returns></returns>
char* SerializeString(const std::string& str);

/// <summary>
/// FirstName length + content
/// LastName length + content 
/// Index length + content
/// PassedClasses length
/// PassedClass name length + content
/// PassedClass grade
/// </summary>
/// <param name="ref"></param>
/// <returns></returns>
char* SerializeStudent(const Student& ref);

/// <summary>
/// Students length
/// Student serialized
/// </summary>
/// <param name="ref"></param>
/// <returns></returns>
char* SerializeStudentDB(const StudentDB& ref);

/// <summary>
/// StudentIndex length + content
/// ClassName length + content
/// Grade 
/// </summary>
/// <param name="ref"></param>
/// <returns></returns>
char* SerializePassedClass(const PassedClassEntry& ref);

std::string DeserializeString(char* buffer);
Student DeserializeStudent(char* buffer);
StudentDB DeserializeStudentDB(char* buffer);
PassedClassEntry DeserializePassedClass(char* buffer);

