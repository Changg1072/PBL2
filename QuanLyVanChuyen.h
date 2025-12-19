#pragma once 
#include <vector>
#include <string>
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <limits>
#include "SieuThi.h"
#include "Kho.h"
#include "PhuongTien.h"
#include "TuyenVanChuyen.h"

using namespace std;

namespace PBL2QuanLyKho {

    // 1. Struct hỗ trợ gom nhóm hàng hóa để lưu DB
    struct HangCanGiao {
        std::string maSP;
        float soLuong;
        std::string maYeuCauGoc;
        float thanhTien;
        std::string maLoHang;
    };

    // 2. Struct lưu thông tin một phương án vận chuyển
    struct PhuongAnToiUu {
        float tongChiPhi;       // Tổng tiền (VNĐ)
        float tongThoiGian;     // Tổng giờ (h)
        float khoangCach;       // Km
        std::string maPhuongTien;
        std::string tenLoaiXe;
        int soChuyen;
        std::string maKhoXuat;
        std::string tenKhoXuat;
        std::string maKhoTrungChuyen; // Có thể rỗng nếu đi thẳng
        std::string ghiChu;     // Giải thích (vd: "Khác tỉnh", "Nội thành")

        // Constructor mặc định để tránh lỗi rác
        PhuongAnToiUu() {
            tongChiPhi = std::numeric_limits<float>::max();
            tongThoiGian = std::numeric_limits<float>::max();
            soChuyen = 0;
            khoangCach = 0;
        }
    };

    // 3. Struct chứa cặp kết quả để người dùng chọn
    struct KetQuaSoSanh {
        PhuongAnToiUu paReNhat;    // Option A: Ưu tiên tiền
        PhuongAnToiUu paNhanhNhat; // Option B: Ưu tiên thời gian
    };

    class QuanLyVanChuyen {
    public:
        // --- THUẬT TOÁN CỐT LÕI (BÀI TOÁN 4) ---

        // 1. Tìm phương án dựa trên Tuyến Vận Chuyển có sẵn trong DB (Thực tế)
        static KetQuaSoSanh TimHaiPhuongAn(
            float tongTrongLuongCan,
            const SieuThi& stDich,
            const std::vector<Kho>& dsKho,
            const std::vector<PhuongTien>& dsXe,
            const std::vector<TuyenVanChuyen>& dsTuyenVC
        );

        // 2. Tìm phương án giả định (Khi kho hết hàng hoặc chưa có tuyến)
        static KetQuaSoSanh TimPhuongAnGiaDinh(
            float tongTrongLuong,
            const SieuThi& stDich,
            const std::vector<Kho>& dsKho,
            const std::vector<PhuongTien>& dsXe
        );

        // --- TƯƠNG TÁC CSDL ---

        // Lưu đơn hàng đã chọn vào SQL Server
        static void LuuDonHangVaoDB(SQLHDBC hDbc, const PhuongAnToiUu& pa, const std::string& maSieuThi, float tongKhoiLuong, const std::vector<HangCanGiao>& dsHang);

        // Hỗ trợ cập nhật trạng thái
        static void CapNhatTrangThaiYeuCau(SQLHDBC hDbc, std::string maYeuCau, std::string trangThai);

        // Batch job (nếu cần)
        static void XuLyTatCaYeuCau(SQLHDBC hDbc);
    };

}