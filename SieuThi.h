#ifndef SIEUTHI_H
#define SIEUTHI_H

#include <string>
#include <vector>
#include "SanPham.h" 
#include "ChiTietYeuCau.h"
// --- THÊM CÁC INCLUDE NÀY ĐỂ NHẬN DIỆN SQLHDBC ---
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
// --------------------------------------------------

// Forward declaration (nếu cần)

class SieuThi {
private:
    std::string maSieuThi;
    std::string tenSieuThi;
    std::string Tinh;
    std::string QuanHuyen;
    std::string diaChi;
    std::string SDT;
    float mucPhatThieu;

    // --- SỬA CÁC KHAI BÁO NÀY ---
    // -----------------------------

public:
    SieuThi();
    SieuThi(std::string maSieuThi, std::string tenSieuThi, float MucPhatThieu,std::string Tinh, std::string QuanHuyen,std::string diaChi, std::string SDT);
    
    // Setters
    void setMaSieuThi(std::string maSieuThi);
    void setTenSieuThi(std::string tenSieuThi);
    void setDiaChi(std::string diaChi);
    void setMucPhatThieu(float mucPhatThieu);
    void setTinh(std::string Tinh);
    void setQuanHuyen(std::string QuanHuyen);
    void setSDT(std::string sdt);
    
    // Getters
    std::string getMaSieuThi() const;
    std::string getTenSieuThi() const;
    std::string getDiaChi() const;
    float getMucPhatThieu() const;
    std::string getTinh() const;
    std::string getQuanHuyen() const;
    std::string getSDT() const;
    
    // --- SỬA CÁC KHAI BÁO NÀY ---
    void NhapInfo(SQLHDBC hDbc); // Thêm hDbc
    std::vector<SanPhamYeuCau> YeuCauTuSieuThi(SQLHDBC hDbc, std::vector<SanPham>& ds) const; // Thêm hDbc
    // -----------------------------
    std::string SinhMaYeuCau(SQLHDBC hDbc, std::string maSieuThi, std::string tenSieuThi);
    bool checkDB_DaTonTai(SQLHDBC hDbc, std::string maKiemTra);
    void printInfo() const;
};

bool isNumber(const std::string& str);

#endif // SIEUTHI_H