#include <iostream>
#include <map>         
#include <algorithm>   
#include <iomanip>
#include <vector>
#include "SanPham.h"
#include "SaoLuu.h"
#include "ChiTietYeuCau.h"
using namespace std;
// Constructor, Setters, Getters (không đổi)
SanPham::SanPham() {
    maSP = "";
    tenSP = "";
    loai = "";
    giaVon = 0.0f;
    thetich1tan = 0.0f;
    NdoThichHop = 0.0f;
    NhomPhanLoai = ""; // Khởi tạo là chuỗi rỗng
    ruiRoLanh = 0.0f; ruiRoNong = 0.0f;
    giaBan = 0.0f; hanSuDung = 30;
}
SanPham::SanPham(std::string maSP, std::string tenSP, std::string loai, float giaVon, 
                 float thetich1tan, float NdoThichHop, std::string NhomPhanLoai,
                 float ruiRoLanh, float ruiRoNong, float giaBan, int hanSuDung) {
    setMaSP(maSP);
    setTenSP(tenSP);
    setLoai(loai);
    setGiaVon(giaVon);
    setThetich1tan(thetich1tan);
    setNdoThichHop(NdoThichHop);
    setNhomPhanLoai(NhomPhanLoai);
    this->ruiRoLanh = ruiRoLanh;
    this->ruiRoNong = ruiRoNong;
    this->giaBan = giaBan;
    this->hanSuDung = hanSuDung;
}
void SanPham::setMaSP(std::string maSP) { this->maSP = maSP; }
void SanPham::setTenSP(std::string tenSP) { this->tenSP = tenSP; }
void SanPham::setLoai(std::string loai) { this->loai = loai; }
void SanPham::setGiaVon(float giaVon) { this->giaVon = giaVon; }
void SanPham::setThetich1tan(float thetich1tan) { this->thetich1tan = thetich1tan; }
void SanPham::setNdoThichHop(float NdoThichHop) { this->NdoThichHop = NdoThichHop; }
void SanPham::setNhomPhanLoai(std::string NhomPhanLoai) { this->NhomPhanLoai = NhomPhanLoai; }
void SanPham::setGiaBan(float g) { this->giaBan = g;}
void SanPham::setRuiRoLanh(float r) { this->ruiRoLanh = r;}
void SanPham::setRuiRoNong(float r) { this->ruiRoNong = r;}
void SanPham::setHanSuDung(int h) { this->hanSuDung = h;}
std::string SanPham::getMaSP() const{ return maSP; }
std::string SanPham::getTenSP() const { return tenSP; }
std::string SanPham::getLoai() const{ return loai; }
float SanPham::getGiaVon() const { return giaVon; }
float SanPham::getThetich1tan() const{ return thetich1tan; }
float SanPham::getNdoThichHop() const { return NdoThichHop; }
std::string SanPham::getNhomPhanLoai() const { return NhomPhanLoai; }
float SanPham::getRuiRoLanh() const { return ruiRoLanh;}
float SanPham::getRuiRoNong() const { return ruiRoNong;}
float SanPham::getGiaBan() const { return giaBan;}
int SanPham::getHanSuDung() const { return hanSuDung;}

// --- KẾT THÚC KHÔNG ĐỔI ---


// --- ĐÃ SỬA ĐỔI ---
// Thêm tham số SQLHDBC hDbc
void SanPham::NhapInfo(SQLHDBC hDbc) {
    cout << "Nhap ten san pham: ";
    getline(cin, tenSP);
    cout << "Nhap loai san pham: ";
    getline(cin, loai);
    cout << "Nhap gia von: ";
    cin >> giaVon;
    cout << "Nhap gia ban du kien: "; cin >> giaBan;
    cout << "Nhap the tich 1 tan: ";
    cin >> thetich1tan;
    cout << "Nhap do thich hop: ";
    cin >> NdoThichHop;
    cout << "Nhap % rui ro khi lanh hon (VD: 0.01 la 1%): "; cin >> ruiRoLanh;
    cout << "Nhap % rui ro khi nong hon (VD: 0.02 la 2%): "; cin >> ruiRoNong;
    cout << "Nhap han su dung tieu chuan (ngay): "; cin >> hanSuDung;
    cin.ignore();
    NhomPhanLoai = "C"; // Mặc định là chuỗi "C"
    
    // Gọi hàm GhifileSanPham và truyền hDbc vào
    // Hàm này sẽ tự sinh MaSP và lưu vào CSDL
    GhifileSanPham(hDbc, *this);
}

