#include <iostream>
#include "NhaCungCap.h"
#include "SaoLuu.h"
#include <string>
#include <vector>
using namespace std;

NhaCungCap::NhaCungCap() {
    maNCC = "";
    tenNCC = "";
    thoiGianCho = 0.0f;
    chiPhiDat = 0.0f;
    Tinh = "";      // << THÊM KHỞI TẠO >>
    QuanHuyen = ""; // << THÊM KHỞI TẠO >>
    DiaChi = "";    // << THÊM KHỞI TẠO >>
    SDT = "";
    dsMaSanPham.clear();
}
NhaCungCap::NhaCungCap(std::string maNCC, std::string tenNCC, float thoiGianCho, float chiPhiDat,
    std::string Tinh, std::string QuanHuyen, std::string DiaChi, std::string SDT) {
    this->maNCC = maNCC;
    this->tenNCC = tenNCC;
    this->thoiGianCho = thoiGianCho;
    this->chiPhiDat = chiPhiDat;
    this->Tinh = Tinh;
    this->QuanHuyen = QuanHuyen;
    this->DiaChi = DiaChi;
    this->SDT = SDT;
    this->dsMaSanPham.clear();
}
void NhaCungCap::setMaNCC(std::string maNCC) { this->maNCC = maNCC; }
void NhaCungCap::setTenNCC(std::string tenNCC) { this->tenNCC = tenNCC; }
void NhaCungCap::setThoiGianCho(float thoiGianCho) { this->thoiGianCho = thoiGianCho; }
void NhaCungCap::setChiPhiDat(float chiPhiDat) { this->chiPhiDat = chiPhiDat; }
void NhaCungCap::setTinh(std::string Tinh) { this->Tinh = Tinh; }
void NhaCungCap::setQuanHuyen(std::string QuanHuyen) { this->QuanHuyen = QuanHuyen;}
void NhaCungCap::setDiaChi(std::string DiaChi) { this->DiaChi = DiaChi;}
void NhaCungCap::setSDT(std::string SDT) { this->SDT = SDT;}

std::string NhaCungCap::getTinh() const { return Tinh;} 
std::string NhaCungCap::getQuanHuyen() const { return QuanHuyen;}
std::string NhaCungCap::getDiaChi() const { return DiaChi;}
std::string NhaCungCap::getSDT() const {return SDT;}
std::string NhaCungCap::getMaNCC() const { return maNCC; }
std::string NhaCungCap::getTenNCC() const { return tenNCC; }
float NhaCungCap::getThoiGianCho() const{ return thoiGianCho; }
float NhaCungCap::getChiPhiDat() const { return chiPhiDat; }
void NhaCungCap::themMaSanPham(std::string maSP) {
    this->dsMaSanPham.push_back(maSP);
}
void NhaCungCap::setDsMaSanPham(const std::vector<std::string>& ds) {
    this->dsMaSanPham = ds;
}
std::vector<std::string> NhaCungCap::getDsMaSanPham() const {
    return this->dsMaSanPham;
}
// ----------------------------------

static bool isNumber(const std::string& str) {
    for (char const& c : str) {
        if (std::isdigit(c) == 0) return false;
    }
    return true;
}


// --- HÀM NhapInfo ĐÃ SỬA ĐỔI ---
void NhaCungCap::NhapInfo(SQLHDBC hDbc) { // Nhận hDbc
    cout << "Nhap ten nha cung cap: ";
    getline(cin, tenNCC);
    cout << "Nhap thoi gian cho (ngay): ";
    cin >> thoiGianCho;
    cout << "Nhap chi phi dat (VND): ";
    cin >> chiPhiDat;
    cin.ignore();
    cout << "Nhap Tinh ( Thanh Pho): ";
    getline(cin, Tinh);
    cout << "Nhap Quan( Huyen: ";
    getline(cin, QuanHuyen);
    cout << "Nhap dia chi: "; 
    getline(cin, DiaChi);
    while (true) {
        cout << "Nhap So Dien Thoai (10 so): ";
        getline(cin, SDT);
        if (SDT.length() == 10 && isNumber(SDT)) {
            // Kiểm tra thêm: sdt bắt đầu bằng số 0
            if (SDT[0] == '0') {
                break; // Dữ liệu đúng, thoát vòng lặp
            } else {
                cout << "--> Loi: SDT phai bat dau bang so 0. Vui long nhap lai!\n";
            }
        } else {
            cout << "--> Loi: SDT phai gom 10 chu so va khong chua chu cai. Vui long nhap lai!\n";
        }
    }
    // Xóa logic cũ (tạo ncc cục bộ)
    // NhaCungCap ncc(maNCC, tenNCC, thoiGianCho, chiPhiDat);
    
    // Tự động lưu đối tượng hiện tại (*this) vào CSDL
    // Hàm này sẽ sinh maNCC và cập nhật lại cho *this
    GhifileNhaCungCap(hDbc, *this); 
}
// --- HÀM printInfo (Không đổi, đã đúng) ---
void NhaCungCap::printInfo() const {
    cout << "Ma NCC: " << maNCC << endl;
    cout << "Ten NCC: " << tenNCC << endl;      
    cout << "Thoi gian cho (ngay): " << thoiGianCho << endl;
    cout << "Chi phi dat (VND): " << chiPhiDat << endl;
    cout << "Tinh( Thanh Pho): " << Tinh << endl;
    cout << "Quan( Huyen): " << QuanHuyen << endl;
    cout << "So Dien Thoai Lien He: " << SDT  << endl;
}