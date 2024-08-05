#include <cassert>
#include <cstring>
#include "graphics.h"
#include "image.h"
#include <string>


/* misc functions */

const std::string private_get_extension(const std::string& filename) 
{
    size_t dot_pos = filename.find_last_of('.');
    return dot_pos == std::string::npos ? "" : filename.substr(dot_pos + 1);
}