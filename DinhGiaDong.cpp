#include "DinhGiaDong.h"
#include "SaoLuu.h" 
#include "Date.h"
#include <iostream>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <limits> 
#include <random>
#include <Windows.h>
#include <sqlext.h> // Thư viện ODBC chuẩn

using namespace std;

// --- HÀM HỖ TRỢ SINH MÃ TẠI CHỖ ---
string SinhMaGia_Local() {
    const string charset = "0123456789";
    string soNgauNhien;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, charset.size() - 1);
    for (int i = 0; i < 6; ++i) soNgauNhien += charset[dis(gen)];
    return "GIA" + soNgauNhien;
}

// =============================================================================
// [QUAN TRỌNG] HÀM LẤY LỊCH SỬ TỪ CSDL ĐỂ HUẤN LUYỆN
// =============================================================================
vector<LichSuBanHang> LayLichSuBanHangTuDB(SQLHDBC hDbc, string maSP) {
    vector<LichSuBanHang> history;
    SQLHSTMT hStmt;

    // Lấy SoLuong (Q) và DonGia (P = GiaTri / SoLuong) từ bảng ChiTietYeuCau
    // Bảng này phản ánh nhu cầu thực tế của Siêu Thị
    string sql = "SELECT SoTanYeuCau, (GiaTri / NULLIF(SoTanYeuCau, 0)) as DonGia FROM ChiTietYeuCau WHERE MaSanPham = ? AND SoTanYeuCau > 0";

    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
        SQLPrepareA(hStmt, (SQLCHAR*)sql.c_str(), SQL_NTS);
        SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)maSP.c_str(), 0, NULL);

        if (SQLExecute(hStmt) == SQL_SUCCESS) {
            float sl, dg;
            SQLLEN lenSL, lenDG;
            while (SQLFetch(hStmt) == SQL_SUCCESS) {
                SQLGetData(hStmt, 1, SQL_C_FLOAT, &sl, 0, &lenSL);
                SQLGetData(hStmt, 2, SQL_C_FLOAT, &dg, 0, &lenDG);

                // Chỉ lấy dữ liệu hợp lệ (Không NULL)
                if (lenSL != SQL_NULL_DATA && lenDG != SQL_NULL_DATA) {
                    LichSuBanHang item;
                    item.luongBan = sl;
                    item.giaBan = dg;
                    history.push_back(item);
                }
            }
        }
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    }
    return history;
}

// =============================================================================
//                           CONSTRUCTOR & SETUP
// =============================================================================

DinhGiaDong::DinhGiaDong() : SanPham() {
    this->heSo_a = 0.0f;
    this->heSo_b = 0.0f;
    this->C_penalty_ngay = 0.0f;
    this->chiPhiLuuKhoThucTe = 30000.0f;
}

DinhGiaDong::DinhGiaDong(const SanPham& sp) : SanPham(sp) {
    this->heSo_a = 0.0f;
    this->heSo_b = 0.0f;
    this->C_penalty_ngay = 0.0f;
    this->chiPhiLuuKhoThucTe = 30000.0f;
}

void DinhGiaDong::setChiPhiLuuKhoThucTe(float cp) {
    if (cp > 0) {
        this->chiPhiLuuKhoThucTe = cp;
        this->C_penalty_ngay = 0.0f;
    }
}

// =============================================================================
//                        CÁC HÀM TÍNH TOÁN CỐT LÕI
// =============================================================================

float DinhGiaDong::tinhGiaBanCoBan(float tyLeLoiNhuanMucTieu) const {
    if (getGiaBan() > 1000.0f) return getGiaBan();
    const int SO_NGAY_QUAY_VONG = 30;
    float giaVon = getGiaVon();
    float chiPhiLuuKhoUocTinh = this->chiPhiLuuKhoThucTe * SO_NGAY_QUAY_VONG;
    float giaHoaVon = giaVon + chiPhiLuuKhoUocTinh;
    if (tyLeLoiNhuanMucTieu >= 1.0f) tyLeLoiNhuanMucTieu = 0.99f;
    return giaHoaVon / (1.0f - tyLeLoiNhuanMucTieu);
}

void DinhGiaDong::tinhCPenaltyNgay() {
    if ((getNhomPhanLoai() == "C" || getNhomPhanLoai() == "c") && this->C_penalty_ngay == 0.0f) {
        const float LAI_SUAT_AN_TOAN = 0.05f;
        const float CHI_PHI_CO_HOI_MAT_BANG_NGAY = 4000000.0f;
        float giaVon = getGiaVon();
        float chiPhiCoHoiVon_Ngay = giaVon * (LAI_SUAT_AN_TOAN / 365.0f);
        this->C_penalty_ngay = this->chiPhiLuuKhoThucTe + chiPhiCoHoiVon_Ngay + CHI_PHI_CO_HOI_MAT_BANG_NGAY;
    }
}

