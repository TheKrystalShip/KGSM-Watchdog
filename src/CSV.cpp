#include "CSV.h"

#include <iterator>
#include <fstream>
#include <sstream>
#include <vector>

/*
TKS::Parsers::CSV::Row
*/

std::string_view TKS::Parsers::CSV::Row::operator[](std::size_t index) const
{
    return std::string_view(&_line[_data[index] + 1], _data[index + 1] - (_data[index] + 1));
}

std::size_t TKS::Parsers::CSV::Row::getSize() const
{
    return _data.size() - 1;
}

void TKS::Parsers::CSV::Row::readNextRow(std::istream &str)
{
    std::getline(str, _line);

    _data.clear();
    _data.emplace_back(-1);
    std::string::size_type pos = 0;

    while ((pos = _line.find(';', pos)) != std::string::npos)
    {
        _data.emplace_back(pos);
        ++pos;
    }

    // This checks for a trailing comma with no data after it.
    pos = _line.size();
    _data.emplace_back(pos);
}

std::string_view TKS::Parsers::CSV::Row::getLine() const
{
    return std::string_view(_line);
}

std::ostream &TKS::Parsers::CSV::operator<<(std::ostream &str, Row const &data)
{
    str << data.getLine();
    return str;
}

std::istream &TKS::Parsers::CSV::operator>>(std::istream &str, Row &data)
{
    data.readNextRow(str);
    return str;
}

/*
TKS::Parsers::CSV::Iterator
*/

TKS::Parsers::CSV::Iterator::Iterator(std::istream &str) : _istream(str.good() ? &str : nullptr)
{
    ++(*this);
}

TKS::Parsers::CSV::Iterator::Iterator() : _istream(nullptr)
{
}

// Pre Increment
TKS::Parsers::CSV::Iterator &TKS::Parsers::CSV::Iterator::operator++()
{
    if (_istream)
    {
        if (!((*_istream) >> _row))
        {
            _istream = nullptr;
        }
    }

    return *this;
}

// Post Increment
TKS::Parsers::CSV::Iterator TKS::Parsers::CSV::Iterator::operator++(int)
{
    Iterator tmp(*this);
    ++(*this);
    return tmp;
}

TKS::Parsers::CSV::Row const &TKS::Parsers::CSV::Iterator::operator*() const
{
    return _row;
}

TKS::Parsers::CSV::Row const *TKS::Parsers::CSV::Iterator::operator->() const
{
    return &_row;
}

bool TKS::Parsers::CSV::Iterator::operator==(Iterator const &rhs)
{
    return ((this == &rhs) || ((this->_istream == nullptr) && (rhs._istream == nullptr)));
}

bool TKS::Parsers::CSV::Iterator::operator!=(Iterator const &rhs)
{
    return !((*this) == rhs);
}

/*
TKS::Parsers::CSV::Range
*/

TKS::Parsers::CSV::Range::Range(std::istream &str) : _istream(str)
{
}

TKS::Parsers::CSV::Iterator TKS::Parsers::CSV::Range::begin() const
{
    return Iterator{_istream};
}

TKS::Parsers::CSV::Iterator TKS::Parsers::CSV::Range::end() const
{
    return Iterator{};
}
