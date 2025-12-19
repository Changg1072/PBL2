// SaoLuu.cpp - phiên bản đã sửa để fix SQL DATE binding, Unicode và các bind parameter
// Dựa trên file gốc của bạn (đã chỉnh sửa những chỗ cần thiết).
// Nguồn gốc file: SaoLuu.cpp (user upload). :contentReference[oaicite:1]{index=1}

#include "SaoLuu.h" // Header đã sửa
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <random>
#include <cstdio>
#include <cwchar>    // Cho wcscpy_s
#include <clocale>   // Cho setlocale
#include <map>       // Dùng cho DocfileLoHang
#include <algorithm> // <-- Cần cái này cho std::find_if
#include <cctype>

// ODBC headers (nếu header khác đã include thì không sao, để an toàn thì include ở đây)
#include <sql.h>
#include <sqlext.h>

 // Include các class definitions của bạn
#include "SanPham.h"   
#include "Kho.h" 
#include "NhaCungCap.h"
#include "LoHang.h"
#include "ChiTietYeuCau.h"
#include "SieuThi.h"    
#include "ChinhSachTon.h"
#include "DinhGiaDong.h"
#include "DuBaoNhuCau.h"
#include "PhuongTien.h"
#include "TuyenVanChuyen.h"


// --- ODBC HELPER FUNCTIONS (KHÔNG ĐỔI) ---
void check_error(SQLHANDLE handle, SQLSMALLINT handle_type, SQLRETURN retcode, const std::string& msg) {
    if (!SQL_SUCCEEDED(retcode)) {
        SQLWCHAR sql_state[6];
        SQLINTEGER native_error;
        SQLWCHAR message_text[SQL_MAX_MESSAGE_LENGTH];
        SQLSMALLINT text_length;

        SQLGetDiagRec(handle_type, handle, 1, sql_state, &native_error, message_text, SQL_MAX_MESSAGE_LENGTH, &text_length);
        
        wstring wmsg(message_text);
        std::string error_msg = msg + " - Loi: " + to_string(native_error) + " - " + std::string(wmsg.begin(), wmsg.end());
        throw std::runtime_error(error_msg);
    }
}
std::wstring s2ws(const std::string& s) {
    if (s.empty()) return L"";
    // SỬA: Đổi CP_ACP thành CP_UTF8 để đồng bộ
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.length(), 0, 0);
    std::wstring r(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.length(), &r[0], len);
    return r;
}
std::string ws2s(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    // Đổi CP_ACP thành CP_UTF8
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}
// --------------------------------------------------

// --- CÁC HÀM SINH MÃ (KHÔNG ĐỔI) ---

std::string trim_right(const std::string& source) {
    std::string s(source);
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
    return s;
}

std::string sinhMaGia() {
    const std::string charset = "0123456789"; 
    std::string soNgauNhien;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, charset.size() - 1);
    for (int i = 0; i < 6; ++i) { 
        soNgauNhien += charset[dis(gen)];
    }
    return "GIA" + soNgauNhien; 
}
std::string sinhMaSanPham() {
    const std::string charset = "0123456789";
    std::string soNgauNhien;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, charset.size() - 1);
    for (int i = 0; i < 6; ++i) {
        soNgauNhien += charset[dis(gen)];
    }
    return "SP" + soNgauNhien;
}
std::string sinhMaKho() {
    const std::string charset = "0123456789";
    std::string soNgauNhien;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, charset.size() - 1);
    for (int i = 0; i < 6; ++i) {
        soNgauNhien += charset[dis(gen)];
    }
    return "KHO" + soNgauNhien;
}
std::string sinhMaNhaCungCap() {
    const std::string charset = "0123456789";
    std::string soNgauNhien;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, charset.size() - 1);
    for (int i = 0; i < 6; ++i) {
        soNgauNhien += charset[dis(gen)];
    }
    return "NCC" + soNgauNhien;
}
std::string sinhMaLoHang() {
    const std::string charset = "0123456789";
    std::string soNgauNhien;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, charset.size() - 1);
    for (int i = 0; i < 7; ++i) {
        soNgauNhien += charset[dis(gen)];
    }
    return "LO" + soNgauNhien;
}
std::string sinhMaSieuThi() {
    const std::string charset = "0123456789";
    std::string soNgauNhien;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, charset.size() - 1);
    for (int i = 0; i < 6; ++i) {
        soNgauNhien += charset[dis(gen)];
    }
    return "STH" + soNgauNhien;
}
std::string sinhMaChinhSach() {
    const std::string charset = "0123456789";
    std::string soNgauNhien;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, charset.size() - 1);
    for (int i = 0; i < 6; ++i) {
        soNgauNhien += charset[dis(gen)];
    }
    return "CS" + soNgauNhien;
}
std::string sinhMaTuyen() {
    std::string charset = "0123456789";
    random_device rd; mt19937 gen(rd());
    uniform_int_distribution<> dis(0, charset.size() - 1);
    std::string ma = "TVC";
    for (int i = 0; i < 5; ++i) ma += charset[dis(gen)];
    return ma;
}
std::string sinhMaPhuongTien() {
    const std::string charset = "0123456789";
    std::string soNgauNhien;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, charset.size() - 1);
    for (int i = 0; i < 6; ++i) {
        soNgauNhien += charset[dis(gen)];
    }
    return "XE" + soNgauNhien;
}
// --------------------------------------------------

// --- SAN PHAM ---
void GhifileSanPham(SQLHDBC hDbc, SanPham& sp) { 
    SQLHSTMT hStmt = SQL_NULL_HSTMT;
    SQLRETURN retcode;

    try {
        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
        
        // [MỚI] Thêm cột: GiaBan, RuiRoLanh, RuiRoNong, HanSuDungTieuChuan
        SQLWCHAR* sqlQuery = (SQLWCHAR*)L"INSERT INTO SanPham "
            L"(MaSanPham, TenSanPham, Loai, GiaVon, TheTich1Tan, NhietDoThichHop, NhomPhanLoai, GiaBan, RuiRoLanh, RuiRoNong, HanSuDungTieuChuan) "
            L"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

        retcode = SQLPrepare(hStmt, sqlQuery, SQL_NTS);

        std::string newMaSP = sinhMaSanPham();
        sp.setMaSP(newMaSP);

        // Chuẩn bị dữ liệu bind
        SQLCHAR sMa[51]; strcpy_s((char*)sMa, 51, sp.getMaSP().c_str());
        SQLWCHAR sTen[256]; wcscpy_s(sTen, 256, s2ws(sp.getTenSP()).c_str());
        SQLWCHAR sLoai[101]; wcscpy_s(sLoai, 101, s2ws(sp.getLoai()).c_str());
        SQLWCHAR sNhom[51]; wcscpy_s(sNhom, 51, s2ws(sp.getNhomPhanLoai()).c_str());
        
        SQLDOUBLE dGiaVon = sp.getGiaVon();
        SQLDOUBLE dTheTich = sp.getThetich1tan();
        SQLDOUBLE dTemp = sp.getNdoThichHop();
        
        // [MỚI] Biến cho cột mới
        SQLDOUBLE dGiaBan = sp.getGiaBan();
        SQLDOUBLE dRuiRoL = sp.getRuiRoLanh();
        SQLDOUBLE dRuiRoN = sp.getRuiRoNong();
        SQLINTEGER iHSD = sp.getHanSuDung();

        SQLLEN cb = SQL_NTS;
        SQLLEN cbNum = 0;

        // Bind tham số (1 -> 11)
        SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMa, 0, &cb);
        SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 255, 0, sTen, 0, &cb);
        SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, sLoai, 0, &cb);
        SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dGiaVon, 0, &cbNum);
        SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 10, 2, &dTheTich, 0, &cbNum);
        SQLBindParameter(hStmt, 6, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 5, 2, &dTemp, 0, &cbNum);
        SQLBindParameter(hStmt, 7, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 10, 0, sNhom, 0, &cb);
        
        // [MỚI] Bind 4 cột cuối
        SQLBindParameter(hStmt, 8, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dGiaBan, 0, &cbNum);
        SQLBindParameter(hStmt, 9, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_FLOAT, 0, 0, &dRuiRoL, 0, &cbNum);
        SQLBindParameter(hStmt, 10, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_FLOAT, 0, 0, &dRuiRoN, 0, &cbNum);
        SQLBindParameter(hStmt, 11, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &iHSD, 0, &cbNum);

        retcode = SQLExecute(hStmt);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "GhifileSanPham");

    } catch (const std::exception& e) { cerr << e.what() << endl; }
    if (hStmt) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

void DocfileSanPham(SQLHDBC hDbc, std::vector<SanPham>& spList) {
    SQLHSTMT hStmt = SQL_NULL_HSTMT;
    spList.clear();

    // [MỚI] Select thêm các cột mới
    SQLWCHAR* sql = (SQLWCHAR*)L"SELECT MaSanPham, TenSanPham, Loai, GiaVon, TheTich1Tan, NhietDoThichHop, NhomPhanLoai, RuiRoLanh, RuiRoNong, GiaBan, HanSuDungTieuChuan FROM SanPham";

    if (SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt))) {
        if (SQL_SUCCEEDED(SQLExecDirect(hStmt, sql, SQL_NTS))) {
            // Khai báo biến đệm
            SQLCHAR sMa[51]; SQLWCHAR sTen[256], sLoai[101], sNhom[51];
            SQLDOUBLE dVon, dVol, dTemp, dBan, dRRL, dRRN;
            SQLINTEGER iHSD;
            SQLLEN len;

            while (SQL_SUCCEEDED(SQLFetch(hStmt))) {
                SQLGetData(hStmt, 1, SQL_C_CHAR, sMa, 51, &len);
                SQLGetData(hStmt, 2, SQL_C_WCHAR, sTen, 256, &len);
                SQLGetData(hStmt, 3, SQL_C_WCHAR, sLoai, 101, &len);
                SQLGetData(hStmt, 4, SQL_C_DOUBLE, &dVon, 0, &len);
                SQLGetData(hStmt, 5, SQL_C_DOUBLE, &dVol, 0, &len);
                SQLGetData(hStmt, 6, SQL_C_DOUBLE, &dTemp, 0, &len);
                SQLGetData(hStmt, 7, SQL_C_WCHAR, sNhom, 51, &len);
                
                // [MỚI] Lấy data cột mới
                SQLGetData(hStmt, 8, SQL_C_DOUBLE, &dRRL, 0, &len);
                SQLGetData(hStmt, 9, SQL_C_DOUBLE, &dRRN, 0, &len);
                SQLGetData(hStmt, 10, SQL_C_DOUBLE, &dBan, 0, &len);
                SQLGetData(hStmt, 11, SQL_C_LONG, &iHSD, 0, &len);

                // Xử lý NULL
                float rrL = (float)dRRL; if(len == SQL_NULL_DATA) rrL = 0.01f;
                float rrN = (float)dRRN; if(len == SQL_NULL_DATA) rrN = 0.02f;
                float giaBan = (float)dBan; if(len == SQL_NULL_DATA) giaBan = 0;
                int hsd = (int)iHSD; if(len == SQL_NULL_DATA) hsd = 30;

                std::string ma = trim_right((char*)sMa);
                std::string ten = ws2s(sTen);
                
                // Tạo đối tượng với Constructor mới
                SanPham sp(ma, ten, ws2s(sLoai), (float)dVon, (float)dVol, (float)dTemp, ws2s(sNhom), rrL, rrN, giaBan, hsd);
                spList.push_back(sp);
            }
        }
    }
    if (hStmt) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

