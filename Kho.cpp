#include <iostream>
#include <string>
#include <vector>
#include "Kho.h"
#include "SaoLuu.h" 
using namespace std;

Kho::Kho() {
    maKho = "";
    tenKho = "";
    sucChua = 0.0f;
    NdoMin = 0.0f;
    NdoMax = 0.0f;
    diaChi = "";
    Tinh = "";      // << THÊM KHỞI TẠO >>
    quanHuyen = ""; // << THÊM KHỞI TẠO >>
    sdt = "";
    congSuatCoBan = 500.0f; heSoCongSuat = 20.0f;
    chiPhiHaNhiet = 5000.0f; chiPhiTangNhiet = 0.0f;
}
Kho::Kho(std::string maKho, std::string tenKho, float sucChua, float NdoMin, float NdoMax, 
         std::string diaChi, std::string Tinh, std::string quanHuyen, std::string sdt,
         float congSuat, float heSo, float cpHa, float cpTang) {
    this->maKho = maKho; this->tenKho = tenKho; this->sucChua = sucChua;
    this->NdoMin = NdoMin; this->NdoMax = NdoMax;
    this->diaChi = diaChi; this->Tinh = Tinh; this->quanHuyen = quanHuyen; this->sdt = sdt;
    // [MỚI]
    this->congSuatCoBan = congSuat;
    this->heSoCongSuat = heSo;
    this->chiPhiHaNhiet = cpHa;
    this->chiPhiTangNhiet = cpTang;
}
void Kho::setMaKho(std::string maKho) { this->maKho = maKho; }
void Kho::setTenKho(std::string tenKho) { this->tenKho = tenKho; }
void Kho::setSucChua(float sucChua) { this->sucChua = sucChua; }
void Kho::setNdoMin(float NdoMin) { this->NdoMin = NdoMin; }
void Kho::setNdoMax(float NdoMax) { this->NdoMax = NdoMax; }
void Kho::setDiaChi(std::string diaChi) { this->diaChi = diaChi; }
void Kho::setTinh(std::string Tinh) { this->Tinh = Tinh; }
void Kho::setQuanHuyen(std::string quanHuyen) { this->quanHuyen = quanHuyen; }
void Kho::setSDT(std::string sdt) { this->sdt = sdt;}
void Kho::setCongSuatCoBan(float cs) { this->congSuatCoBan = cs;}
void Kho::setHeSoCongSuat(float hs) { this->heSoCongSuat = hs;}
void Kho::setChiPhiHaNhiet(float cp) { this->chiPhiHaNhiet = cp;}
void Kho::setChiPhiTangNhiet(float cp) { this->chiPhiTangNhiet = cp;}
std::string Kho::getTinh() const { return Tinh; }
std::string Kho::getQuanHuyen() const { return quanHuyen; }
std::string Kho::getSDT() const { return sdt; }
std::string Kho::getMaKho() const { return maKho; }
std::string Kho::getTenKho() const { return tenKho; }
float Kho::getSucChua() const { return sucChua; }
float Kho::getNdoMin() const { return NdoMin; }
float Kho::getNdoMax()  const { return NdoMax; }
std::string Kho::getDiaChi() const { return diaChi; }
float Kho::getCongSuatCoBan() const { return congSuatCoBan;}
float Kho::getHeSoCongSuat() const { return heSoCongSuat;}
float Kho::getChiPhiHaNhiet() const { return chiPhiHaNhiet;}
float Kho::getChiPhiTangNhiet() const { return chiPhiTangNhiet;}
// --- KẾT THÚC KHÔNG ĐỔI ---

static bool isNumber(const std::string& str) {
    for (char const &c : str) {
        if (std::isdigit(c) == 0) return false;
    }
    return true;
}

// --- HÀM NhapInfo ĐÃ SỬA ĐỔI ---
void Kho::NhapInfo(SQLHDBC hDbc) { // Nhận hDbc
    cout << "Nhap ten kho: ";
    getline(cin, tenKho);
    cout << "Nhap suc chua (tan): ";
    cin >> sucChua;
    cout << "Nhap nhiet do toi thieu (do C): "; // <-- Sửa prompt
    cin >> NdoMin;
    cout << "Nhap nhiet do toi da (do C): "; // <-- Sửa prompt
    cin >> NdoMax;
    cout << "--- CAU HINH MAY LANH ---\n";
    cout << "Nhap Cong suat co ban (kW): "; cin >> congSuatCoBan;
    cout << "Nhap He so giam tai (kW/do): "; cin >> heSoCongSuat;
    cout << "Nhap Chi phi ha nhiet (VND/do): "; cin >> chiPhiHaNhiet;
    cin.ignore();
    cout << "Nhap tinh ( Thanh Pho ): ";
    getline(cin,Tinh);
    cout << "Nhap quan ( Huyen ): ";
    getline(cin,quanHuyen);
    cout << "Nhap dia chi kho: ";
    getline(cin, diaChi);
    while (true) {
        cout << "Nhap So Dien Thoai (10 so): ";
        getline(cin, sdt);
        if (sdt.length() == 10 && isNumber(sdt)) {
            // Kiểm tra thêm: sdt bắt đầu bằng số 0
            if (sdt[0] == '0') {
                break; // Dữ liệu đúng, thoát vòng lặp
            } else {
                cout << "--> Loi: SDT phai bat dau bang so 0. Vui long nhap lai!\n";
            }
        } else {
            cout << "--> Loi: SDT phai gom 10 chu so va khong chua chu cai. Vui long nhap lai!\n";
        }
    }
    // Tự động gọi hàm lưu vào CSDL
    // Hàm này sẽ sinh MaKho và cập nhật lại cho *this
    GhifileKho(hDbc, *this); 
}

// --- HÀM printInfo ĐÃ SỬA ĐỔI ---
void Kho::printInfo() const {
    cout << "Ma Kho: " << maKho << endl; // <-- Thêm MaKho
    cout << "Ten Kho: " << tenKho << endl;                  
    cout << "Suc Chua (tan): " << sucChua << endl;
    cout << "Nhiet Do Kho Toi Thieu (do C): " << NdoMin << endl; // <-- Sửa prompt
    cout << "Nhiet Do Kho Toi Da (do C): " << NdoMax << endl; // <-- Sửa prompt
    cout << "Cau hinh dien: P=" << congSuatCoBan << " - " << heSoCongSuat << "*T" << endl;
    cout << "Tinh(Thanh Pho): " << Tinh << endl;
    cout << "Quan(Huyen): " << quanHuyen << endl;
    cout << "Dia Chi Kho: " << diaChi << endl;
}