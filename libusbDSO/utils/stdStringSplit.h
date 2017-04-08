#pragma once

#include <iterator>
#include <vector>
#include <string>
#include <algorithm>

template <typename Container, typename InputIter, typename ForwardIter>
Container
split(InputIter first, InputIter last,
      ForwardIter s_first, ForwardIter s_last)
{
    Container output;

    while (true) {
        auto pos = std::find_first_of(first, last, s_first, s_last);
        output.emplace_back(first, pos);
        if (pos == last) {
            break;
        }

        first = ++pos;
    }

    return output;
}

template <typename Output = std::vector<std::string>,
          typename Input = std::string,
          typename Delims = std::string>
Output
split(const Input& input, const Delims& delims = " ")
{
    return split<Output>(std::begin(input), std::end(input),
                         std::begin(delims), std::end(delims));
}

std::string section(const std::string& data, int start);

/// \brief Returns the hex dump for the given data.
/// \param dump The string with the hex dump of the data.
/// \param data Pointer to the address where the data bytes should be saved.
/// \param length The maximum length of the data array in bytes.
/// \return The length of the saved data.
unsigned int hexParse(std::string dumpString, unsigned char *data, unsigned length);

std::string hexDump(const unsigned char *data, unsigned len);