void GhiDeFileSanPham(SQLHDBC hDbc, const std::vector<SanPham>& dsSP) {
    SQLHSTMT hStmt;
    SQLRETURN retcode;

    // 1. Cấp phát Statement Handle
    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    if (!SQL_SUCCEEDED(retcode)) {
        std::cerr << "[LOI] Khong the cap phat handle de update SanPham.\n";
        return;
    }

    // 2. Chuẩn bị câu lệnh UPDATE
    // Chúng ta chỉ cần cập nhật NhomPhanLoai dựa trên MaSP
    // Nếu bạn muốn cập nhật thêm các trường khác, hãy thêm vào câu lệnh SQL
    SQLWCHAR* query = (SQLWCHAR*)L"UPDATE SanPham SET NhomPhanLoai = ? WHERE MaSanPham = ?";

    retcode = SQLPrepare(hStmt, query, SQL_NTS);
    if (!SQL_SUCCEEDED(retcode)) {
        std::cerr << "[LOI] SQLPrepare that bai trong GhiDeFileSanPham.\n";
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return;
    }

    // 3. Duyệt qua danh sách và cập nhật từng sản phẩm
    std::cout << "Dang cap nhat phan loai ABC cho " << dsSP.size() << " san pham...\n";
    
    // Biến đệm để bind dữ liệu
    SQLCHAR maSP_param[50];
    SQLCHAR nhomPL_param[10];
    SQLLEN cbMaSP = SQL_NTS, cbNhomPL = SQL_NTS;

    // Bind tham số (Lần lượt là dấu ? thứ nhất và thứ hai)
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 10, 0, nhomPL_param, 0, &cbNhomPL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, maSP_param, 0, &cbMaSP);

    int countSuccess = 0;
    for (const auto& sp : dsSP) {
        // Copy dữ liệu vào biến đệm
        strcpy((char*)maSP_param, sp.getMaSP().c_str());
        strcpy((char*)nhomPL_param, sp.getNhomPhanLoai().c_str());

        // Thực thi
        retcode = SQLExecute(hStmt);
        if (SQL_SUCCEEDED(retcode)) {
            countSuccess++;
        } else {
            std::cerr << "[CANH BAO] Khong the update SP: " << sp.getMaSP() << "\n";
        }
        
        // Reset lại statement để dùng cho vòng lặp sau (cần thiết với một số driver)
        SQLFreeStmt(hStmt, SQL_RESET_PARAMS); // Reset bind nếu cần, nhưng ở đây ta bind 1 lần dùng nhiều lần nên chỉ cần đóng transaction nếu có.
        // Với cách bind trên, ta chỉ cần chạy SQLExecute lại là được, không cần bind lại.
        // Tuy nhiên để an toàn, ta không gọi SQLFreeStmt(RESET) ở đây mà cứ để nguyên bind.
    }

    std::cout << "Da cap nhat thanh cong " << countSuccess << "/" << dsSP.size() << " san pham.\n";

    // 4. Dọn dẹp
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}
// --- KHO ---
void GhifileKho(SQLHDBC hDbc, Kho& k) {
    SQLHSTMT hStmt = SQL_NULL_HSTMT;
    SQLRETURN retcode;

    try {
        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

        // [MỚI] Thêm: CongSuatCoBan, HeSoCongSuat, ChiPhiHaNhiet, ChiPhiTangNhiet
        SQLWCHAR* sqlQuery = (SQLWCHAR*)L"INSERT INTO Kho "
            L"(MaKho, TenKho, SucChua, NhietDoMin, NhietDoMax, Tinh, QuanHuyen, DiaChi, SDT, CongSuatCoBan, HeSoCongSuat, ChiPhiHaNhiet, ChiPhiTangNhiet) "
            L"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
        
        retcode = SQLPrepare(hStmt, sqlQuery, SQL_NTS);

        std::string newMaKho = sinhMaKho();
        k.setMaKho(newMaKho);

        SQLCHAR sMa[51]; strcpy_s((char*)sMa, 51, k.getMaKho().c_str());
        SQLWCHAR sTen[256]; wcscpy_s(sTen, 256, s2ws(k.getTenKho()).c_str());
        SQLWCHAR sTinh[101]; wcscpy_s(sTinh, 101, s2ws(k.getTinh()).c_str());
        SQLWCHAR sQuan[101]; wcscpy_s(sQuan, 101, s2ws(k.getQuanHuyen()).c_str());
        SQLWCHAR sDiaChi[501]; wcscpy_s(sDiaChi, 501, s2ws(k.getDiaChi()).c_str());
        SQLCHAR sSDT[21]; strcpy_s((char*)sSDT, 21, k.getSDT().c_str());

        SQLDOUBLE dSuc = k.getSucChua();
        SQLDOUBLE dMin = k.getNdoMin();
        SQLDOUBLE dMax = k.getNdoMax();

        // [MỚI] Biến cho cột mới
        SQLDOUBLE dCongSuat = k.getCongSuatCoBan();
        SQLDOUBLE dHeSo = k.getHeSoCongSuat();
        SQLDOUBLE dCPHa = k.getChiPhiHaNhiet();
        SQLDOUBLE dCPTang = k.getChiPhiTangNhiet();

        SQLLEN cb = SQL_NTS;
        SQLLEN cbNum = 0;

        // Bind (1 -> 13)
        SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMa, 0, &cb);
        SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 255, 0, sTen, 0, &cb);
        SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dSuc, 0, &cbNum);
        SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 5, 2, &dMin, 0, &cbNum);
        SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 5, 2, &dMax, 0, &cbNum);
        SQLBindParameter(hStmt, 6, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, sTinh, 0, &cb);
        SQLBindParameter(hStmt, 7, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, sQuan, 0, &cb);
        SQLBindParameter(hStmt, 8, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 500, 0, sDiaChi, 0, &cb);
        SQLBindParameter(hStmt, 9, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 20, 0, sSDT, 0, &cb);

        // Bind cột mới
        SQLBindParameter(hStmt, 10, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_FLOAT, 0, 0, &dCongSuat, 0, &cbNum);
        SQLBindParameter(hStmt, 11, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_FLOAT, 0, 0, &dHeSo, 0, &cbNum);
        SQLBindParameter(hStmt, 12, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dCPHa, 0, &cbNum);
        SQLBindParameter(hStmt, 13, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dCPTang, 0, &cbNum);

        retcode = SQLExecute(hStmt);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "GhifileKho");

    } catch (const std::exception& e) { cerr << e.what() << endl; }
    if (hStmt) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

void DocfileKho(SQLHDBC hDbc, std::vector<Kho>& khoList) {
    SQLHSTMT hStmt = SQL_NULL_HSTMT;
    khoList.clear();

    // [MỚI] Select thêm cột
    SQLWCHAR* sql = (SQLWCHAR*)L"SELECT MaKho, TenKho, SucChua, NhietDoMin, NhietDoMax, DiaChi, Tinh, QuanHuyen, SDT, CongSuatCoBan, HeSoCongSuat, ChiPhiHaNhiet, ChiPhiTangNhiet FROM Kho";

    if (SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt))) {
        if (SQL_SUCCEEDED(SQLExecDirect(hStmt, sql, SQL_NTS))) {
            SQLCHAR sMa[51], sSDT[21];
            SQLWCHAR sTen[256], sDC[501], sTinh[101], sQuan[101];
            SQLDOUBLE dSuc, dMin, dMax, dP, dK, dCHa, dCTang;
            SQLLEN len;

            while (SQL_SUCCEEDED(SQLFetch(hStmt))) {
                SQLGetData(hStmt, 1, SQL_C_CHAR, sMa, 51, &len);
                SQLGetData(hStmt, 2, SQL_C_WCHAR, sTen, 256, &len);
                SQLGetData(hStmt, 3, SQL_C_DOUBLE, &dSuc, 0, &len);
                SQLGetData(hStmt, 4, SQL_C_DOUBLE, &dMin, 0, &len);
                SQLGetData(hStmt, 5, SQL_C_DOUBLE, &dMax, 0, &len);
                SQLGetData(hStmt, 6, SQL_C_WCHAR, sDC, 501, &len);
                SQLGetData(hStmt, 7, SQL_C_WCHAR, sTinh, 101, &len);
                SQLGetData(hStmt, 8, SQL_C_WCHAR, sQuan, 101, &len);
                SQLGetData(hStmt, 9, SQL_C_CHAR, sSDT, 21, &len);

                // [MỚI] Lấy data cột mới
                SQLGetData(hStmt, 10, SQL_C_DOUBLE, &dP, 0, &len);
                SQLGetData(hStmt, 11, SQL_C_DOUBLE, &dK, 0, &len);
                SQLGetData(hStmt, 12, SQL_C_DOUBLE, &dCHa, 0, &len);
                SQLGetData(hStmt, 13, SQL_C_DOUBLE, &dCTang, 0, &len);

                // Set mặc định nếu NULL
                float cs = (len == SQL_NULL_DATA) ? 500.0f : (float)dP;
                float hs = (len == SQL_NULL_DATA) ? 20.0f : (float)dK;
                float cp1 = (len == SQL_NULL_DATA) ? 5000.0f : (float)dCHa;
                float cp2 = (len == SQL_NULL_DATA) ? 0.0f : (float)dCTang;

                std::string ma = trim_right((char*)sMa);
                std::string ten = ws2s(sTen);
                std::string sdt = trim_right((char*)sSDT);

                Kho k(ma, ten, (float)dSuc, (float)dMin, (float)dMax, ws2s(sDC), ws2s(sTinh), ws2s(sQuan), sdt,
                      cs, hs, cp1, cp2);
                khoList.push_back(k);
            }
        }
    }
    if (hStmt) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}
// --- NHA CUNG CAP ---
void GhifileNhaCungCap(SQLHDBC hDbc, NhaCungCap& ncc) {
    SQLHSTMT hStmt = SQL_NULL_HSTMT;
    SQLHSTMT hStmtDetail = SQL_NULL_HSTMT; // [MỚI] Handle cho bảng phụ
    SQLRETURN retcode;

    try {
        // --- BƯỚC 1: Lưu thông tin cơ bản vào bảng NhaCungCap ---
        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

        SQLWCHAR* sqlQuery = (SQLWCHAR*)L"INSERT INTO NhaCungCap "
            L"(MaNCC, TenNCC, ThoiGianCho, ChiPhiDat, Tinh, QuanHuyen, DiaChi, SDT) "
            L"VALUES (?, ?, ?, ?, ?, ?, ?, ?)";
        retcode = SQLPrepare(hStmt, sqlQuery, SQL_NTS);

        std::string newMaNCC = sinhMaNhaCungCap();
        ncc.setMaNCC(newMaNCC); // Cập nhật mã cho object

        // (Chuẩn bị dữ liệu bind như cũ - Giữ nguyên phần bind cũ của bạn)
        SQLCHAR sMaNCC[51]; strcpy_s((char*)sMaNCC, 51, ncc.getMaNCC().c_str());
        SQLWCHAR sTenNCC[256]; wcscpy_s(sTenNCC, 256, s2ws(ncc.getTenNCC()).c_str());
        SQLDOUBLE dThoiGianCho = ncc.getThoiGianCho();
        SQLDOUBLE dChiPhiDat = ncc.getChiPhiDat();
        SQLWCHAR sTinh[101]; wcscpy_s(sTinh, 101, s2ws(ncc.getTinh()).c_str());
        SQLWCHAR sQuanHuyen[101]; wcscpy_s(sQuanHuyen, 101, s2ws(ncc.getQuanHuyen()).c_str());
        SQLWCHAR sDiaChi[256]; wcscpy_s(sDiaChi, 256, s2ws(ncc.getDiaChi()).c_str());
        SQLCHAR sSDT[21]; strcpy_s((char*)sSDT, 21, ncc.getSDT().c_str());

        SQLLEN lenStr = SQL_NTS; SQLLEN lenNum = 0;

        SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMaNCC, 0, &lenStr);
        SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 255, 0, sTenNCC, 0, &lenStr);
        SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 10, 2, &dThoiGianCho, 0, &lenNum);
        SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dChiPhiDat, 0, &lenNum);
        SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, sTinh, 0, &lenStr);
        SQLBindParameter(hStmt, 6, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, sQuanHuyen, 0, &lenStr);
        SQLBindParameter(hStmt, 7, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 255, 0, sDiaChi, 0, &lenStr);
        SQLBindParameter(hStmt, 8, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 20, 0, sSDT, 0, &lenStr);

        retcode = SQLExecute(hStmt);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLExecute (NhaCungCap Master)");

        // --- [MỚI] BƯỚC 2: Lưu danh sách Mã Sản Phẩm vào bảng CungCapSanPham ---
        if (!ncc.getDsMaSanPham().empty()) {
            retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmtDetail);
            SQLWCHAR* sqlDetail = (SQLWCHAR*)L"INSERT INTO CungCapSanPham (MaNCC, MaSanPham) VALUES (?, ?)";
            SQLPrepare(hStmtDetail, sqlDetail, SQL_NTS);

            for (const std::string& maSP : ncc.getDsMaSanPham()) {
                SQLCHAR sMaSP[51];
                strcpy_s((char*)sMaSP, 51, maSP.c_str());

                // Bind MaNCC (dùng lại biến sMaNCC ở trên)
                SQLBindParameter(hStmtDetail, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMaNCC, 0, &lenStr);
                // Bind MaSP
                SQLBindParameter(hStmtDetail, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMaSP, 0, &lenStr);

                // Thực thi insert từng dòng
                // Dùng try-catch nhỏ để nếu mã SP sai (không tồn tại trong bảng SanPham) thì không crash chương trình
                try {
                    SQLExecute(hStmtDetail);
                }
                catch (...) {
                    std::cerr << "Canh bao: Khong the them SP " << maSP << " vao NCC (Co the ma SP khong ton tai).\n";
                }
            }
        }
        std::cout << "Da them NCC va danh sach SP thanh cong.\n";

    }
    catch (const std::exception& e) {
        std::cerr << "Loi trong GhifileNhaCungCap: " << e.what() << endl;
    }

    if (hStmt) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    if (hStmtDetail) SQLFreeHandle(SQL_HANDLE_STMT, hStmtDetail);
}
void DocfileNhaCungCap(SQLHDBC hDbc, std::vector<NhaCungCap>& nccList) {
    SQLHSTMT hStmt = SQL_NULL_HSTMT;
    SQLRETURN retcode;
    nccList.clear();

    // Map tạm để gom nhóm sản phẩm theo MaNCC
    std::map<std::string, NhaCungCap> mapNCC;

    try {
        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

        // --- [MỚI] Dùng LEFT JOIN để lấy luôn danh sách sản phẩm ---
        SQLWCHAR* sqlQuery = (SQLWCHAR*)
            L"SELECT n.MaNCC, n.TenNCC, n.ThoiGianCho, n.ChiPhiDat, n.Tinh, n.QuanHuyen, n.DiaChi, n.SDT, c.MaSanPham "
            L"FROM NhaCungCap n "
            L"LEFT JOIN CungCapSanPham c ON n.MaNCC = c.MaNCC";

        retcode = SQLExecDirect(hStmt, sqlQuery, SQL_NTS);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "DocfileNhaCungCap JOIN");

        // Biến đệm
        SQLCHAR sMaNCC[51], sSDT[21], sMaSP[51];
        SQLWCHAR sTen[256], sTinh[101], sQuan[101], sDC[256];
        SQLDOUBLE dTG, dCP;
        SQLLEN lenMa, lenTen, lenTG, lenCP, lenTinh, lenQuan, lenDC, lenSDT, lenMaSP;

        while (SQL_SUCCEEDED(SQLFetch(hStmt))) {
            SQLGetData(hStmt, 1, SQL_C_CHAR, sMaNCC, 51, &lenMa);
            SQLGetData(hStmt, 2, SQL_C_WCHAR, sTen, 256, &lenTen);
            SQLGetData(hStmt, 3, SQL_C_DOUBLE, &dTG, 0, &lenTG);
            SQLGetData(hStmt, 4, SQL_C_DOUBLE, &dCP, 0, &lenCP);
            SQLGetData(hStmt, 5, SQL_C_WCHAR, sTinh, 101, &lenTinh);
            SQLGetData(hStmt, 6, SQL_C_WCHAR, sQuan, 101, &lenQuan);
            SQLGetData(hStmt, 7, SQL_C_WCHAR, sDC, 256, &lenDC);
            SQLGetData(hStmt, 8, SQL_C_CHAR, sSDT, 21, &lenSDT);

            // [MỚI] Lấy MaSanPham
            SQLGetData(hStmt, 9, SQL_C_CHAR, sMaSP, 51, &lenMaSP);

            std::string maNCC = trim_right((char*)sMaNCC);

            // Kiểm tra xem NCC đã có trong map chưa
            if (mapNCC.find(maNCC) == mapNCC.end()) {
                // Chưa có -> Tạo mới
                NhaCungCap ncc(
                    maNCC,
                    lenTen == SQL_NULL_DATA ? "" : ws2s(sTen),
                    lenTG == SQL_NULL_DATA ? 0.0f : (float)dTG,
                    lenCP == SQL_NULL_DATA ? 0.0f : (float)dCP,
                    lenTinh == SQL_NULL_DATA ? "" : ws2s(sTinh),
                    lenQuan == SQL_NULL_DATA ? "" : ws2s(sQuan),
                    lenDC == SQL_NULL_DATA ? "" : ws2s(sDC),
                    lenSDT == SQL_NULL_DATA ? "" : trim_right((char*)sSDT)
                );
                mapNCC[maNCC] = ncc;
            }

            // Nếu có MaSanPham (không NULL) -> Thêm vào vector của NCC đó
            if (lenMaSP != SQL_NULL_DATA) {
                std::string maSP = trim_right((char*)sMaSP);
                mapNCC[maNCC].themMaSanPham(maSP);
            }
        }

        // Chuyển từ Map sang Vector để trả về
        for (auto const& [key, val] : mapNCC) {
            nccList.push_back(val);
        }

    }
    catch (const std::exception& e) {
        std::cerr << "Loi trong DocfileNhaCungCap: " << e.what() << endl;
    }

    if (hStmt) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}
