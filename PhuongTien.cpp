#include "PhuongTien.h"
#include "SaoLuu.h"
#include <limits> 
#include <cmath>
#include <iostream> 
#include <iomanip>  

using namespace std;

// --- Constructor (Giữ nguyên) ---
PhuongTien::PhuongTien() {
    this->maXe = "";
    this->loaiXe = "";
    this->soXe = 1;
    this->sucChuaTan = 0.0f;
    this->chiPhiMoiChuyen = 0.0f;
    this->tinh = "";
    this->quanHuyen = "";
    this->diaChi = "";
    this->sdt = "";
    this->phuThemKhacTinh = 0.0f;
    this->phuThemKhacQuanHuyen = 0.0f;
    this->vanTocTB = 45.0f;
    this->gioCongToiDaNgay = 8.0f;
    this->Lrate = 0.35f; // Mặc định theo ví dụ [cite: 636]
    this->Urate = 0.3f;
    this->Tsetup = 0.5f;
}

PhuongTien::PhuongTien(std::string ma, std::string loai, int soLuongXe, float sucChua, float chiPhi,
    std::string t, std::string quan, std::string dc, std::string s,
    float phuTinh, float phuQuan, float vanToc, float gioCong,
    float lrate, float urate, float tsetup) {
    this->maXe = ma;
    this->loaiXe = loai;
    this->soXe = soLuongXe;
    this->sucChuaTan = sucChua;
    this->chiPhiMoiChuyen = chiPhi;
    this->tinh = t;
    this->quanHuyen = quan;
    this->diaChi = dc;
    this->sdt = s;
    this->phuThemKhacTinh = phuTinh;
    this->phuThemKhacQuanHuyen = phuQuan;

    // Gán các biến mới
    this->vanTocTB = vanToc;
    this->gioCongToiDaNgay = gioCong;
    this->Lrate = lrate;
    this->Urate = urate;
    this->Tsetup = tsetup;
}

void PhuongTien::NhapInfo(SQLHDBC hDbc) {
    cout << "Nhap loai xe (VD: Xe Tai, Xe Van...): ";
    getline(cin, loaiXe);
    cout << "Nhap so luong xe hien co: ";
    cin >> soXe;
    cout << "Nhap suc chua (tan): ";
    cin >> sucChuaTan;
    cout << "Nhap chi phi co ban moi chuyen: ";
    cin >> chiPhiMoiChuyen;
    cin.ignore(); 

    cout << "Nhap Tinh/Thanh pho: "; getline(cin, tinh);
    cout << "Nhap Quan/Huyen: "; getline(cin, quanHuyen);
    cout << "Nhap Dia chi chi tiet (Bai xe): "; getline(cin, diaChi);
    cout << "Nhap So dien thoai lien he: "; getline(cin, sdt);

    cout << "--- Thong tin phu phi van chuyen ---\n";
    cout << "Phu phi khi giao khac Tinh: "; cin >> phuThemKhacTinh;
    cout << "Phu phi khi giao khac Quan/Huyen: "; cin >> phuThemKhacQuanHuyen;
    cout << "Van toc trung binh cua xe (km/h): "; cin >> vanTocTB;
    cout << "Gio cong toi da/ngay (gio): "; cin >> gioCongToiDaNgay;
    cout << "Nhap Lrate (Gio boc hang / tan): "; cin >> Lrate;
    cout << "Nhap Urate (Gio do hang / tan): "; cin >> Urate;
    cout << "Nhap Tsetup (Thoi gian giay to, lui xe / chuyen): "; cin >> Tsetup;
    cin.ignore();
    
    GhifilePhuongTien(hDbc, *this);
}

