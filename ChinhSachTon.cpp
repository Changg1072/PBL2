#include <iostream>
#include <fstream>
#include <numeric>
#include <map>
#include <string>
#include <vector>
#include "ChinhSachTon.h"
#include "cmath"
#include "SaoLuu.h"
#include "Date.h"

using namespace std;

ChinhSachTon::ChinhSachTon() : SanPham() {
    this->d = 0; this->sigma_d = 0; this->Co = 0; this->L = 0;
    this->mucPhucVu = 0; this->hesoZ = 0; this->Ch = 0;
    this->TonAnToan = 0; this->DiemDatHang = 0; this->EOQ = 0;
    
    // Mặc định 30k nếu chưa chạy bài toán nhiệt độ
    this->chiPhiLuuKhoThucTe = 30000.0f; 
    this->LAI_SUAT_AN_TOAN  = 0.05f;
    this->CHI_PHI_CO_HOI_MAT_BANG_NGAY = 4000000.0f;
}
void ChinhSachTon::setChiPhiLuuKhoThucTe(float cp) {
    if (cp > 0) this->chiPhiLuuKhoThucTe = cp;
}
ChinhSachTon::ChinhSachTon(const SanPham& sp) : SanPham(sp) {
    this->d = 0; this->sigma_d = 0; this->Co = 0; this->L = 0;
    this->mucPhucVu = 0; this->hesoZ = 0; this->Ch = 0;
    this->TonAnToan = 0; this->DiemDatHang = 0; this->EOQ = 0;
    
    this->chiPhiLuuKhoThucTe = 30000.0f; // Mặc định
    this->LAI_SUAT_AN_TOAN  = 0.05f;
    this->CHI_PHI_CO_HOI_MAT_BANG_NGAY = 4000000.0f;
}

// --- HÀM setParamsVaTinhToan ĐÃ SỬA ĐỔI ---
void ChinhSachTon::setParamsVaTinhToan(SQLHDBC hDbc,
                                       float d_input, 
                                       float sigma_d_input, 
                                       float Co_input, 
                                       float L_input,
                                       float chiPhiThucTe) // <-- Nhận tham số mới
{
    this->d = d_input;
    this->sigma_d = sigma_d_input;
    this->Co = Co_input;
    this->L = L_input;
    
    // Nếu có truyền chi phí thực tế vào thì cập nhật
    if (chiPhiThucTe > 0) {
        this->chiPhiLuuKhoThucTe = chiPhiThucTe;
    }
    
    tinhMucPhucVu();    
    tinhHeSoZ();        
    tinhChiPhiLuuKho(); // <-- Hàm này sẽ dùng chiPhiLuuKhoThucTe
    tinhTonAnToan();   
    tinhDiemDatHang(); 
    tinhEOQ();          
    
    try {
        CapNhatGhiFileChinhSachTon(hDbc, *this);
    } catch (const std::exception& e) {
        cerr << "[LOI] Khong the cap nhat ChinhSachTon: " << e.what() << endl;
    }
}

// (Các hàm private tinh... không đổi)
void ChinhSachTon::tinhMucPhucVu(){
    std::string phanLoai = getNhomPhanLoai();
    if (phanLoai == "A" || phanLoai == "a") {
        this->mucPhucVu = 0.99; 
    } else if (phanLoai == "B" || phanLoai == "b") {
        this->mucPhucVu = 0.95; 
    } else { 
        this->mucPhucVu = 0.85;
    }
}
void ChinhSachTon::tinhHeSoZ() {
    if (this->mucPhucVu == 0.99) {
        this->hesoZ = 2.33; 
    } else if (this->mucPhucVu == 0.95) {
        this->hesoZ = 1.65; 
    } else { // 0.85
        this->hesoZ = 1.04; 
    }
}
void ChinhSachTon::tinhChiPhiLuuKho() {
    // Sử dụng biến chiPhiLuuKhoThucTe thay vì số cứng 30000
    float chiPhiLuuKhoThat_Nam = this->chiPhiLuuKhoThucTe * 365.0f;
    
    std::string phanLoai = getNhomPhanLoai();
    
    // Nếu là nhóm C (Hàng bán chậm) -> Tính thêm chi phí phạt ảo
    if (phanLoai == "C" || phanLoai == "c") {
        float giaVon = getGiaVon(); 
        float chiPhiCoHoiVon_Ngay = giaVon * (LAI_SUAT_AN_TOAN / 365.0f);
        
        float c_penalty_ngay = this->chiPhiLuuKhoThucTe 
                             + chiPhiCoHoiVon_Ngay 
                             + CHI_PHI_CO_HOI_MAT_BANG_NGAY;
                             
        this->Ch = c_penalty_ngay * 365.0f;
    } else {
        // Nhóm A, B: Chỉ tính chi phí thật
        this->Ch = chiPhiLuuKhoThat_Nam;
    }
}
void ChinhSachTon::tinhTonAnToan() {
    if (this->L < 0) this->L = 0; 
    this->TonAnToan = this->hesoZ * this->sigma_d * sqrt(this->L);
}
void ChinhSachTon::tinhDiemDatHang() {
    this->DiemDatHang = (this->d * this->L) + this->TonAnToan;
}
void ChinhSachTon::tinhEOQ() {
    float D = this->d * 365.0;
    if (this->Ch <= 0) {
        this->EOQ = 0;
        return;
    }
    this->EOQ = sqrt((2.0 * D * this->Co) / this->Ch);
}

