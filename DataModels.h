#pragma once
#include <string>
#include <msclr/marshal_cppstd.h>
using namespace System;
using namespace System::Collections::Generic; // Để dùng List<>

static inline std::string ToStdString(System::String^ s) {
    if (System::String::IsNullOrEmpty(s)) return "";
    msclr::interop::marshal_context context;
    return context.marshal_as<std::string>(s);
}

static inline System::String^ ToSysString(std::string s) {
    return gcnew System::String(s.c_str(), 0, (int)s.length(), System::Text::Encoding::UTF8);
}

namespace PBL2QuanLyKho { // Đảm bảo nằm trong namespace

    // --- STRUCT THÔNG TIN SẢN PHẨM (CŨ) ---
    public ref struct ProductInfo {
        String^ MaSP;
        String^ TenSP;
        String^ Loai;
        double GiaHienTai;
        double GiaGoc;
        double NhietDo;
        String^ TenNCC;
    };

    // --- STRUCT MẶT HÀNG TRONG GIỎ (CŨ) ---
    public ref struct CartItem {
        String^ MaSP;
        String^ TenSP;
        double GiaKg;
        int SoLuong;
        double ThanhTien;
        void UpdateThanhTien() { ThanhTien = GiaKg * SoLuong; }
    };

    // ==========================================
    // CÁC CLASS DATA MODEL MỚI
    // ==========================================

    // 1. Class ImportItem (Chi tiết hàng trong giỏ nhập)
    public ref class ImportItem {
    public:
        String^ MaSP;
        String^ TenSP;
        double GiaVon;
        double SoLuong;
        // Hàm tính thành tiền
        double ThanhTien() { return GiaVon * SoLuong; }
    };

    // 2. Class PredictedLot (Lô hàng dự kiến - Sau khi tối ưu)
    public ref class PredictedLot {
    public:
        String^ MaNCC;
        String^ TenNCC;
        String^ LyDoChon;
        String^ GhiChuRoute;

        // [THÊM MỚI] Biến để lưu Mã Xe
        String^ MaXe;

        double PhiVanChuyen;
        double ThoiGianDuKien;
        List<ImportItem^>^ Items;

        PredictedLot() {
            Items = gcnew List<ImportItem^>();
            PhiVanChuyen = 0;
            ThoiGianDuKien = 0;
            LyDoChon = "";
            GhiChuRoute = "Chưa xác định";
            MaNCC = "UNKNOWN"; // Mặc định
            MaXe = "UNKNOWN";  // Mặc định
        }

        double TongTienHang() {
            double s = 0;
            for each (ImportItem ^ i in Items) s += i->ThanhTien();
            return s;
        }
    };

    // 3. Class CanhBaoItem (Dùng cho chức năng cảnh báo tồn kho ROP)
    public ref class CanhBaoItem {
    public:
        String^ MaSP;
        String^ TenSP;
        float TonTaiKho;
        float DiemDatHang; // ROP
        float EOQ;         // Lượng đặt hàng kinh tế
    };
}