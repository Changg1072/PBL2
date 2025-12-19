#ifndef PHUONGTIEN_H
#define PHUONGTIEN_H
#include "Kho.h"
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h> 
#include <sql.h>
#include <sqlext.h>

class PhuongTien {
private:
    // Thông tin bảng chính (PhuongTien)
    std::string maXe;
    std::string loaiXe;
    int soXe;
    float sucChuaTan;
    float chiPhiMoiChuyen;
    std::string tinh;
    std::string quanHuyen;
    std::string diaChi;
    std::string sdt;

    // Thông tin bảng phụ (ChiTietPhuongTien)
    float phuThemKhacTinh;
    float phuThemKhacQuanHuyen;
    
    float vanTocTB;
    float gioCongToiDaNgay;
    
    // Thuộc tính mới cho bài toán 4
    float Lrate;
    float Urate;
    float Tsetup;

public:
    // Constructor
    PhuongTien();
    // Constructor đầy đủ tham số (Đã cập nhật)
    PhuongTien(std::string ma, std::string loai, int soLuongXe, float sucChua, float chiPhi,
        std::string tinh, std::string quan, std::string dc, std::string sdt,
        float phuTinh, float phuQuan, float vanTocTB, float gioCongToiDa,
        float lrate, float urate, float tsetup);

    // Getters
    std::string getMaXe() const { return maXe; }
    std::string getLoaiXe() const { return loaiXe; }
    int getSoXe() const { return soXe; }
    float getSucChua() const { return sucChuaTan; }
    float getChiPhi() const { return chiPhiMoiChuyen; }
    std::string getTinh() const { return tinh; }
    std::string getQuanHuyen() const { return quanHuyen; }
    std::string getDiaChi() const { return diaChi; }
    std::string getSDT() const { return sdt; }
    float getPhuPhiTinh() const { return phuThemKhacTinh; }
    float getPhuPhiQuan() const { return phuThemKhacQuanHuyen; }
    float getVanTocTB() const { return vanTocTB;}
    float getGioCongToiDa() const { return gioCongToiDaNgay; }
    float getLrate() const { return Lrate; }
    float getUrate() const { return Urate; }
    float getTsetup() const { return Tsetup; }

    // Setters
    void setMaXe(std::string ma) { maXe = ma; }
    void setLoaiXe(std::string loai) { loaiXe = loai; }
    void setSoXe(int n) { soXe = n; }
    void setSucChua(float sc) { sucChuaTan = sc; }
    void setChiPhi(float cp) { chiPhiMoiChuyen = cp; }
    void setTinh(std::string t) { tinh = t; }
    void setQuanHuyen(std::string q) { quanHuyen = q; }
    void setDiaChi(std::string dc) { diaChi = dc; }
    void setSDT(std::string s) { sdt = s; }
    void setPhuPhiTinh(float p) { phuThemKhacTinh = p; }
    void setPhuPhiQuan(float p) { phuThemKhacQuanHuyen = p; }
    void setGioCongToiDa(float gio) { gioCongToiDaNgay = gio; }
    void setVanTocTB(float tb) { vanTocTB = tb;}
    void setLrate(float l) { Lrate = l; }
    void setUrate(float u) { Urate = u; }
    void setTsetup(float t) { Tsetup = t; }

    // Hàm chức năng
    void NhapInfo(SQLHDBC hDbc);
    void HienThi() const;

    struct KetQuaTinhToan {
        int soChuyen;
        float chiPhiCoDinh;
        float chiPhiPhuThem;
        float tongChiPhi;
        
        // Thông tin thời gian
        float khoangCachUocTinh; // km
        float thoiGianDiChuyen;  // giờ
        float thoiGianBocDo;     // giờ
        float tongThoiGian;      // giờ
        std::string ghiChu;
    };

    // ĐÂY LÀ HÀM BỊ THIẾU KHAI BÁO TRONG HEADER CŨ CỦA BẠN
    KetQuaTinhToan TinhChiPhiVaThoiGian(float tongKhoiLuongHang, 
                                       std::string tinhDi, std::string quanDi, 
                                       std::string tinhDen, std::string quanDen) const;

};

#endif