// --- Hồi quy & Dự báo ---
void DinhGiaDong::tinhHoiQuyTuyenTinh(const vector<LichSuBanHang>& history) {
    float sum_x = 0.0f, sum_y = 0.0f, sum_xy = 0.0f, sum_x2 = 0.0f;
    int n = history.size();

    // Nếu ít dữ liệu quá thì không tính được
    if (n < 2) {
        this->heSo_a = 0;
        this->heSo_b = 0;
        return;
    }

    for (const auto& data : history) {
        float x = data.giaBan;
        float y = data.luongBan;
        sum_x += x; sum_y += y; sum_xy += x * y; sum_x2 += x * x;
    }

    float tuSo_a = (n * sum_xy) - (sum_x * sum_y);
    float mauSo_a = (n * sum_x2) - (sum_x * sum_x);

    if (abs(mauSo_a) < 0.0001f) {
        this->heSo_a = 0;
    }
    else {
        this->heSo_a = tuSo_a / mauSo_a;
    }

    float mean_x = sum_x / n;
    float mean_y = sum_y / n;
    this->heSo_b = mean_y - (this->heSo_a * mean_x);

    // [FIX] Logic kinh tế: Nếu giá tăng mà cầu tăng (a > 0) -> Dữ liệu bị nhiễu
    // Ép về hệ số âm nhẹ để thuật toán không bị sai
    if (this->heSo_a > 0) {
        this->heSo_a = -0.01f;
        this->heSo_b = mean_y; // Cầu trung bình
    }
}

float DinhGiaDong::duBaoNhuCau(float giaBan) const {
    // Nếu chưa được huấn luyện (a=0, b=0), trả về 0
    if (this->heSo_b == 0 && this->heSo_a == 0) return 0.0f;

    float demand = (this->heSo_a * giaBan) + this->heSo_b;
    return max(0.0f, demand);
}

// =============================================================================
// CÁC HÀM HỖ TRỢ KHÁC
// =============================================================================

vector<LichSuBanHang> DinhGiaDong::XayDungLichSuBanHang(const vector<ChiTietYeuCau>& dsYC, const std::string& maSPCanHuanLuyen) {
    vector<LichSuBanHang> history;
    for (const auto& ctyc : dsYC) {
        for (const auto& spyc : ctyc.getdsYeucau()) {
            if (spyc.sp != nullptr && spyc.sp->getMaSP() == maSPCanHuanLuyen && spyc.soLuongTan > 0) {
                LichSuBanHang dataPoint;
                dataPoint.luongBan = spyc.soLuongTan;
                if (spyc.soLuongTan > 0.0001f) {
                    dataPoint.giaBan = spyc.ThanhTienRieng / spyc.soLuongTan;
                    history.push_back(dataPoint);
                }
            }
        }
    }
    return history;
}

void DinhGiaDong::HuanLuyenMoHinh(const vector<LichSuBanHang>& history) {
    tinhHoiQuyTuyenTinh(history);
}

// =============================================================================
//                        CHIẾN LƯỢC ĐỊNH GIÁ
// =============================================================================

DinhGiaDong::KetQuaDinhGia DinhGiaDong::DeXuatGia_HangSapHetHan(SQLHDBC hDbc, float St_tong, float q1_sapHong, const vector<float>& cacMucGia) {
    float v = getGiaVon();
    float C_scrap = v; // Giá trị phế liệu coi như bằng vốn (hoặc 0 nếu vứt bỏ)
    vector<float> mucGiaDeXuat = cacMucGia;

    KetQuaDinhGia deXuatTotNhat;
    deXuatTotNhat.loiNhuanKyVong = -numeric_limits<float>::infinity();
    deXuatTotNhat.giaBanDeXuat = 0; // Giá mặc định nếu thuật toán fail
    deXuatTotNhat.luongBanDuKien = 0;

    // Duyệt qua từng mức giá để tìm lợi nhuận tối ưu
    for (float p : mucGiaDeXuat) {
        float nhuCau = duBaoNhuCau(p);
        float luongBan = min(nhuCau, St_tong);

        // Doanh thu từ bán hàng
        float R = luongBan * (p - v);

        // Hàng ế sẽ bị hỏng (Mất trắng vốn)
        float q1_khongBanDuoc = max(0.0f, q1_sapHong - luongBan);
        float V_future = -(q1_khongBanDuoc * C_scrap);

        float V = R + V_future;

        if (V > deXuatTotNhat.loiNhuanKyVong) {
            deXuatTotNhat.loiNhuanKyVong = V;
            deXuatTotNhat.giaBanDeXuat = p;
            deXuatTotNhat.luongBanDuKien = luongBan;
        }
    }

    // Nếu thuật toán không tìm được giá tốt (vd: lịch sử trống), dùng giá cắt lỗ 50%
    if (deXuatTotNhat.giaBanDeXuat == 0) deXuatTotNhat.giaBanDeXuat = v * 0.5f;

    deXuatTotNhat.maGia = SinhMaGia_Local();
    deXuatTotNhat.maSP = getMaSP();
    deXuatTotNhat.ngayTinh = Date::Today();
    deXuatTotNhat.ghiChu = "Giai cuu hang het han.";

    try { GhiFileDinhGia(hDbc, deXuatTotNhat); }
    catch (...) {}
    return deXuatTotNhat;
}

