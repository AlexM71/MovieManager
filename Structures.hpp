#ifndef STRUCTURES_HPP
#define STRUCTURES_HPP

#include <QString>
#include <Enums.hpp>

struct stLogElement {
    QString sText;
    enum eLog eCriticity;
};

struct stFilters {
    QString sName;
    int nMinYear;
    int nMaxYear;
    int nMinRating;
    int nMaxRating;
};

struct stColumn {
    QString sName;
    enum eColumnType eType;
    double nMin;
    double nMax;
    int nPrecision;
    int textMaxLength;
    bool bOptional;
};

#endif // STRUCTURES_HPP
