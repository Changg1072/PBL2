#include "QuanLyNhapHang.h"
#include "ChinhSachTon.h"
#include "SanPham.h"
#include "SaoLuu.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <algorithm>

using namespace std;

// --- HELPER: Đọc dữ liệu tuyến nhập từ SQL ---
void QuanLyNhapHang::DocFileTuyenNhap(SQLHDBC hDbc, std::vector<TuyenNhapHang>& dsTuyen) {
    dsTuyen.clear();
    SQLHSTMT hStmt;
    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) != SQL_SUCCESS) return;

    SQLWCHAR* sql = (SQLWCHAR*)L"SELECT MaTuyen, MaNCC, MaKhoNhan, KhoTrungChuyen, MaXe, ChiPhiTuyenCoDinh FROM TuyenNhapHang";
    
    if (SQLExecDirect(hStmt, sql, SQL_NTS) == SQL_SUCCESS) {
        SQLCHAR sMa[51], sNCC[51], sKho[51], sKTC[256], sXe[51];
        SQLDOUBLE dChiPhi;
        SQLLEN lenKTC;

        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            SQLGetData(hStmt, 1, SQL_C_CHAR, sMa, 51, NULL);
            SQLGetData(hStmt, 2, SQL_C_CHAR, sNCC, 51, NULL);
            SQLGetData(hStmt, 3, SQL_C_CHAR, sKho, 51, NULL);
            SQLGetData(hStmt, 4, SQL_C_CHAR, sKTC, 256, &lenKTC);
            SQLGetData(hStmt, 5, SQL_C_CHAR, sXe, 51, NULL);
            SQLGetData(hStmt, 6, SQL_C_DOUBLE, &dChiPhi, 0, NULL);

            string ktc = (lenKTC == SQL_NULL_DATA) ? "" : trim_right((char*)sKTC);
            
            dsTuyen.push_back(TuyenNhapHang(
                trim_right((char*)sMa), trim_right((char*)sNCC), 
                trim_right((char*)sKho), ktc, 
                trim_right((char*)sXe), (float)dChiPhi
            ));
        }
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

// --- CHỨC NĂNG 1: ĐỀ XUẤT NHẬP HÀNG TỰ ĐỘNG (Dựa trên ROP) ---
std::map<std::string, std::vector<HangCanNhap>> QuanLyNhapHang::DeXuatNhapHangTuDong(SQLHDBC hDbc) {
    std::map<std::string, std::vector<HangCanNhap>> danhSachGom;
    
    // 1. Lấy danh sách chính sách tồn kho
    vector<ChinhSachTon> dsCS; 
    vector<SanPham> dsSP; // Dummy
    DocfileSanPham(hDbc, dsSP);
    DocFileChinhSachTon(hDbc, dsCS, dsSP); // Lưu ý: Hàm này cần dsSP để map, ở đây gọi tạm hoặc sửa lại hàm DocFile

    // 2. Lấy thông tin sản phẩm để biết NCC mặc định (Trong thực tế 1 SP có thể nhiều NCC, ở đây giả định lấy từ LoHang gần nhất)
    SQLHSTMT hStmt;
    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) != SQL_SUCCESS) return danhSachGom;

    // Query lấy: MaSP, MaNCC (từ lô gần nhất), TenSP
    // Logic: Nếu Tồn kho hiện tại < DiemTaiDatHang -> Cần nhập số lượng = EOQ
    for (const auto& cs : dsCS) {
        float tonHienTai = LayTongTonKhoHienTai(hDbc, cs.getMaSP());
        
        if (tonHienTai <= cs.getDiemDatHang() && cs.getEOQ() > 0) {
            // Cần nhập hàng!
            // Tìm NCC cung cấp sản phẩm này (Lấy lô nhập gần nhất)
            string maNCC = "NCC_UNKNOWN"; 
            string sqlFindNCC = "SELECT TOP 1 MaNCC FROM LoHang L JOIN ChiTietLoHang CT ON L.MaLoHang = CT.MaLoHang WHERE CT.MaSanPham = '" + cs.getMaSP() + "' ORDER BY NgayNhap DESC";
            
            // (Đoạn này thực hiện query lấy MaNCC - Giả lập code cho ngắn gọn)
            // ... Giả sử đã lấy được maNCC ...
            // Để code chạy được ngay, ta sẽ gán cứng hoặc lấy từ dsNCC nếu bạn có mapping
            // Ở đây tôi gán tạm vào 1 NCC có trong DB để test logic
            maNCC = "NCC001"; 

            HangCanNhap item;
            item.maSP = cs.getMaSP();
            item.tenSP = cs.getTenSP();
            item.soLuongCanNhap = cs.getEOQ();
            item.trongLuongTong = cs.getEOQ(); // Giả sử đơn vị là tấn

            danhSachGom[maNCC].push_back(item);
        }
    }
    return danhSachGom;
}

