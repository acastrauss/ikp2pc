#pragma once

#include <string>
#include <Windows.h>

struct PassedClassEntry {
    std::string StudentIndex;
    std::string ClassName;
    USHORT Grade;

    PassedClassEntry();

    PassedClassEntry(
        std::string studentIndex,
        std::string className,
        USHORT grade
    );

    PassedClassEntry(
        const PassedClassEntry& ref
    );

    PassedClassEntry& operator=(
        const PassedClassEntry& ref
    );

    size_t GetBufferSize() const;
};
