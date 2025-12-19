#ifndef QUANLYNHAPHANG_H
#define QUANLYNHAPHANG_H

#include <vector>
#include <string>
#include <map>
#include "NhaCungCap.h"
#include "Kho.h"
#include "PhuongTien.h"
#include "SanPham.h"

// Cấu trúc lưu thông tin mặt hàng cần nhập
struct HangCanNhap {
    std::string maSP;
    std::string tenSP;
    float soLuongCanNhap; // = EOQ
    float trongLuongTong;
};

// Cấu trúc tuyến nhập hàng (Mapping với bảng TuyenNhapHang)
class TuyenNhapHang {
public:
    std::string maTuyen;
    std::string maNCC;
    std::string maKhoNhan;
    std::string khoTrungChuyen;
    std::string maXe;
    float chiPhiCoDinh;

    TuyenNhapHang(std::string m, std::string ncc, std::string kho, std::string ktc, std::string xe, float cp)
        : maTuyen(m), maNCC(ncc), maKhoNhan(kho), khoTrungChuyen(ktc), maXe(xe), chiPhiCoDinh(cp) {}
};

class QuanLyNhapHang {
public:
    // Struct lưu kết quả tối ưu
    struct PhuongAnNhapHang {
        float tongChiPhi;
        float tongThoiGian;
        std::string maPhuongTien;
        std::string tenLoaiXe;
        int soChuyen;
        std::string maNCC;
        std::string maKhoNhan;
        std::string ghiChu; 
        // SAI Ở ĐÂY: Bạn đã để hàm NhapHangThuCong ở trong struct này
    };

    struct KetQuaSoSanhNhap {
        PhuongAnNhapHang paReNhat;
        PhuongAnNhapHang paNhanhNhat;
    };

    // --- CHỨC NĂNG CHÍNH (PUBLIC METHODS) ---
    
    // 1. Tự động quét các sản phẩm
    static std::map<std::string, std::vector<HangCanNhap>> DeXuatNhapHangTuDong(SQLHDBC hDbc);

    // 2. Tính toán phương án vận chuyển tối ưu
    static KetQuaSoSanhNhap ToiUuVanChuyenNhapHang(
        float tongTrongLuong,
        const NhaCungCap& ncc,
        const Kho& khoDich,
        const std::vector<PhuongTien>& dsXe,
        const std::vector<TuyenNhapHang>& dsTuyen
    );

    // 3. Hàm xử lý chính
    static void XuLyNhapHang(SQLHDBC hDbc);

    // 4. Hàm nhập tay (ĐÃ CHUYỂN RA NGOÀI STRUCT - ĐÚNG VỊ TRÍ)
    static void NhapHangThuCong(SQLHDBC hDbc); 

private:
    // Helper functions
    static void DocFileTuyenNhap(SQLHDBC hDbc, std::vector<TuyenNhapHang>& dsTuyen);
    static void TaoLoHangDuKien(SQLHDBC hDbc, const PhuongAnNhapHang& pa, const std::vector<HangCanNhap>& dsHang);
    static void ThucHienLuuKho(SQLHDBC hDbc, const PhuongAnNhapHang& pa, const HangCanNhap& hang);
};

#endif