// (Các hàm get... và kiemTraDatHang không đổi)
float ChinhSachTon::getMucPhucVu() const { return mucPhucVu; }
float ChinhSachTon::getHeSoZ() const { return hesoZ; }
float ChinhSachTon::getChiPhiLuuKho() const { return Ch; }
float ChinhSachTon::getTonAnToan() const { return TonAnToan; }
float ChinhSachTon::getDiemDatHang() const { return DiemDatHang; }
float ChinhSachTon::getEOQ() const { return EOQ; }
std::string ChinhSachTon::kiemTraDatHang(float tonKhoHienTai) const {
    if (tonKhoHienTai <= this->DiemDatHang) {
        return "KICH HOAT DAT HANG (So luong de nghi: " + to_string(this->EOQ) + " tan)";
    } else {
        return "CHUA CAN DAT HANG (Ton kho > ROP)";
    }
}

// (Các hàm set... không đổi)
void ChinhSachTon::setMucPhucVu(float val) { this->mucPhucVu = val; }
void ChinhSachTon::setHeSoZ(float val) { this->hesoZ = val; }
void ChinhSachTon::setTonAnToan(float val) { this->TonAnToan = val; }
void ChinhSachTon::setDiemDatHang(float val) { this->DiemDatHang = val; }
void ChinhSachTon::setEOQ(float val) { this->EOQ = val; }

// Hàm này không còn cần thiết, CSDL đã xử lý
// void ChinhSachTon::LuuVaoFile(ofstream& ofs) const { ... }


// (Hàm PhanTichLichSuBanHang không đổi)
ThongSoThongKe PhanTichLichSuBanHang(const std::string& maSP, const std::vector<ChiTietYeuCau>& dsYC, Date ngayBatDau, Date ngayKetThuc) {
    // (Giữ nguyên logic thống kê như cũ)
    ThongSoThongKe ketQua;
    map<long, float> doanhSoTheoNgay;
    int countFound = 0;
    for (const auto& yc : dsYC) {
        Date ngayCuaDon = yc.getNgayNhap();
        if (ngayCuaDon < ngayBatDau || ngayKetThuc < ngayCuaDon) continue; 
        for (const auto& spYc : yc.getdsYeucau()) {
            if (spYc.sp != nullptr && spYc.sp->getMaSP() == maSP) {
                doanhSoTheoNgay[ngayCuaDon.toDays()] += spYc.soLuongTan;
                countFound++;
            }
        }
    }
    if (countFound == 0) return ketQua;

    std::vector<float> lichSuBanHang;
    long soNgayPhanTich = Date::daysBetween(ngayBatDau, ngayKetThuc) + 1;
    if (soNgayPhanTich <= 1) return ketQua;

    Date ngayHienTai = ngayBatDau;
    for (long i = 0; i < soNgayPhanTich; ++i) {
        long keyNgay = ngayHienTai.toDays();
        if (doanhSoTheoNgay.count(keyNgay)) lichSuBanHang.push_back(doanhSoTheoNgay[keyNgay]);
        else lichSuBanHang.push_back(0.0f);
        ngayHienTai.nextDay(); 
    }
    float tongDoanhSo = std::accumulate(lichSuBanHang.begin(), lichSuBanHang.end(), 0.0f);
    ketQua.d = tongDoanhSo / soNgayPhanTich;
    float tongBinhPhuongHieu = 0.0f;
    for (float x : lichSuBanHang) tongBinhPhuongHieu += (x - ketQua.d) * (x - ketQua.d);
    ketQua.sigma_d = sqrt(tongBinhPhuongHieu / (soNgayPhanTich - 1));
    return ketQua;
}

// --- HÀM ChayPhanTichLop2 ĐÃ SỬA ĐỔI ---
void ChayPhanTichLop2(SQLHDBC hDbc,
                      ChinhSachTon& cs, 
                      const std::vector<ChiTietYeuCau>& dsYC,
                      float Co_input,
                      float L_input,
                      float chiPhiThucTe) // <-- Nhận tham số
{
    if (dsYC.empty()) {
        cout << "Khong co du lieu Yeu Cau de phan tich." << endl;
        return;
    }
    Date ngayDauTien = dsYC.front().getNgayNhap();
    Date ngayCuoiCung = dsYC.back().getNgayNhap();
    
    ThongSoThongKe thongSo = PhanTichLichSuBanHang(cs.getMaSP(), dsYC, ngayDauTien, ngayCuoiCung);
    
    // Truyền chi phí thực tế vào hàm tính toán
    cs.setParamsVaTinhToan(hDbc, 
                           thongSo.d, 
                           thongSo.sigma_d, 
                           Co_input, 
                           L_input, 
                           chiPhiThucTe); // <-- Truyền tiếp
}