// --- LO HANG ---
// --- CẬP NHẬT TRONG SAOLUU.CPP ---

void GhifileLoHang(SQLHDBC hDbc, LoHang& lo) {
    SQLHSTMT hStmtLoHang = SQL_NULL_HSTMT;
    SQLHSTMT hStmtChiTiet = SQL_NULL_HSTMT;
    SQLHSTMT hStmtTonKho = SQL_NULL_HSTMT;       // <--- MỚI
    SQLHSTMT hStmtChiTietTon = SQL_NULL_HSTMT;   // <--- MỚI
    SQLRETURN retcode;

    // Tắt chế độ tự động commit (BẮT ĐẦU GIAO DỊCH)
    retcode = SQLSetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, 0);
    if (!SQL_SUCCEEDED(retcode)) return;

    try {
        // =================================================================================
        // BƯỚC 1: Ghi vào bảng LoHang (Master - Lịch sử nhập hàng)
        // =================================================================================
        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmtLoHang);
        SQLWCHAR* sqlLoHang = (SQLWCHAR*)L"INSERT INTO LoHang (MaLoHang, MaKho, MaNCC, NgayNhap, SoLuongTanTong, GiaTriTong) VALUES (?, ?, ?, ?, ?, ?)";
        SQLPrepare(hStmtLoHang, sqlLoHang, SQL_NTS);

        // Sinh mã nếu chưa có
        if (lo.getMaLoHang().empty()) lo.setMaLoHang(sinhMaLoHang());

        SQLCHAR sMaLoHang[51]; strcpy_s((char*)sMaLoHang, 51, lo.getMaLoHang().c_str());
        SQLCHAR sMaKho[51];    strcpy_s((char*)sMaKho, 51, lo.getKho().getMaKho().c_str());
        SQLCHAR sMaNCC[51];    strcpy_s((char*)sMaNCC, 51, lo.getNCC().getMaNCC().c_str());
        
        Date ngayNhap = lo.getNgayNhap();
        SQL_DATE_STRUCT sqlNgayNhap = { (SQLSMALLINT)ngayNhap.getYear(), (SQLUSMALLINT)ngayNhap.getMonth(), (SQLUSMALLINT)ngayNhap.getDay() };
        
        SQLDOUBLE dTongSL = lo.getSoLuongTan();
        SQLDOUBLE dTongGiaTri = lo.getGiaTri();
        SQLLEN cb = SQL_NTS, cbNum = 0;

        SQLBindParameter(hStmtLoHang, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMaLoHang, 0, &cb);
        SQLBindParameter(hStmtLoHang, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMaKho, 0, &cb);
        SQLBindParameter(hStmtLoHang, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMaNCC, 0, &cb);
        SQLBindParameter(hStmtLoHang, 4, SQL_PARAM_INPUT, SQL_C_TYPE_DATE, SQL_DATE, 10, 0, &sqlNgayNhap, 0, &cb);
        SQLBindParameter(hStmtLoHang, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dTongSL, 0, &cbNum);
        SQLBindParameter(hStmtLoHang, 6, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dTongGiaTri, 0, &cbNum);

        retcode = SQLExecute(hStmtLoHang);
        if (!SQL_SUCCEEDED(retcode)) throw std::runtime_error("Loi Insert LoHang Master");

        // =================================================================================
        // BƯỚC 2: Ghi vào bảng ChiTietLoHang (Detail - Lịch sử chi tiết)
        // =================================================================================
        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmtChiTiet);
        SQLWCHAR* sqlChiTiet = (SQLWCHAR*)L"INSERT INTO ChiTietLoHang (MaLoHang, MaSanPham, SoLuongTan, HanSuDung, GiaTriSP, TheTich) VALUES (?, ?, ?, ?, ?, ?)";
        SQLPrepare(hStmtChiTiet, sqlChiTiet, SQL_NTS);

        for (const auto& sp : lo.getDanhSachSanPham()) {
            SQLCHAR sMaSP[51]; strcpy_s((char*)sMaSP, 51, sp.sanPham.getMaSP().c_str());
            SQLDOUBLE dSL = sp.soLuong;
            SQLDOUBLE dGia = sp.giaTriSP;
            SQLDOUBLE dTheTich = sp.thetich;
            SQL_DATE_STRUCT sqlHSD = { (SQLSMALLINT)sp.hsd.getYear(), (SQLUSMALLINT)sp.hsd.getMonth(), (SQLUSMALLINT)sp.hsd.getDay() };

            SQLBindParameter(hStmtChiTiet, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMaLoHang, 0, &cb); // Dùng lại sMaLoHang
            SQLBindParameter(hStmtChiTiet, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMaSP, 0, &cb);
            SQLBindParameter(hStmtChiTiet, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dSL, 0, &cbNum);
            SQLBindParameter(hStmtChiTiet, 4, SQL_PARAM_INPUT, SQL_C_TYPE_DATE, SQL_DATE, 10, 0, &sqlHSD, 0, &cb);
            SQLBindParameter(hStmtChiTiet, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dGia, 0, &cbNum);
            SQLBindParameter(hStmtChiTiet, 6, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dTheTich, 0, &cbNum);

            retcode = SQLExecute(hStmtChiTiet);
            if (!SQL_SUCCEEDED(retcode)) throw std::runtime_error("Loi Insert ChiTietLoHang");
        }

        // =================================================================================
        // BƯỚC 3: Ghi vào bảng TonKho (Master - Quản lý số lượng hiện tại) <<--- MỚI THÊM
        // =================================================================================
        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmtTonKho);
        // Dùng MERGE hoặc INSERT. Ở đây dùng INSERT vì lô mới luôn tạo dòng tồn kho mới (quản lý theo lô)
        SQLWCHAR* sqlTonKho = (SQLWCHAR*)L"INSERT INTO TonKho (MaLoHang, MaKho, SoLuongTanTong) VALUES (?, ?, ?)";
        SQLPrepare(hStmtTonKho, sqlTonKho, SQL_NTS);

        // Bind lại tham số (Dùng biến cũ vẫn được)
        SQLBindParameter(hStmtTonKho, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMaLoHang, 0, &cb);
        SQLBindParameter(hStmtTonKho, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMaKho, 0, &cb);
        SQLBindParameter(hStmtTonKho, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_FLOAT, 0, 0, &dTongSL, 0, &cbNum);

        retcode = SQLExecute(hStmtTonKho);
        if (!SQL_SUCCEEDED(retcode)) throw std::runtime_error("Loi Insert TonKho Master");

        // =================================================================================
        // BƯỚC 4: Ghi vào bảng ChiTietTonKho (Detail - Quản lý tồn chi tiết) <<--- MỚI THÊM
        // =================================================================================
        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmtChiTietTon);

        // Query khớp với hình ảnh DB
        SQLWCHAR* sqlChiTietTon = (SQLWCHAR*)L"INSERT INTO ChiTietTonKho (MaLoHang, MaSanPham, SoLuongTan, HanSuDung, GiaTriSanPham, TheTich) VALUES (?, ?, ?, ?, ?, ?)";
        SQLPrepare(hStmtChiTietTon, sqlChiTietTon, SQL_NTS);

        for (const auto& sp : lo.getDanhSachSanPham()) {
            SQLCHAR sMaLoHangCT[256]; strcpy_s((char*)sMaLoHangCT, 256, lo.getMaLoHang().c_str()); // [FIX] Tăng size
            SQLCHAR sMaSP[256];       strcpy_s((char*)sMaSP, 256, sp.sanPham.getMaSP().c_str());   // [FIX] Tăng size

            SQLDOUBLE dSL = sp.soLuong;
            SQLDOUBLE dGia = sp.giaTriSP;
            SQLDOUBLE dTheTich = sp.thetich;
            SQL_DATE_STRUCT sqlHSD = { (SQLSMALLINT)sp.hsd.getYear(), (SQLUSMALLINT)sp.hsd.getMonth(), (SQLUSMALLINT)sp.hsd.getDay() };

            // [FIX] Bind tham số 1 và 2 với độ dài 255
            SQLBindParameter(hStmtChiTietTon, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, sMaLoHangCT, 0, &cb);
            SQLBindParameter(hStmtChiTietTon, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, sMaSP, 0, &cb);

            SQLBindParameter(hStmtChiTietTon, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_FLOAT, 0, 0, &dSL, 0, &cbNum);
            SQLBindParameter(hStmtChiTietTon, 4, SQL_PARAM_INPUT, SQL_C_TYPE_DATE, SQL_DATE, 10, 0, &sqlHSD, 0, &cb);
            SQLBindParameter(hStmtChiTietTon, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dGia, 0, &cbNum);
            SQLBindParameter(hStmtChiTietTon, 6, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_FLOAT, 0, 0, &dTheTich, 0, &cbNum);

            retcode = SQLExecute(hStmtChiTietTon);
            if (!SQL_SUCCEEDED(retcode)) throw std::runtime_error("Loi Insert ChiTietTonKho (SaoLuu.cpp)");
        }

        // --- Nếu mọi thứ thành công, COMMIT giao dịch ---
        SQLEndTran(SQL_HANDLE_DBC, hDbc, SQL_COMMIT);
        cout << ">> [THANH CONG] Da luu Lo Hang & Cap nhat Ton Kho: " << (char*)sMaLoHang << endl;

    } catch (const std::exception& e) {
        cerr << "!! [LOI TRANSACTION] " << e.what() << endl;
        SQLEndTran(SQL_HANDLE_DBC, hDbc, SQL_ROLLBACK);
    }

    // Giải phóng
    if (hStmtLoHang != SQL_NULL_HSTMT) SQLFreeHandle(SQL_HANDLE_STMT, hStmtLoHang);
    if (hStmtChiTiet != SQL_NULL_HSTMT) SQLFreeHandle(SQL_HANDLE_STMT, hStmtChiTiet);
    if (hStmtTonKho != SQL_NULL_HSTMT) SQLFreeHandle(SQL_HANDLE_STMT, hStmtTonKho);
    if (hStmtChiTietTon != SQL_NULL_HSTMT) SQLFreeHandle(SQL_HANDLE_STMT, hStmtChiTietTon);

    // Bật lại tự động commit
    SQLSetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_ON, 0);
}
void DocfileLoHang(SQLHDBC hDbc, std::vector<LoHang>& loList, const std::vector<SanPham>& spList, const std::vector<Kho>& khoList, const std::vector<NhaCungCap>& nccList) {
    SQLHSTMT hStmt = SQL_NULL_HSTMT;
    SQLRETURN retcode;
    loList.clear();

    map<std::string, LoHang> loHangMap;
    map<std::string, std::vector<SanPhamTrongLo>> chiTietMap;

    try {
        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLAllocHandle (DocfileLoHang)");

        SQLWCHAR* sqlQuery = (SQLWCHAR*)
            L"SELECT l.MaLoHang, l.MaKho, l.MaNCC, l.NgayNhap, l.SoLuongTanTong, l.GiaTriTong, "
            L"c.MaSanPham, c.SoLuongTan, c.HanSuDung, c.GiaTriSP, c.TheTich "
            L"FROM LoHang l "
            L"LEFT JOIN ChiTietLoHang c ON l.MaLoHang = c.MaLoHang "
            L"ORDER BY l.MaLoHang";
        
        retcode = SQLExecDirect(hStmt, sqlQuery, SQL_NTS);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLExecDirect (DocfileLoHang)");

        // Đệm
        SQLCHAR sMaLoHang[51], sMaKho[51], sMaNCC[51], sMaSanPham[51];
        SQL_DATE_STRUCT ngayNhapSQL, hsdSQL;
        SQLDOUBLE dTongSL, dTongGiaTri, dSoLuongTan, dGiaTriSP, dTheTich;
        SQLLEN lenMaLo, lenMaKho, lenMaNCC, lenNgayNhap, lenTongSL, lenTongGiaTri;
        SQLLEN lenMaSP, lenSLTan, lenHSD, lenGiaTriSP, lenTheTich;

        // Bind
        SQLBindCol(hStmt, 1, SQL_C_CHAR, sMaLoHang, 51, &lenMaLo);
        SQLBindCol(hStmt, 2, SQL_C_CHAR, sMaKho, 51, &lenMaKho);
        SQLBindCol(hStmt, 3, SQL_C_CHAR, sMaNCC, 51, &lenMaNCC);
        SQLBindCol(hStmt, 4, SQL_C_TYPE_DATE, &ngayNhapSQL, sizeof(SQL_DATE_STRUCT), &lenNgayNhap);
        SQLBindCol(hStmt, 5, SQL_C_DOUBLE, &dTongSL, sizeof(SQLDOUBLE), &lenTongSL);
        SQLBindCol(hStmt, 6, SQL_C_DOUBLE, &dTongGiaTri, sizeof(SQLDOUBLE), &lenTongGiaTri);
        SQLBindCol(hStmt, 7, SQL_C_CHAR, sMaSanPham, 51, &lenMaSP);
        SQLBindCol(hStmt, 8, SQL_C_DOUBLE, &dSoLuongTan, sizeof(SQLDOUBLE), &lenSLTan);
        SQLBindCol(hStmt, 9, SQL_C_TYPE_DATE, &hsdSQL, sizeof(SQL_DATE_STRUCT), &lenHSD);
        SQLBindCol(hStmt, 10, SQL_C_DOUBLE, &dGiaTriSP, sizeof(SQLDOUBLE), &lenGiaTriSP);
        SQLBindCol(hStmt, 11, SQL_C_DOUBLE, &dTheTich, sizeof(SQLDOUBLE), &lenTheTich);

        while (SQL_SUCCEEDED(retcode = SQLFetch(hStmt))) {
            std::string maLo = (lenMaLo == SQL_NULL_DATA) ? "" : trim_right((char*)sMaLoHang);
            
            if (loHangMap.find(maLo) == loHangMap.end()) {
                std::string maKho = (lenMaKho == SQL_NULL_DATA) ? "" : trim_right((char*)sMaKho);
                std::string maNCC = (lenMaNCC == SQL_NULL_DATA) ? "" : trim_right((char*)sMaNCC);

                LoHang newLo;
                newLo.setMaLoHang(maLo);
                newLo.setNgayNhap(Date(ngayNhapSQL.day, ngayNhapSQL.month, ngayNhapSQL.year));
                // *** SỬA: Dùng setSoLuongTan() thay vì setTongTheTich() ***
                newLo.setSoLuongTan((float)dTongSL);
                newLo.setGiaTri((float)dTongGiaTri);

                for (const auto& k : khoList) if (k.getMaKho() == maKho) newLo.setKho(k);
                for (const auto& n : nccList) if (n.getMaNCC() == maNCC) newLo.setNCC(n);
                
                loHangMap[maLo] = newLo;
            }

            // Chỉ thêm chi tiết nếu MaSanPham không NULL
            if (lenMaSP != SQL_NULL_DATA) {
                std::string maSP = trim_right((char*)sMaSanPham);
                SanPhamTrongLo spTL;
                for (const auto& sp : spList) if (sp.getMaSP() == maSP) spTL.sanPham = sp;
                
                spTL.soLuong= (float)dSoLuongTan;
                spTL.hsd = Date(hsdSQL.day, hsdSQL.month, hsdSQL.year);
                spTL.giaTriSP = (float)dGiaTriSP;
                spTL.thetich = (float)dTheTich;

                chiTietMap[maLo].push_back(spTL);
            }
        }
        
        // Ghép nối
        for (auto& pair : loHangMap) {
            std::string maLo = pair.first;
            pair.second.setDanhSachSanPham(chiTietMap[maLo]);
            loList.push_back(pair.second);
        }

    } catch (const std::exception& e) {
        cerr << "Loi trong DocfileLoHang: " << e.what() << endl;
    }

    if (hStmt != SQL_NULL_HSTMT) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

// --- SIEU THI ---
void GhifileSieuThi(SQLHDBC hDbc, SieuThi& st) {
    SQLHSTMT hStmt = SQL_NULL_HSTMT;
    SQLRETURN retcode;

    try {
        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLAllocHandle (GhifileSieuThi)");

        SQLWCHAR* sqlQuery = (SQLWCHAR*)L"INSERT INTO SieuThi "
            L"(MaSieuThi, TenSieuThi, MucPhatThieu,Tinh, QuanHuyen, DiaChi, SDT) "
            L"VALUES (?, ?, ?, ?, ?, ?, ?)";
        retcode = SQLPrepare(hStmt, sqlQuery, SQL_NTS);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLPrepare (GhifileSieuThi)");

        std::string newMaST = sinhMaSieuThi();
        st.setMaSieuThi(newMaST);

        // *** FIX 1 (MaSieuThi) ***
        SQLCHAR sMaST[51];
        strcpy_s((char*)sMaST, 51, st.getMaSieuThi().c_str());
        SQLLEN lenMaST = SQL_NTS;

        SQLWCHAR sTenST[256];
        std::wstring wTenST = s2ws(st.getTenSieuThi());
        wcscpy_s(sTenST, 256, wTenST.c_str());
        SQLLEN lenTenST = SQL_NTS;

        SQLWCHAR sTinh[101];
        std::wstring wTinh = s2ws(st.getTinh());
        wcscpy_s(sTinh, 101, wTinh.c_str());
        SQLLEN lenTinh = SQL_NTS;

        SQLWCHAR sQuanHuyen[101];
        std::wstring wQuanHuyen = s2ws(st.getQuanHuyen());
        wcscpy_s(sQuanHuyen, 101, wQuanHuyen.c_str());
        SQLLEN lenQuanHuyen = SQL_NTS;

        SQLCHAR sSDT[21];
        strcpy_s((char*)sSDT, 21, st.getSDT().c_str());
        SQLLEN lenSDT = SQL_NTS;

        SQLWCHAR sDiaChi[501];
        std::wstring wDiaChi = s2ws(st.getDiaChi());
        wcscpy_s(sDiaChi, 501, wDiaChi.c_str());
        SQLLEN lenDiaChi = SQL_NTS;

        SQLDOUBLE dMucPhat = st.getMucPhatThieu();
        SQLLEN lenNum = 0;

        // Bind
        // *** FIX 1 (MaSieuThi) ***
        retcode = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMaST, 0, &lenMaST);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLBindParameter (MaSieuThi)");
        
        retcode = SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 255, 0, sTenST, 0, &lenTenST);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLBindParameter (TenSieuThi)");

        retcode = SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dMucPhat, 0, &lenNum);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLBindParameter (MucPhatThieu)");
        
        retcode  = SQLBindParameter(hStmt, 4,SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, sTinh, 0, &lenTinh);
        check_error(hStmt, SQL_HANDLE_STMT, retcode , "SQLBindParameter (Tinh)");

        retcode = SQLBindParameter(hStmt, 5,SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, sQuanHuyen, 0, &lenQuanHuyen);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLBindParameter (QuanHuyen)");

        retcode = SQLBindParameter(hStmt, 6, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 500, 0, sDiaChi, 0, &lenDiaChi);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLBindParameter (DiaChi)");

        retcode = SQLBindParameter(hStmt, 7, SQL_PARAM_INPUT, SQL_C_CHAR,SQL_VARCHAR, 20, 0 , sSDT, 0, &lenSDT);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLBindParameter (SDT)");

        retcode = SQLExecute(hStmt);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLExecute (GhifileSieuThi)");

    } catch (const std::exception& e) {
        cerr << "Loi trong GhifileSieuThi: " << e.what() << endl;
    }

    if (hStmt != SQL_NULL_HSTMT) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}
