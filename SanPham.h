#ifndef SANPHAM_H
#define SANPHAM_H

#include <string>
#include <iostream>
#include <string>
#include <vector>
// Thêm thư viện ODBC
#include <windows.h>
#include <sql.h>
#include <sqlext.h>

#include "Date.h"

// Struct này không đổi
struct SKUPhanTich {
    std::string maSP;
    std::string tenSP;
    float tongDoanhThu = 0.0f;
    float tongSoLuong = 0.0f;
    float phanTramDoanhThu = 0.0f;
    float phanTramCongDon = 0.0f;
    char nhomABC = ' ';
};

class SanPham {
private: 
    std::string maSP;
    std::string tenSP;
    std::string loai;
    float giaVon;
    float thetich1tan;
    float NdoThichHop;
    std::string NhomPhanLoai; // Giữ là std::string (chuỗi)
    float ruiRoLanh;   // % hỏng nếu lạnh hơn chuẩn (0.01 = 1%)
    float ruiRoNong;   // % hỏng nếu nóng hơn chuẩn
    float giaBan;      // Giá bán dự kiến (cho bài định giá động)
    int hanSuDung;     // Hạn sử dụng tiêu chuẩn (ngày)

public:
    SanPham();
    SanPham(std::string maSP, std::string tenSP, std::string loai, float giaVon, 
            float thetich1tan, float NdoThichHop, std::string NhomPhanLoai,
            float ruiRoLanh = 0.01f, float ruiRoNong = 0.02f, 
            float giaBan = 0.0f, int hanSuDung = 30);
    
    // Setters/Getters không đổi
    void setMaSP(std::string maSP);
    void setTenSP(std::string tenSP);
    void setLoai(std::string loai);
    void setGiaVon(float giaVon);
    void setThetich1tan(float thetich1tan);
    void setNdoThichHop(float NdoThichHop);
    void setNhomPhanLoai(std::string NhomPhanLoai);
    void setRuiRoLanh(float r);
    void setRuiRoNong(float r);
    void setGiaBan(float g);
    void setHanSuDung(int h);
    std::string getMaSP() const;
    std::string getTenSP() const;
    std::string getLoai() const;
    float getGiaVon() const;
    float getThetich1tan() const;
    float getNdoThichHop() const;
    std::string getNhomPhanLoai() const;
    float getRuiRoLanh() const;
    float getRuiRoNong() const;
    float getGiaBan() const;
    int getHanSuDung() const;
    
    // --- ĐÃ SỬA ĐỔI ---
    // Các hàm này giờ cần biết kết nối DB nào để làm việc
    void NhapInfo(SQLHDBC hDbc);
    void printInfo() const;
    void ThucHienPhanTichNhom(SQLHDBC hDbc, std::vector<SanPham>& dsSP);
};

bool sosanhDoanhThu(const SKUPhanTich& a, const SKUPhanTich& b);
#endif