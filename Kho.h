#ifndef KHO_H
#define KHO_H

#include <string>
#include <iostream>
#ifndef NOMINMAX
#define NOMINMAX
#endif
// --- THÊM THƯ VIỆN ODBC ---
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
// -------------------------
class Kho {
private:
    std::string maKho;
    std::string tenKho;
    float sucChua;
    float NdoMin; // Nhiệt độ Min
    float NdoMax; // Nhiệt độ Max
    std::string diaChi;
    std::string Tinh;
    std::string quanHuyen;
    std::string sdt;
    float congSuatCoBan;    // Base Power (kW)
    float heSoCongSuat;     // Temp Factor (kW/độ)
    float chiPhiHaNhiet;    // Cost Down (VND/độ)
    float chiPhiTangNhiet;  // Cost Up (VND/độ)
public:
    Kho();
    Kho(std::string maKho, std::string tenKho, float sucChua, float NdoMin, float NdoMax, 
        std::string diaChi, std::string Tinh, std::string quanHuyen, std::string sdt,
        float congSuat = 500.0f, float heSo = 20.0f, 
        float cpHa = 5000.0f, float cpTang = 0.0f);
    
    // (Các hàm set/get không đổi)
    void setMaKho(std::string maKho);
    void setTenKho(std::string tenKho); 
    void setSucChua(float sucChua);
    void setNdoMin(float NdoMin);
    void setNdoMax(float NdoMax);
    void setDiaChi(std::string diaChi);
    void setTinh(std::string Tinh);
    void setQuanHuyen(std::string quanHuyen);
    void setSDT(std::string sdt);
    void setCongSuatCoBan(float cs);
    void setHeSoCongSuat(float hs);
    void setChiPhiHaNhiet(float cp);
    void setChiPhiTangNhiet(float cp);
    std::string getTinh() const;
    std::string getQuanHuyen() const;
    std::string getSDT() const;
    std::string getMaKho() const;
    std::string getTenKho() const;
    float getSucChua() const;
    float getNdoMin() const;
    float getNdoMax() const;
    std::string getDiaChi() const;
    float getCongSuatCoBan() const;
    float getHeSoCongSuat() const;
    float getChiPhiHaNhiet() const;
    float getChiPhiTangNhiet() const;
    
    // --- SỬA ĐỔI CHỮ KÝ HÀM ---
    void NhapInfo(SQLHDBC hDbc); // Cần hDbc để lưu vào CSDL
    
    void printInfo() const ;
};

#endif