#include "TuyenVanChuyen.h"
#include <limits> // Để dùng cin.ignore()

TuyenVanChuyen::TuyenVanChuyen() {
    maTuyen = "";
    khoDi = "";
    khoTrungChuyen = ""; 
    sieuThiNhan = "";
    maXe = "";
    chiPhiCoDinh = 0.0f;
}

TuyenVanChuyen::TuyenVanChuyen(std::string ma, std::string kDi, std::string kTC, std::string stNhan, std::string xe,float chiPhi) {
    maTuyen = ma;
    khoDi = kDi;
    khoTrungChuyen = kTC;
    sieuThiNhan = stNhan;
    maXe = xe;
    chiPhiCoDinh = chiPhi;
}

void TuyenVanChuyen::NhapInfo() {
    // Mã tuyến sẽ được sinh tự động trong SaoLuu.cpp, không cần nhập
    // Hoặc nếu muốn nhập tay:
    // cout << "Nhap Ma Tuyen: "; getline(cin, maTuyen);

    cout << "Nhap Ma Kho Di: ";
    getline(cin, khoDi);

    cout << "Tuyen nay co Kho Trung Chuyen khong? (1: Co, 0: Khong): ";
    int chon;
    cin >> chon;
    cin.ignore();

    if (chon == 1) {
        cout << "Nhap Ma Kho Trung Chuyen: ";
        getline(cin, khoTrungChuyen);
    } else {
        khoTrungChuyen = ""; // Chuỗi rỗng biểu thị NULL
    }

    cout << "Nhap Ma Sieu Thi Nhan: ";
    getline(cin, sieuThiNhan);

    cout << "Nhap Ma Xe van chuyen: ";
    getline(cin, maXe);
    cout << "Nhap Chi phi tuyen co dinh (VND): ";
    cin >> chiPhiCoDinh;
    cin.ignore();
}

void TuyenVanChuyen::HienThi() const {
    cout << "-----------------------------------" << endl;
    cout << "Ma Tuyen: " << maTuyen << endl;
    cout << "Lo trinh: " << khoDi;
    
    if (!khoTrungChuyen.empty()) {
        cout << " -> " << khoTrungChuyen << " (Trung chuyen)";
    } else {
        cout << " -> (Truc tiep)";
    }
    
    cout << " -> " << sieuThiNhan << endl;
    cout << "Phuong tien: " << maXe << endl;
    cout << "Chi phi co dinh: " << (size_t)chiPhiCoDinh << " VND" << endl;
    cout << "-----------------------------------" << endl;
}