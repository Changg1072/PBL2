#include <iostream>
#include <string>
#include <vector>
#include "LoHang.h"
#include "SanPham.h"    
#include "Kho.h"
#include "Date.h"
#include "SaoLuu.h"
#include "NhaCungCap.h"
using namespace std;
LoHang::LoHang() {
    maLoHang = "";
    danhSachSanPham.clear();
    kho = Kho();
    ncc = NhaCungCap();
    NgayNhap = Date();
    soLuongTan = 0.0f;
    tongTheTich = 0.0f;
    // hsd = Date(); // Bỏ
    giaTri = 0.0f;
}

// (Các hàm set/get không đổi - ngoại trừ HSD đã bị loại bỏ)
void LoHang::setMaLoHang(std::string maLoHang) { this->maLoHang = maLoHang; }
void LoHang::setKho(Kho kho) { this->kho = kho; }
void LoHang::setNgayNhap(Date ngayNhap) { this->NgayNhap = ngayNhap; }
void LoHang::setSoLuongTan(float soLuongTan) { this->soLuongTan = soLuongTan; }
void LoHang::setTongTheTich(float tongTheTich) { this->tongTheTich = tongTheTich; }
void LoHang::setGiaTri(float giaTri) { this->giaTri = giaTri; }
void LoHang::setDanhSachSanPham(vector<SanPhamTrongLo> ds) { this->danhSachSanPham = ds; }
void LoHang::setNCC(NhaCungCap ncc){ this->ncc = ncc; }
NhaCungCap LoHang::getNCC() const{ return ncc; }
std::string LoHang::getMaLoHang() const { return maLoHang; }
vector<SanPhamTrongLo> LoHang::getDanhSachSanPham() const{ return danhSachSanPham; }
Kho LoHang::getKho() const{ return kho; }
Date LoHang::getNgayNhap() const { return NgayNhap; }
float LoHang::getSoLuongTan() const { return soLuongTan; }
float LoHang::getTongTheTich() const{ return tongTheTich; }
float LoHang::getGiaTri() const { return giaTri; }
// --- KẾT THÚC HÀM SET/GET ---