void SanPham::printInfo() const {
    // Không đổi
    cout << "Ma San Pham: " << maSP << endl; // Thêm MaSP
    cout << "Ten San Pham: " << tenSP << endl;
    cout << "Loai San Pham: " << loai << endl;
    cout << "Gia Von: " << giaVon << " | Gia Ban: " << (long long)giaBan<< endl;
    cout << "The Tich 1 Tan: " << thetich1tan << endl;
    cout << "Do Thich Hop: " << NdoThichHop << endl;
    cout << "Nhom Phan Loai: " << NhomPhanLoai << endl;
    cout << "Temp Ideal: " << NdoThichHop << "C" << endl;
    cout << "Rui ro: Lanh(" << ruiRoLanh << ") - Nong(" << ruiRoNong << ")" << endl;
}       

bool sosanhDoanhThu(const SKUPhanTich& a,const SKUPhanTich& b){
    // Không đổi
    return a.tongDoanhThu > b.tongDoanhThu;
}

// --- ĐÃ SỬA ĐỔI ---
// Thêm tham số SQLHDBC hDbc
void SanPham::ThucHienPhanTichNhom(SQLHDBC hDbc, std::vector<SanPham>& dsSP){
    // ----- BƯỚC 1: THU THẬP VÀ TỔNG HỢP -----
    std::map<std::string, SKUPhanTich> mapSKU;
    float tongDoanhThuToanBo = 0.0f;
    std::vector<ChiTietYeuCau> dsYC;
    
    // Truyền hDbc vào hàm DocFileYeuCau
    DocFileYeuCau(hDbc, dsYC, dsSP);
    
    // Logic còn lại không đổi
    for (const auto& yc : dsYC) {
        for (const auto& spChiTiet : yc.getdsYeucau()) { 
            std::string ma = spChiTiet.sp->getMaSP();
            if (mapSKU.find(ma) == mapSKU.end()) {
                mapSKU[ma].maSP = ma;
                mapSKU[ma].tenSP = spChiTiet.sp->getTenSP();
            }
            mapSKU[ma].tongDoanhThu += spChiTiet.ThanhTienRieng;
            mapSKU[ma].tongSoLuong += spChiTiet.soLuongTan;
        }
    }
    
    std::vector<SKUPhanTich> listSKU;
    for (auto const& [ma, data] : mapSKU) {
        listSKU.push_back(data);
    }
    std::sort(listSKU.begin(), listSKU.end(), sosanhDoanhThu);
    
    for(const auto& sku : listSKU) {
        tongDoanhThuToanBo += sku.tongDoanhThu;
    }
    if (tongDoanhThuToanBo == 0) {
        std::cout << "Khong co du lieu doanh thu de phan tich." << std::endl;
        return;
    }

    float phanTramCongDon = 0.0f;
    for (auto& sku : listSKU) {
        sku.phanTramDoanhThu = (sku.tongDoanhThu / tongDoanhThuToanBo) * 100.0f;
        phanTramCongDon += sku.phanTramDoanhThu;
        sku.phanTramCongDon = phanTramCongDon;
    }
    
    for (auto& sku : listSKU) {
        if (sku.phanTramCongDon <= 80.0f) {
            sku.nhomABC = 'A';
        } else if (sku.phanTramCongDon <= 95.0f) {
            sku.nhomABC = 'B';
        } else {
            sku.nhomABC = 'C';
        }
    }
    
    // ----- BƯỚC 4: CẬP NHẬT DANH SÁCH SẢN PHẨM CHÍNH (dsSP) -----
    std::map<std::string, char> phanLoaiMap;
    for(const auto& sku : listSKU) {
        phanLoaiMap[sku.maSP] = sku.nhomABC;
    }
    
    for (auto& sp : dsSP) {
        if (phanLoaiMap.count(sp.getMaSP())) {
            std::string nhom(1, phanLoaiMap[sp.getMaSP()]); // Chuyển char 'A' -> string "A"
            sp.setNhomPhanLoai(nhom);
        } else {
            sp.setNhomPhanLoai("C"); // Gán chuỗi "C"
        }
    }

    // ----- BƯỚC 5: GHI LẠI TOÀN BỘ CSDL -----
    // Truyền hDbc vào hàm GhiDeFileSanPham
    GhiDeFileSanPham(hDbc, dsSP);
}