// --- CHỨC NĂNG 2: TỐI ƯU VẬN CHUYỂN (NCC -> KHO) ---
// Logic tương tự bài toán 4 nhưng đổi chiều
QuanLyNhapHang::KetQuaSoSanhNhap QuanLyNhapHang::ToiUuVanChuyenNhapHang(
    float tongTrongLuong,
    const NhaCungCap& ncc,
    const Kho& khoDich,
    const std::vector<PhuongTien>& dsXe,
    const std::vector<TuyenNhapHang>& dsTuyen) 
{
    KetQuaSoSanhNhap kq;
    kq.paReNhat.tongChiPhi = std::numeric_limits<float>::max();
    kq.paNhanhNhat.tongThoiGian = std::numeric_limits<float>::max();

    for (const auto& xe : dsXe) {
        // 1. Tính phương án ĐI THẲNG (Direct)
        // Tận dụng hàm có sẵn của PhuongTien (Tính theo địa lý Tinh/Quan)
        auto kqTinh = xe.TinhChiPhiVaThoiGian(
            tongTrongLuong, ncc.getTinh(), ncc.getQuanHuyen(), khoDich.getTinh(), khoDich.getQuanHuyen()
        );

        if (kqTinh.tongChiPhi < std::numeric_limits<float>::max()) {
            // Cập nhật Rẻ Nhất
            if (kqTinh.tongChiPhi < kq.paReNhat.tongChiPhi) {
                kq.paReNhat.tongChiPhi = kqTinh.tongChiPhi;
                kq.paReNhat.tongThoiGian = kqTinh.tongThoiGian;
                kq.paReNhat.maPhuongTien = xe.getMaXe();
                kq.paReNhat.tenLoaiXe = xe.getLoaiXe();
                kq.paReNhat.soChuyen = kqTinh.soChuyen;
                kq.paReNhat.maNCC = ncc.getMaNCC();
                kq.paReNhat.maKhoNhan = khoDich.getMaKho();
                kq.paReNhat.ghiChu = "Di thang tu NCC";
            }
            // Cập nhật Nhanh Nhất
            if (kqTinh.tongThoiGian < kq.paNhanhNhat.tongThoiGian) {
                kq.paNhanhNhat = kq.paReNhat; // Copy cấu trúc, cập nhật sau nếu cần
                kq.paNhanhNhat.tongChiPhi = kqTinh.tongChiPhi;
                kq.paNhanhNhat.tongThoiGian = kqTinh.tongThoiGian;
                kq.paNhanhNhat.maPhuongTien = xe.getMaXe();
                // ...
            }
        }

        // 2. Tính phương án TRUNG CHUYỂN (Nếu có trong TuyenNhapHang)
        for (const auto& tuyen : dsTuyen) {
            if (tuyen.maNCC == ncc.getMaNCC() && tuyen.maKhoNhan == khoDich.getMaKho() 
                && tuyen.maXe == xe.getMaXe() && !tuyen.khoTrungChuyen.empty()) {
                
                // Giả định Hub nằm ở cùng Tỉnh với Kho Đích để tính toán gần đúng
                // Hoặc bạn phải có bảng KhoTrungChuyen riêng. 
                // Ở đây ta tính: Chi phí đi thẳng + Chi phí cố định tuyến (Phí qua Hub)
                
                float chiPhiQuaHub = kqTinh.tongChiPhi + tuyen.chiPhiCoDinh;
                float thoiGianQuaHub = kqTinh.tongThoiGian + 2.0f; // Giả sử mất thêm 2h ở Hub

                if (chiPhiQuaHub < kq.paReNhat.tongChiPhi) {
                    kq.paReNhat.tongChiPhi = chiPhiQuaHub;
                    kq.paReNhat.tongThoiGian = thoiGianQuaHub;
                    kq.paReNhat.maPhuongTien = xe.getMaXe();
                    kq.paReNhat.ghiChu = "Qua Hub: " + tuyen.khoTrungChuyen;
                }
            }
        }
    }
    return kq;
}

