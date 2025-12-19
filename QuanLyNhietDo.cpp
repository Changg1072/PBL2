#include "QuanLyNhietDo.h"
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <limits>
#include <iostream>

using namespace std;

// =============================================================================
//                        1. KHỞI TẠO (CONSTRUCTOR)
// =============================================================================

QuanLyNhietDo::QuanLyNhietDo(std::string ma) : maKho(ma) {
    // Giá trị mặc định phòng khi chưa kết nối được DB
    tenKho = "Unknown";
    minTemp = -20;
    maxTemp = 10;
    basePower = 500.0;
    tempFactor = 20.0;
    chiPhiGiamDo = 5000.0;
    chiPhiTangDo = 0.0;

    // Khởi tạo bảng giá điện theo khung giờ (Giờ bắt đầu, Giờ kết thúc, Giá)
    // Dữ liệu này thường cố định theo quy định nhà nước (hoặc có thể load từ DB nếu muốn)
    bangGiaDien.push_back({ 0, 3, 1146 });   // Thấp điểm đêm
    bangGiaDien.push_back({ 4, 9, 1811 });   // Bình thường sáng
    bangGiaDien.push_back({ 10, 11, 3266 }); // Cao điểm sáng
    bangGiaDien.push_back({ 12, 16, 1811 }); // Bình thường chiều
    bangGiaDien.push_back({ 17, 20, 3266 }); // Cao điểm tối
    bangGiaDien.push_back({ 21, 21, 1811 }); // Bình thường tối
    bangGiaDien.push_back({ 22, 23, 1146 }); // Thấp điểm đêm
}

// =============================================================================
//                        2. LOAD DỮ LIỆU TỪ SQL SERVER
// =============================================================================

