#pragma once
#include <windows.h> 
#include <sql.h>
#include <sqlext.h>
#include "DataModels.h"
#include <msclr/marshal_cppstd.h>
#include <cmath>
#include <limits>
#include <vector>
#include <string>
#include <algorithm> 

namespace PBL2QuanLyKho {

    using namespace System;
    using namespace System::Collections::Generic;

    public ref class ImportOptimizer {
    public:
        // --- HÀM CHÍNH: TỐI ƯU HÓA (Đã áp dụng Batch Query) ---
        static List<PredictedLot^>^ OptimizeCart(
            SQLHDBC hDbc,
            List<ImportItem^>^ cart,
            String^ currentMaKho
        ) {
            // 1. [BATCH QUERY] Lấy toàn bộ NCC của cả giỏ hàng trong 1 lần gọi
            // Thay vì gọi FindBestSupplier n lần, ta gọi BatchFindSuppliers đúng 1 lần.
            Dictionary<String^, String^>^ supplierMap = BatchFindSuppliers(hDbc, cart);

            Dictionary<String^, PredictedLot^>^ mapLots = gcnew Dictionary<String^, PredictedLot^>();

            for each (ImportItem ^ item in cart) {
                // 2. Tra cứu NCC từ kết quả đã lấy (Không gọi SQL nữa -> Siêu nhanh)
                String^ maNCC = "UNKNOWN";

                // Chuẩn hóa key để tìm kiếm chính xác
                String^ keySP = item->MaSP->Trim();

                if (supplierMap->ContainsKey(keySP)) {
                    maNCC = supplierMap[keySP];
                }

                // 3. Gom nhóm theo NCC
                if (!mapLots->ContainsKey(maNCC)) {
                    PredictedLot^ lot = gcnew PredictedLot();
                    lot->MaNCC = maNCC;

                    // Chỉ truy vấn tên NCC nếu chưa có (Có thể tối ưu thêm batch ở đây nếu muốn)
                    if (maNCC == "UNKNOWN")
                        lot->TenNCC = L"Không rõ nguồn gốc";
                    else
                        lot->TenNCC = GetSupplierName(hDbc, maNCC);

                    mapLots->Add(maNCC, lot);
                }

                mapLots[maNCC]->Items->Add(item);
            }

            // 4. Tính toán vận chuyển cho từng lô
            List<PredictedLot^>^ result = gcnew List<PredictedLot^>();

            for each (auto kvp in mapLots) {
                PredictedLot^ lot = kvp.Value;

                // Bỏ qua lô lỗi
                if (lot->MaNCC == "UNKNOWN") {
                    lot->LyDoChon = L"Không tìm thấy NCC cung cấp";
                    lot->PhiVanChuyen = 0;
                    result->Add(lot);
                    continue;
                }

                double totalWeight = 0;
                for each (ImportItem ^ i in lot->Items)
                    totalWeight += i->SoLuong;

                CalculateBestRoute(hDbc, lot, totalWeight, currentMaKho);

                // Chỉ lấy các phương án khả thi
                if (lot->PhiVanChuyen > 0 && lot->PhiVanChuyen < 1e9)
                    result->Add(lot);
            }

            return result;
        }

    private:
        static std::string ToStdString(String^ s) {
            if (String::IsNullOrEmpty(s)) return "";
            msclr::interop::marshal_context context;
            return context.marshal_as<std::string>(s);
        }

        // --- KỸ THUẬT BATCH QUERY (GỘP NHIỀU QUERY THÀNH 1) ---
        // Input: Danh sách sản phẩm
        // Output: Dictionary<Mã SP, Mã NCC>
        static Dictionary<String^, String^>^ BatchFindSuppliers(SQLHDBC hDbc, List<ImportItem^>^ cart) {
            Dictionary<String^, String^>^ results = gcnew Dictionary<String^, String^>();
            if (cart == nullptr || cart->Count == 0) return results;
            if (hDbc == SQL_NULL_HDBC) return results;

            SQLHSTMT hStmt;
            if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) != SQL_SUCCESS) return results;

