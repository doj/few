/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#pragma once
#include <string>

/**
 * a class to manage a temporary file.
 * If the object goes out of scope the file is unlinked.
 */
class TemporaryFile
{
    std::wstring filename_;

public:
    /**
     * create a temporary file.
     * @throws std::runtime_error if no temporary file could be created.
     */
    TemporaryFile();
    ~TemporaryFile();
    /// @return the file name of the temporary file.
    const std::wstring& filename();
};