DinhGiaDong::KetQuaDinhGia DinhGiaDong::DeXuatGia_HangBanCham(SQLHDBC hDbc, float St_tong, const vector<float>& cacMucGia) {
    KetQuaDinhGia deXuatTotNhat;
    deXuatTotNhat.maGia = SinhMaGia_Local();
    deXuatTotNhat.maSP = getMaSP();
    deXuatTotNhat.ngayTinh = Date::Today();

    vector<float> mucGiaDeXuat = cacMucGia;
    deXuatTotNhat.loiNhuanKyVong = -numeric_limits<float>::infinity();

    tinhCPenaltyNgay();
    float v = getGiaVon();

    for (float p : mucGiaDeXuat) {
        float nhuCau = duBaoNhuCau(p);
        float luongBan = min(nhuCau, St_tong);

        float R = luongBan * (p - v);

        // Phạt tồn kho cho hàng bán chậm
        float St_conLai = St_tong - luongBan;
        float V_future = -(St_conLai * this->C_penalty_ngay);

        float V = R + V_future;

        if (V > deXuatTotNhat.loiNhuanKyVong) {
            deXuatTotNhat.loiNhuanKyVong = V;
            deXuatTotNhat.giaBanDeXuat = p;
            deXuatTotNhat.luongBanDuKien = luongBan;
        }
    }

    if (deXuatTotNhat.giaBanDeXuat == 0) deXuatTotNhat.giaBanDeXuat = v * 1.05f; // Mặc định lãi 5%

    deXuatTotNhat.ghiChu = "Xa hang (Nhom C).";
    try { GhiFileDinhGia(hDbc, deXuatTotNhat); }
    catch (...) {}
    return deXuatTotNhat;
}

// =============================================================================
// CHIẾN LƯỢC 3: HÀNG BÌNH THƯỜNG -> INSERT TRỰC TIẾP
// =============================================================================
void DinhGiaDong::GhiNhanGiaNiemYet(SQLHDBC hDbc) {
    string maGia = SinhMaGia_Local();
    string maSP = getMaSP();
    float p0 = tinhGiaBanCoBan(0.20f);
    if (p0 == 0) p0 = getGiaVon() * 1.2f; // Fallback

    Date now = Date::Today();
    char sNgayTinh[20];
    sprintf_s(sNgayTinh, 20, "%04d-%02d-%02d", now.getYear(), now.getMonth(), now.getDay());
    wstring ghiChu = L"Hang thong thuong. Gia niem yet.";

    SQLHSTMT hStmt;
    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
        SQLWCHAR* sqlQuery = (SQLWCHAR*)L"INSERT INTO DinhGiaDong (MaGia, MaSanPham, GiaBanDeXuat, LuongBanDuKien, LoiNhuanKyVong, NgayTinh, GhiChu) VALUES (?, ?, ?, ?, ?, ?, ?)";
        if (SQLPrepare(hStmt, sqlQuery, SQL_NTS) == SQL_SUCCESS) {
            SQLCHAR sMaGia[51]; strcpy_s((char*)sMaGia, 51, maGia.c_str());
            SQLCHAR sMaSP_Bind[51]; strcpy_s((char*)sMaSP_Bind, 51, maSP.c_str());
            SQLDOUBLE dP0 = p0;
            SQLDOUBLE dL = 0;
            SQLDOUBLE dLN = 0;
            SQLWCHAR sGC[200]; wcscpy_s(sGC, 200, ghiChu.c_str());

            SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMaGia, 0, NULL);
            SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMaSP_Bind, 0, NULL);
            SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dP0, 0, NULL);
            SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dL, 0, NULL);
            SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dLN, 0, NULL);
            SQLBindParameter(hStmt, 6, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 20, 0, sNgayTinh, 0, NULL);
            SQLBindParameter(hStmt, 7, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 200, 0, sGC, 0, NULL);
            SQLExecute(hStmt);
        }
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    }
}

