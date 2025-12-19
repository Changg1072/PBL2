#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <limits>
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <clocale>
#include "QuanLyBaicon1.h"

// Include các header của bạn
#include "SanPham.h"
#include "Kho.h"
#include "NhaCungCap.h"
#include "LoHang.h"
#include "SieuThi.h"
#include "ChiTietYeuCau.h"
#include "DinhGiaDong.h"
#include "ChinhSachTon.h"
#include "Date.h"
#include "DuBaoNhuCau.h"
#include "SaoLuu.h"

using namespace std;

// --- BIẾN TOÀN CỤC (LƯU DỮ LIỆU RAM) ---
vector<SanPham> dsSP;
vector<NhaCungCap> dsNCC;
vector<Kho> dsKho;
vector<SieuThi> dsST;
vector<LoHang> dsLoHang;
vector<ChiTietYeuCau> dsYC;
// Các đối tượng xử lý logic
vector<ChinhSachTon> dsChinhSachTon;
vector<DinhGiaDong> dsDinhGiaDong;

// --- HÀM TIỆN ÍCH ---

void TaiDuLieuTuDB(SQLHDBC hDbc) {
    cout << "\n--- DANG TAI DU LIEU TU SQL SERVER ---\n";
    DocfileSanPham(hDbc, dsSP);
    DocfileNhaCungCap(hDbc, dsNCC);
    DocfileKho(hDbc, dsKho);
    DocfileSieuThi(hDbc, dsST);
    DocfileLoHang(hDbc, dsLoHang, dsSP, dsKho, dsNCC);
    DocFileYeuCau(hDbc, dsYC, dsSP);
    
    cout << ">> Da tai: " << dsSP.size() << " SP | " << dsNCC.size() << " NCC | " << dsYC.size() << " Don Yeu Cau.\n";
    
    // Khởi tạo đối tượng phân tích tương ứng với từng sản phẩm
    dsChinhSachTon.clear();
    dsDinhGiaDong.clear();
    for (const auto& sp : dsSP) {
        dsChinhSachTon.push_back(ChinhSachTon(sp));
        dsDinhGiaDong.push_back(DinhGiaDong(sp));
    }
    // Load dữ liệu cũ của chính sách tồn (nếu có)
    DocFileChinhSachTon(hDbc, dsChinhSachTon, dsSP);
    cout << ">> Da khoi tao cac doi tuong phan tich.\n";
}

// =================================================================================
//                           CÁC CHỨC NĂNG CHÍNH
// =================================================================================

// 1. PHÂN TÍCH ABC
void ChucNang_PhanTichABC(SQLHDBC hDbc) {
    cout << "\n==================================================\n";
    cout << "   [LOP 1] PHAN TICH NHOM ABC (PARETO 80/20)      \n";
    cout << "==================================================\n";
    
    if (dsSP.empty()) { cout << "!! Khong co du lieu San Pham.\n"; return; }
    if (dsYC.empty()) { cout << "!! Khong co du lieu Yeu Cau (Lich su ban hang) de phan tich.\n"; return; }

    // Gọi hàm static của SanPham để chạy phân tích
    try {
        SanPham spTemp; // Tạo đối tượng tạm
        spTemp.ThucHienPhanTichNhom(hDbc, dsSP);
        cout << ">> Phan tich va cap nhat CSDL hoan tat.\n";
        
        // Hiển thị kết quả sơ bộ
        int a = 0, b = 0, c = 0;
        for(const auto& sp : dsSP) {
            if (sp.getNhomPhanLoai() == "A") a++;
            else if (sp.getNhomPhanLoai() == "B") b++;
            else c++;
        }
        cout << ">> Ket qua: Nhom A (" << a << ") | Nhom B (" << b << ") | Nhom C (" << c << ")\n";
        
        // Reload lại các đối tượng phụ thuộc vì nhóm phân loại đã đổi
        dsChinhSachTon.clear();
        dsDinhGiaDong.clear();
        for (const auto& sp : dsSP) {
            dsChinhSachTon.push_back(ChinhSachTon(sp));
            dsDinhGiaDong.push_back(DinhGiaDong(sp));
        }

    } catch (exception& e) {
        cout << "!! Loi: " << e.what() << endl;
    }
}

