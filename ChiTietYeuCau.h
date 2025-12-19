#ifndef CHITIET_H
#define CHITIET_H

#include <string>
#include <vector>
// #include "Kho.h" // <-- Xóa dòng này vì không dùng đến
#include "Date.h"

// Cần khai báo trước (forward declaration) thay vì include đầy đủ
class SanPham; 

struct SanPhamYeuCau {
    SanPham* sp;
    float soLuongTan;
    float ThanhTienRieng;
};

class ChiTietYeuCau {
private:
    std::string MaYeuCau;
    std::vector<SanPhamYeuCau> dsYeuCau;
    float soLuongTan;
    float giaTri;
    Date NgayNhap;
    std::string HoanThanh;
public:
    ChiTietYeuCau();
    ChiTietYeuCau(std::string MaYeuCau,const std::vector<SanPhamYeuCau>& dsYeuCau);
    void setMaYeuCau(std::string MaYeuCau);
    void setdsYeuCau(std::vector<SanPhamYeuCau> ds);
    void setSoLuongTan(float soLuongTan);
    void setGiaTri(float giaTri);
    void setNgayNhap(Date NgayNhap);
    void setHoanThanh(std::string status);
    std::string getHoanThanh() const;
    std::string getMaYeuCau() const;
    std::vector<SanPhamYeuCau> getdsYeucau() const;
    float getSoLuongTan()const;
    float getGiaTri() const;
    Date getNgayNhap() const;
};
#endif