            // 1. Xây dựng câu SQL động: WHERE MaSanPham IN (?, ?, ?, ...)
            std::wstring sql = L"SELECT MaSanPham, MIN(MaNCC) FROM CungCapSanPham WHERE MaSanPham IN (";

            // Tạo chuỗi dấu hỏi (?,?,?)
            for (int i = 0; i < cart->Count; i++) {
                sql += (i == 0) ? L"?" : L", ?";
            }
            sql += L") GROUP BY MaSanPham"; //

            SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);

            // 2. Bind tham số (Cần giữ vùng nhớ buffer tồn tại đến khi Execute)
            // Ta dùng vector<vector<char>> để lưu trữ các chuỗi char* an toàn
            std::vector<std::vector<char>> paramBuffers(cart->Count, std::vector<char>(51));

            for (int i = 0; i < cart->Count; i++) {
                std::string sID = ToStdString(cart[i]->MaSP->Trim());

                // Copy string vào buffer
                strncpy_s(paramBuffers[i].data(), 51, sID.c_str(), _TRUNCATE);

                // Bind buffer đó vào dấu hỏi thứ i+1
                SQLBindParameter(hStmt, i + 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, paramBuffers[i].data(), 0, NULL);
            }

            // 3. Thực thi 1 lần duy nhất
            if (SQLExecute(hStmt) == SQL_SUCCESS) {
                SQLCHAR cSP[51], cNCC[51];
                while (SQLFetch(hStmt) == SQL_SUCCESS) {
                    SQLGetData(hStmt, 1, SQL_C_CHAR, cSP, 51, NULL);
                    SQLGetData(hStmt, 2, SQL_C_CHAR, cNCC, 51, NULL);

                    String^ key = gcnew String((char*)cSP);
                    String^ val = gcnew String((char*)cNCC);

                    // Lưu vào Map để dùng lại
                    if (!results->ContainsKey(key->Trim())) {
                        results->Add(key->Trim(), val->Trim());
                    }
                }
            }
            SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
            return results;
        }

        static String^ GetSupplierName(SQLHDBC hDbc, String^ maNCC) {
            String^ res = maNCC; // Mặc định trả về mã nếu không tìm thấy tên
            SQLHSTMT hStmt;
            if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
                std::wstring sql = L"SELECT TenNCC FROM NhaCungCap WHERE MaNCC = ?";
                SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);

                std::string sID = ToStdString(maNCC);
                SQLCHAR cID[51]; strcpy_s((char*)cID, 51, sID.c_str());
                SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cID, 0, NULL);

                if (SQLExecute(hStmt) == SQL_SUCCESS && SQLFetch(hStmt) == SQL_SUCCESS) {
                    SQLWCHAR wName[256] = { 0 };
                    SQLGetData(hStmt, 1, SQL_C_WCHAR, wName, 256, NULL);
                    res = gcnew String(wName);
                }
            }
            SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
            return res;
        }

        // Thay thế hàm cũ trong ImportLogic.h
        static void CalculateBestRoute(SQLHDBC hDbc, PredictedLot^ lot, double weight, String^ maKho) {
            // Mặc định gán giá trị lớn để tìm min
            lot->PhiVanChuyen = 9999999999.0;
            lot->LyDoChon = L"Chưa tìm thấy xe phù hợp";

            // Đảm bảo khối lượng tối thiểu để không bị lỗi chia cho 0
            double totalWeight = (weight > 0) ? weight : 0.1;

            SQLHSTMT hStmt;
            if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) != SQL_SUCCESS) return;

            // [SỬA ĐỔI QUAN TRỌNG]: Bỏ điều kiện WHERE SucChuaTan >= ?
            // Lấy thêm cột p.SucChuaTan để tính toán
            std::wstring sql = L"SELECT p.MaXe, p.LoaiXe, p.ChiPhiMoiChuyen, p.VanTocTB, p.SucChuaTan, t.ChiPhiTuyenCoDinh "
                L"FROM PhuongTien p "
                L"LEFT JOIN TuyenNhapHang t ON p.MaXe = t.MaXe AND t.MaNCC = ? AND t.MaKhoNhan = ?";

            SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);

            std::string sNCC = ToStdString(lot->MaNCC);
            std::string sKho = ToStdString(maKho);
            SQLCHAR cNCC[100], cKho[100];
            strcpy_s((char*)cNCC, 100, sNCC.c_str());
            strcpy_s((char*)cKho, 100, sKho.c_str());

            // Chỉ bind 2 tham số (Bỏ tham số weight vì đã xóa dấu ? thứ 3)
            SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cNCC, 0, NULL);
            SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cKho, 0, NULL);

            if (SQLExecute(hStmt) == SQL_SUCCESS) {
                SQLCHAR cMaXe[51];
                SQLWCHAR cLoaiXe[101];
                SQLDOUBLE dChiPhiXe = 0, dVanToc = 50, dSucChua = 0, dPhiTuyen = 0;
                SQLLEN lenPhiTuyen = 0;

                while (SQLFetch(hStmt) == SQL_SUCCESS) {
                    SQLGetData(hStmt, 1, SQL_C_CHAR, cMaXe, 51, NULL);
                    SQLGetData(hStmt, 2, SQL_C_WCHAR, cLoaiXe, 101, NULL);
                    SQLGetData(hStmt, 3, SQL_C_DOUBLE, &dChiPhiXe, 0, NULL);
                    SQLGetData(hStmt, 4, SQL_C_DOUBLE, &dVanToc, 0, NULL);
                    SQLGetData(hStmt, 5, SQL_C_DOUBLE, &dSucChua, 0, NULL); // Lấy sức chứa
                    SQLGetData(hStmt, 6, SQL_C_DOUBLE, &dPhiTuyen, 0, &lenPhiTuyen);

                    if (dSucChua <= 0) dSucChua = 1; // Tránh lỗi chia cho 0

                    // --- [LOGIC MỚI: TÍNH SỐ XE CẦN] ---
                    // Ví dụ: 15 tấn / xe 4 tấn = 3.75 -> Làm tròn lên là 4 xe
                    double soXeCan = std::ceil(totalWeight / dSucChua);

                    double finalCost = 0;
                    String^ method = "";

                    // Ưu tiên 1: Tuyến cố định (nếu có)
                    if (lenPhiTuyen != SQL_NULL_DATA && dPhiTuyen > 0) {
                        // Giả sử phí tuyến cố định là trọn gói cho 1 xe, thì nhân lên số xe
                        finalCost = dPhiTuyen * soXeCan;
                        method = L"Tuyến Cố Định (x" + soXeCan + L" xe)";
                    }
                    // Ưu tiên 2: Thuê ngoài
                    else {
                        // Tính theo công thức cũ của bạn nhưng nhân số lượng xe
                        double costPerTrip = (dChiPhiXe > 200000) ? dChiPhiXe : dChiPhiXe * 50;
                        finalCost = costPerTrip * soXeCan;
                        method = L"Thuê Xe Ngoài (x" + soXeCan + L" xe)";
                    }

                    // So sánh tìm phương án rẻ nhất
                    if (finalCost < lot->PhiVanChuyen) {
                        lot->PhiVanChuyen = finalCost;
                        lot->LyDoChon = gcnew String(cLoaiXe);
                        lot->MaXe = gcnew String((char*)cMaXe);
                        lot->GhiChuRoute = method;
                        lot->ThoiGianDuKien = 50.0 / ((dVanToc <= 0) ? 40 : dVanToc);
                    }
                }
            }
            SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        }
    };
}