float DinhGiaDong::LayGiaBanHienTai(SQLHDBC hDbc, const std::string& maSP) {
    float giaBan = 0.0f;
    SQLHSTMT hStmt;
    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
        string sql = "SELECT TOP 1 GiaBanDeXuat FROM DinhGiaDong WHERE MaSanPham = ? ORDER BY NgayTinh DESC";
        SQLPrepareA(hStmt, (SQLCHAR*)sql.c_str(), SQL_NTS);
        SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)maSP.c_str(), 0, NULL);
        if (SQLExecute(hStmt) == SQL_SUCCESS) {
            if (SQLFetch(hStmt) == SQL_SUCCESS) SQLGetData(hStmt, 1, SQL_C_FLOAT, &giaBan, 0, NULL);
        }
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    }
    if (giaBan <= 0.0001f) {
        if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
            string sqlSP = "SELECT GiaBan FROM SanPham WHERE MaSanPham = ?";
            SQLPrepareA(hStmt, (SQLCHAR*)sqlSP.c_str(), SQL_NTS);
            SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)maSP.c_str(), 0, NULL);
            if (SQLExecute(hStmt) == SQL_SUCCESS) {
                if (SQLFetch(hStmt) == SQL_SUCCESS) SQLGetData(hStmt, 1, SQL_C_FLOAT, &giaBan, 0, NULL);
            }
            SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        }
    }
    return giaBan;
}

// =============================================================================
// HÀM CHẠY TỰ ĐỘNG KHI LOGIN (ĐÃ SỬA LỖI LOGIC)
// =============================================================================
void DinhGiaDong::TuDongChayDinhGiaHomNay(SQLHDBC hDbc) {
    SQLHSTMT hStmt;
    bool daCoDuLieu = false;

    // 1. Kiểm tra hôm nay đã chạy chưa
    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
        string sql = "SELECT TOP 1 MaGia FROM DinhGiaDong WHERE CAST(NgayTinh AS DATE) = CAST(GETDATE() AS DATE)";
        SQLPrepareA(hStmt, (SQLCHAR*)sql.c_str(), SQL_NTS);
        if (SQLExecute(hStmt) == SQL_SUCCESS) {
            if (SQLFetch(hStmt) == SQL_SUCCESS) daCoDuLieu = true;
        }
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    }

    if (daCoDuLieu) return; // Đã chạy rồi thì thôi

    // 2. Load sản phẩm
    vector<SanPham> dsSP;
    DocfileSanPham(hDbc, dsSP); // Hàm này có trong SaoLuu.cpp (đã include SaoLuu.h)

    vector<float> cacMucGiaTest;

    // 3. Chạy vòng lặp định giá
    for (auto& sp : dsSP) {
        DinhGiaDong algo = DinhGiaDong(sp);

        // --- [SỬA LỖI] BƯỚC HUẤN LUYỆN MODEL ---
        // Lấy lịch sử bán từ DB
        vector<LichSuBanHang> lichSu = LayLichSuBanHangTuDB(hDbc, sp.getMaSP());

        // Chỉ huấn luyện nếu có dữ liệu
        bool duDuLieu = false;
        if (lichSu.size() >= 2) {
            algo.HuanLuyenMoHinh(lichSu);
            duDuLieu = true;
        }
        // ---------------------------------------

        // Lấy thông tin kho (Hàm từ SaoLuu.cpp)
        float St_tong = LayTongTonKhoHienTai(hDbc, sp.getMaSP());
        float q1_sapHong = LayTonKhoSapHetHan(hDbc, sp.getMaSP(), 7); // 7 ngày
        string nhom = sp.getNhomPhanLoai();

        cacMucGiaTest.clear();
        float giaVon = sp.getGiaVon();

        if (giaVon > 0) {
            cacMucGiaTest.push_back(giaVon * 1.05f);
            cacMucGiaTest.push_back(giaVon * 1.10f);
            cacMucGiaTest.push_back(giaVon * 1.15f);
            cacMucGiaTest.push_back(giaVon * 1.20f);
            cacMucGiaTest.push_back(giaVon * 0.90f);
        }

        // --- PHÂN LUỒNG CHIẾN LƯỢC ---
        // Chỉ chạy thuật toán khi CÓ HÀNG và ĐÃ HUẤN LUYỆN
        if (St_tong > 0 && duDuLieu) {
            if (q1_sapHong > 0) {
                // Thêm giá cắt lỗ sâu
                cacMucGiaTest.push_back(giaVon * 0.5f);
                cacMucGiaTest.push_back(giaVon * 0.7f);
                algo.DeXuatGia_HangSapHetHan(hDbc, St_tong, q1_sapHong, cacMucGiaTest);
            }
            else if ((nhom == "C" || nhom == "c")) {
                algo.DeXuatGia_HangBanCham(hDbc, St_tong, cacMucGiaTest);
            }
            else {
                algo.GhiNhanGiaNiemYet(hDbc);
            }
        }
        else {
            // SP mới hoặc hết hàng -> Dùng giá niêm yết
            algo.GhiNhanGiaNiemYet(hDbc);
        }
    }
}