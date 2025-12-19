#ifndef CHINHSACHTON_H
#define CHINHSACHTON_H

#include <cmath>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>
#include <sql.h>
#include <sqlext.h>

#include "SanPham.h"
#include "ChiTietYeuCau.h"

struct ThongSoThongKe {
    float d = 0.0f;       
    float sigma_d = 0.0f; 
};

class ChinhSachTon : public SanPham{
private:
    float d;
    float sigma_d;
    float Co;
    float L;
    float Ch; // Chi phí lưu kho/năm
    float mucPhucVu;
    float hesoZ;
    float TonAnToan;
    float DiemDatHang;
    float EOQ;
    
    // [MỚI] Biến lưu chi phí thực tế (đơn vị: VND/tấn/ngày)
    // Được tính từ Bài toán Nhiệt độ
    float chiPhiLuuKhoThucTe; 

    // Các hằng số khác giữ nguyên
    float LAI_SUAT_AN_TOAN  = 0.05;
    float CHI_PHI_CO_HOI_MAT_BANG_NGAY = 4000000;
    
    // Hàm private
    void tinhMucPhucVu();
    void tinhHeSoZ();
    void tinhTonAnToan();
    void tinhDiemDatHang();
    void tinhEOQ();
    
    // [SỬA ĐỔI] Hàm này sẽ dùng chiPhiLuuKhoThucTe
    void tinhChiPhiLuuKho(); 

public:
    ChinhSachTon();
    ChinhSachTon(const SanPham& sp);

    // [MỚI] Hàm Setter để nạp chi phí từ bên ngoài
    void setChiPhiLuuKhoThucTe(float cp);

    // [SỬA ĐỔI] Thêm tham số chi phí vào đây (mặc định = 0 nếu chưa tính)
    void setParamsVaTinhToan(SQLHDBC hDbc,
                              float d_input,
                              float sigma_d_input,
                              float Co_input,
                              float L_input,
                              float chiPhiThucTe = 0.0f); // <-- THÊM THAM SỐ
    
    // Getters/Setters không đổi
    float getMucPhucVu() const;
    float getHeSoZ() const;
    float getChiPhiLuuKho() const;
    float getTonAnToan() const;
    float getDiemDatHang() const;
    float getEOQ() const;
    std::string kiemTraDatHang(float tonKhoHienTai) const;
    void setMucPhucVu(float val);
    void setHeSoZ(float val);
    void setTonAnToan(float val);
    void setDiemDatHang(float val);
    void setEOQ(float val);
};

ThongSoThongKe PhanTichLichSuBanHang(const std::string& maSP,
                                     const std::vector<ChiTietYeuCau>& dsYC,
                                     Date ngayBatDau,
                                     Date ngayKetThuc);

void ChayPhanTichLop2(SQLHDBC hDbc,
                      ChinhSachTon& cs, 
                      const std::vector<ChiTietYeuCau>& dsYC,
                      float Co_input,
                      float L_input,
                      float chiPhiThucTe = 0.0f); // <-- THÊM THAM SỐ

#endif