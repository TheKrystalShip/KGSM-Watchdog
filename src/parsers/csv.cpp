#ifndef __TKS_PARSERS_CSV_CPP__
#define __TKS_PARSERS_CSV_CPP__

#include "csv.h"

#include <iterator>
#include <fstream>
#include <sstream>
#include <vector>

/*
TKS::Parsers::CSV::Row
*/

std::string_view TKS::Parsers::CSV::Row::operator[](std::size_t index) const
{
    return std::string_view(&m_line[m_data[index] + 1], m_data[index + 1] - (m_data[index] + 1));
}

constexpr std::size_t TKS::Parsers::CSV::Row::getSize() const
{
    return m_data.size() - 1;
}

void TKS::Parsers::CSV::Row::readNextRow(std::istream &str)
{
    std::getline(str, m_line);

    m_data.clear();
    m_data.emplace_back(-1);
    std::string::size_type pos = 0;

    while ((pos = m_line.find(';', pos)) != std::string::npos)
    {
        m_data.emplace_back(pos);
        ++pos;
    }

    // This checks for a trailing comma with no data after it.
    pos = m_line.size();
    m_data.emplace_back(pos);
}

constexpr std::string_view TKS::Parsers::CSV::Row::getLine() const
{
    return std::string_view(m_line);
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

TKS::Parsers::CSV::Iterator::Iterator(std::istream &str) : m_str(str.good() ? &str : nullptr)
{
    ++(*this);
}

TKS::Parsers::CSV::Iterator::Iterator() : m_str(nullptr)
{
}

// Pre Increment
TKS::Parsers::CSV::Iterator &TKS::Parsers::CSV::Iterator::operator++()
{
    if (m_str)
    {
        if (!((*m_str) >> m_row))
        {
            m_str = nullptr;
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
    return m_row;
}

TKS::Parsers::CSV::Row const *TKS::Parsers::CSV::Iterator::operator->() const
{
    return &m_row;
}

bool TKS::Parsers::CSV::Iterator::operator==(Iterator const &rhs)
{
    return ((this == &rhs) || ((this->m_str == nullptr) && (rhs.m_str == nullptr)));
}

bool TKS::Parsers::CSV::Iterator::operator!=(Iterator const &rhs)
{
    return !((*this) == rhs);
}

/*
TKS::Parsers::CSV::Range
*/

TKS::Parsers::CSV::Range::Range(std::istream &str) : stream(str)
{
}

TKS::Parsers::CSV::Iterator TKS::Parsers::CSV::Range::begin() const
{
    return Iterator{stream};
}

TKS::Parsers::CSV::Iterator TKS::Parsers::CSV::Range::end() const
{
    return Iterator{};
}

#endif // __TKS_PARSERS_CSV_CPP__