// 2. CHÍNH SÁCH TỒN KHO (ĐÃ SỬA LOGIC CHỌN NCC)
void ChucNang_PhanTichTonKho(SQLHDBC hDbc) {
    cout << "\n==================================================\n";
    cout << "   [LOP 2] TINH TOAN CHINH SACH TON KHO (EOQ/ROP) \n";
    cout << "==================================================\n";

    if (dsNCC.empty()) { cout << "!! Can it nhat 1 Nha Cung Cap de tinh toan.\n"; return; }

    // --- BƯỚC 1: HIỂN THỊ & CHỌN NHÀ CUNG CẤP ---
    cout << "Danh sach Nha Cung Cap hien co:\n";
    cout << left << setw(10) << "Ma NCC" << setw(30) << "Ten NCC" 
         << setw(15) << "TG Cho (L)" << "Chi Phi Dat (Co)\n";
    cout << string(70, '-') << endl;
    
    for (const auto& ncc : dsNCC) {
        cout << left << setw(10) << ncc.getMaNCC() 
             << setw(30) << ncc.getTenNCC()
             << setw(15) << ncc.getThoiGianCho()
             << (long long)ncc.getChiPhiDat() << " VND" << endl;
    }
    cout << string(70, '-') << endl;

    string maNCCChon;
    cout << ">> Nhap MA NCC ban muon dung de tinh toan (Nhap 'all' de dung NCC dau tien): ";
    cin >> maNCCChon;
    cin.ignore();

    NhaCungCap nccSelected;
    bool found = false;

    if (maNCCChon == "all") {
        nccSelected = dsNCC[0];
        found = true;
    } else {
        for(const auto& n : dsNCC) {
            if (n.getMaNCC() == maNCCChon) {
                nccSelected = n;
                found = true;
                break;
            }
        }
    }

    if (!found) {
        cout << "!! Ma NCC khong ton tai.\n";
        return;
    }

    cout << "\n>> DANG CHAY PHAN TICH VOI NCC: " << nccSelected.getTenNCC() << "...\n";
    cout << "   - Thoi gian cho (L): " << nccSelected.getThoiGianCho() << " ngay\n";
    cout << "   - Chi phi dat hang (Co): " << (long long)nccSelected.getChiPhiDat() << " VND/don\n";

    // --- BƯỚC 2: CHẠY TÍNH TOÁN CHO TỪNG SẢN PHẨM ---
    cout << "\n" << left << setw(10) << "Ma SP" << setw(20) << "Ten SP" 
         << setw(8) << "Nhom" << setw(10) << "SS(tan)" << setw(10) << "ROP(tan)" << "EOQ(tan)\n";
    
    for (auto& cs : dsChinhSachTon) {
        // Hàm này sẽ tính toán và tự động LƯU vào CSDL
        ChayPhanTichLop2(hDbc, cs, dsYC, nccSelected.getChiPhiDat(), nccSelected.getThoiGianCho());
        
        cout << left << setw(10) << cs.getMaSP() 
             << setw(20) << cs.getTenSP().substr(0, 19)
             << setw(8) << cs.getNhomPhanLoai()
             << setw(10) << setprecision(2) << fixed << cs.getTonAnToan()
             << setw(10) << cs.getDiemDatHang()
             << cs.getEOQ() << endl;
    }
    cout << ">> Da cap nhat toan bo Chinh Sach Ton vao CSDL.\n";
}

// 3. ĐỊNH GIÁ ĐỘNG
void ChucNang_DinhGiaDong(SQLHDBC hDbc) {
    cout << "\n==================================================\n";
    cout << "   [LOP 3] DINH GIA DONG (TOI UU LOI NHUAN)       \n";
    cout << "==================================================\n";
    
    int count = 0;
    for (auto& model : dsDinhGiaDong) {
        string maSP = model.getMaSP();
        float tonKho = LayTongTonKhoHienTai(hDbc, maSP);
        
        // Chỉ xử lý nếu có tồn kho
        if (tonKho > 0) {
            cout << "\n[*] SP: " << model.getTenSP() << " (Ton: " << tonKho << " tan)\n";
            
            // 1. Huấn luyện
            auto history = model.XayDungLichSuBanHang(dsYC, maSP);
            if (history.size() < 2) {
                cout << "   -> Bo qua: Khong du du lieu lich su ban hang.\n";
                continue;
            }
            model.HuanLuyenMoHinh(history);
            
            // 2. Kiểm tra kịch bản
            float sapHetHan = LayTonKhoSapHetHan(hDbc, maSP, 3); // Sắp hỏng trong 3 ngày
            float giaVon = model.getGiaVon();
            vector<float> dsGiaTest = {giaVon * 1.5f, giaVon * 1.2f, giaVon * 1.1f, giaVon * 0.9f, giaVon * 0.8f};
            
            DinhGiaDong::KetQuaDinhGia kq;
            bool coDeXuat = false;

            if (sapHetHan > 0) {
                cout << "   -> Phat hien " << sapHetHan << " tan sap het han -> Chay Kich Ban 1 (Giai cuu)\n";
                kq = model.DeXuatGia_HangSapHetHan(hDbc, tonKho, sapHetHan, dsGiaTest);
                coDeXuat = true;
            } 
            else if (model.getNhomPhanLoai() == "C") {
                cout << "   -> SP thuoc Nhom C (Ban cham) -> Chay Kich Ban 2 (Xa hang)\n";
                kq = model.DeXuatGia_HangBanCham(hDbc, tonKho, dsGiaTest);
                coDeXuat = true;
            } else {
                // --- ĐOẠN SỬA LỖI Ở ĐÂY ---
                cout << "   -> SP Nhom A/B an toan. Luu gia niem yet vao he thong.\n";
                // Vẫn gọi hàm này để nó ghi giá chuẩn vào CSDL
                kq = model.DeXuatGia_HangBanCham(hDbc, tonKho, dsGiaTest); 
                coDeXuat = true; // Bật cờ lên để đếm và in kết quả
            }

            if (coDeXuat) {
                cout << "   => GIA DE XUAT: " << (long long)kq.giaBanDeXuat << " VND (" << kq.ghiChu << ")\n";
                count++;
            }
        }
    }
    cout << "\n>> Hoan tat. Da tao " << count << " de xuat gia moi.\n";
}

