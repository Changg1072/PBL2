#ifndef DINHGIADONG_H
#define DINHGIADONG_H

#include "SanPham.h"
#include "Date.h"
#include "ChiTietYeuCau.h" 
#include <vector>
#include <string>

// Struct hỗ trợ hồi quy tuyến tính
struct LichSuBanHang {
    float giaBan;
    float luongBan;
};

class DinhGiaDong : public SanPham {
private:
    float heSo_a; 
    float heSo_b; 
    float C_penalty_ngay;
    
    // Biến lưu chi phí thực tế
    float chiPhiLuuKhoThucTe; 

    // Các hàm tính toán nội bộ (Private)
    void tinhHoiQuyTuyenTinh(const std::vector<LichSuBanHang>& history);
    float duBaoNhuCau(float giaBan) const;
    
    // Hàm tính phí phạt lưu kho (dùng nội bộ cho bài toán Xả hàng)
    void tinhCPenaltyNgay();

public:
    // Struct lưu kết quả trả về
    struct KetQuaDinhGia {
        std::string maGia;
        std::string maSP;
        float giaBanDeXuat;
        float luongBanDuKien;
        float loiNhuanKyVong;
        Date ngayTinh;
        std::string ghiChu;
    };

    DinhGiaDong();
    DinhGiaDong(const SanPham& sp);

    // Hàm Setter để nạp chi phí từ bên ngoài vào
    void setChiPhiLuuKhoThucTe(float cp);

    // [ĐÃ SỬA: CHUYỂN TỪ PRIVATE SANG PUBLIC]
    // Hàm này cần Public để Main có thể gọi lấy giá P0
    float tinhGiaBanCoBan(float tyLeLoiNhuan) const; 

    // Các hàm chức năng (API)
    std::vector<LichSuBanHang> XayDungLichSuBanHang(const std::vector<ChiTietYeuCau>& dsYC, const std::string& maSP);
    void HuanLuyenMoHinh(const std::vector<LichSuBanHang>& history);
    
    // 2 Chiến lược định giá chính
    KetQuaDinhGia DeXuatGia_HangSapHetHan(SQLHDBC hDbc, float St_tong, float q1_sapHong, const std::vector<float>& cacMucGia);
    KetQuaDinhGia DeXuatGia_HangBanCham(SQLHDBC hDbc, float St_tong, const std::vector<float>& cacMucGia);
    
    // Hàm tiện ích static để lấy giá hiện tại nhanh
    static float LayGiaBanHienTai(SQLHDBC hDbc, const std::string& maSP);
    static void TuDongChayDinhGiaHomNay(SQLHDBC hDbc);
    void GhiNhanGiaNiemYet(SQLHDBC hDbc);
};

#endif