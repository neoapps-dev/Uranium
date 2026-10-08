#pragma once
#include <stdexcept>
#include <string>
namespace uranium {
class Error : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class ValueError : public Error {
public:
    using Error::Error;
};

class DomainError : public Error {
public:
    using Error::Error;
};

class ParseError : public Error {
public:
    using Error::Error;
};

class IoError : public Error {
public:
    using Error::Error;
};
}