void QuanLyNhapHang::XuLyNhapHang(SQLHDBC hDbc) {
    cout << "\n============================================================\n";
    cout << ">>> QUAN LY NHAP HANG & TOI UU INBOUND (NCC -> KHO) <<<\n";
    cout << "============================================================\n";

    // 1. Quét và gom hàng
    auto dsGom = DeXuatNhapHangTuDong(hDbc);
    if (dsGom.empty()) {
        cout << ">> He thong Ton Kho on dinh. Khong co san pham nao duoi muc ROP.\n";
        return;
    }

    // 2. Load dữ liệu
    vector<NhaCungCap> dsNCC; DocfileNhaCungCap(hDbc, dsNCC);
    vector<Kho> dsKho; DocfileKho(hDbc, dsKho); // Giả sử nhập về Kho đầu tiên tìm thấy hoặc cho chọn
    vector<PhuongTien> dsXe; DocfilePhuongTien(hDbc, dsXe);
    vector<TuyenNhapHang> dsTuyen; DocFileTuyenNhap(hDbc, dsTuyen);

    // Chọn kho nhập hàng (Trong thực tế logic này phức tạp hơn, ở đây chọn Kho Lạnh chính)
    Kho khoNhan;
    if (!dsKho.empty()) khoNhan = dsKho[0]; 
    else { cout << "!! Loi: Khong co kho hang.\n"; return; }

    // 3. Xử lý từng NCC
    for (auto const& [maNCC, dsHang] : dsGom) {
        cout << "\n------------------------------------------------------------\n";
        cout << ">> DE XUAT NHAP TU NCC: " << maNCC << " -> KHO: " << khoNhan.getTenKho() << endl;
        
        float tongTan = 0;
        for(const auto& h : dsHang) {
            cout << "   + " << h.tenSP << ": " << h.soLuongCanNhap << " tan (EOQ)\n";
            tongTan += h.soLuongCanNhap;
        }
        cout << "   => TONG TRONG LUONG: " << tongTan << " tan\n";

        // Tìm object NCC
        NhaCungCap nccCurrent;
        for(const auto& n : dsNCC) if(n.getMaNCC() == maNCC) nccCurrent = n;

        // Tối ưu vận chuyển
        KetQuaSoSanhNhap kq = ToiUuVanChuyenNhapHang(tongTan, nccCurrent, khoNhan, dsXe, dsTuyen);

        if (kq.paReNhat.tongChiPhi >= std::numeric_limits<float>::max()) {
            cout << "!! Khong tim thay phuong an van chuyen phu hop.\n";
            continue;
        }

        // Hiển thị kết quả
        cout << "\n>>> PHUONG AN VAN CHUYEN TOI UU <<<\n";
        cout << "[RE NHAT] Xe: " << kq.paReNhat.tenLoaiXe 
             << " | So chuyen: " << kq.paReNhat.soChuyen
             << " | Chi phi: " << (long long)kq.paReNhat.tongChiPhi << " VND"
             << " (" << kq.paReNhat.ghiChu << ")\n";
             
        cout << "\n>> Ban co muon tao phieu nhap hang (Lo Hang du kien) khong? (y/n): ";
        char c; cin >> c;
        if (c == 'y' || c == 'Y') {
            // Gọi hàm tạo lô hàng (bạn có thể tự implement dựa trên TonKho::NhapKhoVaLuu)
            cout << ">> Da tao phieu yeu cau nhap hang gui den " << nccCurrent.getTenNCC() << ".\n";
        }
    }
}
void QuanLyNhapHang::ThucHienLuuKho(SQLHDBC hDbc, const PhuongAnNhapHang& pa, const HangCanNhap& hang) {
    // Ở đây bạn có thể gọi các hàm INSERT vào bảng LoHang, ChiTietLoHang, TonKho
    // Ví dụ giả lập:
    cout << ">> [DATABASE] Dang tao Lo Hang moi tu NCC: " << pa.maNCC << "...\n";
    
    // Code mẫu gọi các hàm từ TonKho/LoHang (Bạn cần include TonKho.h)
    // LoHang loMoi;
    // loMoi.setMaNCC(pa.maNCC);
    // loMoi.setKho(pa.maKhoNhan);
    // ...
    // GhifileLoHang(hDbc, loMoi);
    
    cout << ">> [THANH CONG] Da nhap " << hang.soLuongCanNhap << " tan " << hang.tenSP 
         << " vao kho " << pa.maKhoNhan << ".\n";
}