void DocfileSieuThi(SQLHDBC hDbc, std::vector<SieuThi>& stList) {
    SQLHSTMT hStmt = SQL_NULL_HSTMT;
    SQLRETURN retcode;
    stList.clear();

    try {
        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLAllocHandle (DocfileSieuThi)");

        SQLWCHAR* sqlQuery = (SQLWCHAR*)L"SELECT MaSieuThi, TenSieuThi, MucPhatThieu, Tinh, QuanHuyen, DiaChi, SDT FROM SieuThi";
        retcode = SQLExecDirect(hStmt, sqlQuery, SQL_NTS);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLExecDirect (DocfileSieuThi)");

        SQLCHAR sMaST[51];
        SQLWCHAR sTenST[256];
        SQLWCHAR sDiaChi[501];
        SQLDOUBLE dMucPhat;
        SQLWCHAR sTinh[101];
        SQLWCHAR sQuanHuyen[101];
        SQLCHAR sSDT[21];
        SQLLEN lenMaST, lenTenST, lenDiaChi, lenMucPhat, lenTinh, lenQuanHuyen, lenSDT;

        while (SQL_SUCCEEDED(retcode = SQLFetch(hStmt))) {
            SQLGetData(hStmt, 1, SQL_C_CHAR, sMaST, 51, &lenMaST);
            SQLGetData(hStmt, 2, SQL_C_WCHAR, sTenST, 256, &lenTenST);
            SQLGetData(hStmt, 3, SQL_C_DOUBLE, &dMucPhat, sizeof(SQLDOUBLE), &lenMucPhat);
            SQLGetData(hStmt, 4, SQL_C_WCHAR, sTinh, 101, &lenTinh);
            SQLGetData(hStmt, 5, SQL_C_WCHAR, sQuanHuyen, 101, &lenQuanHuyen);
            SQLGetData(hStmt, 6, SQL_C_WCHAR, sDiaChi, 501, &lenDiaChi);
            SQLGetData(hStmt, 7, SQL_C_CHAR, sSDT, 21,&lenSDT);

            std::string maST = (lenMaST == SQL_NULL_DATA) ? "" : trim_right((char*)sMaST);
            wstring wTenST = (lenTenST == SQL_NULL_DATA) ? L"" : sTenST;
            wstring wDiaChi = (lenDiaChi == SQL_NULL_DATA) ? L"" : sDiaChi;
            float mucPhat = (lenMucPhat == SQL_NULL_DATA) ? 0.0f : (float)dMucPhat;
            wstring wTinh = (lenTinh == SQL_NULL_DATA) ? L"" : sTinh;
            wstring wQuanHuyen = (lenQuanHuyen == SQL_NULL_DATA) ? L"" : sQuanHuyen;
            std::string SDT = (lenSDT == SQL_NULL_DATA) ? "" : trim_right((char*) sSDT);

            SieuThi st(maST, ws2s(wTenST),mucPhat, ws2s(wTinh), ws2s(wQuanHuyen),ws2s(wDiaChi),SDT);
            stList.push_back(st);
        }

    } catch (const std::exception& e) {
        cerr << "Loi trong DocfileSieuThi: " << e.what() << endl;
    }

    if (hStmt != SQL_NULL_HSTMT) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

// --- YEU CAU ---
void GhiFileYeuCau(SQLHDBC hDbc, const ChiTietYeuCau& ctyc, const std::string& maSieuThi) {
    SQLHSTMT hStmtMaster = SQL_NULL_HSTMT;
    SQLHSTMT hStmtDetail = SQL_NULL_HSTMT;
    SQLRETURN retcode;

    try {
        // --- Bước 1: Ghi vào bảng YeuCau (master) ---
        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmtMaster);
        check_error(hStmtMaster, SQL_HANDLE_STMT, retcode, "SQLAllocHandle (GhiFileYeuCau - Master)");

        SQLWCHAR* sqlMaster = (SQLWCHAR*)L"INSERT INTO YeuCau "
            L"(MaYeuCau, MaSieuThi, NgayYeuCau, GiaTriTong, HoanThanh) "
            L"VALUES (?, ?, ?, ?, ?)";
        retcode = SQLPrepare(hStmtMaster, sqlMaster, SQL_NTS);
        check_error(hStmtMaster, SQL_HANDLE_STMT, retcode, "SQLPrepare (GhiFileYeuCau - Master)");

        // --- Chuẩn bị dữ liệu ---
        
        // 1. MaYeuCau
        SQLCHAR sMaYC[51];
        strcpy_s((char*)sMaYC, 51, ctyc.getMaYeuCau().c_str());
        SQLLEN lenMaYC = SQL_NTS;

        // 2. MaSieuThi
        SQLCHAR sMaST[51];
        strcpy_s((char*)sMaST, 51, maSieuThi.c_str());
        SQLLEN lenMaST = SQL_NTS;

        // 3. NgayYeuCau (SỬA LỖI: Chuyển sang chuỗi "YYYY-MM-DD")
        char sNgayString[20];
        Date ngay = ctyc.getNgayNhap();
        // Format thành chuỗi: 2025-11-19
        sprintf_s(sNgayString, 20, "%04d-%02d-%02d", ngay.getYear(), ngay.getMonth(), ngay.getDay());
        SQLLEN lenNgayStr = SQL_NTS;

        // 4. GiaTriTong
        SQLDOUBLE dGiaTriTong = ctyc.getGiaTri();
        SQLLEN lenNum = 0;

        // 5. HoanThanh
        SQLWCHAR sHoanThanh[51];
        std::wstring wHoanThanh = s2ws(ctyc.getHoanThanh()); 
        wcscpy_s(sHoanThanh, 51, wHoanThanh.c_str());
        SQLLEN lenHoanThanh = SQL_NTS;

        // --- BIND PARAMETERS ---
        
        retcode = SQLBindParameter(hStmtMaster, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMaYC, 0, &lenMaYC);
        retcode = SQLBindParameter(hStmtMaster, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMaST, 0, &lenMaST);
        
        // *** FIX QUAN TRỌNG: Bind ngày tháng dưới dạng SQL_VARCHAR ***
        // SQL Server sẽ tự động convert chuỗi 'YYYY-MM-DD' sang Date
        retcode = SQLBindParameter(hStmtMaster, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 20, 0, sNgayString, 0, &lenNgayStr);
        
        retcode = SQLBindParameter(hStmtMaster, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dGiaTriTong, 0, &lenNum);
        retcode = SQLBindParameter(hStmtMaster, 5, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 50, 0, sHoanThanh, 0, &lenHoanThanh);

        retcode = SQLExecute(hStmtMaster);
        check_error(hStmtMaster, SQL_HANDLE_STMT, retcode, "SQLExecute (GhiFileYeuCau - Master)");

        // --- Bước 2: Ghi vào bảng ChiTietYeuCau (detail) ---
        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmtDetail);
        check_error(hStmtDetail, SQL_HANDLE_STMT, retcode, "SQLAllocHandle (GhiFileYeuCau - Detail)");

        SQLWCHAR* sqlDetail = (SQLWCHAR*)L"INSERT INTO ChiTietYeuCau "
            L"(MaYeuCau, MaSanPham, SoTanYeuCau, GiaTri) "
            L"VALUES (?, ?, ?, ?)";
        retcode = SQLPrepare(hStmtDetail, sqlDetail, SQL_NTS);
        check_error(hStmtDetail, SQL_HANDLE_STMT, retcode, "SQLPrepare (GhiFileYeuCau - Detail)");

        for (const auto& spyc : ctyc.getdsYeucau()) {
            SQLCHAR sMaYC_D[51];
            strcpy_s((char*)sMaYC_D, 51, ctyc.getMaYeuCau().c_str());
            SQLLEN lenMaYC_D = SQL_NTS;

            SQLCHAR sMaSP[51];
            strcpy_s((char*)sMaSP, 51, spyc.sp->getMaSP().c_str());
            SQLLEN lenMaSP = SQL_NTS;

            SQLDOUBLE dSoTan = spyc.soLuongTan;
            SQLDOUBLE dGiaTri = spyc.ThanhTienRieng;
            SQLLEN lenNumCT = 0;

            retcode = SQLBindParameter(hStmtDetail, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMaYC_D, 0, &lenMaYC_D);
            retcode = SQLBindParameter(hStmtDetail, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMaSP, 0, &lenMaSP);
            retcode = SQLBindParameter(hStmtDetail, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 4, &dSoTan, 0, &lenNumCT);
            retcode = SQLBindParameter(hStmtDetail, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dGiaTri, 0, &lenNumCT);

            retcode = SQLExecute(hStmtDetail);
            check_error(hStmtDetail, SQL_HANDLE_STMT, retcode, "SQLExecute (GhiFileYeuCau - Detail Loop)");
        }

    } catch (const std::exception& e) {
        cerr << "Loi trong GhiFileYeuCau: " << e.what() << endl;
    }

    if (hStmtMaster != SQL_NULL_HSTMT) SQLFreeHandle(SQL_HANDLE_STMT, hStmtMaster);
    if (hStmtDetail != SQL_NULL_HSTMT) SQLFreeHandle(SQL_HANDLE_STMT, hStmtDetail);
}
// Thêm đoạn này vào SaoLuu.cpp
void DocFileYeuCau(SQLHDBC hDbc, std::vector<ChiTietYeuCau>& dsYC, std::vector<SanPham>& dsSP) {
    SQLHSTMT hStmt = SQL_NULL_HSTMT;
    SQLRETURN retcode;
    dsYC.clear();

    // Map dùng để gom nhóm các chi tiết theo Mã Yêu Cầu
    std::map<std::string, ChiTietYeuCau> ycMap;
    std::map<std::string, std::vector<SanPhamYeuCau>> chiTietMap;

    try {
        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLAllocHandle (DocFileYeuCau)");

        // LEFT JOIN để lấy cả thông tin Yêu Cầu lẫn Chi Tiết
        SQLWCHAR* sqlQuery = (SQLWCHAR*)
            L"SELECT y.MaYeuCau, y.MaSieuThi, y.NgayYeuCau, y.GiaTriTong, y.HoanThanh, "
            L"c.MaSanPham, c.SoTanYeuCau, c.GiaTri "
            L"FROM YeuCau y "
            L"LEFT JOIN ChiTietYeuCau c ON y.MaYeuCau = c.MaYeuCau "
            L"ORDER BY y.MaYeuCau";

        retcode = SQLExecDirect(hStmt, sqlQuery, SQL_NTS);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLExecDirect (DocFileYeuCau)");

        SQLCHAR sMaYC[51], sMaST[51], sMaSP[51];
        SQL_DATE_STRUCT ngayYC;
        SQLDOUBLE dGiaTriTong, dSoTan, dGiaTri;
        SQLWCHAR sHoanThanh[11];
        SQLLEN lenMaYC, lenMaST, lenNgay, lenTong, lenMaSP, lenSoTan, lenGiaTri, lenHoanThanh;

        // Bind cột
        SQLBindCol(hStmt, 1, SQL_C_CHAR, sMaYC, 51, &lenMaYC);
        SQLBindCol(hStmt, 2, SQL_C_CHAR, sMaST, 51, &lenMaST);
        SQLBindCol(hStmt, 3, SQL_C_TYPE_DATE, &ngayYC, sizeof(SQL_DATE_STRUCT), &lenNgay);
        SQLBindCol(hStmt, 4, SQL_C_DOUBLE, &dGiaTriTong, sizeof(SQLDOUBLE), &lenTong);
        SQLBindCol(hStmt, 5, SQL_C_WCHAR, sHoanThanh, 11, &lenHoanThanh);
        SQLBindCol(hStmt, 6, SQL_C_CHAR, sMaSP, 51, &lenMaSP);
        SQLBindCol(hStmt, 7, SQL_C_DOUBLE, &dSoTan, sizeof(SQLDOUBLE), &lenSoTan);
        SQLBindCol(hStmt, 8, SQL_C_DOUBLE, &dGiaTri, sizeof(SQLDOUBLE), &lenGiaTri);

        while (SQL_SUCCEEDED(retcode = SQLFetch(hStmt))) {
            std::string maYC = trim_right((char*)sMaYC);
            
            // Nếu mã yêu cầu chưa có trong map thì tạo mới
            if (ycMap.find(maYC) == ycMap.end()) {
                ChiTietYeuCau newYC;
                newYC.setMaYeuCau(maYC);
                // Set MaSieuThi nếu class có hỗ trợ, ở đây tạm thời bỏ qua hoặc set nếu cần
                newYC.setNgayNhap(Date(ngayYC.day, ngayYC.month, ngayYC.year));
                newYC.setGiaTri((float)dGiaTriTong);
                
                std::wstring wHT = (lenHoanThanh == SQL_NULL_DATA) ? L"C" : sHoanThanh;
                newYC.setHoanThanh(ws2s(wHT));
                
                ycMap[maYC] = newYC;
            }

            // Nếu có chi tiết sản phẩm (MaSP không NULL)
            if (lenMaSP != SQL_NULL_DATA) {
                std::string maSP = trim_right((char*)sMaSP);
                SanPhamYeuCau spyc;
                bool found = false;
                
                // Tìm con trỏ sản phẩm trong danh sách SP truyền vào
                for (auto& sp : dsSP) {
                    if (sp.getMaSP() == maSP) {
                        spyc.sp = &sp;
                        found = true;
                        break;
                    }
                }
                
                if (found) {
                    spyc.soLuongTan = (float)dSoTan;
                    spyc.ThanhTienRieng = (float)dGiaTri;
                    chiTietMap[maYC].push_back(spyc);
                }
            }
        }

        // Ghép danh sách chi tiết vào Yêu Cầu cha
        for (auto& pair : ycMap) {
            pair.second.setdsYeuCau(chiTietMap[pair.first]);
            
            // Tính lại tổng số lượng tấn nếu cần (optional)
            float tongTan = 0;
            for(auto& item : chiTietMap[pair.first]) tongTan += item.soLuongTan;
            pair.second.setSoLuongTan(tongTan);

            dsYC.push_back(pair.second);
        }

    } catch (const std::exception& e) {
        cerr << "Loi trong DocFileYeuCau: " << e.what() << endl;
    }

    if (hStmt != SQL_NULL_HSTMT) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}
