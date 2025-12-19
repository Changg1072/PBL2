#include <iostream>
#include <string>
#include <vector>
#include <random>       // Để tạo số ngẫu nhiên
#include <stdexcept>    // Để sử dụng random_device
#include "DuBaoNhuCau.h"
#include "Date.h"       // Cần cho đối tượng Date
#include "ChiTietYeuCau.h" // Cần cho vector<ChiTietYeuCau>
#include "ChinhSachTon.h" // Cần cho hàm PhanTichLichSuBanHang và struct ThongSoThongKe


std::string sinhMaDuBao() {
    const std::string charset = "0123456789"; // Chỉ dùng số
    std::string soNgauNhien;
    
    // Sử dụng random_device để có seed ngẫu nhiên an toàn
    static random_device rd;
    // Khởi tạo bộ sinh số ngẫu nhiên Mersenne Twister
    static mt19937 gen(rd()); 
    // Phân phối đều trong khoảng [0, charset.size() - 1]
    static uniform_int_distribution<> dis(0, charset.size() - 1);

    for (int i = 0; i < 7; ++i) { // Tạo 7 số theo yêu cầu
        soNgauNhien += charset[dis(gen)];
    }
    return "DB" + soNgauNhien; // Ghép "DB" vào đầu
}

KetQuaDuBao DuBaoNhuCau(const std::string& maSP,
                        const std::vector<ChiTietYeuCau>& dsYC,
                        Date tuNgay,
                        Date denNgay) 
{
    KetQuaDuBao ketQua;
    ketQua.maSP = maSP;
    ketQua.tuNgay = tuNgay;
    ketQua.denNgay = denNgay;

    // 1. Sinh mã dự báo ngẫu nhiên
    ketQua.maDuBao = sinhMaDuBao();
    ThongSoThongKe thongSo = PhanTichLichSuBanHang(maSP, 
                                                  dsYC, 
                                                  tuNgay, 
                                                  denNgay);
    ketQua.nhuCauTrungBinh = thongSo.d;
    ketQua.doLechChuan = thongSo.sigma_d;
    return ketQua;
}