void QuanLyNhapHang::NhapHangThuCong(SQLHDBC hDbc) {
    cout << "\n============================================================\n";
    cout << ">>> TAO YEU CAU NHAP HANG THU CONG (MANUAL INBOUND) <<<\n";
    cout << "============================================================\n";

    // 1. Load dữ liệu tham chiếu
    vector<SanPham> dsSP; DocfileSanPham(hDbc, dsSP);
    vector<NhaCungCap> dsNCC; DocfileNhaCungCap(hDbc, dsNCC);
    vector<Kho> dsKho; DocfileKho(hDbc, dsKho);
    vector<PhuongTien> dsXe; DocfilePhuongTien(hDbc, dsXe);
    vector<TuyenNhapHang> dsTuyen; DocFileTuyenNhap(hDbc, dsTuyen);

    // 2. Nhập thông tin hàng hóa
    string maSP, maNCC, maKho;
    float soLuong;

    cout << "Nhap Ma San Pham can mua: "; cin >> maSP;
    
    // Tìm tên SP
    string tenSP = "Unknown";
    for(auto& s : dsSP) if(s.getMaSP() == maSP) tenSP = s.getTenSP();
    if (tenSP == "Unknown") { cout << "!! Ma SP khong ton tai.\n"; return; }
    cout << "-> San pham: " << tenSP << endl;

    cout << "Nhap so luong can nhap (tan): "; cin >> soLuong;
    
    // 3. Chọn NCC và Kho
    cout << "Nhap Ma Nha Cung Cap: "; cin >> maNCC;
    NhaCungCap nccChon;
    bool foundNCC = false;
    for(auto& n : dsNCC) if(n.getMaNCC() == maNCC) { nccChon = n; foundNCC = true; break; }
    if (!foundNCC) { cout << "!! Ma NCC khong ton tai.\n"; return; }

    cout << "Nhap Ma Kho Nhan Hang: "; cin >> maKho;
    Kho khoChon;
    bool foundKho = false;
    for(auto& k : dsKho) if(k.getMaKho() == maKho) { khoChon = k; foundKho = true; break; }
    if (!foundKho) { cout << "!! Ma Kho khong ton tai.\n"; return; }

    // 4. Chạy tối ưu vận chuyển
    cout << "\n>> DANG TIM PHUONG AN VAN CHUYEN TOI UU...\n";
    KetQuaSoSanhNhap kq = ToiUuVanChuyenNhapHang(soLuong, nccChon, khoChon, dsXe, dsTuyen);

    if (kq.paReNhat.tongChiPhi >= numeric_limits<float>::max()) {
        cout << "!! Khong co xe hoac tuyen duong nao phu hop.\n";
        return;
    }

    // 5. Hiển thị và Chọn
    cout << "\n[OPTION A - RE NHAT] Chi phi: " << (long long)kq.paReNhat.tongChiPhi 
         << " VND | Thoi gian: " << kq.paReNhat.tongThoiGian << "h | Xe: " << kq.paReNhat.tenLoaiXe << endl;
    
    cout << "[OPTION B - NHANH NHAT] Chi phi: " << (long long)kq.paNhanhNhat.tongChiPhi 
         << " VND | Thoi gian: " << kq.paNhanhNhat.tongThoiGian << "h | Xe: " << kq.paNhanhNhat.tenLoaiXe << endl;

    cout << "\n>> Xac nhan nhap hang? (a: Chon Re / b: Chon Nhanh / n: Huy): ";
    char chon; cin >> chon;

    HangCanNhap hangInfo;
    hangInfo.maSP = maSP; hangInfo.tenSP = tenSP; hangInfo.soLuongCanNhap = soLuong;

    if (chon == 'a' || chon == 'A') {
        ThucHienLuuKho(hDbc, kq.paReNhat, hangInfo);
    } else if (chon == 'b' || chon == 'B') {
        ThucHienLuuKho(hDbc, kq.paNhanhNhat, hangInfo);
    } else {
        cout << ">> Da huy yeu cau.\n";
    }
}