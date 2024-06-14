#include "Macro.h"

int Macro::getId() const
{
    return _id;
}

const char* Macro::getName() const
{
    return _name;
}

char* Macro::getSequence() const
{
    return _sequence;
}

void Macro::setId(int id)
{
    _id = id;
}

void Macro::setName(char *name)
{
    _name = name;
}

void Macro::setSequence(char *sequence)
{
    _sequence = sequence;
}