// --- HÀM NhapInfo ĐÃ SỬA ĐỔI ---
void LoHang::NhapInfo(SQLHDBC hDbc) { // Nhận hDbc
    int n;
    cout << "So luong loai san pham trong lo hang: ";
    cin >> n;
    cin.ignore();
    
    // Load danh sach san pham de tra cuu TỪ CSDL
    vector<SanPham> masterProductList;
    DocfileSanPham(hDbc, masterProductList); // <-- Truyền hDbc

    int day, month, year;
    float giaTriSP;
    this->tongTheTich = 0;
    this->giaTri = 0;
    this->soLuongTan = 0;

    for (int i = 0; i < n; i++) {
        SanPham sp; 
        float soLuong;
        cout << "\n=== Nhap thong tin san pham thu " << i + 1 << " ===\n";
        
        std::string maSPInput;
        cout << "Nhap ma san pham de tra cuu: ";
        getline(cin, maSPInput);

        bool spFound = false;
        for (const auto& p : masterProductList) {
            if (p.getMaSP() == maSPInput) {
                sp = p; 
                spFound = true;
                cout << "--> Da tim thay san pham: " << sp.getTenSP() << endl;
                break;
            }
        }

        if (!spFound) {
            cout << "--> Ma SP khong ton tai. Vui long nhap thong tin san pham moi.\n";
            // Gọi NhapInfo() của SanPham và truyền hDbc
            sp.NhapInfo(hDbc); // <-- Truyền hDbc
            // Hàm NhapInfo của SanPham đã tự lưu vào CSDL
            masterProductList.push_back(sp); // Thêm vào danh sách tạm thời
        }
        
        cout << "Nhap so luong san pham (tan): ";
        cin >> soLuong;
        cin.ignore();
        cout << "Nhap han su dung (dd mm yyyy): ";
        cin >> day >> month >> year;
        cin.ignore();
        
        Date hsdInput(day, month, year);
        float theTichSP = sp.getThetich1tan() * soLuong;
        giaTriSP = sp.getGiaVon() * soLuong;
        
        this->tongTheTich += theTichSP;
        this->giaTri += giaTriSP;
        this->soLuongTan += soLuong;
        
        SanPhamTrongLo spTrongLo = {sp, soLuong, hsdInput, giaTriSP, theTichSP};
        danhSachSanPham.push_back(spTrongLo);
    }
    
    // --- XỬ LÝ NHÀ CUNG CẤP ---
    cout << "\nNhap ma nha cung cap: ";
    std::string maNCCInput;
    getline(cin, maNCCInput);
    vector<NhaCungCap> nccList;
    DocfileNhaCungCap(hDbc, nccList); // <-- Truyền hDbc
    
    bool nccFound = false;
    for (const auto& n : nccList) {
        if (n.getMaNCC() == maNCCInput) {
            this->ncc = n;
            nccFound = true;
            break;
        }
    }
    if (!nccFound) {
        cout << "Ma nha cung cap khong ton tai. Vui long nhap lai thong tin NCC.\n";
        // Giả định NhaCungCap.h/cpp đã được sửa giống SanPham.h/cpp
        this->ncc.NhapInfo(hDbc); // <-- Truyền hDbc
    }

    // --- XỬ LÝ KHO ---
    cout << "\nNhap ma kho: ";
    std::string maKhoInput;
    getline(cin, maKhoInput);
    
    vector<Kho> khoList;
    DocfileKho(hDbc, khoList); // <-- Truyền hDbc
    bool found = false;
    for (const auto& k : khoList) {
        if (k.getMaKho() == maKhoInput) {
            this->kho = k;
            found = true;
            break;
        }
    }
    
    if (!found) {
        cout << "Ma kho khong ton tai. Vui long nhap lai thong tin kho.\n";
        // Giả định Kho.h/cpp đã được sửa giống SanPham.h/cpp
        this->kho.NhapInfo(hDbc); // <-- Truyền hDbc
    }
    
    cout << "\nNhap ngay nhap kho (dd mm yyyy): ";
    cin >> day >> month >> year;
    cin.ignore();
    Date ngayNhapInput(day, month, year);
    setNgayNhap(ngayNhapInput);
    
    // Tự động lưu Lô hàng mới này vào CSDL
    GhifileLoHang(hDbc, *this); // <-- Truyền hDbc
    
    // Cập nhật lại MaLoHang cho đối tượng hiện tại (vì nó được sinh trong Ghifile)
    // (Đảm bảo GhifileLoHang(hDbc, LoHang& lo) nhận bằng tham chiếu &)
    cout << "\n=== Da luu lo hang voi MA: " << this->getMaLoHang() << " ===\n";
}

void LoHang::printInfo() const {
    cout << "\n========== THONG TIN LO HANG ==========\n";
    cout << "Ma Lo Hang: " << this->maLoHang << endl; // <-- Thêm MaLoHang
    cout << "Nha Cung Cap: " << this->ncc.getMaNCC() << " - " << this->ncc.getTenNCC() << endl; 
    cout << "Kho: " << this->kho.getMaKho() << " - " << this->kho.getTenKho() << endl;
    cout << "Ngay Nhap: ";
    this->NgayNhap.printDate();
    cout << endl;
    cout << "Tong Gia Tri: " << this->giaTri << " VND" << endl;
    cout << "Tong The Tich: " << this->tongTheTich << " m3" << endl;
    cout << "Tong So Luong: " << this->soLuongTan << " tan" << endl;
    cout << "\n--- Danh Sach San Pham ---\n";
    
    for (size_t i = 0; i < danhSachSanPham.size(); i++) {
        const auto& spL = danhSachSanPham[i];
        cout << i + 1 << ". Ma SP: " << spL.sanPham.getMaSP()
             << " | Ten: " << spL.sanPham.getTenSP()
             << " | So Luong: " << spL.soLuong << " tan"
             << " | HSD: ";
        spL.hsd.printDate();
        cout << " | Gia tri: " << spL.giaTriSP << " VND" << endl;
    }
    cout << "======================================\n";
}