#ifndef LOHANG_H
#define LOHANG_H

#include <string>
#include <vector>

// --- THÊM THƯ VIỆN ODBC ---
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
// -------------------------

#include "Kho.h"
#include "SanPham.h"
#include "Date.h"
#include "NhaCungCap.h"


struct SanPhamTrongLo {
    SanPham sanPham;
    float soLuong;
    Date hsd;
    float giaTriSP;
    float thetich;
};

class LoHang {
private:
    std::string maLoHang;
    std::vector<SanPhamTrongLo> danhSachSanPham;
    Kho kho;
    Date NgayNhap;
    float soLuongTan;
    float tongTheTich;
    // Date hsd; // Biến này dường như không được dùng, HSD là của từng sản phẩm
    float giaTri;
    NhaCungCap ncc;
public:
    LoHang();
    // (Các hàm set/get không đổi)
    void setMaLoHang(std::string maLoHang);
    void setKho(Kho kho);
    void setNgayNhap(Date ngayNhap);
    void setSoLuongTan(float soLuongTan);
    void setTongTheTich(float tongTheTich);
    void setDanhSachSanPham(std::vector<SanPhamTrongLo> ds);
    // void setHsd(Date hsd); // Bỏ
    void setGiaTri(float giaTri);
    std::string getMaLoHang() const;
    Kho getKho() const;
    Date getNgayNhap() const;
    float getSoLuongTan() const;
    float getTongTheTich() const;
    // Date getHsd() const; // Bỏ
    float getGiaTri() const;
    void setNCC(NhaCungCap ncc);
    NhaCungCap getNCC() const;
    std::vector<SanPhamTrongLo> getDanhSachSanPham() const;
    
    // --- SỬA ĐỔI CHỮ KÝ HÀM ---
    void NhapInfo(SQLHDBC hDbc); // Cần hDbc để gọi các hàm Doc/Ghi
    
    void printInfo() const;
};
#endif