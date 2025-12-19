#pragma once
#include <string>
#include <vector>
#include <windows.h>
#include <sql.h>
#include <sqlext.h>

// Struct trả về kết quả cho Giao diện hiển thị
struct KetQuaLichTrinh {
    int gio;
    int nhietDo;
    double giaDien;
    double chiPhiVanHanh;
    double chiPhiRuiRo;
};

// Struct lưu thông tin sản phẩm nội bộ
struct SanPhamLuuKho {
    std::string tenSP;
    double tongGiaTri;
    int nhietDoThichHop;
    double ruiRoLanh;
    double ruiRoNong;
};

struct KhungGioDien {
    int gioBatDau;
    int gioKetThuc;
    double giaDien;
};

class QuanLyNhietDo {
private:
    std::string maKho;
    std::string tenKho;
    int minTemp, maxTemp;
    double basePower, tempFactor;
    double chiPhiGiamDo, chiPhiTangDo;

    std::vector<SanPhamLuuKho> dsSanPham;
    std::vector<KhungGioDien> bangGiaDien;

    // Vector lưu kết quả tối ưu
    std::vector<KetQuaLichTrinh> lichTrinhToiUu;

public:
    QuanLyNhietDo(std::string ma);

    // Hàm nạp dữ liệu từ SQL
    void LayDuLieuTuDB(SQLHDBC hDbc);

    // Các hàm tính toán con
    double TinhCongSuat(int T);
    double LayGiaDien(int gio);
    double TinhChiPhiRuiRo(int T);
    double TinhCostRun(int gio, int T);
    double TinhCostSwitch(int T_current, int T_next);

    // Hàm chính: Quy hoạch động
    double GiaiBaiToanToiUu(int gioBatDau, int gioKetThuc);

    // Hàm lấy kết quả về để hiển thị
    std::vector<KetQuaLichTrinh> GetLichTrinh() const { return lichTrinhToiUu; }
    double LayGiaDienTheoGio(int gio) {
        // Giờ thấp điểm (1-3h, 22-24h): 1.146
        if ((gio >= 1 && gio <= 3) || (gio >= 22 && gio <= 24)) return 1.146;

        // Giờ cao điểm (10-11h, 17-20h): 3.266
        if ((gio >= 10 && gio <= 11) || (gio >= 17 && gio <= 20)) return 3.266;

        // Giờ bình thường: 1.811
        return 1.811;
    }
    double TinhRuiRo(double nhietDoHienTai) {
        double tongRuiRo = 0;

        // [SỬA LỖI 1]: Đổi danhSachSanPham -> dsSanPham
        for (const auto& sp : this->dsSanPham) {
            double chenhLech = 0;
            double heSoRuiRo = 0;

            if (nhietDoHienTai < sp.nhietDoThichHop) {
                chenhLech = sp.nhietDoThichHop - nhietDoHienTai;
                heSoRuiRo = sp.ruiRoLanh;
            }
            // Nếu nóng quá (Nhiệt độ hiện tại > Nhiệt độ thích hợp)
            else if (nhietDoHienTai > sp.nhietDoThichHop) {
                chenhLech = nhietDoHienTai - sp.nhietDoThichHop;
                heSoRuiRo = sp.ruiRoNong;
            }

            double ruiRoSP = chenhLech * heSoRuiRo * sp.tongGiaTri;
            tongRuiRo += ruiRoSP;
        }

        return tongRuiRo;
    }
};