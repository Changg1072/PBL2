#ifndef TUYENVANCHUYEN_H
#define TUYENVANCHUYEN_H

#include <iostream>
#include <string>
#include <vector>
// Nếu bạn cần sử dụng các kiểu dữ liệu SQL trong class (thường không bắt buộc ở .h)
// #include <windows.h>
// #include <sql.h>

using namespace std;
class TuyenVanChuyen {
private:
    std::string maTuyen;
    std::string khoDi;          // Mã kho đi
    std::string khoTrungChuyen; // Mã kho trung chuyển (có thể rỗng/NULL)
    std::string sieuThiNhan;    // Mã siêu thị nhận
    std::string maXe;           // Mã phương tiện vận chuyển
    float chiPhiCoDinh;

public:
    // Constructors
    TuyenVanChuyen();
    TuyenVanChuyen(std::string ma, std::string kDi, std::string kTC, std::string stNhan, std::string xe,float chiPhi);

    // Getters
    std::string getMaTuyen() const { return maTuyen; }
    std::string getKhoDi() const { return khoDi; }
    std::string getKhoTrungChuyen() const { return khoTrungChuyen; }
    std::string getSieuThiNhan() const { return sieuThiNhan; }
    std::string getMaXe() const { return maXe; }
    float getChiPhiCoDinh() const { return chiPhiCoDinh; }

    // Setters
    void setMaTuyen(std::string ma) { maTuyen = ma; }
    void setKhoDi(std::string k) { khoDi = k; }
    void setKhoTrungChuyen(std::string k) { khoTrungChuyen = k; }
    void setSieuThiNhan(std::string st) { sieuThiNhan = st; }
    void setMaXe(std::string xe) { maXe = xe; }
    void setChiPhiCoDinh(float cp) { chiPhiCoDinh = cp; }

    // Hàm nhập xuất
    void NhapInfo(); 
    void HienThi() const; 
};

#endif