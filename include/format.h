#ifndef FORMAT_H
#define FORMAT_H

#include <string>

namespace Format {
std::string AddPadding(std::string str, char ch, size_t len);
std::string ElapsedTime(long int times);
};                                    // namespace Format

#endif