// 4. DỰ BÁO NHU CẦU
void ChucNang_DuBao(SQLHDBC hDbc) {
    cout << "\n==================================================\n";
    cout << "   DU BAO NHU CAU (DEMAND FORECASTING)            \n";
    cout << "==================================================\n";
    
    string maSP;
    cout << "Nhap Ma SP can du bao: ";
    cin >> maSP;
    
    bool exist = false;
    for(auto& sp : dsSP) if(sp.getMaSP() == maSP) exist = true;
    if(!exist) { cout << "!! Ma SP khong ton tai.\n"; return; }

    int d1, m1, y1, d2, m2, y2;
    cout << "Nhap khoang thoi gian du lieu lich su:\n";
    cout << "Tu ngay (d m y): "; cin >> d1 >> m1 >> y1;
    cout << "Den ngay (d m y): "; cin >> d2 >> m2 >> y2;

    KetQuaDuBao kq = DuBaoNhuCau(maSP, dsYC, Date(d1, m1, y1), Date(d2, m2, y2));
    
    cout << "\n>> KET QUA DU BAO:\n";
    cout << "   - Nhu cau trung binh (d): " << kq.nhuCauTrungBinh << " tan/ngay\n";
    cout << "   - Do lech chuan (sigma):  " << kq.doLechChuan << "\n";
    
    GhiFileDuBaoNhuCau(hDbc, kq);
    cout << ">> Da luu ket qua vao CSDL.\n";
}

// =================================================================================
//                                  MAIN
// =================================================================================
/*
int main() {
    // Hỗ trợ hiển thị tiếng Việt (tùy môi trường console)
    SetConsoleOutputCP(65001); 

    SQLHENV hEnv = SQL_NULL_HENV;
    SQLHDBC hDbc = SQL_NULL_HDBC;

    connect_db(hEnv, hDbc);
    TaiDuLieuTuDB(hDbc);

    int chon;
    while (true) {
        cout << "\n===================================================\n";
        cout << "    HE THONG QUAN LY KHO THONG MINH (WMS AI)       \n";
        cout << "===================================================\n";
        cout << "1. Tai lai du lieu (Refresh Data)\n";
        cout << "2. Phan tich Nhom ABC (Lop 1)\n";
        cout << "3. Phan tich Chinh Sach Ton Kho (Lop 2) [EOQ/ROP]\n";
        cout << "4. Dinh Gia Dong (Lop 3) [Giai cuu/Xa hang]\n";
        cout << "5. Du bao nhu cau (Forecasting)\n";
        cout << "6. Xem Tong Quan Ton Kho Hien Tai\n";
        cout << "0. Thoat\n";
        cout << "Lua chon: ";
        
        if (!(cin >> chon)) {
            cin.clear(); 
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        cin.ignore();

        switch (chon) {
            case 1: TaiDuLieuTuDB(hDbc); break;
            case 2: ChucNang_PhanTichABC(hDbc); break;
            case 3: ChucNang_PhanTichTonKho(hDbc); break;
            case 4: ChucNang_DinhGiaDong(hDbc); break;
            case 5: ChucNang_DuBao(hDbc); break;
            case 6: 
                cout << "\n--- TON KHO HIEN TAI ---\n";
                for(auto& sp : dsSP) {
                    float ton = LayTongTonKhoHienTai(hDbc, sp.getMaSP());
                    cout << sp.getMaSP() << " - " << sp.getTenSP() << ": " << ton << " tan\n";
                }
                break;
            case 0: 
                cout << "Ket thuc chuong trinh.\n";
                SQLDisconnect(hDbc);
                SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
                SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
                return 0;
            default: cout << "Lua chon khong hop le.\n";
        }
        cout << "\nAn Enter de tiep tuc...";
        cin.get();
        system("cls");
    }

    return 0;
}
*/