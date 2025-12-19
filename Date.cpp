#include <iostream>
#include "Date.h"
#include <ctime>
using namespace std;   
Date::Date(int day, int month, int year) {
    setDay(day);
    setMonth(month);
    setYear(year);
}
void Date::setDay(int day) {
    if (day >= 1 && day <= monthDays(month, year)) {
        this->day = day;
    } else {
        this->day = 1; 
    }
}
void Date::setMonth(int month) {
    if (month >= 1 && month <= 12) {
        this->month = month;
    } else {
        this->month = 1; 
    }
}
void Date::setYear(int year) {
    if (year >= 0) {
        this->year = year;
    } else {
        this->year = 2000;
    }
}
int Date::getDay() const{
    return day;
}
int Date::getMonth() const {
    return month;
}
int Date::getYear() const {
    return year;
}
bool Date::isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}
int Date::monthDays(int month, int year) {
    switch (month) {
        case 1: case 3: case 5: case 7: case 8: case 10: case 12:
            return 31;
        case 4: case 6: case 9: case 11:
            return 30;
        case 2:
            return isLeapYear(year) ? 29 : 28;
        default:
            return 30; // Default case, should not reach here
    }
}
void Date::nextDay() {
    day++;
    if (day > monthDays(month, year)) {
        day = 1;
        month++;
        if (month > 12) {
            month = 1;
            year++;
        }
    }
}
void Date::printDate() const {
    cout << (day < 10 ? "0" : "") << day << "/"
         << (month < 10 ? "0" : "") << month << "/"
         << year;
}   
Date Date::Today() {
    time_t now = time(0);           
    tm* ltm = localtime(&now);     
    int day = ltm->tm_mday;
    int month = ltm->tm_mon + 1;   
    int year = ltm->tm_year + 1900; 

    return Date(day, month, year);
}
long Date::toDays() const {
    int m = month, y = year;
    if (m <= 2) {
        m += 12;
        y -= 1;
    }
    return 365*y + y/4 - y/100 + y/400 + (153*m + 8)/5 + day;
}
bool Date::operator<(const Date& other) const {
    return this->toDays() < other.toDays();
}

bool Date::operator==(const Date& other) const {
    return this->toDays() == other.toDays();
}

bool Date::operator!=(const Date& other) const {
    return this->toDays() != other.toDays();
}

long Date::daysBetween(const Date& date1, const Date& date2) {
    long days1 = date1.toDays();
    long days2 = date2.toDays();
    return abs(days2 - days1);
}