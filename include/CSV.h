#pragma once

#include <iterator>
#include <fstream>
#include <sstream>
#include <vector>

namespace TKS::Parsers::CSV
{
    class Row
    {
    public:
        std::string_view operator[](std::size_t index) const;
        std::size_t getSize() const;
        std::string_view getLine() const;
        void readNextRow(std::istream &str);

    private:
        std::string _line;
        std::vector<int> _data;
    };

    std::ostream &operator<<(std::ostream &str, Row const &data);
    std::istream &operator>>(std::istream &str, Row &data);

    class Iterator
    {
    public:
        typedef std::input_iterator_tag iterator_category;
        typedef Row value_type;
        typedef std::size_t difference_type;
        typedef Row *pointer;
        typedef Row &reference;

        Iterator(std::istream &str);
        Iterator();

        // Pre Increment
        Iterator &operator++();
        // Post increment
        Iterator operator++(int);
        Row const &operator*() const;
        Row const *operator->() const;

        bool operator==(Iterator const &rhs);
        bool operator!=(Iterator const &rhs);

    private:
        std::istream *_istream;
        Row _row;
    };

    class Range
    {
    public:
        Range(std::istream &str);
        Iterator begin() const;
        Iterator end() const;

    private:
        std::istream &_istream;
    };
}