void PhuongTien::HienThi() const {
    cout << "-------------------------------------------\n";
    cout << "Ma xe: " << maXe << " | Loai: " << loaiXe << " | So luong: " << soXe << endl;
    cout << "Tai trong: " << sucChuaTan << " tan | Chi phi goc: " << (long long)chiPhiMoiChuyen << " VND" << endl;
    cout << "Dia chi: " << diaChi << ", " << quanHuyen << ", " << tinh << endl;
    cout << "SDT: " << sdt << endl;
    cout << "Phu phi (Khac Tinh/Khac Quan): " << (long long)phuThemKhacTinh << " / " << (long long)phuThemKhacQuanHuyen << " VND" << endl;
    cout << "Van toc TB: " << vanTocTB << " km/h | Gio cong TB/Ngay: " << gioCongToiDaNgay << " gio" << endl;
    cout << "-------------------------------------------\n";
}

float TinhKhoangCach(std::string tinhDi, std::string quanDi, std::string tinhDen, std::string quanDen) {
    if (tinhDi == tinhDen && quanDi == quanDen) return 10.0f; // Nội thành
    if (tinhDi == tinhDen) return 30.0f;                      // Khác huyện
    return 150.0f;                                            // Khác tỉnh
}

PhuongTien::KetQuaTinhToan PhuongTien::TinhChiPhiVaThoiGian(float tongKhoiLuongHang,
    std::string tinhDi, std::string quanDi,
    std::string tinhDen, std::string quanDen) const {
    KetQuaTinhToan kq;
    kq.tongChiPhi = std::numeric_limits<float>::max();
    kq.tongThoiGian = std::numeric_limits<float>::max();

    if (this->sucChuaTan <= 0 || this->soXe <= 0) {
        kq.ghiChu = "Loi: Xe khong kha dung";
        return kq;
    }

    kq.soChuyen = (int)std::ceil(tongKhoiLuongHang / this->sucChuaTan);

    kq.khoangCachUocTinh = TinhKhoangCach(tinhDi, quanDi, tinhDen, quanDen);

    // Xác định C' (Chi phí phụ thêm/tấn) dựa trên địa lý [cite: 824]
    float C_prime = 0.0f; // Chi phí biến đổi (C'ijv)
    if (tinhDi != tinhDen) {
        C_prime = this->phuThemKhacTinh;
        kq.ghiChu = "Khac Tinh";
    }
    else if (quanDi != quanDen) {
        C_prime = this->phuThemKhacQuanHuyen;
        kq.ghiChu = "Khac Huyen";
    }
    else {
        C_prime = 0; // Nội thành không phụ thu
        kq.ghiChu = "Noi Thanh";
    }

    // Xác định C (Chi phí cố định/chuyến) [cite: 526]
    // Giả sử phí mở máy + xăng dầu theo quãng đường là chi phí cố định cho 1 chuyến
    float phiXangDau = kq.khoangCachUocTinh * 20000.0f; // 20k/km
    float C_fixed = this->chiPhiMoiChuyen + phiXangDau;

    // --- 3. TÍNH TỔNG CHI PHÍ (TotalCost) ---
    // Công thức PBL2: TotalCost = (N * Cijv) + (Q * C'ijv)
    kq.chiPhiCoDinh = kq.soChuyen * C_fixed;          // (N * Cijv)
    kq.chiPhiPhuThem = tongKhoiLuongHang * C_prime;   // (Q * C'ijv)
    kq.tongChiPhi = kq.chiPhiCoDinh + kq.chiPhiPhuThem;


    float vanToc = (this->vanTocTB > 0) ? this->vanTocTB : 45.0f;
    float T_travel = kq.khoangCachUocTinh / vanToc;
    float T_boc = this->Lrate * this->sucChuaTan;
    float T_do = this->Urate * this->sucChuaTan;
    float H_mot_chuyen = this->Tsetup + T_boc + T_travel + T_do;
    float tongGioCong = H_mot_chuyen * kq.soChuyen;
    kq.tongThoiGian = tongGioCong / this->soXe;

    kq.thoiGianDiChuyen = T_travel * kq.soChuyen;
    kq.thoiGianBocDo = (T_boc + T_do) * kq.soChuyen;

    return kq;
}