void CapNhatTrangThaiYeuCau(SQLHDBC hDbc, std::string maYeuCau, std::string trangThaiMoi) {
    SQLHSTMT hStmt = SQL_NULL_HSTMT;
    SQLRETURN retcode;

    try {
        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLAllocHandle (Update Status)");

        SQLWCHAR* sqlQuery = (SQLWCHAR*)L"UPDATE YeuCau SET HoanThanh = ? WHERE MaYeuCau = ?";
        retcode = SQLPrepare(hStmt, sqlQuery, SQL_NTS);

        SQLWCHAR sTrangThai[11];
        wcscpy_s(sTrangThai, 11, s2ws(trangThaiMoi).c_str());
        SQLLEN lenTT = SQL_NTS;

        SQLCHAR sMaYC[51];
        strcpy_s((char*)sMaYC, 51, maYeuCau.c_str());
        SQLLEN lenMa = SQL_NTS;

        SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 10, 0, sTrangThai, 0, &lenTT);
        SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMaYC, 0, &lenMa);

        retcode = SQLExecute(hStmt);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLExecute (Update Status)");
        
        cout << "Da cap nhat trang thai yeu cau " << maYeuCau << " thanh " << trangThaiMoi << endl;

    } catch (const std::exception& e) {
        cerr << "Loi Update Trang Thai: " << e.what() << endl;
    }
    if (hStmt != SQL_NULL_HSTMT) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

//--- CHINH SACH TON ---- 
void DocFileChinhSachTon(SQLHDBC hDbc, std::vector<ChinhSachTon>& dsCS, const std::vector<SanPham>& dsSP) {
    SQLHSTMT hStmt = SQL_NULL_HSTMT;
    SQLRETURN retcode;
    dsCS.clear();

    try {
        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLAllocHandle (DocFileChinhSachTon)");

        SQLWCHAR* sqlQuery = (SQLWCHAR*)L"SELECT MaChinhSach, MaSanPham, MucPhucVu, HeSoZ, TonKhoAnToan, DiemTaiDatHang, EOQ FROM ChinhSachTon";
        retcode = SQLExecDirect(hStmt, sqlQuery, SQL_NTS);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLExecDirect (DocFileChinhSachTon)");

        // Đệm
        SQLCHAR sMaCS[51], sMaSP[51];
        SQLDOUBLE dMucPV, dHeSoZ, dTonAT, dDiemDat, dEOQ;
        // Độ dài / NULL
        SQLLEN lenMaCS, lenMaSP, lenMucPV, lenHeSo, lenTon, lenDiem, lenEOQ;

        // Bind
        SQLBindCol(hStmt, 1, SQL_C_CHAR, sMaCS, 51, &lenMaCS);
        SQLBindCol(hStmt, 2, SQL_C_CHAR, sMaSP, 51, &lenMaSP);
        SQLBindCol(hStmt, 3, SQL_C_DOUBLE, &dMucPV, sizeof(SQLDOUBLE), &lenMucPV);
        SQLBindCol(hStmt, 4, SQL_C_DOUBLE, &dHeSoZ, sizeof(SQLDOUBLE), &lenHeSo);
        SQLBindCol(hStmt, 5, SQL_C_DOUBLE, &dTonAT, sizeof(SQLDOUBLE), &lenTon);
        SQLBindCol(hStmt, 6, SQL_C_DOUBLE, &dDiemDat, sizeof(SQLDOUBLE), &lenDiem);
        SQLBindCol(hStmt, 7, SQL_C_DOUBLE, &dEOQ, sizeof(SQLDOUBLE), &lenEOQ);

        while (SQL_SUCCEEDED(retcode = SQLFetch(hStmt))) {
            std::string maSP = (lenMaSP == SQL_NULL_DATA) ? "" : trim_right((char*)sMaSP);
            
            // Tìm SanPham gốc từ std::vector
            SanPham spGoc;
            bool found = false;
            for (const auto& sp : dsSP) {
                if (sp.getMaSP() == maSP) {
                    spGoc = sp;
                    found = true;
                    break;
                }
            }
            if (!found) continue; // Bỏ qua nếu không tìm thấy SP

            // Tạo ChinhSachTon
            ChinhSachTon cs(spGoc);
            cs.setMucPhucVu(lenMucPV == SQL_NULL_DATA ? 0.0f : (float)dMucPV);
            cs.setHeSoZ(lenHeSo == SQL_NULL_DATA ? 0.0f : (float)dHeSoZ);
            cs.setTonAnToan(lenTon == SQL_NULL_DATA ? 0.0f : (float)dTonAT);
            cs.setDiemDatHang(lenDiem == SQL_NULL_DATA ? 0.0f : (float)dDiemDat);
            cs.setEOQ(lenEOQ == SQL_NULL_DATA ? 0.0f : (float)dEOQ);

            dsCS.push_back(cs);
        }

    } catch (const std::exception& e) {
        cerr << "Loi trong DocFileChinhSachTon: " << e.what() << endl;
    }

    if (hStmt != SQL_NULL_HSTMT) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

void GhiDeFileChinhSachTon(SQLHDBC hDbc,const std::vector<ChinhSachTon>& dsCS) {
    SQLHSTMT hStmt = SQL_NULL_HSTMT;
    SQLRETURN retcode;

    try {
        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLAllocHandle (GhiDeChinhSachTon - TRUNCATE)");
        SQLWCHAR* sqlTruncate = (SQLWCHAR*)L"TRUNCATE TABLE ChinhSachTon";
        retcode = SQLExecDirect(hStmt, sqlTruncate, SQL_NTS);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLExecDirect (GhiDeChinhSachTon - TRUNCATE)");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);

        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLAllocHandle (GhiDeChinhSachTon - INSERT)");
        
        SQLWCHAR* sqlQuery = (SQLWCHAR*)L"INSERT INTO ChinhSachTon "
            L"(MaChinhSach, MaSanPham, MucPhucVu, HeSoZ, TonKhoAnToan, DiemTaiDatHang, EOQ) "
            L"VALUES (?, ?, ?, ?, ?, ?, ?)";
        retcode = SQLPrepare(hStmt, sqlQuery, SQL_NTS);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLPrepare (GhiDeChinhSachTon)");

        for (const auto& cs : dsCS) {
            // *** FIX 1 (MaChinhSach, MaSanPham) ***
            SQLCHAR sMaCS[51];
            strcpy_s((char*)sMaCS, 51, sinhMaChinhSach().c_str());
            SQLLEN lenMaCS = SQL_NTS;

            SQLCHAR sMaSP[51];
            strcpy_s((char*)sMaSP, 51, cs.getMaSP().c_str());
            SQLLEN lenMaSP = SQL_NTS;

            SQLDOUBLE dMucPV = cs.getMucPhucVu();
            SQLDOUBLE dHeSoZ = cs.getHeSoZ();
            SQLDOUBLE dTonAT = cs.getTonAnToan();
            SQLDOUBLE dDiemDat = cs.getDiemDatHang();
            SQLDOUBLE dEOQ = cs.getEOQ();
            SQLLEN lenNum = 0;

            // Bind
            // *** FIX 1 (MaChinhSach, MaSanPham) ***
            retcode = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMaCS, 0, &lenMaCS);
            retcode = SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMaSP, 0, &lenMaSP);
            retcode = SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 5, 4, &dMucPV, 0, &lenNum);
            retcode = SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 5, 2, &dHeSoZ, 0, &lenNum);
            retcode = SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dTonAT, 0, &lenNum);
            retcode = SQLBindParameter(hStmt, 6, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dDiemDat, 0, &lenNum);
            retcode = SQLBindParameter(hStmt, 7, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dEOQ, 0, &lenNum);

            retcode = SQLExecute(hStmt);
            check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLExecute (GhiDeChinhSachTon - Loop)");
        }

    } catch (const std::exception& e) {
        cerr << "Loi trong GhiDeChinhSachTon: " << e.what() << endl;
    }

    if (hStmt != SQL_NULL_HSTMT) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

