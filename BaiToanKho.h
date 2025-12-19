#ifndef BAITOANKHO_H
#define BAITOANKHO_H

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include "Kho.h"
#include "SanPham.h"

// Thông tin lô hàng nhập khẩu dự kiến
struct LoHangNhap {
    std::string maLoTam;    // Mã lô tạm thời
    std::string maSP;
    std::string tenSP;      // <--- ĐÃ THÊM THUỘC TÍNH NÀY ĐỂ SỬA LỖI
    float soLuongTan;       // Khối lượng
    float theTichLo;        // Thể tích chiếm chỗ (m3)
    float giaTriLo;         // Giá trị kinh tế
    bool laHangCapBach;     // TRUE nếu đang dưới mức tồn kho an toàn (ROP)
    
    // Tính mật độ giá trị (Để tham lam nếu cần)
    float tyTrongGiaTri() const { return (theTichLo > 0) ? giaTriLo / theTichLo : 0; }
};

// Kết quả phân phối cho 1 kho
struct KetQuaKho {
    Kho kho;
    std::vector<LoHangNhap> dsLoHangDuocChon;
    float tongTheTichSuDung;
    float tongGiaTri;
    
    KetQuaKho() : tongTheTichSuDung(0), tongGiaTri(0) {}
};

class QuanLySucChuaKho {
public:
    // Hàm chính: Phân phối hàng vào chuỗi kho
    static std::vector<KetQuaKho> ToiUuNhapHangVaoKho(
        std::vector<Kho> dsKhoSanCo, 
        std::vector<LoHangNhap> dsHangCanNhap
    );

private:
    // Thuật toán Knapsack 0/1 cho 1 kho đơn lẻ
    static std::vector<LoHangNhap> GiaiBaiToanBaloChoMotKho(
        float sucChuaKho, 
        std::vector<LoHangNhap>& dsUngVien
    );
};

#endif