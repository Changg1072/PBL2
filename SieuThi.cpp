#include <iostream>
#include <string>
#include <random>
#include <sstream>   
#include <cctype>
#include "SieuThi.h"
#include "SanPham.h"
#include <vector>
#include <iomanip>
#include <ctime>
#include "ChiTietYeuCau.h"
#include "SaoLuu.h"
using namespace std;
// (Constructor, Setters, Getters không đổi)
SieuThi::SieuThi() {
    maSieuThi = "";
    tenSieuThi = "";
    diaChi = "";
    mucPhatThieu = 0.0f;
    Tinh = "";      // << THÊM KHỞI TẠO >>
    QuanHuyen = ""; // << THÊM KHỞI TẠO >>
    SDT = "";
}
SieuThi::SieuThi(std::string maSieuThi, std::string tenSieuThi, float MucPhatThieu,std::string Tinh, std::string QuanHuyen,std::string diaChi, std::string SDT) {
    setMaSieuThi(maSieuThi);
    setTenSieuThi(tenSieuThi);
    setDiaChi(diaChi);
    setMucPhatThieu(MucPhatThieu);
    setTinh(Tinh);
    setQuanHuyen(QuanHuyen);
    setSDT(SDT);    
}
void SieuThi::setMaSieuThi(std::string maSieuThi) { this->maSieuThi = maSieuThi; }
void SieuThi::setTenSieuThi(std::string tenSieuThi) { this->tenSieuThi = tenSieuThi; }
void SieuThi::setDiaChi(std::string diaChi) { this->diaChi = diaChi; }
void SieuThi::setMucPhatThieu(float mucPhatThieu) { this->mucPhatThieu = mucPhatThieu; }
void SieuThi::setTinh(std::string Tinh) { this->Tinh = Tinh;}
void SieuThi::setQuanHuyen(std::string QuanHuyen) { this->QuanHuyen = QuanHuyen;}
void SieuThi::setSDT(std::string SDT) { this->SDT = SDT; }
std::string SieuThi::getMaSieuThi() const { return maSieuThi; }
std::string SieuThi::getTenSieuThi() const { return tenSieuThi; }
std::string SieuThi::getDiaChi() const { return diaChi; }
float SieuThi::getMucPhatThieu() const { return mucPhatThieu; }
std::string SieuThi::getTinh() const { return Tinh; }
std::string SieuThi::getQuanHuyen() const { return QuanHuyen; }
std::string SieuThi::getSDT() const { return SDT; }
// --- KẾT THÚC KHÔNG ĐỔI ---

bool isNumber(const std::string& str) {
    for (char const &c : str) {
        if (std::isdigit(c) == 0) return false;
    }
    return true;
}

// --- HÀM NhapInfo ĐÃ SỬA ĐỔI ---
void SieuThi::NhapInfo(SQLHDBC hDbc) { // Nhận hDbc
    std::vector<SieuThi> ds;
    DocfileSieuThi(hDbc, ds); // <-- Truyền hDbc
    cout << "Nhap ten sieu thi: ";
    getline(cin, tenSieuThi);
    cout << "Nhap Tinh: ";
    getline(cin,Tinh);
    cout << "Nhap Quan( Huyen): ";
    getline(cin, QuanHuyen);
    cout << "Nhap dia chi sieu thi: ";
    getline(cin, diaChi);
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
    cout << "Nhap muc phat thieu (VND): ";
    cin >> mucPhatThieu;
    cin.ignore();
    GhifileSieuThi(hDbc, *this); // <-- Truyền hDbc
    // Mã của *this* sẽ được cập nhật bên trong GhifileSieuThi
}

void SieuThi::printInfo() const {
    // (Không đổi)
    cout << "Ma Sieu Thi: " << maSieuThi << endl; // <-- Thêm mã cho đầy đủ
    cout << "Ten Sieu Thi: " << tenSieuThi << endl;
    cout << "Tinh: " << Tinh << endl;
    cout << "Quan ( Huyen): " << QuanHuyen << endl; 
    cout << "Dia Chi: " << diaChi << endl;
    cout << "So dien thoai lien he: " << SDT<< endl;
    cout << "Muc Phat Thieu: " << mucPhatThieu << " VND" << endl;
}