void CapNhatGhiFileChinhSachTon(SQLHDBC hDbc, const ChinhSachTon& csMoi){
    SQLHSTMT hStmt = SQL_NULL_HSTMT;
    SQLRETURN retcode;

    try{
        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
        check_error(hStmt,SQL_HANDLE_STMT,retcode, "SQLAllocHandle (CAP NHAT FILE GHI)");

        SQLWCHAR* sqlQuery = (SQLWCHAR*) L"MERGE INTO ChinhSachTon AS T "
            L"USING (VALUES (?,?,?,?,?,?,?)) AS S "
            L"(MaChinhSach,MaSanPham,MucPhucVu,HeSoZ,TonKhoAnToan,DiemTaiDatHang,EOQ) "
            L"ON(T.MaSanPham = S.MaSanPham) "
            L"WHEN MATCHED THEN "
            L"UPDATE SET T.MucPhucVu = S.MucPhucVu, T.HeSoZ = S.HeSoZ,T.TonKhoAnToan = S.TonKhoAnToan, T.DiemTaiDatHang = S.DiemTaiDatHang, T.EOQ = S.EOQ "
            L"WHEN NOT MATCHED THEN "
            L"INSERT (MaChinhSach, MaSanPham, MucPhucVu, HeSoZ, TonKhoAnToan, DiemTaiDatHang, EOQ) "
            L"VALUES (S.MaChinhSach,S.MaSanPham,S.MucPhucVu,S.HeSoZ, S.TonKhoAnToan,S.DiemTaiDatHang,S.EOQ);";

        retcode = SQLPrepare(hStmt,sqlQuery, SQL_NTS);
        check_error(hStmt,SQL_HANDLE_STMT,retcode, "SQLPrepare (CapNhatFile)");

        SQLCHAR sMaCS[51];
        strcpy_s((char*)sMaCS,51,sinhMaChinhSach().c_str());
        SQLLEN lenMaCS = SQL_NTS;

        SQLCHAR sMaSP[51];
        strcpy_s((char*)sMaSP, 51, csMoi.getMaSP().c_str());
        SQLLEN lenMaSP = SQL_NTS;

        SQLDOUBLE dMucPV = csMoi.getMucPhucVu();
        SQLDOUBLE dHeSoZ = csMoi.getHeSoZ();
        SQLDOUBLE dTonAT = csMoi.getTonAnToan();
        SQLDOUBLE dDiemDat = csMoi.getDiemDatHang();
        SQLDOUBLE dEOQ = csMoi.getEOQ();
        SQLLEN lenNum = 0;

        retcode = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMaCS, 0, &lenMaCS);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLBind (MaChinhSach)");

        retcode = SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMaSP, 0, &lenMaSP);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLBind (MaSanPham)");

        retcode = SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 5, 4, &dMucPV, 0, &lenNum);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLBind (MucPhucVu)");

        retcode = SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 5, 2, &dHeSoZ, 0, &lenNum);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLBind (HeSoZ)");

        retcode = SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dTonAT, 0, &lenNum);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLBind (TonKhoAnToan)");

        retcode = SQLBindParameter(hStmt, 6, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dDiemDat, 0, &lenNum);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLBind (DiemTaiDatHang)");

        retcode = SQLBindParameter(hStmt, 7, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dEOQ, 0, &lenNum);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLBind (EOQ)");

        // 5. Thuc thi cau lenh MERGE
        retcode = SQLExecute(hStmt);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLExecute (CapNhatGhiFile MERGE)");
    } catch(const std::exception& e){
        cerr << "Loi trong CapNhatGhiFileChinhSachTon: " << e.what() << endl;
    }
    if (hStmt != SQL_NULL_HSTMT) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

// === [SaoLuu.cpp] Thay thế hàm này ===
void GhiFileDinhGia(SQLHDBC hDbc, const DinhGiaDong::KetQuaDinhGia& kq) {
    SQLHSTMT hStmt = SQL_NULL_HSTMT;
    SQLRETURN retcode;

    try {
        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
        
        // Query có 7 tham số
        SQLWCHAR* sqlQuery = (SQLWCHAR*)L"INSERT INTO DinhGiaDong "
            L"(MaGia, MaSanPham, GiaBanDeXuat, LuongBanDuKien, LoiNhuanKyVong, NgayTinh, GhiChu) "
            L"VALUES (?, ?, ?, ?, ?, ?, ?)";
        retcode = SQLPrepare(hStmt, sqlQuery, SQL_NTS);

        // 1. MaGia & MaSP
        SQLCHAR sMaGia[51]; strcpy_s((char*)sMaGia, 51, kq.maGia.c_str());
        SQLCHAR sMaSP[51];  strcpy_s((char*)sMaSP, 51, kq.maSP.c_str());
        
        // 2. Các số liệu
        SQLDOUBLE dGiaDX = kq.giaBanDeXuat;
        SQLDOUBLE dLuongDK = kq.luongBanDuKien;
        SQLDOUBLE dLoiNhuan = kq.loiNhuanKyVong;

        // 3. [FIX QUAN TRỌNG] Chuyển Ngày sang Chuỗi "YYYY-MM-DD"
        // Thay vì dùng SQL_DATE_STRUCT gây lỗi, ta dùng char[]
        char sNgayTinh[20];
        sprintf_s(sNgayTinh, 20, "%04d-%02d-%02d", 
                  kq.ngayTinh.getYear(), kq.ngayTinh.getMonth(), kq.ngayTinh.getDay());

        // 4. Ghi chú (Unicode)
        SQLWCHAR sGhiChu[1001];
        wcscpy_s(sGhiChu, 1001, s2ws(kq.ghiChu).c_str());

        // Độ dài bind
        SQLLEN cbString = SQL_NTS;
        SQLLEN cbNum = 0;

        // --- BIND PARAMETERS (Thứ tự 1 -> 7) ---
        
        // Param 1: MaGia
        SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMaGia, 0, &cbString);
        
        // Param 2: MaSanPham
        SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMaSP, 0, &cbString);
        
        // Param 3: GiaBanDeXuat
        SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dGiaDX, 0, &cbNum);
        
        // Param 4: LuongBanDuKien
        SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dLuongDK, 0, &cbNum);
        
        // Param 5: LoiNhuanKyVong
        SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dLoiNhuan, 0, &cbNum);
        
        // Param 6: NgayTinh [ĐÃ SỬA] -> Bind là SQL_C_CHAR (Chuỗi)
        SQLBindParameter(hStmt, 6, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 20, 0, sNgayTinh, 0, &cbString);
        
        // Param 7: GhiChu
        SQLBindParameter(hStmt, 7, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 1000, 0, sGhiChu, 0, &cbString);

        // Thực thi
        retcode = SQLExecute(hStmt);
        
        // Kiểm tra lỗi kỹ hơn
        if (!SQL_SUCCEEDED(retcode)) {
            check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLExecute (GhiFileDinhGia)");
        }

    } catch (const std::exception& e) {
        std::cerr << "!! Loi SQL Dinh Gia (Chi tiet): " << e.what() << endl;
    }

    if (hStmt != SQL_NULL_HSTMT) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}
void DocFileDinhGia(SQLHDBC hDbc, std::vector<DinhGiaDong::KetQuaDinhGia>& dsKQ) {
    SQLHSTMT hStmt = SQL_NULL_HSTMT;
    SQLRETURN retcode;
    dsKQ.clear();

    try {
        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLAllocHandle (DocFileDinhGia)");

        SQLWCHAR* sqlQuery = (SQLWCHAR*)L"SELECT MaGia, MaSanPham, GiaBanDeXuat, LuongBanDuKien, LoiNhuanKyVong, NgayTinh, GhiChu FROM DinhGiaDong";
        retcode = SQLExecDirect(hStmt, sqlQuery, SQL_NTS);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLExecDirect (DocFileDinhGia)");

        SQLCHAR sMaGia[51], sMaSP[51];
        SQLDOUBLE dGiaDX, dLuongDK, dLoiNhuan;
        SQL_DATE_STRUCT ngayTinh;
        SQLWCHAR sGhiChu[1001];
        SQLLEN lenMaGia, lenMaSP, lenGia, lenLuong, lenLoiNhuan, lenNgay, lenGhiChu;

        while (SQL_SUCCEEDED(retcode = SQLFetch(hStmt))) {
            SQLGetData(hStmt, 1, SQL_C_CHAR, sMaGia, 51, &lenMaGia);
            SQLGetData(hStmt, 2, SQL_C_CHAR, sMaSP, 51, &lenMaSP);
            SQLGetData(hStmt, 3, SQL_C_DOUBLE, &dGiaDX, sizeof(SQLDOUBLE), &lenGia);
            SQLGetData(hStmt, 4, SQL_C_DOUBLE, &dLuongDK, sizeof(SQLDOUBLE), &lenLuong);
            SQLGetData(hStmt, 5, SQL_C_DOUBLE, &dLoiNhuan, sizeof(SQLDOUBLE), &lenLoiNhuan);
            SQLGetData(hStmt, 6, SQL_C_TYPE_DATE, &ngayTinh, sizeof(SQL_DATE_STRUCT), &lenNgay);
            SQLGetData(hStmt, 7, SQL_C_WCHAR, sGhiChu, 1001, &lenGhiChu);

            DinhGiaDong::KetQuaDinhGia kq;
            kq.maGia = (lenMaGia == SQL_NULL_DATA) ? "" : trim_right((char*)sMaGia);
            kq.maSP = (lenMaSP == SQL_NULL_DATA) ? "" : trim_right((char*)sMaSP);
            kq.giaBanDeXuat = (lenGia == SQL_NULL_DATA) ? 0.0f : (float)dGiaDX;
            kq.luongBanDuKien = (lenLuong == SQL_NULL_DATA) ? 0.0f : (float)dLuongDK;
            kq.loiNhuanKyVong = (lenLoiNhuan == SQL_NULL_DATA) ? 0.0f : (float)dLoiNhuan;
            if (lenNgay != SQL_NULL_DATA)
                kq.ngayTinh = Date(ngayTinh.day, ngayTinh.month, ngayTinh.year);
            kq.ghiChu = (lenGhiChu == SQL_NULL_DATA) ? "" : ws2s(sGhiChu);
            
            dsKQ.push_back(kq);
        }

    } catch (const std::exception& e) {
        cerr << "Loi trong DocFileDinhGia: " << e.what() << endl;
    }

    if (hStmt != SQL_NULL_HSTMT) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}
// --- Thêm vào cuối file SaoLuu.cpp ---
// Đảm bảo bạn đã #include "DuBaoNhuCau.h" và các thư viện SQL ở đầu file

