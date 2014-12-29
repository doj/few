/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#pragma once
#include <string>
#include <cstdio>

/**
 * a class to manage a temporary file.
 * If the object goes out of scope the file is unlinked.
 */
class TemporaryFile
{
    std::string filename_;
    FILE *f_;

    bool can_create_file(const std::string& dir);

public:
    /**
     * create a temporary file.
     * @throws std::runtime_error if no temporary file could be created.
     */
    TemporaryFile();
    ~TemporaryFile();
    /// @return the file name of the temporary file.
    const std::string& filename();

    /**
     * open the temporary file with a standard stream for reading and writing.
     * @return pointer to FILE object.
     */
    FILE* file();

    /**
     * close the standard stream if it was created before.
     * @return true upon success; false upon error or if no standard stream was created previously.
     */
    bool close();
};
