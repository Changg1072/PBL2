#include "TonKho.h"
#include <iostream>
#include <string>
#include <vector>
#include "Date.h"
#include "SaoLuu.h" 

using namespace std;

TonKho::TonKho() : LoHang() {
}

void TonKho::LuuChiTietTonKho(SQLHDBC hDbc) {
    SQLHSTMT hStmt;
    // [FIX] Đảm bảo thứ tự cột khớp với hình ảnh: MaLoHang, MaSanPham, SoLuongTan, HanSuDung, GiaTriSanPham, TheTich
    // (Bỏ qua cột ID vì nó là tự động tăng - Identity)
    std::string sqlChiTiet = "INSERT INTO dbo.ChiTietTonKho (MaLoHang, MaSanPham, SoLuongTan, HanSuDung, GiaTriSanPham, TheTich) VALUES (?, ?, ?, ?, ?, ?)";

    std::vector<SanPhamTrongLo> dsSP = this->getDanhSachSanPham();
    std::string maLo = this->getMaLoHang();

    int countSuccess = 0;

    for (const auto& item : dsSP) {
        if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
            SQLPrepareA(hStmt, (SQLCHAR*)sqlChiTiet.c_str(), SQL_NTS);

            std::string maSP = item.sanPham.getMaSP();
            float sl = item.soLuong;
            float giaTri = item.giaTriSP;
            float theTich = item.thetich;

            // [FIX QUAN TRỌNG] Sửa kích thước từ 50 lên 255 theo hình ảnh DB
            SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLPOINTER)maLo.c_str(), 0, NULL);
            SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLPOINTER)maSP.c_str(), 0, NULL);

            SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_FLOAT, 0, 0, &sl, 0, NULL);

            // Xử lý Date
            SQL_DATE_STRUCT sqlDate;
            sqlDate.year = item.hsd.getYear();
            sqlDate.month = item.hsd.getMonth();
            sqlDate.day = item.hsd.getDay();
            SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_TYPE_DATE, SQL_TYPE_DATE, 0, 0, &sqlDate, 0, NULL);

            SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_DECIMAL, 18, 2, &giaTri, 0, NULL);
            SQLBindParameter(hStmt, 6, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_FLOAT, 0, 0, &theTich, 0, NULL);

            if (SQLExecute(hStmt) == SQL_SUCCESS) {
                countSuccess++;
            }
            else {
                cout << "!! Loi khi luu chi tiet SP (TonKho.cpp): " << maSP << endl;
            }
            SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        }
    }
    cout << ">> Da luu chi tiet " << countSuccess << " san pham vao kho.\n";
}
void TonKho::DongBoTongTonKho(SQLHDBC hDbc) {
    SQLHSTMT hStmt;
    std::string sqlSync = "UPDATE dbo.TonKho SET SoLuongTanTong = (SELECT SUM(SoLuongTan) FROM dbo.ChiTietTonKho WHERE MaLoHang = ?) WHERE MaLoHang = ?";

    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
        SQLPrepareA(hStmt, (SQLCHAR*)sqlSync.c_str(), SQL_NTS);
        std::string maLo = this->getMaLoHang();
        SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)maLo.c_str(), 0, NULL);
        SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)maLo.c_str(), 0, NULL);

        if (SQLExecute(hStmt) == SQL_SUCCESS) {
            cout << ">> He thong da tu dong dong bo lai Tong So Luong Ton Kho.\n";
        }
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    }
}

void TonKho::CapNhatSoLuongChiTiet(SQLHDBC hDbc, std::string maSanPham, float soLuongMoi) {
    SQLHSTMT hStmt;
    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
        std::string sql = "UPDATE dbo.ChiTietTonKho SET SoLuongTan = ? WHERE MaLoHang = ? AND MaSanPham = ?";
        SQLPrepareA(hStmt, (SQLCHAR*)sql.c_str(), SQL_NTS);

        std::string maLo = this->getMaLoHang();
        SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_FLOAT, 0, 0, &soLuongMoi, 0, NULL);
        SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)maLo.c_str(), 0, NULL);
        SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)maSanPham.c_str(), 0, NULL);

        if (SQLExecute(hStmt) == SQL_SUCCESS) {
            cout << ">> Da cap nhat so luong SP " << maSanPham << " thanh " << soLuongMoi << " tan.\n";
            // Update RAM
            for (auto &item : this->getDanhSachSanPham()) {
                if (item.sanPham.getMaSP() == maSanPham) {
                    item.soLuong = soLuongMoi;
                    break;
                }
            }
        } else {
            cout << "!! Loi khi cap nhat chi tiet san pham.\n";
        }
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    }
    // Gọi đồng bộ sau khi update chi tiết
    DongBoTongTonKho(hDbc);
}

void TonKho::NhapKhoVaLuu(SQLHDBC hDbc) {
    // 1. Nhập thông tin cơ bản và lưu vào bảng LoHang
    LoHang::NhapInfo(hDbc); 

    // 2. Lưu vào bảng TonKho (Master)
    cout << ">> (Dong bo TonKho hoan tat thong qua SaoLuu)\n";
}

void TonKho::CapNhatSoLuongTon(SQLHDBC hDbc, float soLuongMoi) {
    // Hàm này update thẳng vào bảng Master (ít dùng hơn CapNhatSoLuongChiTiet)
    this->setSoLuongTan(soLuongMoi);
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    std::string sql = "UPDATE dbo.TonKho SET SoLuongTanTong = ? WHERE MaLoHang = ?";
    
    if (SQLPrepareA(hStmt, (SQLCHAR*)sql.c_str(), SQL_NTS) == SQL_SUCCESS) {
        std::string maLo = this->getMaLoHang();
        SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_FLOAT, 0, 0, &soLuongMoi, 0, NULL);
        SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)maLo.c_str(), 0, NULL);
        SQLExecute(hStmt);
        cout << ">> Cap nhat so luong ton kho (Tong) thanh cong.\n";
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

bool TonKho::LayThongTinTuDB(SQLHDBC hDbc, std::string maLoHangTraCuu) {
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    std::string sql = "SELECT SoLuongTanTong FROM dbo.TonKho WHERE MaLoHang = ?";
    
    SQLPrepareA(hStmt, (SQLCHAR*)sql.c_str(), SQL_NTS);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)maLoHangTraCuu.c_str(), 0, NULL);
    
    if (SQLExecute(hStmt) == SQL_SUCCESS) {
        float slTon = 0;
        SQLLEN cbData;
        if (SQLFetch(hStmt) == SQL_SUCCESS) {
            SQLGetData(hStmt, 1, SQL_C_FLOAT, &slTon, 0, &cbData);
            this->setMaLoHang(maLoHangTraCuu);
            this->setSoLuongTan(slTon);
            SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
            return true;
        }
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return false;
}

void TonKho::printInfo() const {
    LoHang::printInfo(); 
    cout << "--- TRANG THAI TON KHO ---\n";
    cout << "So luong thuc te dang giu trong kho: " << this->getSoLuongTan() << " tan" << endl;
    if (this->getSoLuongTan() <= 0) cout << ">> CANH BAO: Lo hang nay da het hang!\n";
    cout << "======================================\n";
}