void DocFileDuBaoNhuCau(SQLHDBC hDbc, std::vector<KetQuaDuBao>& ds) {
    ds.clear();
    SQLHSTMT hStmt;
    SQLRETURN ret;
    ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    if (!SQL_SUCCEEDED(ret)) return;

    // Câu lệnh truy vấn (Giả định bảng tên là DuBaoNhuCau)
    // Cấu trúc bảng giả định: MaDuBao (char), MaSP (char), TuNgay (date), DenNgay (date), NhuCauTB (float), DoLechChuan (float)
    SQLWCHAR* query = (SQLWCHAR*)L"SELECT MaDuBao, MaSP, TuNgay, DenNgay, NhuCauTB, DoLechChuan FROM DuBaoNhuCau";
    
    if (SQL_SUCCEEDED(SQLExecDirect(hStmt, query, SQL_NTS))) {
        SQLCHAR maDB[50], maSP[50];
        SQL_DATE_STRUCT tuNgaySql, denNgaySql;
        float nhuCau, doLech;
        SQLLEN len;

        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            // Lấy dữ liệu từng cột
            SQLGetData(hStmt, 1, SQL_C_CHAR, maDB, sizeof(maDB), &len);
            SQLGetData(hStmt, 2, SQL_C_CHAR, maSP, sizeof(maSP), &len);
            SQLGetData(hStmt, 3, SQL_C_TYPE_DATE, &tuNgaySql, 0, &len);
            SQLGetData(hStmt, 4, SQL_C_TYPE_DATE, &denNgaySql, 0, &len);
            SQLGetData(hStmt, 5, SQL_C_FLOAT, &nhuCau, 0, &len);
            SQLGetData(hStmt, 6, SQL_C_FLOAT, &doLech, 0, &len);

            // Đổ vào struct
            KetQuaDuBao kq;
            kq.maDuBao = trim_right((char*)maDB);
            kq.maSP = trim_right((char*)maSP);
            kq.tuNgay = Date(tuNgaySql.day, tuNgaySql.month, tuNgaySql.year);
            kq.denNgay = Date(denNgaySql.day, denNgaySql.month, denNgaySql.year);
            kq.nhuCauTrungBinh = nhuCau;
            kq.doLechChuan = doLech;

            ds.push_back(kq);
        }
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}
// === [SaoLuu.cpp] Thay thế hàm này ===
void GhiFileDuBaoNhuCau(SQLHDBC hDbc, const KetQuaDuBao& kq) {
    SQLHSTMT hStmt = SQL_NULL_HSTMT;
    SQLRETURN retcode;

    try {
        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
        
        SQLWCHAR* sqlQuery = (SQLWCHAR*)L"INSERT INTO DuBaoNhuCau "
            L"(MaDuBao, MaSanPham, TuNgay, DenNgay, NhuCauTrungBinh, DoLechChuan) "
            L"VALUES (?, ?, ?, ?, ?, ?)";
        retcode = SQLPrepare(hStmt, sqlQuery, SQL_NTS);

        // 1. MaDuBao & MaSP
        SQLCHAR sMaDB[51]; strcpy_s((char*)sMaDB, 51, kq.maDuBao.c_str());
        SQLCHAR sMaSP[51]; strcpy_s((char*)sMaSP, 51, kq.maSP.c_str());
        
        // 2. XỬ LÝ NGÀY THÁNG -> CHUỖI (FIX LỖI SQL)
        char sTuNgay[20], sDenNgay[20];
        sprintf_s(sTuNgay, 20, "%04d-%02d-%02d", kq.tuNgay.getYear(), kq.tuNgay.getMonth(), kq.tuNgay.getDay());
        sprintf_s(sDenNgay, 20, "%04d-%02d-%02d", kq.denNgay.getYear(), kq.denNgay.getMonth(), kq.denNgay.getDay());

        // 3. Số liệu
        SQLDOUBLE dTB = kq.nhuCauTrungBinh;
        SQLDOUBLE dDLCH = kq.doLechChuan;
        
        SQLLEN cbString = SQL_NTS;
        SQLLEN cbNum = 0;

        // --- BIND ---
        SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMaDB, 0, &cbString);
        SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMaSP, 0, &cbString);
        
        // Bind Ngày tháng dạng STRING (SQL_C_CHAR)
        SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 20, 0, sTuNgay, 0, &cbString);
        SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 20, 0, sDenNgay, 0, &cbString);
        
        SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dTB, 0, &cbNum);
        SQLBindParameter(hStmt, 6, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dDLCH, 0, &cbNum);

        retcode = SQLExecute(hStmt);
        check_error(hStmt, SQL_HANDLE_STMT, retcode, "SQLExecute (GhiFileDuBaoNhuCau)");

    } catch (const std::exception& e) {
        // In lỗi nhưng không làm crash chương trình
        std::cerr << "!! Loi SQL Du Bao: " << e.what() << endl;
    }

    if (hStmt != SQL_NULL_HSTMT) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}
void GhifilePhuongTien(SQLHDBC hDbc, PhuongTien& pt) {
    SQLHSTMT hStmt;
    if(!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt))) return;

    if (pt.getMaXe().empty()) pt.setMaXe(sinhMaPhuongTien());

    // SQL Query: MERGE (Update nếu tồn tại, Insert nếu chưa)
    SQLWCHAR* sql = (SQLWCHAR*)L"MERGE INTO PhuongTien AS T "
        L"USING (VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)) "
        L"       AS S(MaXe, LoaiXe, SoXe, SucChuaTan, ChiPhiMoiChuyen, Tinh, QuanHuyen, DiaChi, SDT, "
        L"            PhuThemKhacTinh, PhuThemKhacQuanHuyen, VanTocTB, GioCong, Lrate, Urate, Tsetup) "
        L"ON T.MaXe = S.MaXe "
        L"WHEN MATCHED THEN "
        L"   UPDATE SET LoaiXe=S.LoaiXe, SoXe=S.SoXe, SucChuaTan=S.SucChuaTan, ChiPhiMoiChuyen=S.ChiPhiMoiChuyen, "
        L"              Tinh=S.Tinh, QuanHuyen=S.QuanHuyen, DiaChi=S.DiaChi, SDT=S.SDT, "
        L"              PhuThemKhacTinh=S.PhuThemKhacTinh, PhuThemKhacQuanHuyen=S.PhuThemKhacQuanHuyen, "
        L"              VanTocTB=S.VanTocTB, GioCong=S.GioCong, Lrate=S.Lrate, Urate=S.Urate, Tsetup=S.Tsetup "
        L"WHEN NOT MATCHED THEN "
        L"   INSERT (MaXe, LoaiXe, SoXe, SucChuaTan, ChiPhiMoiChuyen, Tinh, QuanHuyen, DiaChi, SDT, "
        L"           PhuThemKhacTinh, PhuThemKhacQuanHuyen, VanTocTB, GioCong, Lrate, Urate, Tsetup) "
        L"   VALUES (S.MaXe, S.LoaiXe, S.SoXe, S.SucChuaTan, S.ChiPhiMoiChuyen, S.Tinh, S.QuanHuyen, S.DiaChi, S.SDT, "
        L"           S.PhuThemKhacTinh, S.PhuThemKhacQuanHuyen, S.VanTocTB, S.GioCong, S.Lrate, S.Urate, S.Tsetup);";

    SQLPrepare(hStmt, sql, SQL_NTS);

    // Chuẩn bị dữ liệu để Bind
    SQLCHAR sMa[51]; strcpy_s((char*)sMa, 51, pt.getMaXe().c_str());
    SQLWCHAR sLoai[101]; wcscpy_s(sLoai, 101, s2ws(pt.getLoaiXe()).c_str());
    SQLINTEGER iSoXe = pt.getSoXe();
    SQLDOUBLE dSucChua = pt.getSucChua();
    SQLDOUBLE dChiPhi = pt.getChiPhi();
    
    SQLWCHAR sTinh[101]; wcscpy_s(sTinh, 101, s2ws(pt.getTinh()).c_str());
    SQLWCHAR sQuan[101]; wcscpy_s(sQuan, 101, s2ws(pt.getQuanHuyen()).c_str());
    SQLWCHAR sDiaChi[256]; wcscpy_s(sDiaChi, 256, s2ws(pt.getDiaChi()).c_str());
    SQLCHAR sSDT[21]; strcpy_s((char*)sSDT, 21, pt.getSDT().c_str());

    SQLDOUBLE dPhuTinh = pt.getPhuPhiTinh();
    SQLDOUBLE dPhuQuan = pt.getPhuPhiQuan();
    SQLDOUBLE dVanToc = pt.getVanTocTB();
    SQLDOUBLE dGioCong = pt.getGioCongToiDa();
    SQLDOUBLE dLrate = pt.getLrate();
    SQLDOUBLE dUrate = pt.getUrate();
    SQLDOUBLE dTsetup = pt.getTsetup();

    SQLLEN cb = SQL_NTS;
    SQLLEN cbNum = 0;

    // Bind tham số
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMa, 0, &cb);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, sLoai, 0, &cb);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &iSoXe, 0, &cbNum);
    SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_FLOAT, 0, 0, &dSucChua, 0, &cbNum);
    SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dChiPhi, 0, &cbNum);
    
    SQLBindParameter(hStmt, 6, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, sTinh, 0, &cb);
    SQLBindParameter(hStmt, 7, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, sQuan, 0, &cb);
    SQLBindParameter(hStmt, 8, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 255, 0, sDiaChi, 0, &cb);
    SQLBindParameter(hStmt, 9, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 20, 0, sSDT, 0, &cb);

    SQLBindParameter(hStmt, 10, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dPhuTinh, 0, &cbNum);
    SQLBindParameter(hStmt, 11, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dPhuQuan, 0, &cbNum);
    
    SQLBindParameter(hStmt, 12, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_FLOAT, 0, 0, &dVanToc, 0, &cbNum);
    SQLBindParameter(hStmt, 13, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_FLOAT, 0, 0, &dGioCong, 0, &cbNum);
    SQLBindParameter(hStmt, 14, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_FLOAT, 0, 0, &dLrate, 0, &cbNum);
    SQLBindParameter(hStmt, 15, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_FLOAT, 0, 0, &dUrate, 0, &cbNum);
    SQLBindParameter(hStmt, 16, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_FLOAT, 0, 0, &dTsetup, 0, &cbNum);

    SQLExecute(hStmt);
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

