#ifndef NHACUNGCAP_H
#define NHACUNGCAP_H
#include <string>
#include <vector>
#include <iostream>
#ifndef NOMINMAX
#define NOMINMAX
#endif
// --- THÊM THƯ VIỆN ODBC ---
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
// -------------------------
class NhaCungCap {
private:
    std::string maNCC;
    std::string tenNCC;
    float thoiGianCho;
    float chiPhiDat;
    std::string Tinh;
    std::string QuanHuyen;
    std::string DiaChi;
    std::string SDT;
    std::vector<std::string> dsMaSanPham;
public:
    NhaCungCap();
    NhaCungCap(std::string maNCC, std::string tenNCC, float thoiGianCho, float chiPhiDat,std::string Tinh, std::string QuanHuyen,std::string DiaChi,std::string SDT);
    
    // (Các hàm set/get không đổi)
    void setMaNCC(std::string maNCC);
    void setTenNCC(std::string tenNCC);
    void setThoiGianCho(float thoiGianCho);
    void setChiPhiDat(float chiPhiDat);
    void setTinh(std::string Tinh);
    void setQuanHuyen(std::string QuanHuyen);
    void setDiaChi(std::string DiaChi);
    void setSDT(std::string SDT);
    void themMaSanPham(std::string maSP);
    void setDsMaSanPham(const std::vector<std::string>& ds);
    std::vector<std::string> getDsMaSanPham() const;
    std::string getTinh() const;
    std::string getQuanHuyen() const;
    std::string getDiaChi() const;
    std::string getSDT() const;
    std::string getMaNCC() const;
    std::string getTenNCC() const;
    float getThoiGianCho() const; 
    float getChiPhiDat() const;
    
    // --- SỬA ĐỔI CHỮ KÝ HÀM ---
    void NhapInfo(SQLHDBC hDbc); // Cần hDbc để lưu
    
    void printInfo() const;
};

#endif