void QuanLyNhietDo::LayDuLieuTuDB(SQLHDBC hDbc) {
    SQLHSTMT hStmt;

    // --- BƯỚC 1: Lấy Cấu hình KHO ---
    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
        // Query lấy thông số kỹ thuật của kho
        string sqlKho = "SELECT TenKho, NhietDoMin, NhietDoMax, CongSuatCoBan, HeSoCongSuat, ChiPhiHaNhiet, ChiPhiTangNhiet FROM Kho WHERE MaKho = ?";

        SQLPrepareA(hStmt, (SQLCHAR*)sqlKho.c_str(), SQL_NTS);
        SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)maKho.c_str(), 0, NULL);

        if (SQLExecute(hStmt) == SQL_SUCCESS) {
            if (SQLFetch(hStmt) == SQL_SUCCESS) {
                char sTen[256];
                double dMin, dMax, dBase, dFactor, dCostDown, dCostUp;
                SQLLEN len1, len2, len3, len4, len5, len6, len7;

                SQLGetData(hStmt, 1, SQL_C_CHAR, sTen, sizeof(sTen), &len1);
                SQLGetData(hStmt, 2, SQL_C_DOUBLE, &dMin, 0, &len2);
                SQLGetData(hStmt, 3, SQL_C_DOUBLE, &dMax, 0, &len3);
                SQLGetData(hStmt, 4, SQL_C_DOUBLE, &dBase, 0, &len4);
                SQLGetData(hStmt, 5, SQL_C_DOUBLE, &dFactor, 0, &len5);
                SQLGetData(hStmt, 6, SQL_C_DOUBLE, &dCostDown, 0, &len6);
                SQLGetData(hStmt, 7, SQL_C_DOUBLE, &dCostUp, 0, &len7);

                tenKho = (len1 == SQL_NULL_DATA) ? "Unknown" : sTen;
                minTemp = (len2 == SQL_NULL_DATA) ? -20 : (int)dMin;
                maxTemp = (len3 == SQL_NULL_DATA) ? 10 : (int)dMax;
                basePower = (len4 == SQL_NULL_DATA) ? 500 : dBase;
                tempFactor = (len5 == SQL_NULL_DATA) ? 20 : dFactor;
                chiPhiGiamDo = (len6 == SQL_NULL_DATA) ? 5000 : dCostDown;
                chiPhiTangDo = (len7 == SQL_NULL_DATA) ? 0 : dCostUp;
            }
        }
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    }

    // --- BƯỚC 2: Lấy Danh sách Hàng hóa đang tồn ---
    dsSanPham.clear();
    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
        // [FIX QUAN TRỌNG] 
        // Query này lấy dữ liệu từ ChiTietTonKho. 
        // Vì bạn đã thêm cột MaLoHang vào ChiTietTonKho, ta JOIN trực tiếp qua MaLoHang để chính xác hơn.
        string sqlSP = R"(
        SELECT 
            SP.TenSanPham, 
            SP.NhietDoThichHop, 
            ISNULL(SP.RuiRoLanh, 0.01) as RuiRoLanh, 
            ISNULL(SP.RuiRoNong, 0.02) as RuiRoNong,
            SUM(CT.GiaTriSanPham) as TongGiaTriBaoQuan
        FROM ChiTietTonKho CT
        JOIN TonKho TK ON CT.MaLoHang = TK.MaLoHang 
        JOIN SanPham SP ON CT.MaSanPham = SP.MaSanPham
        WHERE LTRIM(RTRIM(TK.MaKho)) = ?   
        GROUP BY SP.TenSanPham, SP.NhietDoThichHop, SP.RuiRoLanh, SP.RuiRoNong
            HAVING SUM(CT.GiaTriSanPham) > 0
        )";
        // Lưu ý: Nếu trong DB cũ, cột CT.MaLoHang đang là NULL thì dòng này sẽ KHÔNG trả về kết quả.
        // Bạn cần Nhập Lại Hàng Hóa Mới để code chạy đúng.

        SQLPrepareA(hStmt, (SQLCHAR*)sqlSP.c_str(), SQL_NTS);
        SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)maKho.c_str(), 0, NULL);

        if (SQLExecute(hStmt) == SQL_SUCCESS) {
            char sTenSP[256];
            double dTempIdeal, dRLanh, dRNong, dTongGiaTri;
            SQLLEN len;

            while (SQLFetch(hStmt) == SQL_SUCCESS) {
                SQLGetData(hStmt, 1, SQL_C_CHAR, sTenSP, sizeof(sTenSP), &len);
                SQLGetData(hStmt, 2, SQL_C_DOUBLE, &dTempIdeal, 0, &len);
                SQLGetData(hStmt, 3, SQL_C_DOUBLE, &dRLanh, 0, &len);
                SQLGetData(hStmt, 4, SQL_C_DOUBLE, &dRNong, 0, &len);
                SQLGetData(hStmt, 5, SQL_C_DOUBLE, &dTongGiaTri, 0, &len);

                SanPhamLuuKho sp;
                sp.tenSP = sTenSP;
                sp.nhietDoThichHop = (int)dTempIdeal;
                sp.ruiRoLanh = dRLanh;
                sp.ruiRoNong = dRNong;
                sp.tongGiaTri = dTongGiaTri;

                dsSanPham.push_back(sp);
            }
        }
        else {
            cout << "Khong lay duoc danh sach san pham. Hay kiem tra lai du lieu Ton Kho!" << endl;
        }
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    }
}

// =============================================================================
//                        3. CÁC HÀM TÍNH TOÁN CHI PHÍ
// =============================================================================

double QuanLyNhietDo::TinhCongSuat(int T) {
    // Công thức: P = P_base - (HeSo * T)
    // Nếu T càng lạnh (âm nhiều), công suất càng cao.
    double p = basePower - (tempFactor * T);
    return (p > 0) ? p : 0; // Không thể âm công suất
}

double QuanLyNhietDo::LayGiaDien(int gio) {
    for (const auto& kg : bangGiaDien) {
        if (gio >= kg.gioBatDau && gio <= kg.gioKetThuc) {
            return kg.giaDien;
        }
    }
    return 1811.0; // Giá mặc định nếu không khớp
}

double QuanLyNhietDo::TinhChiPhiRuiRo(int T) {
    double tongRuiRo = 0;
    for (const auto& sp : dsSanPham) {
        double thietHai = 0;
        // Nếu nhiệt độ kho LẠNH HƠN nhiệt độ chuẩn của SP
        if (T < sp.nhietDoThichHop) {
            double doLech = std::abs(T - sp.nhietDoThichHop);
            thietHai = doLech * sp.ruiRoLanh * sp.tongGiaTri;
        }
        // Nếu nhiệt độ kho NÓNG HƠN nhiệt độ chuẩn của SP
        else if (T > sp.nhietDoThichHop) {
            double doLech = std::abs(T - sp.nhietDoThichHop);
            thietHai = doLech * sp.ruiRoNong * sp.tongGiaTri;
        }
        tongRuiRo += thietHai;
    }
    return tongRuiRo;
}