// --- CẬP NHẬT HÀM: DocfilePhuongTien (SỬA LỖI KHAI BÁO BIẾN) ---
void DocfilePhuongTien(SQLHDBC hDbc, std::vector<PhuongTien>& ptList) {
    ptList.clear();
    SQLHSTMT hStmt;
    if(!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt))) return;

    SQLWCHAR* sql = (SQLWCHAR*)L"SELECT MaXe, LoaiXe, SoXe, SucChuaTan, ChiPhiMoiChuyen, "
        L"Tinh, QuanHuyen, DiaChi, SDT, PhuThemKhacTinh, PhuThemKhacQuanHuyen, "
        L"VanTocTB, GioCong, Lrate, Urate, Tsetup FROM PhuongTien";

    if (SQL_SUCCEEDED(SQLExecDirect(hStmt, sql, SQL_NTS))) {
        // 1. KHAI BÁO BIẾN ĐỆM (Buffer) ĐỂ HỨNG DỮ LIỆU TỪ SQL
        SQLCHAR sMa[51];
        SQLWCHAR sLoai[101], sTinh[101], sQuan[101], sDiaChi[256];
        SQLCHAR sSDT[21];
        SQLINTEGER iSoXe;
        SQLDOUBLE dSucChua, dChiPhi, dPhuTinh, dPhuQuan, dVanToc, dGioCong, dLrate, dUrate, dTsetup;
        
        SQLLEN lenMa, lenLoai, lenSoXe, lenSucChua, lenChiPhi, lenTinh, lenQuan, lenDC, lenSDT;
        SQLLEN lenPhuTinh, lenPhuQuan, lenVT, lenGC, lenLr, lenUr, lenTs;

        // 2. VÒNG LẶP FETCH DỮ LIỆU
        while (SQL_SUCCEEDED(SQLFetch(hStmt))) {
            // Lấy dữ liệu vào buffer
            SQLGetData(hStmt, 1, SQL_C_CHAR, sMa, 51, &lenMa);
            SQLGetData(hStmt, 2, SQL_C_WCHAR, sLoai, 101, &lenLoai);
            SQLGetData(hStmt, 3, SQL_C_LONG, &iSoXe, 0, &lenSoXe);
            SQLGetData(hStmt, 4, SQL_C_DOUBLE, &dSucChua, 0, &lenSucChua);
            SQLGetData(hStmt, 5, SQL_C_DOUBLE, &dChiPhi, 0, &lenChiPhi);
            SQLGetData(hStmt, 6, SQL_C_WCHAR, sTinh, 101, &lenTinh);
            SQLGetData(hStmt, 7, SQL_C_WCHAR, sQuan, 101, &lenQuan);
            SQLGetData(hStmt, 8, SQL_C_WCHAR, sDiaChi, 256, &lenDC);
            SQLGetData(hStmt, 9, SQL_C_CHAR, sSDT, 21, &lenSDT);
            SQLGetData(hStmt, 10, SQL_C_DOUBLE, &dPhuTinh, 0, &lenPhuTinh);
            SQLGetData(hStmt, 11, SQL_C_DOUBLE, &dPhuQuan, 0, &lenPhuQuan);
            SQLGetData(hStmt, 12, SQL_C_DOUBLE, &dVanToc, 0, &lenVT);
            SQLGetData(hStmt, 13, SQL_C_DOUBLE, &dGioCong, 0, &lenGC);
            SQLGetData(hStmt, 14, SQL_C_DOUBLE, &dLrate, 0, &lenLr);
            SQLGetData(hStmt, 15, SQL_C_DOUBLE, &dUrate, 0, &lenUr);
            SQLGetData(hStmt, 16, SQL_C_DOUBLE, &dTsetup, 0, &lenTs);

            // 3. CHUYỂN ĐỔI TỪ BUFFER SANG C++ TYPES (KHAI BÁO BIẾN CỤC BỘ Ở ĐÂY)
            std::string ma = (lenMa == SQL_NULL_DATA) ? "" : trim_right((char*)sMa);
            std::string loai = (lenLoai == SQL_NULL_DATA) ? "" : ws2s(sLoai);
            int soXe = (lenSoXe == SQL_NULL_DATA) ? 0 : (int)iSoXe;
            float sucChua = (lenSucChua == SQL_NULL_DATA) ? 0.0f : (float)dSucChua;
            float chiPhi = (lenChiPhi == SQL_NULL_DATA) ? 0.0f : (float)dChiPhi;
            std::string tinh = (lenTinh == SQL_NULL_DATA) ? "" : ws2s(sTinh);
            std::string quan = (lenQuan == SQL_NULL_DATA) ? "" : ws2s(sQuan);
            std::string dc = (lenDC == SQL_NULL_DATA) ? "" : ws2s(sDiaChi);
            std::string sdt = (lenSDT == SQL_NULL_DATA) ? "" : trim_right((char*)sSDT);
            
            float phuTinh = (lenPhuTinh == SQL_NULL_DATA) ? 0.0f : (float)dPhuTinh;
            float phuQuan = (lenPhuQuan == SQL_NULL_DATA) ? 0.0f : (float)dPhuQuan;
            float vanToc = (lenVT == SQL_NULL_DATA) ? 45.0f : (float)dVanToc;
            float gioCong = (lenGC == SQL_NULL_DATA) ? 8.0f : (float)dGioCong;
            float lrate = (lenLr == SQL_NULL_DATA) ? 0.35f : (float)dLrate;
            float urate = (lenUr == SQL_NULL_DATA) ? 0.30f : (float)dUrate;
            float tsetup = (lenTs == SQL_NULL_DATA) ? 0.5f : (float)dTsetup;

            // 4. GỌI CONSTRUCTOR (Giờ thì biến 'ma', 'loai'... đã tồn tại)
            PhuongTien pt(ma, loai, soXe, sucChua, chiPhi, tinh, quan, dc, sdt, 
                          phuTinh, phuQuan, vanToc, gioCong, lrate, urate, tsetup);
            ptList.push_back(pt);
        }
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}
void GhifileTuyenVanChuyen(SQLHDBC hDbc, TuyenVanChuyen& tvc) {
    SQLHSTMT hStmt;
    if (!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt))) return;

    // --- CẬP NHẬT SQL: Thêm cột ChiPhiTuyenCoDinh ---
    SQLWCHAR* sql = (SQLWCHAR*)L"INSERT INTO TuyenVanChuyen "
        L"(MaTuyen, KhoDi, KhoTrungChuyen, SieuThiNhan, MaXe, ChiPhiTuyenCoDinh) "
        L"VALUES (?, ?, ?, ?, ?, ?)";

    SQLPrepare(hStmt, sql, SQL_NTS);

    if (tvc.getMaTuyen().empty()) tvc.setMaTuyen(sinhMaTuyen());

    // Chuẩn bị biến cũ
    SQLCHAR sMa[51]; strcpy_s((char*)sMa, 51, tvc.getMaTuyen().c_str());
    SQLCHAR sKhoDi[256]; strcpy_s((char*)sKhoDi, 256, tvc.getKhoDi().c_str());
    SQLCHAR sKhoTC[256]; 
    SQLCHAR sSieuThi[256]; strcpy_s((char*)sSieuThi, 256, tvc.getSieuThiNhan().c_str());
    SQLCHAR sXe[51]; strcpy_s((char*)sXe, 51, tvc.getMaXe().c_str());
    
    // --- BIẾN MỚI ---
    SQLDOUBLE dChiPhi = tvc.getChiPhiCoDinh();
    SQLLEN cbNull = 0; // Dùng cho số

    SQLLEN cb = SQL_NTS;
    SQLLEN cbKhoTC;

    if (tvc.getKhoTrungChuyen().empty()) {
        cbKhoTC = SQL_NULL_DATA; 
    } else {
        strcpy_s((char*)sKhoTC, 256, tvc.getKhoTrungChuyen().c_str());
        cbKhoTC = SQL_NTS;
    }

    // Bind tham số (Thêm tham số thứ 6)
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMa, 0, &cb);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, sKhoDi, 0, &cb);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, sKhoTC, 0, &cbKhoTC);
    SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, sSieuThi, 0, &cb);
    SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sXe, 0, &cb);
    
    // --- BIND CỘT MỚI ---
    SQLBindParameter(hStmt, 6, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dChiPhi, 0, &cbNull);

    if (SQL_SUCCEEDED(SQLExecute(hStmt))) {
        cout << ">> Da them tuyen van chuyen: " << tvc.getMaTuyen() << endl;
    } else {
        // In thêm chi tiết lỗi nếu cần debug (tùy chọn)
        cout << "!! Loi khi them tuyen van chuyen.\n";
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}
void DocfileTuyenVanChuyen(SQLHDBC hDbc, std::vector<TuyenVanChuyen>& dsTVC) {
    dsTVC.clear();
    SQLHSTMT hStmt;
    if (!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt))) return;

    // --- CẬP NHẬT SQL: Thêm cột ChiPhiTuyenCoDinh ---
    SQLWCHAR* sql = (SQLWCHAR*)L"SELECT MaTuyen, KhoDi, KhoTrungChuyen, SieuThiNhan, MaXe, ChiPhiTuyenCoDinh FROM TuyenVanChuyen";

    if (SQL_SUCCEEDED(SQLExecDirect(hStmt, sql, SQL_NTS))) {
        SQLCHAR sMa[51], sKhoDi[256], sKhoTC[256], sSieuThi[256], sXe[51];
        SQLDOUBLE dChiPhi; // Biến nhận giá trị mới
        SQLLEN lenMa, lenKhoDi, lenKhoTC, lenSieuThi, lenXe, lenChiPhi;

        while (SQL_SUCCEEDED(SQLFetch(hStmt))) {
            SQLGetData(hStmt, 1, SQL_C_CHAR, sMa, 51, &lenMa);
            SQLGetData(hStmt, 2, SQL_C_CHAR, sKhoDi, 256, &lenKhoDi);
            SQLGetData(hStmt, 3, SQL_C_CHAR, sKhoTC, 256, &lenKhoTC);
            SQLGetData(hStmt, 4, SQL_C_CHAR, sSieuThi, 256, &lenSieuThi);
            SQLGetData(hStmt, 5, SQL_C_CHAR, sXe, 51, &lenXe);
            
            // --- LẤY DATA CỘT MỚI (Cột 6) ---
            SQLGetData(hStmt, 6, SQL_C_DOUBLE, &dChiPhi, sizeof(dChiPhi), &lenChiPhi);

            std::string ma = trim_right((char*)sMa);
            std::string kDi = trim_right((char*)sKhoDi);
            std::string st = trim_right((char*)sSieuThi);
            std::string xe = trim_right((char*)sXe);
            
            std::string kTC = "";
            if (lenKhoTC != SQL_NULL_DATA) {
                kTC = trim_right((char*)sKhoTC);
            }
            
            // Xử lý chi phí (nếu NULL thì bằng 0)
            float chiPhi = (lenChiPhi == SQL_NULL_DATA) ? 0.0f : (float)dChiPhi;

            // Gọi Constructor mới đã cập nhật
            dsTVC.push_back(TuyenVanChuyen(ma, kDi, kTC, st, xe, chiPhi));
        }
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}
// Thêm vào cuối file SaoLuu.cpp

// Hàm 1: Lấy tổng số lượng tấn hiện có trong kho của 1 sản phẩm
float LayTongTonKhoHienTai(SQLHDBC hDbc, const std::string& maSP) {
    SQLHSTMT hStmt;
    float tongTon = 0.0f;

    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
        // Truy vấn tổng từ bảng chi tiết tồn kho
        std::string sql = "SELECT SUM(SoLuongTan) FROM ChiTietTonKho WHERE MaSanPham = ?";
        
        SQLPrepareA(hStmt, (SQLCHAR*)sql.c_str(), SQL_NTS);
        SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)maSP.c_str(), 0, NULL);
        
        if (SQLExecute(hStmt) == SQL_SUCCESS) {
            if (SQLFetch(hStmt) == SQL_SUCCESS) {
                SQLLEN lenData;
                SQLGetData(hStmt, 1, SQL_C_FLOAT, &tongTon, 0, &lenData);
                if (lenData == SQL_NULL_DATA) tongTon = 0.0f;
            }
        }
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    }
    return tongTon;
}

// Hàm 2: Lấy số lượng hàng sắp hết hạn (Ví dụ: còn < 3 ngày nữa là hỏng)
// Logic: DATEDIFF(day, GETDATE(), HanSuDung) <= soNgayCanhBao
float LayTonKhoSapHetHan(SQLHDBC hDbc, const std::string& maSP, int soNgayCanhBao) {
    SQLHSTMT hStmt;
    float luongSapHong = 0.0f;

    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
        std::string sql = "SELECT SUM(SoLuongTan) FROM ChiTietTonKho "
                          "WHERE MaSanPham = ? AND DATEDIFF(day, GETDATE(), HanSuDung) <= ?";
        
        SQLPrepareA(hStmt, (SQLCHAR*)sql.c_str(), SQL_NTS);
        SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)maSP.c_str(), 0, NULL);
        SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &soNgayCanhBao, 0, NULL);
        
        if (SQLExecute(hStmt) == SQL_SUCCESS) {
            if (SQLFetch(hStmt) == SQL_SUCCESS) {
                SQLLEN lenData;
                SQLGetData(hStmt, 1, SQL_C_FLOAT, &luongSapHong, 0, &lenData);
                if (lenData == SQL_NULL_DATA) luongSapHong = 0.0f;
            }
        }
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    }
    return luongSapHong;
}
// Hàm lưu vết nguồn hàng (Hỗ trợ 1 đơn nhiều kho)
// Dán vào cuối file SaoLuu.cpp
void GhiNhatKyNguonHang(SQLHDBC hDbc, std::string maYeuCau, std::string maKho, std::wstring loaiNguon, std::wstring ghiChu, double chiPhi) {
    SQLHSTMT hStmt = SQL_NULL_HSTMT;
    try {
        if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
            // [CẬP NHẬT] Thêm cột ChiPhiVanChuyen vào câu lệnh INSERT
            SQLWCHAR* sql = (SQLWCHAR*)L"INSERT INTO PhanBoNguonHang (MaYeuCau, MaKho, LoaiNguon, GhiChu, ChiPhiVanChuyen) VALUES (?, ?, ?, ?, ?)";
            SQLPrepare(hStmt, sql, SQL_NTS);

            // Chuẩn bị biến
            SQLCHAR sMaYC[51]; strcpy_s((char*)sMaYC, 51, maYeuCau.c_str());
            SQLCHAR sMaKho[51]; strcpy_s((char*)sMaKho, 51, maKho.c_str());
            SQLWCHAR sLoai[51]; wcscpy_s(sLoai, 51, loaiNguon.c_str());
            SQLWCHAR sGhiChu[256]; wcscpy_s(sGhiChu, 256, ghiChu.c_str());

            // Biến chi phí (Mới)
            SQLDOUBLE dChiPhi = chiPhi;

            // Bind tham số (Thêm tham số thứ 5)
            SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMaYC, 0, NULL);
            SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMaKho, 0, NULL);
            SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 50, 0, sLoai, 0, NULL);
            SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 255, 0, sGhiChu, 0, NULL);

            // Bind cột mới
            SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dChiPhi, 0, NULL);

            SQLExecute(hStmt);
        }
    }
    catch (...) {}
    if (hStmt) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}
// Thêm hàm này vào SupermarketForm.h (Tầng Logic/Backend C++)
// Thêm hàm này vào SupermarketForm.h (Tầng Logic/Backend C++)
// Hàm lấy tổng số lượng tấn của 1 sản phẩm TRONG KHO CỤ THỂ
float LayTonKhoTaiKho(SQLHDBC hDbc, const std::string& maKho, const std::string& maSP) {
    SQLHSTMT hStmt;
    float tongTon = 0.0f;

    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
        // JOIN bảng ChiTiet với bảng TonKho để lọc theo MaKho
        std::wstring sql = L"SELECT SUM(c.SoLuongTan) "
            L"FROM ChiTietTonKho c "
            L"JOIN TonKho t ON c.MaLoHang = t.MaLoHang "
            L"WHERE t.MaKho = ? AND c.MaSanPham = ?";

        SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);

        // Bind tham số MaKho
        SQLCHAR cMaKho[51]; strcpy_s((char*)cMaKho, 51, maKho.c_str());
        SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMaKho, 0, NULL);

        // Bind tham số MaSP
        SQLCHAR cMaSP[51]; strcpy_s((char*)cMaSP, 51, maSP.c_str());
        SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMaSP, 0, NULL);

        if (SQLExecute(hStmt) == SQL_SUCCESS) {
            if (SQLFetch(hStmt) == SQL_SUCCESS) {
                SQLLEN lenData;
                SQLGetData(hStmt, 1, SQL_C_FLOAT, &tongTon, 0, &lenData);
                if (lenData == SQL_NULL_DATA) tongTon = 0.0f;
            }
        }
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    }
    return tongTon;
}