// --- HÀM checkDB_DaTonTai ĐÃ SỬA ĐỔI ---
bool SieuThi::checkDB_DaTonTai(SQLHDBC hDbc, std::string maKiemTra){ // Nhận hDbc
    std::vector<ChiTietYeuCau> ds;
    std::vector<SanPham> sp; 
    // Cần phải nạp dsSP trước, dù không dùng, vì DocFileYeuCau cần
    DocfileSanPham(hDbc, sp); 
    DocFileYeuCau(hDbc, ds, sp); // <-- Truyền hDbc
    
    bool check = false;
    for(int i = 0; i<ds.size();i++){
        if(maKiemTra == ds[i].getMaYeuCau()){
            check = true;
            break;
        }
    }
    return check;
}

// --- HÀM SinhMaYeuCau ĐÃ SỬA ĐỔI ---
std::string SieuThi::SinhMaYeuCau(SQLHDBC hDbc, std::string maSieuThi, std::string tenSieuThi){ // Nhận hDbc
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, 99999);
    std::string finalCode = "";
    bool isDuplicate = true;
    while (isDuplicate) {
        int randomNum = dis(gen); 
        std::stringstream ss;
        ss << std::setw(5) << std::setfill('0') << randomNum;
        std::string randomPart = ss.str();
        finalCode = maSieuThi + "YC" +randomPart;
        
        if(checkDB_DaTonTai(hDbc, finalCode)){ // <-- Truyền hDbc
            continue;
        } else {
            isDuplicate = false;
        }
    }
    return finalCode;
}

// --- HÀM YeuCauTuSieuThi ĐÃ SỬA ĐỔI ---
std::vector<SanPhamYeuCau> SieuThi::YeuCauTuSieuThi(SQLHDBC hDbc, std::vector<SanPham>& ds) const { 
    DocfileSanPham(hDbc, ds); 
    std::vector<SanPhamYeuCau> spYeuCau;
    
    cout << "Nhap so luong san pham yeu cau: ";
    int n;
    cin >> n;
    cin.ignore();

    // <--- 1. Khởi tạo biến tổng tiền
    float tongGiaTriDonHang = 0.0f; 

    for (int i = 0; i < n; ++i) {
        std::string maSP;
        cout << "Nhap ma san pham thu " << i + 1 << ": ";
        getline(cin, maSP);
        bool found = false;
        for (auto& sp : ds) {
            if (sp.getMaSP() == maSP) {
                float soLuong;
                cout << "So luong tan yeu cau cho san pham " << sp.getTenSP() << ": ";
                cin >> soLuong;
                cin.ignore();
                
                float giaRieng = soLuong * sp.getGiaVon();
                spYeuCau.push_back({ &sp, soLuong, giaRieng});
                
                // <--- 2. Cộng dồn vào tổng tiền
                tongGiaTriDonHang += giaRieng; 

                found = true;
                break;
            }
        }
        if (!found) {
            cout << "Khong tim thay san pham voi ma: " << maSP << endl;
        }
    }

    cout << "\nDanh sach san pham yeu cau cua sieu thi:\n";
    
    // Lưu ý: SinhMaYeuCau nên trả về string, đảm bảo hàm này hoạt động đúng
    std::string maYeuCau = const_cast<SieuThi*>(this)->SinhMaYeuCau(hDbc, this->maSieuThi, this->tenSieuThi);
    
    for (auto& yc : spYeuCau) {
        cout << "- " << yc.sp->getTenSP()
             << " (" << yc.sp->getMaSP() << ") : "
             << yc.soLuongTan << " tan | Gia: "
             << (size_t)yc.ThanhTienRieng << " VND\n";
    }

    // Tạo đối tượng ChiTietYeuCau
    ChiTietYeuCau ctyc(maYeuCau, spYeuCau);

    // <--- 3. QUAN TRỌNG: Gán giá trị tổng vào đối tượng trước khi ghi file
    ctyc.setGiaTri(tongGiaTriDonHang); 

    // Xử lý ngày tháng
    time_t now = time(0);
    tm *ltm = localtime(&now);
    // Lưu ý: localtime trả về con trỏ static, không cần delete, nhưng warning bảo mật Visual Studio có thể yêu cầu localtime_s
    
    int ngayHienTai = ltm->tm_mday;
    int thangHienTai = 1 + ltm->tm_mon;  
    int namHienTai = 1900 + ltm->tm_year; 
    Date ngayNhap(ngayHienTai, thangHienTai, namHienTai);
    
    ctyc.setNgayNhap(ngayNhap);
    
    // Ghi xuống Database
    GhiFileYeuCau(hDbc, ctyc, this->getMaSieuThi()); 
    
    return spYeuCau;
}