double QuanLyNhietDo::TinhCostRun(int gio, int T) {
    double congSuat = TinhCongSuat(T);
    double giaDien = LayGiaDien(gio);
    double ruiRo = TinhChiPhiRuiRo(T);

    // Chi phí 1 giờ = (Công suất kW * Giá điện/kWh) + Rủi ro hỏng hàng
    return (congSuat * giaDien) + ruiRo;
}

double QuanLyNhietDo::TinhCostSwitch(int T_current, int T_next) {
    // Chi phí chuyển đổi nhiệt độ
    if (T_next < T_current) {
        // Kéo nhiệt độ xuống (Làm lạnh thêm) -> Tốn kém
        return (T_current - T_next) * chiPhiGiamDo;
    }
    else if (T_next > T_current) {
        // Thả nhiệt độ lên (Tắt máy/Giảm tải) -> Ít tốn kém hoặc miễn phí
        return (T_next - T_current) * chiPhiTangDo;
    }
    return 0.0;
}

// =============================================================================
//                        4. THUẬT TOÁN QUY HOẠCH ĐỘNG (DP)
// =============================================================================

double QuanLyNhietDo::GiaiBaiToanToiUu(int gioBatDau, int gioKetThuc) {
    lichTrinhToiUu.clear(); // Xóa kết quả cũ

    // Nếu không có hàng hóa thì chi phí bằng 0
    if (dsSanPham.empty()) return 0.0;

    int soGio = gioKetThuc - gioBatDau + 1;
    // Kích thước mảng nhiệt độ
    int rangeT = maxTemp - minTemp + 1;

    vector<vector<double>> dp(soGio + 2, vector<double>(rangeT, 1e18));

    vector<vector<int>> trace(soGio + 1, vector<int>(rangeT, 0));

    int stepCuoi = soGio;
    int realHourEnd = gioKetThuc;

    for (int T = minTemp; T <= maxTemp; ++T) {
        int idx = T - minTemp;
        dp[stepCuoi][idx] = TinhCostRun(realHourEnd, T);
    }

    for (int t = soGio - 1; t >= 1; --t) {
        int realHour = gioBatDau + t - 1;

        for (int T_curr = minTemp; T_curr <= maxTemp; ++T_curr) {
            int idxCurr = T_curr - minTemp;

            int low = std::max(minTemp, T_curr - 5);
            int high = std::min(maxTemp, T_curr + 5);

            // Duyệt qua các nhiệt độ tiếp theo có thể xảy ra (T_next)
            for (int T_next = low; T_next <= high; ++T_next) {
                int idxNext = T_next - minTemp;

                double val = TinhCostRun(realHour, T_curr)
                    + TinhCostSwitch(T_curr, T_next)
                    + dp[t + 1][idxNext];

                // Cập nhật Min
                if (val < dp[t][idxCurr]) {
                    dp[t][idxCurr] = val;
                    trace[t][idxCurr] = T_next; // Lưu vết: Từ T_curr nên đi tới T_next
                }
            }
        }
    }

    // --- BƯỚC 3: TÌM ĐIỂM KHỞI ĐẦU TỐT NHẤT ---
    double minTotalCost = 1e18;
    int currT = minTemp;

    // Tại giờ đầu tiên (t=1), chọn nhiệt độ nào có tổng chi phí thấp nhất
    for (int T = minTemp; T <= maxTemp; ++T) {
        int idx = T - minTemp;
        if (dp[1][idx] < minTotalCost) {
            minTotalCost = dp[1][idx];
            currT = T;
        }
    }

    // --- BƯỚC 4: TRUY VẾT LẠI ĐỂ RA LỊCH TRÌNH (Reconstruct Path) ---
    for (int t = 1; t <= soGio; ++t) {
        int realHour = gioBatDau + t - 1;

        KetQuaLichTrinh kq;
        kq.gio = realHour;
        kq.nhietDo = currT;
        kq.giaDien = LayGiaDien(realHour);
        double congSuat = TinhCongSuat(currT);
        kq.chiPhiVanHanh = congSuat * kq.giaDien;
        kq.chiPhiRuiRo = TinhChiPhiRuiRo(currT);
        lichTrinhToiUu.push_back(kq);
        if (t < soGio) {
            int idx = currT - minTemp;
            currT = trace[t][idx];
        }
    }

    return minTotalCost;
}