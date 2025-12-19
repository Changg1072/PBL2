#ifndef DATE_H
#define DATE_H

class Date {
private:
    int day;
    int month;
    int year;
    bool isLeapYear(int year);
    int monthDays(int month, int year);

public:
    Date(int day = 1, int month = 1, int year = 2000);
    void setDay(int day);
    void setMonth(int month);
    void setYear(int year);
    int getDay() const;
    int getMonth() const;
    int getYear() const;
    void nextDay();
    void printDate()const ;
    static Date Today();
    long toDays() const;
    // OPERATOR 
    bool operator<(const Date& other) const;
    bool operator==(const Date& other) const;
    bool operator!=(const Date& other) const;
    // Hàm tính khoảng cách giữa 2 mốc thời gian
    static long daysBetween(const Date& date1, const Date& date2);
};

#endif
