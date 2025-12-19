#include "QuanLyVanChuyen.h"
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <cmath>
#include <string>
#include <vector>
#include <ctime>
#include <random>
#include <limits> 
#include <cctype>

using namespace std;

namespace PBL2QuanLyKho {

    // =================================================================================
    //                           PHẦN 1: HÀM HỖ TRỢ XỬ LÝ CHUỖI & ĐỊA LÝ
    // =================================================================================

    // Xử lý chuỗi: Chỉ xóa khoảng trắng thừa, giữ nguyên dấu Tiếng Việt
    std::string CleanString(const std::string& str) {
        if (str.empty()) return "";
        size_t first = str.find_first_not_of(" \t\r\n");
        if (std::string::npos == first) return "";
        size_t last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, (last - first + 1));
    }

    std::string ToLowerAscii(std::string str) {
        for (char& c : str) if (c >= 'A' && c <= 'Z') c += 32;
        return str;
    }

    // So sánh địa chỉ kiểu "Chứa" (Contains) để bắt được "Đà Nẵng" trong "TP Đà Nẵng"
    bool SoSanhDiaChi(std::string dataGoc, std::string dataSoSanh) {
        std::string s1 = ToLowerAscii(CleanString(dataGoc));
        std::string s2 = ToLowerAscii(CleanString(dataSoSanh));
        if (s1.empty() || s2.empty()) return false;

        // Logic: Nếu chuỗi này chứa chuỗi kia -> Trùng
        if (s1.find(s2) != std::string::npos) return true;
        if (s2.find(s1) != std::string::npos) return true;
        return false;
    }

    std::string sinhMaDonVC() {
        std::string charset = "0123456789";
        random_device rd; mt19937 gen(rd());
        uniform_int_distribution<> dis(0, charset.size() - 1);
        std::string ma = "DVC";
        for (int i = 0; i < 6; ++i) ma += charset[dis(gen)];
        return ma;
    }

    // --- LOGIC ĐỊA LÝ ---
    enum MoiQuanHeDiaLy { CUNG_HUYEN, KHAC_HUYEN, KHAC_TINH };

    MoiQuanHeDiaLy XacDinhQuanHeDiaLy(std::string tinhA, std::string huyenA, std::string tinhB, std::string huyenB) {
        if (tinhA.empty() || tinhB.empty()) return KHAC_TINH; // Mặc định xa nếu thiếu data
        if (!SoSanhDiaChi(tinhA, tinhB)) return KHAC_TINH;
        if (huyenA.empty() || huyenB.empty()) return KHAC_HUYEN;
        if (!SoSanhDiaChi(huyenA, huyenB)) return KHAC_HUYEN;
        return CUNG_HUYEN;
    }

    // Khoảng cách ước lượng (Km)
    float UocLuongKhoangCach(MoiQuanHeDiaLy mqh) {
        switch (mqh) {
        case CUNG_HUYEN: return 15.0f;
        case KHAC_HUYEN: return 50.0f;
        case KHAC_TINH: return 250.0f;
        default: return 250.0f;
        }
    }

    void TruTruHangTonKho(SQLHDBC hDbc, std::string maKho, std::string maSP, float soLuongCanGiao) {}

    void QuanLyVanChuyen::CapNhatTrangThaiYeuCau(SQLHDBC hDbc, std::string maYeuCau, std::string trangThai) {
        if (maYeuCau.empty()) return;
        SQLHSTMT hStmt;
        if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
            std::string sql = "UPDATE ChiTietYeuCau SET HoanThanh = ? WHERE MaYeuCau = ?";
            SQLPrepareA(hStmt, (SQLCHAR*)sql.c_str(), SQL_NTS);
            SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)trangThai.c_str(), 0, NULL);
            SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)maYeuCau.c_str(), 0, NULL);
            SQLExecute(hStmt);
            SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        }
    }

    // =================================================================================
    //                  PHẦN 2: THUẬT TOÁN TỐI ƯU HÓA (CORE)
    // =================================================================================

    // --- CASE 1: CÓ TUYẾN CỐ ĐỊNH ---
    KetQuaSoSanh QuanLyVanChuyen::TimHaiPhuongAn(
        float tongTrongLuongCan,
        const SieuThi& stDich,
        const std::vector<Kho>& dsKho,
        const std::vector<PhuongTien>& dsXe,
        const std::vector<TuyenVanChuyen>& dsTuyenVC)
    {
        KetQuaSoSanh kq;
        kq.paReNhat.tongChiPhi = std::numeric_limits<float>::max();
        kq.paNhanhNhat.tongThoiGian = std::numeric_limits<float>::max();

        for (const auto& kho : dsKho) {
            MoiQuanHeDiaLy mqh = XacDinhQuanHeDiaLy(kho.getTinh(), kho.getQuanHuyen(), stDich.getTinh(), stDich.getQuanHuyen());
            float dist = UocLuongKhoangCach(mqh);

            for (const auto& xe : dsXe) {
                if (xe.getSucChua() <= 0) continue;

                const TuyenVanChuyen* tuyenPhuHop = nullptr;
                for (const auto& t : dsTuyenVC) {
                    if (SoSanhDiaChi(t.getKhoDi(), kho.getMaKho()) &&
                        SoSanhDiaChi(t.getSieuThiNhan(), stDich.getMaSieuThi()) &&
                        SoSanhDiaChi(t.getMaXe(), xe.getMaXe())) {
                        tuyenPhuHop = &t;
                        break;
                    }
                }
                if (tuyenPhuHop == nullptr) continue;

                // --- TÍNH TOÁN ---
                int soChuyen = (int)std::ceil(tongTrongLuongCan / xe.getSucChua());

                // [FIX 2] GIẢM CHI PHÍ NHIÊN LIỆU (2.000đ/km thay vì 20.000đ)
                double chiPhiXang = (double)dist * 2000.0;

                double donGiaPhu = (mqh == KHAC_TINH) ? xe.getPhuPhiTinh() :
                    ((mqh == KHAC_HUYEN) ? xe.getPhuPhiQuan() : 0);
                double chiPhiPhu = (double)tongTrongLuongCan * donGiaPhu;

                double phiCoDinh1Chuyen = tuyenPhuHop->getChiPhiCoDinh() + xe.getChiPhi() + chiPhiXang;
                double tongChiPhi = (soChuyen * phiCoDinh1Chuyen) + chiPhiPhu;

                // Thời gian
                double vanToc = (xe.getVanTocTB() > 0) ? xe.getVanTocTB() : 40.0;
                double tDiChuyen = dist / vanToc;
                double tBocDo = (xe.getLrate() + xe.getUrate()) * xe.getSucChua();
                double thoiGian1Chuyen = xe.getTsetup() + tBocDo + tDiChuyen;

                int pool = (xe.getSoXe() > 0) ? xe.getSoXe() : 1;
                double tongThoiGian = ((double)soChuyen * thoiGian1Chuyen) / (double)pool;

                // Update
                PhuongAnToiUu pa;
                pa.maKhoXuat = kho.getMaKho(); pa.tenKhoXuat = kho.getTenKho();
                pa.maKhoTrungChuyen = tuyenPhuHop->getKhoTrungChuyen();
                pa.maPhuongTien = xe.getMaXe(); pa.tenLoaiXe = xe.getLoaiXe();
                pa.soChuyen = soChuyen; pa.khoangCach = dist;
                pa.tongChiPhi = (float)tongChiPhi; pa.tongThoiGian = (float)tongThoiGian;
                pa.ghiChu = "Tuyen Co Dinh";

                if (pa.tongChiPhi < kq.paReNhat.tongChiPhi) kq.paReNhat = pa;
                if (pa.tongThoiGian < kq.paNhanhNhat.tongThoiGian) kq.paNhanhNhat = pa;
            }
        }

        // Điều chỉnh giá Nhanh
        if (kq.paReNhat.tongChiPhi > 0) {
            if (kq.paNhanhNhat.tongChiPhi <= kq.paReNhat.tongChiPhi) {
                kq.paNhanhNhat.tongChiPhi = kq.paReNhat.tongChiPhi * 1.2f;
                kq.paNhanhNhat.ghiChu += " (Uu tien)";
            }
        }
        if (kq.paReNhat.soChuyen == 0) { kq.paReNhat.tongChiPhi = 0; kq.paNhanhNhat.tongChiPhi = 0; }
        return kq;
    }

    // --- CASE 2: GIẢ ĐỊNH ---
    KetQuaSoSanh QuanLyVanChuyen::TimPhuongAnGiaDinh(
        float tongTrongLuong,
        const SieuThi& stDich,
        const std::vector<Kho>& dsKho,
        const std::vector<PhuongTien>& dsXe)
    {
        KetQuaSoSanh kq;
        kq.paReNhat.tongChiPhi = std::numeric_limits<float>::max();
        kq.paNhanhNhat.tongThoiGian = std::numeric_limits<float>::max();

        if (dsKho.empty()) return kq;

        // B1: Tìm Kho Gần Nhất
        const Kho* khoGanNhat = nullptr;
        float minDist = std::numeric_limits<float>::max();
        MoiQuanHeDiaLy mqhTotNhat = KHAC_TINH;

        for (const auto& kho : dsKho) {
            MoiQuanHeDiaLy mqh = XacDinhQuanHeDiaLy(kho.getTinh(), kho.getQuanHuyen(), stDich.getTinh(), stDich.getQuanHuyen());
            float dist = UocLuongKhoangCach(mqh);
            if (dist < minDist) { minDist = dist; khoGanNhat = &kho; mqhTotNhat = mqh; }
        }
        if (khoGanNhat == nullptr) { khoGanNhat = &dsKho[0]; minDist = 50.0f; }

        // B2: Tính Toán
        std::vector<PhuongTien> dsXeTinh = dsXe;
        if (dsXeTinh.empty()) dsXeTinh.push_back(PhuongTien("XE_AO", "Xe Tai Nho", 5, 2.0f, 150000.0f, "", "", "", "", 20000.0f, 50000.0f, 40.0f, 8.0f, 0.2f, 0.2f, 0.2f));

        for (const auto& xe : dsXeTinh) {
            if (xe.getSucChua() <= 0) continue;
            int soChuyen = (int)std::ceil(tongTrongLuong / xe.getSucChua());

            // [FIX 3] GIẢM GIÁ NHIÊN LIỆU (Xe tải nhỏ/Ship): 1.500đ/km
            double chiPhiXang = (double)minDist * 1500.0;

            double donGiaPhu = (mqhTotNhat == KHAC_TINH) ? xe.getPhuPhiTinh() :
                ((mqhTotNhat == KHAC_HUYEN) ? xe.getPhuPhiQuan() : 0);
            double chiPhiPhu = (double)tongTrongLuong * donGiaPhu;

            // [FIX 4] CHẾ ĐỘ GHÉP ĐƠN (LTL - Less Than Truckload)
            // Nếu hàng ít (< 20% xe), chỉ tính 30% phí mở cửa xe (coi như đi ghép)
            double heSoGhep = 1.0;
            if (tongTrongLuong < (xe.getSucChua() * 0.2)) heSoGhep = 0.3;

            // TỔNG TIỀN = (Số chuyến * (Phí Mở Cửa * Hệ số + Xăng)) + Phí Phụ
            double phiMoCuaThucTe = xe.getChiPhi() * heSoGhep;
            double tongChiPhi = (soChuyen * (phiMoCuaThucTe + chiPhiXang)) + chiPhiPhu;

            // Thời Gian
            double vanToc = (xe.getVanTocTB() > 0) ? xe.getVanTocTB() : 40.0;
            double tDi = minDist / vanToc;
            double tBocDo = (xe.getLrate() + xe.getUrate()) * xe.getSucChua(); // Bốc full xe tốn tgian
            double t1Chuyen = xe.getTsetup() + tBocDo + tDi;
            int pool = (xe.getSoXe() > 0) ? xe.getSoXe() : 1;
            double tongThoiGian = ((double)soChuyen * t1Chuyen) / (double)pool;

            // Tạo PA
            PhuongAnToiUu pa;
            pa.maKhoXuat = khoGanNhat->getMaKho();
            pa.tenKhoXuat = khoGanNhat->getTenKho();
            pa.maPhuongTien = xe.getMaXe();
            pa.tenLoaiXe = xe.getLoaiXe();
            pa.soChuyen = soChuyen;
            pa.khoangCach = minDist;
            pa.tongChiPhi = (float)tongChiPhi;
            pa.tongThoiGian = (float)tongThoiGian;
            pa.ghiChu = "Gia dinh (Tu dong)";

            if (pa.tongChiPhi < kq.paReNhat.tongChiPhi) kq.paReNhat = pa;
            if (pa.tongThoiGian < kq.paNhanhNhat.tongThoiGian) kq.paNhanhNhat = pa;
        }

        // Fix giá Nhanh
        if (kq.paReNhat.tongChiPhi > 0) {
            if (kq.paNhanhNhat.tongChiPhi <= kq.paReNhat.tongChiPhi) {
                kq.paNhanhNhat.tongChiPhi = kq.paReNhat.tongChiPhi * 1.25f; // Đắt hơn 25%
                kq.paNhanhNhat.ghiChu += " (Express)";
            }
        }
        if (kq.paReNhat.soChuyen == 0) { kq.paReNhat.tongChiPhi = 0; kq.paNhanhNhat.tongChiPhi = 0; }

        return kq;
    }
    void TuDongTaoLoAo(SQLHDBC hDbc, std::string maKho) {
        SQLHSTMT hStmt = SQL_NULL_HSTMT; // Luôn khởi tạo NULL để an toàn

        // 1. TẠO NHÀ CUNG CẤP ẢO
        if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
            std::string sqlNCC =
                "IF NOT EXISTS (SELECT * FROM NhaCungCap WHERE MaNCC = 'NCC_AO') "
                "BEGIN "
                "   INSERT INTO NhaCungCap (MaNCC, TenNCC, DiaChi, SDT, Email) "
                "   VALUES ('NCC_AO', N'Nhà Cung Cấp Ảo', N'Hệ Thống', '0000000000', 'system@ao.com') "
                "END";
            SQLExecDirectA(hStmt, (SQLCHAR*)sqlNCC.c_str(), SQL_NTS);
            SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        }

        // 2. TẠO LÔ HÀNG ẢO
        if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
            std::string sqlLo =
                "IF NOT EXISTS (SELECT * FROM LoHang WHERE MaLoHang = 'LO_AO') "
                "BEGIN "
                "   INSERT INTO LoHang (MaLoHang, MaKho, MaNCC, NgayNhap, TrangThai) "
                "   VALUES ('LO_AO', ?, 'NCC_AO', GETDATE(), 'DaKiemDinh') "
                "END";

            SQLPrepareA(hStmt, (SQLCHAR*)sqlLo.c_str(), SQL_NTS);

            std::string sKho = CleanString(maKho);
            // [FIX] Khai báo biến độ dài cho chuỗi
            SQLLEN lenNTS = SQL_NTS;

            // [FIX] Truyền &lenNTS thay vì NULL vào cuối
            SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)sKho.c_str(), 0, &lenNTS);

            SQLExecute(hStmt);
            SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        }
    }
    void QuanLyVanChuyen::LuuDonHangVaoDB(SQLHDBC hDbc, const PhuongAnToiUu& pa, const std::string& maSieuThi, float tongKhoiLuong, const std::vector<HangCanGiao>& dsHang) {
        SQLHSTMT hStmt = SQL_NULL_HSTMT;
        SQLHSTMT hStmtBatch = SQL_NULL_HSTMT;

        bool isVirtualOrder = (pa.ghiChu.find("Gia dinh") != std::string::npos);

        // Tắt AutoCommit
        SQLSetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, 0);

        try {
            // ---------------------------------------------------------
            // BƯỚC 1: TẠO HEADER (DonVanChuyen)
            // ---------------------------------------------------------
            std::string maDonVC = sinhMaDonVC();
            std::string cMaDon = CleanString(maDonVC);

            if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
                std::string sql = "INSERT INTO DonVanChuyen (MaDonVC, MaXe, MaKho, MaSieuThi, TongKhoiLuong, TongChiPhi, ThoiGianDuKien, NgayTao, TrangThai, MaYeuCauGoc) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
                SQLPrepareA(hStmt, (SQLCHAR*)sql.c_str(), SQL_NTS);

                std::string cMaXe = CleanString(pa.maPhuongTien);
                std::string cMaKho = CleanString(pa.maKhoXuat);
                std::string cMaST = CleanString(maSieuThi); 
                std::wstring wTrangThai = L"Đang Chờ Xử Lý";

                time_t now = time(0); tm* ltm = localtime(&now);
                char sNgayTao[20]; sprintf_s(sNgayTao, 20, "%04d-%02d-%02d", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday);

                SQLDOUBLE dKL = tongKhoiLuong;
                SQLDOUBLE dChiPhi = pa.tongChiPhi;
                SQLDOUBLE dThoiGian = pa.tongThoiGian;

                // [FIX] Biến độ dài cho chuỗi (Bắt buộc phải có địa chỉ bộ nhớ hợp lệ)
                SQLLEN lenNTS = SQL_NTS;

                // [FIX] Thay thế NULL bằng &lenNTS ở tham số cuối cùng
                SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)cMaDon.c_str(), 0, &lenNTS);
                SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)cMaXe.c_str(), 0, &lenNTS);
                SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)cMaKho.c_str(), 0, &lenNTS);
                SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)cMaST.c_str(), 0, &lenNTS);

                SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dKL, 0, NULL);
                SQLBindParameter(hStmt, 6, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 0, &dChiPhi, 0, NULL);
                SQLBindParameter(hStmt, 7, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_FLOAT, 0, 0, &dThoiGian, 0, NULL);

                SQLBindParameter(hStmt, 8, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 20, 0, (SQLPOINTER)sNgayTao, 0, &lenNTS);
                SQLBindParameter(hStmt, 9, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 50, 0, (SQLPOINTER)wTrangThai.c_str(), 0, NULL);
                std::string sMaYeuCau = "";
                if (!dsHang.empty()) {
                    sMaYeuCau = CleanString(dsHang[0].maYeuCauGoc);
                }
                SQLBindParameter(hStmt, 10, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)sMaYeuCau.c_str(), 0, &lenNTS);
                if (SQLExecute(hStmt) != SQL_SUCCESS) throw runtime_error("Loi INSERT DonVanChuyen");
                SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
            }

            // ---------------------------------------------------------
            // BƯỚC 2: XỬ LÝ CHI TIẾT
            // ---------------------------------------------------------

            if (isVirtualOrder) {
                // === TRƯỜNG HỢP 1: ĐƠN GIẢ ĐỊNH (KHO ẢO) ===
                TuDongTaoLoAo(hDbc, pa.maKhoXuat);

                std::string virtualBatch = "LO_AO";

                for (const auto& h : dsHang) {
                    std::string sSP = CleanString(h.maSP);
                    double soLuong = h.soLuong;
                    double thanhTien = h.thanhTien;

                    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
                        std::string sqlDetail = "INSERT INTO ChiTietDonVC (MaDonVC, MaSanPham, MaLoHang, SoLuong, ThanhTien) VALUES (?, ?, ?, ?, ?)";
                        SQLPrepareA(hStmt, (SQLCHAR*)sqlDetail.c_str(), SQL_NTS);

                        SQLLEN lenNTS = SQL_NTS; // [FIX]

                        // [FIX] Truyền &lenNTS vào tất cả tham số chuỗi
                        SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)cMaDon.c_str(), 0, &lenNTS);
                        SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)sSP.c_str(), 0, &lenNTS);
                        SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)virtualBatch.c_str(), 0, &lenNTS);

                        SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &soLuong, 0, NULL);
                        SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 0, &thanhTien, 0, NULL);

                        SQLExecute(hStmt);
                        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
                    }
                    CapNhatTrangThaiYeuCau(hDbc, h.maYeuCauGoc, "Đang Chờ Xử Lý");
                }
            }
            else {
                // === TRƯỜNG HỢP 2: ĐƠN THẬT (FEFO) ===
                for (const auto& h : dsHang) {
                    double soLuongCan = h.soLuong;
                    double donGia = (h.soLuong > 0) ? (h.thanhTien / h.soLuong) : 0;
                    std::string sSP = CleanString(h.maSP);

                    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmtBatch) == SQL_SUCCESS) {
                        std::string sqlFindBatch = "SELECT c.MaLoHang, c.SoLuongTan FROM ChiTietTonKho c "
                            "JOIN TonKho t ON c.MaLoHang = t.MaLoHang "
                            "WHERE t.MaKho = ? AND c.MaSanPham = ? AND c.SoLuongTan > 0 "
                            "ORDER BY c.HanSuDung ASC";

                        SQLPrepareA(hStmtBatch, (SQLCHAR*)sqlFindBatch.c_str(), SQL_NTS);
                        std::string sKho = CleanString(pa.maKhoXuat);

                        SQLLEN lenNTS = SQL_NTS; // [FIX]
                        SQLBindParameter(hStmtBatch, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)sKho.c_str(), 0, &lenNTS);
                        SQLBindParameter(hStmtBatch, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)sSP.c_str(), 0, &lenNTS);

                        if (SQLExecute(hStmtBatch) == SQL_SUCCESS) {
                            SQLCHAR sMaLo[51];
                            SQLDOUBLE dTonKho;

                            while (soLuongCan > 0 && SQLFetch(hStmtBatch) == SQL_SUCCESS) {
                                SQLGetData(hStmtBatch, 1, SQL_C_CHAR, sMaLo, 50, NULL);
                                SQLGetData(hStmtBatch, 2, SQL_C_DOUBLE, &dTonKho, 0, NULL);

                                std::string currentMaLo = CleanString((char*)sMaLo);
                                double layRa = (dTonKho >= soLuongCan) ? soLuongCan : dTonKho;
                                double thanhTienRow = layRa * donGia;

                                // Insert Chi Tiết
                                if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
                                    std::string sqlDetail = "INSERT INTO ChiTietDonVC (MaDonVC, MaSanPham, MaLoHang, SoLuong, ThanhTien) VALUES (?, ?, ?, ?, ?)";
                                    SQLPrepareA(hStmt, (SQLCHAR*)sqlDetail.c_str(), SQL_NTS);

                                    SQLLEN lenNTS_Inner = SQL_NTS; // [FIX]

                                    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)cMaDon.c_str(), 0, &lenNTS_Inner);
                                    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)sSP.c_str(), 0, &lenNTS_Inner);
                                    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)currentMaLo.c_str(), 0, &lenNTS_Inner);

                                    SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &layRa, 0, NULL);
                                    SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 0, &thanhTienRow, 0, NULL);

                                    SQLExecute(hStmt);
                                    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
                                }

                                // Trừ Kho
                                if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
                                    std::string sqlUpdateKho = "UPDATE ChiTietTonKho SET SoLuongTan = SoLuongTan - ? WHERE MaLoHang = ? AND MaSanPham = ?";
                                    SQLPrepareA(hStmt, (SQLCHAR*)sqlUpdateKho.c_str(), SQL_NTS);

                                    SQLLEN lenNTS_Upd = SQL_NTS; // [FIX]

                                    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &layRa, 0, NULL);
                                    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)currentMaLo.c_str(), 0, &lenNTS_Upd);
                                    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)sSP.c_str(), 0, &lenNTS_Upd);

                                    SQLExecute(hStmt);
                                    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
                                }
                                soLuongCan -= layRa;
                            }
                        }
                        SQLFreeHandle(SQL_HANDLE_STMT, hStmtBatch);
                    }
                    CapNhatTrangThaiYeuCau(hDbc, h.maYeuCauGoc, "Đang Chờ Xử Lý");
                }
            }

            SQLEndTran(SQL_HANDLE_DBC, hDbc, SQL_COMMIT);
        }
        catch (...) {
            SQLEndTran(SQL_HANDLE_DBC, hDbc, SQL_ROLLBACK);
            SQLSetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_ON, 0);
            throw;
        }
        SQLSetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_ON, 0);
    }
    void QuanLyVanChuyen::XuLyTatCaYeuCau(SQLHDBC hDbc) {}
}