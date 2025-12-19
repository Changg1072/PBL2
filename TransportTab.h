#pragma once
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <string>
#include <vector>
#include <tuple>
#include <iostream>
#include "SaoLuu.h" 
#include "ImportGoodsForm.h"

using namespace System::Runtime::InteropServices;

namespace PBL2QuanLyKho {

    using namespace System;
    using namespace System::ComponentModel;
    using namespace System::Windows::Forms;
    using namespace System::Data;
    using namespace System::Drawing;
    using namespace System::Collections::Generic;

    // =================================================================================
    // 1. FORM POPUP: CHI TIẾT LÔ HÀNG (ĐÃ ĐỔI TÊN ĐỂ TRÁNH LỖI C2011)
    // =================================================================================
    public ref class TransportProductBatchForm : public System::Windows::Forms::Form {
    public:
        // Đổi tên Constructor
        TransportProductBatchForm(String^ maSP, String^ tenSP, String^ maDonVC) {
            this->Text = L"Chi tiết lô & Hạn sử dụng: " + tenSP;
            this->Size = System::Drawing::Size(600, 400);
            this->StartPosition = FormStartPosition::CenterParent;
            this->BackColor = Color::White;

            Label^ lblTitle = gcnew Label();
            lblTitle->Text = L"Sản phẩm: " + tenSP + L"\n(Ưu tiên xuất HSD gần nhất)";
            lblTitle->Dock = DockStyle::Top;
            lblTitle->Height = 60;
            lblTitle->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, FontStyle::Bold));
            lblTitle->ForeColor = Color::FromArgb(0, 114, 188);
            lblTitle->TextAlign = ContentAlignment::MiddleCenter;
            this->Controls->Add(lblTitle);

            DataGridView^ dgv = gcnew DataGridView();
            dgv->Dock = DockStyle::Fill;
            dgv->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;
            dgv->BackgroundColor = Color::WhiteSmoke;
            dgv->ReadOnly = true;
            dgv->AllowUserToAddRows = false;
            dgv->RowTemplate->Height = 35;

            dgv->Columns->Add("MaLo", L"Mã Lô Hàng");
            dgv->Columns->Add("NgayNhap", L"Ngày Nhập");
            dgv->Columns->Add("HSD", L"Hạn Sử Dụng");
            dgv->Columns->Add("SL", L"Tồn (Tấn)");

            // Format cột HSD màu đỏ
            dgv->Columns["HSD"]->DefaultCellStyle->ForeColor = Color::Red;
            dgv->Columns["HSD"]->DefaultCellStyle->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, FontStyle::Bold));

            this->Controls->Add(dgv);
            lblTitle->SendToBack();

            LoadBatchData(dgv, maSP, maDonVC);
        }

    private:
        std::string ToAnsi(String^ s) {
            if (String::IsNullOrEmpty(s)) return "";
            cli::array<unsigned char>^ bytes = System::Text::Encoding::UTF8->GetBytes(s);
            pin_ptr<unsigned char> p = &bytes[0];
            return std::string((char*)p, bytes->Length);
        }

        // Lưu ý: Tham số thứ 3 giờ là Mã Đơn Vận Chuyển (maDonVC) chứ không phải Mã Kho
        // Trong class TransportProductBatchForm (File TransportTab.h)
        void LoadBatchData(DataGridView^ dgv, String^ maSP, String^ maDonVC) {
            SQLHENV hEnv; SQLHDBC hDbc; SQLHSTMT hStmt;
            SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
            SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
            SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
            SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

            if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {

                std::wstring sql = L"SELECT ct.MaLoHang, lh.NgayNhap, ctlh.HanSuDung, ct.SoLuong "
                    L"FROM ChiTietDonVC ct "
                    L"JOIN LoHang lh ON ct.MaLoHang = lh.MaLoHang "
                    L"JOIN ChiTietLoHang ctlh ON ct.MaLoHang = ctlh.MaLoHang AND ct.MaSanPham = ctlh.MaSanPham "
                    L"WHERE ct.MaDonVC = ? AND ct.MaSanPham = ? "
                    L"ORDER BY ctlh.HanSuDung ASC";

                SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);

                std::string sMaDon = ToAnsi(maDonVC);
                std::string sSP = ToAnsi(maSP);
                SQLCHAR cMaDon[256]; strcpy_s((char*)cMaDon, 256, sMaDon.c_str());
                SQLCHAR cSP[256];    strcpy_s((char*)cSP, 256, sSP.c_str());

                SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, cMaDon, 0, NULL);
                SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, cSP, 0, NULL);

                if (SQLExecute(hStmt) == SQL_SUCCESS) {
                    SQLCHAR sMaLo[256];
                    SQLDOUBLE dSL;
                    SQL_DATE_STRUCT dHSD, dNhap;

                    while (SQLFetch(hStmt) == SQL_SUCCESS) {
                        SQLGetData(hStmt, 1, SQL_C_CHAR, sMaLo, 256, NULL);
                        SQLGetData(hStmt, 2, SQL_C_TYPE_DATE, &dNhap, 0, NULL);
                        SQLGetData(hStmt, 3, SQL_C_TYPE_DATE, &dHSD, 0, NULL); // Lấy được HSD rồi nhé
                        SQLGetData(hStmt, 4, SQL_C_DOUBLE, &dSL, 0, NULL);

                        String^ strHSD = String::Format("{0:00}/{1:00}/{2:0000}", dHSD.day, dHSD.month, dHSD.year);
                        String^ strNhap = String::Format("{0:00}/{1:00}/{2:0000}", dNhap.day, dNhap.month, dNhap.year);

                        dgv->Rows->Add(
                            gcnew String((char*)sMaLo),
                            strNhap,
                            strHSD,
                            dSL.ToString("N2")
                        );
                    }
                }
            }
            SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        }
    };

    // =================================================================================
    // 2. FORM POPUP CHI TIẾT ĐƠN HÀNG
    // =================================================================================
    public ref class TransportDetailForm : public System::Windows::Forms::Form {
    private:
        DataGridView^ dgv;
        Label^ lblInfo;
        String^ CurrentMaKho;
        String^ CurrentMaDon;

    public:
        TransportDetailForm(String^ maDon, String^ maKho) {
            this->CurrentMaKho = maKho;
            this->CurrentMaDon = maDon;
            this->Text = L"Chi tiết đơn hàng: " + maDon;
            this->Size = System::Drawing::Size(900, 600);
            this->StartPosition = FormStartPosition::CenterParent;
            this->BackColor = Color::White;

            GroupBox^ grpInfo = gcnew GroupBox();
            grpInfo->Text = L"Thông tin vận chuyển"; grpInfo->Dock = DockStyle::Top; grpInfo->Height = 130;
            grpInfo->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, FontStyle::Bold));

            lblInfo = gcnew Label(); lblInfo->Dock = DockStyle::Fill;
            lblInfo->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11, FontStyle::Regular));
            lblInfo->Padding = System::Windows::Forms::Padding(10);
            grpInfo->Controls->Add(lblInfo);
            this->Controls->Add(grpInfo);

            Label^ lblHint = gcnew Label();
            lblHint->Text = L"* Bấm vào từng sản phẩm bên dưới để xem Hạn Sử Dụng (HSD) ưu tiên *";
            lblHint->Dock = DockStyle::Top;
            lblHint->ForeColor = Color::Red;
            lblHint->TextAlign = ContentAlignment::MiddleCenter;
            lblHint->Height = 30;
            this->Controls->Add(lblHint);
            lblHint->BringToFront();

            dgv = gcnew DataGridView(); dgv->Dock = DockStyle::Fill;
            dgv->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;
            dgv->BackgroundColor = Color::WhiteSmoke;
            dgv->ReadOnly = true; dgv->RowTemplate->Height = 35;
            dgv->SelectionMode = DataGridViewSelectionMode::FullRowSelect;

            dgv->Columns->Add("MaSP", L"Mã SP");
            dgv->Columns["MaSP"]->Visible = false;
            dgv->Columns->Add("SP", L"Sản Phẩm");
            dgv->Columns->Add("MaLo", L"Mã Lô Hàng");
            dgv->Columns->Add("SL", L"SL Yêu Cầu (Tấn)");
            dgv->Columns->Add("Tien", L"Thành Tiền");
            dgv->Columns["Tien"]->DefaultCellStyle->Format = "N0";

            dgv->CellClick += gcnew DataGridViewCellEventHandler(this, &TransportDetailForm::OnCellClick);

            this->Controls->Add(dgv); grpInfo->SendToBack();

            LoadTransportInfo(maDon);
            LoadProductDetails(maDon);
        }

    private:
        void OnCellClick(Object^ sender, DataGridViewCellEventArgs^ e) {
            if (e->RowIndex >= 0) {
                String^ maSP = dgv->Rows[e->RowIndex]->Cells["MaSP"]->Value->ToString();
                String^ tenSP = dgv->Rows[e->RowIndex]->Cells["SP"]->Value->ToString();

                TransportProductBatchForm^ frmBatch = gcnew TransportProductBatchForm(maSP, tenSP, this->CurrentMaDon);
                frmBatch->ShowDialog();
            }
        }

        std::string ToAnsi(String^ s) {
            if (String::IsNullOrEmpty(s)) return "";
            cli::array<unsigned char>^ bytes = System::Text::Encoding::UTF8->GetBytes(s);
            pin_ptr<unsigned char> p = &bytes[0];
            return std::string((char*)p, bytes->Length);
        }

        void LoadTransportInfo(String^ maDon) {
            SQLHENV hEnv; SQLHDBC hDbc; SQLHSTMT hStmt;
            if (!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv))) return;
            SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
            SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
            SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

            if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
                std::wstring sql = L"SELECT MaXe, MaSieuThi, TongKhoiLuong, TongChiPhi, NgayTao FROM DonVanChuyen WHERE MaDonVC = ?";
                SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);

                std::string sMa = ToAnsi(maDon);
                SQLCHAR cMa[51]; strcpy_s((char*)cMa, 51, sMa.c_str());
                SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMa, 0, NULL);

                if (SQLExecute(hStmt) == SQL_SUCCESS && SQLFetch(hStmt) == SQL_SUCCESS) {
                    SQLCHAR sXe[51], sST[51], sNgay[21]; SQLDOUBLE dKL, dPhi;
                    SQLGetData(hStmt, 1, SQL_C_CHAR, sXe, 51, NULL); SQLGetData(hStmt, 2, SQL_C_CHAR, sST, 51, NULL);
                    SQLGetData(hStmt, 3, SQL_C_DOUBLE, &dKL, 0, NULL); SQLGetData(hStmt, 4, SQL_C_DOUBLE, &dPhi, 0, NULL);
                    SQLGetData(hStmt, 5, SQL_C_CHAR, sNgay, 21, NULL);

                    lblInfo->Text = String::Format(L"🚚 Xe: {0} | 🏢 Siêu Thị: {1}\n📅 Ngày: {2}\n📦 KL: {3:N2} Tấn | 💰 Phí: {4:N0} VNĐ",
                        gcnew String((char*)sXe), gcnew String((char*)sST), gcnew String((char*)sNgay), dKL, dPhi);
                }
                SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
            }
            SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        }
        // =================================================================================
// CẬP NHẬT: LOAD CHI TIẾT THÔNG MINH (DỰ BÁO FEFO + LỊCH SỬ)
// =================================================================================
        void LoadProductDetails(String^ maDon) {
            dgv->Rows->Clear();

            SQLHENV hEnv; SQLHDBC hDbc; SQLHSTMT hStmt;
            if (!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv))) return;
            SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
            SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
            SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

            // B1: Lấy trạng thái đơn hàng để quyết định hiển thị Lịch sử hay Dự báo
            String^ trangThaiDon = L"";
            if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
                std::wstring sqlStatus = L"SELECT TrangThai FROM DonVanChuyen WHERE MaDonVC = ?";
                SQLPrepare(hStmt, (SQLWCHAR*)sqlStatus.c_str(), SQL_NTS);
                std::string sMa = ToAnsi(maDon);
                SQLCHAR cMa[51]; strcpy_s((char*)cMa, 51, sMa.c_str());
                SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMa, 0, NULL);
                if (SQLExecute(hStmt) == SQL_SUCCESS && SQLFetch(hStmt) == SQL_SUCCESS) {
                    SQLWCHAR wStatus[51];
                    SQLGetData(hStmt, 1, SQL_C_WCHAR, wStatus, 51, NULL);
                    trangThaiDon = gcnew String(wStatus);
                }
                SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
            }
            trangThaiDon = trangThaiDon->Trim();

            // =======================================================================
            // TRƯỜNG HỢP A: ĐÃ XỬ LÝ (Đang giao / Đã giao) -> LẤY LỊCH SỬ TỪ DB
            // =======================================================================
            if (trangThaiDon == L"Đang Vận Chuyển" || trangThaiDon == L"Đã Giao Hàng") {
                if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
                    // Lấy từ ChiTietDonVC (Kết quả thực tế đã lưu)
                    std::wstring sqlHistory = L"SELECT ct.MaSanPham, sp.TenSanPham, ct.MaLoHang, ct.SoLuong, ct.ThanhTien, lh.HanSuDung "
                        L"FROM ChiTietDonVC ct "
                        L"JOIN SanPham sp ON ct.MaSanPham = sp.MaSanPham "
                        L"LEFT JOIN ChiTietLoHang lh ON ct.MaLoHang = lh.MaLoHang AND ct.MaSanPham = lh.MaSanPham "
                        L"WHERE ct.MaDonVC = ?";

                    SQLPrepare(hStmt, (SQLWCHAR*)sqlHistory.c_str(), SQL_NTS);
                    std::string sMa = ToAnsi(maDon);
                    SQLCHAR cMa[51]; strcpy_s((char*)cMa, 51, sMa.c_str());
                    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMa, 0, NULL);

                    if (SQLExecute(hStmt) == SQL_SUCCESS) {
                        SQLCHAR sSP[51], sLo[51]; SQLWCHAR wTen[256];
                        SQLDOUBLE dSL, dTien; SQL_DATE_STRUCT dHSD;
                        SQLLEN lenLo, lenHSD;

                        while (SQLFetch(hStmt) == SQL_SUCCESS) {
                            SQLGetData(hStmt, 1, SQL_C_CHAR, sSP, 51, NULL);
                            SQLGetData(hStmt, 2, SQL_C_WCHAR, wTen, 256, NULL);
                            SQLGetData(hStmt, 3, SQL_C_CHAR, sLo, 51, &lenLo);
                            SQLGetData(hStmt, 4, SQL_C_DOUBLE, &dSL, 0, NULL);
                            SQLGetData(hStmt, 5, SQL_C_DOUBLE, &dTien, 0, NULL);
                            SQLGetData(hStmt, 6, SQL_C_TYPE_DATE, &dHSD, 0, &lenHSD);

                            String^ displayLo = L"";
                            Color rowColor = Color::Black;
                            Color bgColor = Color::White;

                            // Nếu có mã lô -> Đã xuất thành công
                            if (lenLo != SQL_NULL_DATA && strlen((char*)sLo) > 0) {
                                String^ strHSD = (lenHSD != SQL_NULL_DATA)
                                    ? String::Format("{0:00}/{1:00}/{2:0000}", dHSD.day, dHSD.month, dHSD.year)
                                    : L"N/A";
                                displayLo = gcnew String((char*)sLo) + L" (" + strHSD + L")";
                                rowColor = Color::DarkGreen; // Màu xanh lá cây (Thành công)
                            }
                            else {
                                // Nếu mã lô NULL -> Lúc xuất kho bị thiếu
                                displayLo = L"⚠️ THIẾU HÀNG (Không giao được)";
                                rowColor = Color::Red;
                                bgColor = Color::MistyRose;
                            }

                            int idx = dgv->Rows->Add(
                                gcnew String((char*)sSP),
                                gcnew String(wTen),
                                displayLo,
                                dSL,
                                dTien
                            );
                            dgv->Rows[idx]->DefaultCellStyle->ForeColor = rowColor;
                            dgv->Rows[idx]->DefaultCellStyle->BackColor = bgColor;
                        }
                    }
                    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
                }
            }
            // =======================================================================
            // TRƯỜNG HỢP B: CHƯA XỬ LÝ -> CHẠY MÔ PHỎNG FEFO (DỰ BÁO)
            // =======================================================================
            else {
                // Danh sách nhu cầu tổng: Tuple<MaSP, TenSP, SoLuongCan, DonGia>
                std::vector<std::tuple<std::string, std::wstring, double, double>> listNhuCau;

                // B1. Lấy tổng nhu cầu từ ChiTietYeuCau (Khách đặt bao nhiêu?)
                if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
                    std::wstring sqlDemand = L"SELECT s.MaSanPham, s.TenSanPham, ct.SoTanYeuCau, ct.GiaTri "
                        L"FROM DonVanChuyen dvc JOIN ChiTietYeuCau ct ON LTRIM(RTRIM(dvc.MaYeuCauGoc)) = LTRIM(RTRIM(ct.MaYeuCau)) "
                        L"JOIN SanPham s ON ct.MaSanPham = s.MaSanPham WHERE dvc.MaDonVC = ?";

                    SQLPrepare(hStmt, (SQLWCHAR*)sqlDemand.c_str(), SQL_NTS);
                    std::string sMa = ToAnsi(maDon);
                    while (!sMa.empty() && isspace(sMa.back())) sMa.pop_back();
                    SQLCHAR cMa[51]; strcpy_s((char*)cMa, 51, sMa.c_str());
                    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMa, 0, NULL);

                    if (SQLExecute(hStmt) == SQL_SUCCESS) {
                        SQLCHAR sMaSP[51]; SQLWCHAR wTen[256]; SQLDOUBLE dSL, dTien;
                        while (SQLFetch(hStmt) == SQL_SUCCESS) {
                            SQLGetData(hStmt, 1, SQL_C_CHAR, sMaSP, 51, NULL);
                            SQLGetData(hStmt, 2, SQL_C_WCHAR, wTen, 256, NULL);
                            SQLGetData(hStmt, 3, SQL_C_DOUBLE, &dSL, 0, NULL);
                            SQLGetData(hStmt, 4, SQL_C_DOUBLE, &dTien, 0, NULL);

                            std::string strMaSP = std::string((char*)sMaSP);
                            // Clean string
                            size_t first = strMaSP.find_first_not_of(" ");
                            if (std::string::npos != first) {
                                size_t last = strMaSP.find_last_not_of(" ");
                                strMaSP = strMaSP.substr(first, (last - first + 1));
                            }

                            listNhuCau.push_back(std::make_tuple(
                                strMaSP,
                                std::wstring(wTen),
                                dSL,
                                (dSL > 0 ? dTien / dSL : 0)
                            ));
                        }
                    }
                    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
                }

                // B2. Chạy vòng lặp FEFO cho từng sản phẩm
                for (const auto& item : listNhuCau) {
                    std::string currentMaSP = std::get<0>(item);
                    std::wstring currentTenSP = std::get<1>(item);
                    double remain = std::get<2>(item);
                    double currentPrice = std::get<3>(item);

                    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
                        // Query: Tìm các lô hàng trong kho hiện tại, sắp xếp HSD tăng dần
                        std::wstring sqlFEFO = L"SELECT t.MaLoHang, t.SoLuongTan, t.HanSuDung FROM ChiTietTonKho t "
                            L"JOIN TonKho k ON t.MaLoHang = k.MaLoHang "
                            L"WHERE LTRIM(RTRIM(t.MaSanPham)) = ? AND LTRIM(RTRIM(k.MaKho)) = ? AND t.SoLuongTan > 0 "
                            L"ORDER BY t.HanSuDung ASC";

                        SQLPrepare(hStmt, (SQLWCHAR*)sqlFEFO.c_str(), SQL_NTS);
                        SQLCHAR cSP[51]; strcpy_s((char*)cSP, 51, currentMaSP.c_str());
                        std::string sKho = ToAnsi(this->CurrentMaKho);
                        while (!sKho.empty() && isspace(sKho.back())) sKho.pop_back();
                        SQLCHAR cKho[51]; strcpy_s((char*)cKho, 51, sKho.c_str());

                        SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cSP, 0, NULL);
                        SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cKho, 0, NULL);

                        if (SQLExecute(hStmt) == SQL_SUCCESS) {
                            SQLCHAR cLo[51]; SQLDOUBLE dTon; SQL_DATE_STRUCT dHSD;
                            // Lặp qua các lô hàng có sẵn
                            while (remain > 0.0001 && SQLFetch(hStmt) == SQL_SUCCESS) {
                                SQLGetData(hStmt, 1, SQL_C_CHAR, cLo, 51, NULL);
                                SQLGetData(hStmt, 2, SQL_C_DOUBLE, &dTon, 0, NULL);
                                SQLGetData(hStmt, 3, SQL_C_TYPE_DATE, &dHSD, 0, NULL);

                                double take = (remain < dTon) ? remain : dTon;
                                String^ hsd = String::Format("{0:00}/{1:00}/{2:0000}", dHSD.day, dHSD.month, dHSD.year);

                                // Hiển thị dòng "Dự kiến lấy" (Màu xanh dương)
                                int idx = dgv->Rows->Add(
                                    gcnew String(currentMaSP.c_str()),
                                    gcnew String(currentTenSP.c_str()),
                                    gcnew String((char*)cLo) + L" (" + hsd + L")",
                                    take,
                                    take * currentPrice
                                );
                                dgv->Rows[idx]->DefaultCellStyle->ForeColor = Color::Blue;
                                remain -= take;
                            }
                        }
                        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
                    }

                    // B3. Nếu quét hết kho mà vẫn còn thiếu -> Báo Đỏ ngay
                    if (remain > 0.0001) {
                        int idx = dgv->Rows->Add(
                            gcnew String(currentMaSP.c_str()),
                            gcnew String(currentTenSP.c_str()),
                            L"⚠️ THIẾU HÀNG KHO", // Cột Mã Lô sẽ hiện cảnh báo
                            remain,
                            remain * currentPrice
                        );
                        // Format: Chữ Đỏ, Nền Vàng để gây chú ý
                        dgv->Rows[idx]->DefaultCellStyle->ForeColor = Color::Red;
                        dgv->Rows[idx]->DefaultCellStyle->BackColor = Color::Yellow;
                        dgv->Rows[idx]->DefaultCellStyle->Font = gcnew System::Drawing::Font(dgv->Font, FontStyle::Bold);
                    }
                }
            }

            SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        }
    };
    struct ItemCanGiao {
        std::string MaSP;
        double SoLuongCan;
    };
    // =================================================================================
    // 3. MODULE CHÍNH: TRANSPORT TAB
    // =================================================================================
    public ref class TransportTab : public System::Windows::Forms::Panel {
    private:
        Panel^ pnlFilters; Button^ btnChoXuLy; Button^ btnDangGiao; Button^ btnDaGiao; Button^ btnAutoImport;
        DataGridView^ dgvList; String^ CurrentMaKho; String^ CurrentStatus;

    public:
        TransportTab(String^ maKho) {
            this->CurrentMaKho = maKho; this->Dock = DockStyle::Fill; this->BackColor = Color::White;
            InitializeUI();

            btnChoXuLy->Tag = L"Đang Chờ Xử Lý";
            OnTabClick(btnChoXuLy, nullptr);
        }

    private:
        std::string ToAnsi(String^ s) {
            if (String::IsNullOrEmpty(s)) return "";
            cli::array<unsigned char>^ bytes = System::Text::Encoding::UTF8->GetBytes(s);
            pin_ptr<unsigned char> p = &bytes[0];
            return std::string((char*)p, bytes->Length);
        }
        void ToWChar(String^ s, SQLWCHAR* output, int maxLen) {
            if (String::IsNullOrEmpty(s)) output[0] = L'\0';
            else {
                pin_ptr<const wchar_t> wch = PtrToStringChars(s);
                wcsncpy_s((wchar_t*)output, maxLen, wch, _TRUNCATE);
            }
        }
        std::string CleanString(std::string str) {
            if (str.empty()) return "";
            // Xóa khoảng trắng đầu
            size_t first = str.find_first_not_of(" \t\r\n");
            if (std::string::npos == first) return "";
            // Xóa khoảng trắng cuối
            size_t last = str.find_last_not_of(" \t\r\n");
            return str.substr(first, (last - first + 1));
        }
        void InitializeUI() {
            // 1. Panel chứa các nút lọc và nút chức năng
            pnlFilters = gcnew Panel();
            pnlFilters->Dock = DockStyle::Top;
            pnlFilters->Height = 60;
            pnlFilters->BackColor = Color::WhiteSmoke;
            pnlFilters->Padding = System::Windows::Forms::Padding(20, 10, 20, 10);

            // 2. Tạo các nút lọc trạng thái đơn hàng (Code cũ)
            btnChoXuLy = CreateBtn(L"⏳ Chờ xử lý", L"Đang Chờ Xử Lý", 20);
            btnDangGiao = CreateBtn(L"🚚 Đang giao", L"Đang Vận Chuyển", 200);
            btnDaGiao = CreateBtn(L"✅ Đã giao", L"Đã Giao Hàng", 380);

            // 3. [MỚI] Tạo nút "Nhập hàng thiếu" (Màu cam nổi bật)
            btnAutoImport = gcnew Button();
            btnAutoImport->Text = L"⚠️ Nhập hàng thiếu";
            btnAutoImport->Tag = L"AutoImport"; // Tag để đánh dấu nếu cần
            btnAutoImport->Location = Point(560, 10); // Đặt tiếp theo sau btnDaGiao (380 + 180 = 560)
            btnAutoImport->Size = System::Drawing::Size(160, 40);
            btnAutoImport->BackColor = Color::OrangeRed; // Màu cam đỏ báo hiệu xử lý sự cố
            btnAutoImport->ForeColor = Color::White;
            btnAutoImport->FlatStyle = FlatStyle::Flat;
            btnAutoImport->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, FontStyle::Bold));

            // Gắn sự kiện click vào hàm xử lý Logic tự động nhập
            btnAutoImport->Click += gcnew EventHandler(this, &TransportTab::OnAutoImportClick);

            // 4. Thêm tất cả nút vào Panel
            pnlFilters->Controls->Add(btnAutoImport); // Thêm nút mới
            pnlFilters->Controls->Add(btnDaGiao);
            pnlFilters->Controls->Add(btnDangGiao);
            pnlFilters->Controls->Add(btnChoXuLy);

            // 5. Cấu hình DataGridView hiển thị danh sách đơn (Code cũ)
            dgvList = gcnew DataGridView();
            dgvList->Dock = DockStyle::Fill;
            dgvList->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;
            dgvList->BackgroundColor = Color::White;
            dgvList->BorderStyle = System::Windows::Forms::BorderStyle::None;
            dgvList->ReadOnly = true;
            dgvList->AllowUserToAddRows = false;
            dgvList->RowTemplate->Height = 40;
            dgvList->EnableHeadersVisualStyles = false;

            // Style Header
            dgvList->ColumnHeadersDefaultCellStyle->BackColor = Color::FromArgb(23, 42, 69);
            dgvList->ColumnHeadersDefaultCellStyle->ForeColor = Color::White;
            dgvList->ColumnHeadersDefaultCellStyle->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, FontStyle::Bold));

            // Gắn sự kiện click cho bảng
            dgvList->CellContentClick += gcnew DataGridViewCellEventHandler(this, &TransportTab::OnContentClick);
            dgvList->CellClick += gcnew DataGridViewCellEventHandler(this, &TransportTab::OnRowClick);

            // 6. Thêm vào giao diện chính của Tab
            this->Controls->Add(dgvList);
            this->Controls->Add(pnlFilters);
        }

        Button^ CreateBtn(String^ txt, String^ tag, int x) {
            Button^ b = gcnew Button(); b->Text = txt; b->Tag = tag;
            b->Location = Point(x, 10); b->Size = System::Drawing::Size(160, 40);
            b->FlatStyle = FlatStyle::Flat; b->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, FontStyle::Bold));
            b->Click += gcnew EventHandler(this, &TransportTab::OnTabClick);
            return b;
        }

        void OnTabClick(Object^ s, EventArgs^ e) {
            Button^ b = (Button^)s;
            btnChoXuLy->BackColor = Color::White; btnChoXuLy->ForeColor = Color::DimGray;
            btnDangGiao->BackColor = Color::White; btnDangGiao->ForeColor = Color::DimGray;
            btnDaGiao->BackColor = Color::White; btnDaGiao->ForeColor = Color::DimGray;
            b->BackColor = Color::FromArgb(0, 114, 188); b->ForeColor = Color::White;

            this->CurrentStatus = b->Tag->ToString();
            LoadDataFromSQL(this->CurrentStatus);
        }
        // [FIX] Hàm Load dữ liệu sửa lỗi không hiện đơn do khoảng trắng
        void LoadDataFromSQL(String^ status) {
            dgvList->Columns->Clear(); dgvList->Rows->Clear();
            dgvList->Columns->Add("MaDon", L"Mã Đơn");
            dgvList->Columns->Add("SieuThi", L"Siêu Thị");
            dgvList->Columns->Add("Ngay", L"Ngày");
            dgvList->Columns->Add("Xe", L"Xe");
            dgvList->Columns->Add("ChiPhi", L"Cước Phí");

            // Chỉ hiện nút xác nhận nếu đang ở tab Chờ Xử Lý
            if (status == L"Đang Chờ Xử Lý") {
                DataGridViewButtonColumn^ btn = gcnew DataGridViewButtonColumn();
                btn->Name = "Action"; btn->HeaderText = L"Thao tác"; btn->Text = L"Xác nhận giao";
                btn->UseColumnTextForButtonValue = true; btn->FlatStyle = FlatStyle::Popup;
                btn->DefaultCellStyle->BackColor = Color::LightGreen;
                dgvList->Columns->Add(btn);
            }

            SQLHENV hEnv; SQLHDBC hDbc; SQLHSTMT hStmt;
            if (!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv))) return;
            SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
            SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
            SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

            if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
                // [QUAN TRỌNG] Thêm LTRIM/RTRIM để xử lý lỗi khoảng trắng
                std::wstring sql = L"SELECT MaDonVC, MaSieuThi, NgayTao, MaXe, TongChiPhi "
                    L"FROM DonVanChuyen "
                    L"WHERE LTRIM(RTRIM(MaKho)) = ? AND LTRIM(RTRIM(TrangThai)) LIKE ?";

                SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);

                // 1. Bind Mã Kho (Đã làm sạch)
                std::string sKho = CleanString(ToAnsi(this->CurrentMaKho));
                SQLCHAR cKho[51]; strcpy_s((char*)cKho, 51, sKho.c_str());
                SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cKho, 0, NULL);

                // 2. Bind Trạng Thái (Unicode)
                SQLWCHAR cStatus[51]; ToWChar(status, cStatus, 50);
                SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 50, 0, cStatus, 0, NULL);

                if (SQLExecute(hStmt) == SQL_SUCCESS) {
                    SQLCHAR sMaDon[51], sMaST[51], sNgay[21], sMaXe[51]; SQLDOUBLE dChiPhi;
                    while (SQLFetch(hStmt) == SQL_SUCCESS) {
                        SQLGetData(hStmt, 1, SQL_C_CHAR, sMaDon, 51, NULL);
                        SQLGetData(hStmt, 2, SQL_C_CHAR, sMaST, 51, NULL);
                        SQLGetData(hStmt, 3, SQL_C_CHAR, sNgay, 21, NULL);
                        SQLGetData(hStmt, 4, SQL_C_CHAR, sMaXe, 51, NULL);
                        SQLGetData(hStmt, 5, SQL_C_DOUBLE, &dChiPhi, 0, NULL);

                        dgvList->Rows->Add(
                            gcnew String((char*)sMaDon),
                            gcnew String((char*)sMaST),
                            gcnew String((char*)sNgay),
                            gcnew String((char*)sMaXe),
                            dChiPhi.ToString("N0")
                        );
                    }
                }
            }
            SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
            SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        }
        void PhanBoLoHangTuDong(String^ maDon) {
            SQLHENV hEnv; SQLHDBC hDbc; SQLHSTMT hStmt;
            SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
            SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
            SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
            SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

            std::string sMaDon = CleanString(ToAnsi(maDon)); // CleanString lấy từ TransportTab

            // 1. LẤY DANH SÁCH SẢN PHẨM CẦN GIAO
            std::vector<std::tuple<std::string, double, double>> listItems;
            if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
                // Group by để tránh trùng lặp nếu lỡ có lỗi dữ liệu
                std::wstring sql = L"SELECT MaSanPham, SUM(SoLuong), SUM(ThanhTien)/SUM(SoLuong) FROM ChiTietDonVC WHERE MaDonVC = ? GROUP BY MaSanPham";
                SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);
                SQLCHAR cMa[51]; strcpy_s((char*)cMa, 51, sMaDon.c_str());
                SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMa, 0, NULL);
                if (SQLExecute(hStmt) == SQL_SUCCESS) {
                    SQLCHAR cSP[51]; double dSL, dGia;
                    while (SQLFetch(hStmt) == SQL_SUCCESS) {
                        SQLGetData(hStmt, 1, SQL_C_CHAR, cSP, 51, NULL);
                        SQLGetData(hStmt, 2, SQL_C_DOUBLE, &dSL, 0, NULL);
                        SQLGetData(hStmt, 3, SQL_C_DOUBLE, &dGia, 0, NULL);
                        listItems.push_back(std::make_tuple(CleanString((char*)cSP), dSL, dGia));
                    }
                }
                SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
            }

            // 2. XỬ LÝ TỪNG MÓN (TRANSACTION)
            SQLSetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, 0);
            try {
                for (const auto& req : listItems) {
                    std::string reqMaSP = std::get<0>(req);
                    double reqQty = std::get<1>(req);
                    double reqPrice = std::get<2>(req);

                    // A. LUÔN LUÔN XÓA DÒNG CŨ TRƯỚC (QUAN TRỌNG !!!)
                    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
                        std::wstring sqlDel = L"DELETE FROM ChiTietDonVC WHERE MaDonVC = ? AND MaSanPham = ?";
                        SQLPrepare(hStmt, (SQLWCHAR*)sqlDel.c_str(), SQL_NTS);
                        SQLCHAR cMa[51]; strcpy_s((char*)cMa, 51, sMaDon.c_str());
                        SQLCHAR cSP[51]; strcpy_s((char*)cSP, 51, reqMaSP.c_str());
                        SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMa, 0, NULL);
                        SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cSP, 0, NULL);
                        SQLExecute(hStmt);
                        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
                    }

                    // B. Tìm lô hàng khả dụng (FEFO)
                    std::vector<std::pair<std::string, double>> allocation;
                    double remainNeed = reqQty;

                    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
                        std::wstring sqlFEFO = L"SELECT t.MaLoHang, t.SoLuongTan FROM ChiTietTonKho t JOIN TonKho k ON t.MaLoHang = k.MaLoHang WHERE t.MaSanPham = ? AND k.MaKho = ? AND t.SoLuongTan > 0 ORDER BY t.HanSuDung ASC";
                        SQLPrepare(hStmt, (SQLWCHAR*)sqlFEFO.c_str(), SQL_NTS);
                        std::string sKho = CleanString(ToAnsi(this->CurrentMaKho));
                        SQLCHAR cSP[51]; strcpy_s((char*)cSP, 51, reqMaSP.c_str());
                        SQLCHAR cKho[51]; strcpy_s((char*)cKho, 51, sKho.c_str());
                        SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cSP, 0, NULL);
                        SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cKho, 0, NULL);
                        if (SQLExecute(hStmt) == SQL_SUCCESS) {
                            SQLCHAR cLo[51]; double dTon;
                            while (remainNeed > 0 && SQLFetch(hStmt) == SQL_SUCCESS) {
                                SQLGetData(hStmt, 1, SQL_C_CHAR, cLo, 51, NULL);
                                SQLGetData(hStmt, 2, SQL_C_DOUBLE, &dTon, 0, NULL);
                                double take = (remainNeed < dTon) ? remainNeed : dTon;
                                allocation.push_back({ CleanString((char*)cLo), take });
                                remainNeed -= take;
                            }
                        }
                        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
                    }

                    // C. Insert lại (Có lô hoặc NULL)
                    std::wstring sqlIns = L"INSERT INTO ChiTietDonVC (MaDonVC, MaSanPham, SoLuong, ThanhTien, MaLoHang) VALUES (?, ?, ?, ?, ?)";

                    // Insert các lô tìm được
                    for (const auto& alloc : allocation) {
                        if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
                            SQLPrepare(hStmt, (SQLWCHAR*)sqlIns.c_str(), SQL_NTS);
                            std::string sLo = alloc.first; double dQ = alloc.second; double dT = dQ * reqPrice;
                            SQLCHAR cMa[51]; strcpy_s(reinterpret_cast<char*>(cMa), 51, sMaDon.c_str());
                            SQLCHAR cSP[51]; strcpy_s(reinterpret_cast<char*>(cSP), 51, reqMaSP.c_str());
                            SQLCHAR cLo[51]; strcpy_s(reinterpret_cast<char*>(cLo), 51, sLo.c_str());

                            SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMa, 0, NULL);
                            SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cSP, 0, NULL);
                            SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &dQ, 0, NULL);
                            SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &dT, 0, NULL);
                            SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cLo, 0, NULL);
                            SQLExecute(hStmt); SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
                        }
                    }

                    // Insert phần thiếu (QUAN TRỌNG: Insert NULL để báo thiếu)
                    if (remainNeed > 0) {
                        if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
                            // SQL riêng cho NULL
                            std::wstring sqlInsNull = L"INSERT INTO ChiTietDonVC (MaDonVC, MaSanPham, SoLuong, ThanhTien, MaLoHang) VALUES (?, ?, ?, ?, NULL)";
                            SQLPrepare(hStmt, (SQLWCHAR*)sqlInsNull.c_str(), SQL_NTS);
                            double dT = remainNeed * reqPrice;
                            SQLCHAR cMa[51]; strcpy_s((char*)cMa, 51, sMaDon.c_str());
                            SQLCHAR cSP[51]; strcpy_s((char*)cSP, 51, reqMaSP.c_str());

                            SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMa, 0, NULL);
                            SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cSP, 0, NULL);
                            SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &remainNeed, 0, NULL);
                            SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &dT, 0, NULL);
                            SQLExecute(hStmt); SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
                        }
                    }
                }
                SQLEndTran(SQL_HANDLE_DBC, hDbc, SQL_COMMIT);
            }
            catch (...) {
                SQLEndTran(SQL_HANDLE_DBC, hDbc, SQL_ROLLBACK);
            }
            SQLSetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_ON, 0);
            SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        }
        // Hàm kiểm tra xem đơn hàng có đang ở trạng thái chờ không
        bool IsOrderPending(String^ maDon) {
            bool isPending = false;
            SQLHENV hEnv; SQLHDBC hDbc; SQLHSTMT hStmt;
            if (!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv))) return false;
            SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
            SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
            SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

            if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
                // Dùng LIKE để bắt trạng thái chính xác
                std::wstring sql = L"SELECT 1 FROM DonVanChuyen WHERE MaDonVC = ? AND LTRIM(RTRIM(TrangThai)) LIKE N'Đang Chờ Xử Lý%'";
                SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);

                std::string sMa = CleanString(ToAnsi(maDon));
                SQLCHAR cMa[51]; strcpy_s((char*)cMa, 51, sMa.c_str());
                SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMa, 0, NULL);

                if (SQLExecute(hStmt) == SQL_SUCCESS) {
                    if (SQLFetch(hStmt) == SQL_SUCCESS) {
                        isPending = true; // Tìm thấy đơn đang chờ
                    }
                }
            }
            SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
            return isPending;
        }
        void ThucHienXuatKho(String^ maDon) {
            SQLHENV hEnv; SQLHDBC hDbc; SQLHSTMT hStmt;

            // 1. Khởi tạo kết nối
            if (!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv))) return;
            SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
            SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
            SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

            // List chứa dữ liệu cần trừ: <MaLo, MaSP, SoLuong>
            std::vector<std::tuple<std::string, std::string, double>> listToDeduct;
            std::string sMaDon = CleanString(ToAnsi(maDon));

            // --- BƯỚC 1: ĐỌC KẾT QUẢ PHÂN BỔ TỪ ChiTietDonVC ---
            if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
                std::wstring sqlGet = L"SELECT MaLoHang, MaSanPham, SoLuong FROM ChiTietDonVC WHERE MaDonVC = ? AND MaLoHang IS NOT NULL AND MaLoHang <> ''";
                SQLPrepare(hStmt, (SQLWCHAR*)sqlGet.c_str(), SQL_NTS);

                SQLCHAR cMa[51];
                // Ép kiểu sang char* để tránh lỗi C2664
                strcpy_s((char*)cMa, 51, sMaDon.c_str());

                SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMa, 0, NULL);

                if (SQLExecute(hStmt) == SQL_SUCCESS) {
                    SQLCHAR cLo[51], cSP[51];
                    SQLDOUBLE dSL;
                    while (SQLFetch(hStmt) == SQL_SUCCESS) {
                        SQLGetData(hStmt, 1, SQL_C_CHAR, cLo, 51, NULL);
                        SQLGetData(hStmt, 2, SQL_C_CHAR, cSP, 51, NULL);
                        SQLGetData(hStmt, 3, SQL_C_DOUBLE, &dSL, 0, NULL);

                        // Lưu vào vector
                        listToDeduct.push_back(std::make_tuple(
                            CleanString((char*)cLo),
                            CleanString((char*)cSP),
                            (double)dSL
                        ));
                    }
                }
                SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
            }

            if (listToDeduct.empty()) {
                SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
                return;
            }

            // --- BƯỚC 2: THỰC HIỆN TRỪ KHO (TRANSACTION) ---
            SQLSetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, 0);
            try {
                // SQL Update ChiTietTonKho
                std::wstring sqlUpdChiTiet = L"UPDATE ChiTietTonKho SET SoLuongTan = SoLuongTan - ? "
                    L"WHERE LTRIM(RTRIM(MaLoHang)) = LTRIM(RTRIM(?)) "
                    L"AND LTRIM(RTRIM(MaSanPham)) = LTRIM(RTRIM(?))";

                // SQL Update TonKho (Tạm khóa để tránh trừ gấp đôi do Trigger)
                /*
                std::wstring sqlUpdTong = L"UPDATE TonKho SET SoLuongTanTong = SoLuongTanTong - ? "
                    L"WHERE LTRIM(RTRIM(MaLoHang)) = LTRIM(RTRIM(?))";
                */

                for (const auto& item : listToDeduct) {
                    std::string sLo = std::get<0>(item);
                    std::string sSP = std::get<1>(item);
                    double dQty = std::get<2>(item);

                    SQLCHAR cLo[51], cSP[51];
                    // [FIX LỖI C2664]: Ép kiểu (char*)
                    strcpy_s((char*)cLo, 51, sLo.c_str());
                    strcpy_s((char*)cSP, 51, sSP.c_str());

                    // --- A. TRỪ CHI TIẾT (QUAN TRỌNG NHẤT) ---
                    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
                        SQLPrepare(hStmt, (SQLWCHAR*)sqlUpdChiTiet.c_str(), SQL_NTS);
                        SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &dQty, 0, NULL);
                        SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cLo, 0, NULL);
                        SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cSP, 0, NULL);

                        if (SQLExecute(hStmt) != SQL_SUCCESS) {
                            // [FIX LỖI C2665]: Thêm .c_str() khi tạo gcnew String
                            throw gcnew Exception("Lỗi Update ChiTietTonKho: " + gcnew String(sLo.c_str()));
                        }
                        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
                    }

                    // --- B. TRỪ TỔNG (TonKho) ---
                    // Đã comment lại để tránh lỗi trừ 2 lần.
                    // Nếu DB của bạn KHÔNG có Trigger tự động, hãy mở comment phần này ra.
                    /*
                    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
                        SQLPrepare(hStmt, (SQLWCHAR*)sqlUpdTong.c_str(), SQL_NTS);
                        SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &dQty, 0, NULL);
                        SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cLo, 0, NULL);
                        SQLExecute(hStmt);
                        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
                    }
                    */
                }

                SQLEndTran(SQL_HANDLE_DBC, hDbc, SQL_COMMIT);
            }
            catch (Exception^ ex) {
                SQLEndTran(SQL_HANDLE_DBC, hDbc, SQL_ROLLBACK);
                MessageBox::Show(L"Lỗi trừ kho: " + ex->Message, L"Lỗi Database", MessageBoxButtons::OK, MessageBoxIcon::Error);
            }

            SQLSetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_ON, 0);
            SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        }
        List<ImportItem^>^ TinhTongHangThieu() {
            List<ImportItem^>^ listThieu = gcnew List<ImportItem^>();

            SQLHENV hEnv; SQLHDBC hDbc; SQLHSTMT hStmt;
            if (!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv))) return listThieu;
            SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
            SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
            SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

            if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
                // QUERY LOGIC ĐÃ SỬA:
                // 1. Thay JOIN ChiTietDonVC bằng JOIN ChiTietYeuCau
                // 2. Thay SUM(c.SoLuong) bằng SUM(c.SoTanYeuCau)
                // 3. Sử dụng LTRIM(RTRIM(...)) cho các khóa ngoại để tránh lỗi chuỗi
                std::wstring sql =
                    L"SELECT DEMAND.MaSanPham, SP.TenSanPham, SP.GiaVon, (DEMAND.TongCan - ISNULL(STOCK.TongCo, 0)) AS SoLuongThieu "
                    L"FROM "
                    L"( "
                    L"   SELECT c.MaSanPham, SUM(c.SoTanYeuCau) AS TongCan "
                    L"   FROM DonVanChuyen d "
                    L"   JOIN ChiTietYeuCau c ON LTRIM(RTRIM(d.MaYeuCauGoc)) = LTRIM(RTRIM(c.MaYeuCau)) " // <--- SỬA TẠI ĐÂY
                    L"   WHERE LTRIM(RTRIM(d.MaKho)) = ? AND d.TrangThai = N'Đang Chờ Xử Lý' "
                    L"   GROUP BY c.MaSanPham "
                    L") DEMAND "
                    L"LEFT JOIN "
                    L"( "
                    L"   SELECT t.MaSanPham, SUM(t.SoLuongTan) AS TongCo "
                    L"   FROM ChiTietTonKho t "
                    L"   JOIN TonKho tk ON t.MaLoHang = tk.MaLoHang "
                    L"   WHERE LTRIM(RTRIM(tk.MaKho)) = ? "
                    L"   GROUP BY t.MaSanPham "
                    L") STOCK ON DEMAND.MaSanPham = STOCK.MaSanPham "
                    L"JOIN SanPham SP ON DEMAND.MaSanPham = SP.MaSanPham "
                    L"WHERE DEMAND.TongCan > ISNULL(STOCK.TongCo, 0)"; // Chỉ lấy những món thiếu

                SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);

                std::string sKho = ToAnsi(this->CurrentMaKho);
                // Clean chuỗi mã kho để đảm bảo khớp
                std::string sKhoClean = CleanString(sKho);
                SQLCHAR cKho[51]; strcpy_s((char*)cKho, 51, sKhoClean.c_str());

                // Bind tham số (MaKho dùng cho cả 2 chỗ ? trong subquery)
                SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cKho, 0, NULL);
                SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cKho, 0, NULL);

                if (SQLExecute(hStmt) == SQL_SUCCESS) {
                    SQLCHAR sMaSP[51];
                    SQLWCHAR wTen[256];
                    SQLDOUBLE dGia, dThieu;

                    while (SQLFetch(hStmt) == SQL_SUCCESS) {
                        SQLGetData(hStmt, 1, SQL_C_CHAR, sMaSP, 51, NULL);
                        SQLGetData(hStmt, 2, SQL_C_WCHAR, wTen, 256, NULL);
                        SQLGetData(hStmt, 3, SQL_C_DOUBLE, &dGia, 0, NULL);
                        SQLGetData(hStmt, 4, SQL_C_DOUBLE, &dThieu, 0, NULL);

                        ImportItem^ item = gcnew ImportItem();
                        item->MaSP = gcnew String((char*)sMaSP);
                        item->TenSP = gcnew String(wTen);
                        item->GiaVon = dGia;
                        item->SoLuong = ceil(dThieu); // Làm tròn lên

                        listThieu->Add(item);
                    }
                }
            }
            SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
            return listThieu;
        }
        void OnAutoImportClick(Object^ sender, EventArgs^ e) {
            // 1. Tính toán
            List<ImportItem^>^ listCanMua = TinhTongHangThieu();

            if (listCanMua->Count == 0) {
                MessageBox::Show(L"Kho đang ĐỦ HÀNG cho tất cả các đơn chờ xử lý!\nKhông cần nhập thêm.",
                    L"Thông báo", MessageBoxButtons::OK, MessageBoxIcon::Information);
                return;
            }

            // 2. Hiển thị xác nhận sơ bộ
            String^ msg = L"Phát hiện thiếu hụt cho các đơn hàng chờ:\n";
            for (int i = 0; i < Math::Min(5, listCanMua->Count); i++) {
                msg += L"- " + listCanMua[i]->TenSP + L": Thiếu " + listCanMua[i]->SoLuong + L" tấn\n";
            }
            if (listCanMua->Count > 5) msg += L"... và " + (listCanMua->Count - 5) + L" sản phẩm khác.\n";

            msg += L"\nBạn có muốn tạo đơn nhập hàng tự động cho các món này không?";

            if (MessageBox::Show(msg, L"Xác nhận nhập bù", MessageBoxButtons::YesNo, MessageBoxIcon::Question) == System::Windows::Forms::DialogResult::Yes) {

                // 3. Mở ImportGoodsForm với Constructor mới
                ImportGoodsForm^ frm = gcnew ImportGoodsForm(this->CurrentMaKho, listCanMua);

                // 4. Nếu nhập thành công (DialogResult::OK) thì load lại danh sách
                if (frm->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
                    LoadDataFromSQL(this->CurrentStatus); // Refresh lại list đơn hàng
                }
            }
        }

        bool KiemTraDuHang(String^ maDon, String^% loiChiTiet) {
            bool duHang = true;
            loiChiTiet = "";
            SQLHENV hEnv; SQLHDBC hDbc; SQLHSTMT hStmt;
            if (!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv))) return false;
            SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
            SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
            SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

            if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
                // Điều kiện: MaLoHang IS NULL hoặc MaLoHang = '' (chuỗi rỗng)
                std::wstring sql = L"SELECT s.TenSanPham, c.SoLuong FROM ChiTietDonVC c "
                    L"JOIN SanPham s ON c.MaSanPham = s.MaSanPham "
                    L"WHERE c.MaDonVC = ? AND (c.MaLoHang IS NULL OR c.MaLoHang = '')";

                SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);
                std::string sMa = CleanString(ToAnsi(maDon));
                SQLCHAR cMa[51]; strcpy_s((char*)cMa, 51, sMa.c_str());
                SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMa, 0, NULL);

                if (SQLExecute(hStmt) == SQL_SUCCESS) {
                    SQLWCHAR wTen[256]; SQLDOUBLE dSL;
                    while (SQLFetch(hStmt) == SQL_SUCCESS) {
                        SQLGetData(hStmt, 1, SQL_C_WCHAR, wTen, 256, NULL);
                        SQLGetData(hStmt, 2, SQL_C_DOUBLE, &dSL, 0, NULL);
                        duHang = false;
                        loiChiTiet += L"- " + gcnew String(wTen) + L": Thiếu " + dSL + L" tấn\n";
                    }
                }
            }
            SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
            return duHang;
        }
        // =======================================================================
        // CẬP NHẬT SỰ KIỆN CLICK ĐỂ GỌI TRỪ KHO
        // =======================================================================
        void OnContentClick(Object^ s, DataGridViewCellEventArgs^ e) {
            if (e->RowIndex >= 0 && dgvList->Columns[e->ColumnIndex]->Name == "Action") {
                String^ maDon = dgvList->Rows[e->RowIndex]->Cells["MaDon"]->Value->ToString();

                // [CHECK 1] Kiểm tra ngay đầu hàm
                if (!IsOrderPending(maDon)) {
                    MessageBox::Show(L"Đơn hàng này đã được xử lý rồi!", L"Cảnh báo", MessageBoxButtons::OK, MessageBoxIcon::Warning);
                    LoadDataFromSQL(this->CurrentStatus); // Refresh lại bảng
                    return;
                }

                // BƯỚC 1: TỰ ĐỘNG PHÂN BỔ (FEFO)
                PhanBoLoHangTuDong(maDon);

                // BƯỚC 2: KIỂM TRA ĐỦ HÀNG
                String^ loiChiTiet = "";
                if (!KiemTraDuHang(maDon, loiChiTiet)) {
                    MessageBox::Show(L"Không đủ hàng trong kho!\n" + loiChiTiet, L"Thiếu hàng", MessageBoxButtons::OK, MessageBoxIcon::Error);
                    return;
                }

                // BƯỚC 3: XÁC NHẬN
                if (MessageBox::Show(L"Xác nhận xuất kho cho đơn " + maDon + L"?", L"Xác nhận", MessageBoxButtons::YesNo) == System::Windows::Forms::DialogResult::Yes) {

                    // [CHECK 2 - QUAN TRỌNG NHẤT] Kiểm tra lại lần nữa trước khi trừ kho
                    // Phòng trường hợp click đúp hoặc lag mạng
                    if (!IsOrderPending(maDon)) {
                        MessageBox::Show(L"Đơn hàng đã được xử lý bởi thao tác trước đó!", L"Thông báo");
                        return;
                    }

                    // Gọi hàm trừ kho
                    ThucHienXuatKho(maDon);

                    // Cập nhật trạng thái ngay lập tức
                    UpdateStatus(maDon);

                    LoadDataFromSQL(this->CurrentStatus);
                    MessageBox::Show(L"Đã xuất kho thành công!", L"Thông báo");
                }
            }
        }

        void OnRowClick(Object^ s, DataGridViewCellEventArgs^ e) {
            if (e->RowIndex >= 0 && dgvList->Columns[e->ColumnIndex]->Name != "Action") {
                TransportDetailForm^ frm = gcnew TransportDetailForm(dgvList->Rows[e->RowIndex]->Cells["MaDon"]->Value->ToString(), this->CurrentMaKho);
                frm->ShowDialog();
            }
        }
        // Hàm hiển thị lỗi SQL chi tiết (Thêm vào class TransportTab hoặc để bên ngoài)
        void ShowSQLError(SQLHANDLE hStmt) {
            SQLCHAR sqlState[6];
            SQLCHAR msg[SQL_MAX_MESSAGE_LENGTH];
            SQLINTEGER nativeError;
            SQLSMALLINT msgLen;
            if (SQLGetDiagRecA(SQL_HANDLE_STMT, hStmt, 1, sqlState, &nativeError, msg, sizeof(msg), &msgLen) == SQL_SUCCESS) {
                String^ errMsg = gcnew String((char*)msg);
                MessageBox::Show(errMsg, L"Chi tiết lỗi SQL", MessageBoxButtons::OK, MessageBoxIcon::Error);
            }
        }
        void UpdateStatus(String^ maDon) {
            SQLHENV hEnv; SQLHDBC hDbc; SQLHSTMT hStmt;
            if (!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv))) return;
            SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
            SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
            SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

            std::string sMaDon = CleanString(ToAnsi(maDon));
            std::string sMaYeuCauGoc = "";

            // B1. Lấy mã gốc
            if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
                std::wstring sqlGet = L"SELECT MaYeuCauGoc FROM DonVanChuyen WHERE MaDonVC = ?";
                SQLPrepare(hStmt, (SQLWCHAR*)sqlGet.c_str(), SQL_NTS);
                SQLCHAR cMa[51]; strcpy_s((char*)cMa, 51, sMaDon.c_str());
                SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMa, 0, NULL);
                if (SQLExecute(hStmt) == SQL_SUCCESS && SQLFetch(hStmt) == SQL_SUCCESS) {
                    SQLCHAR cGoc[256]; SQLLEN len;
                    SQLGetData(hStmt, 1, SQL_C_CHAR, cGoc, 256, &len);
                    if (len != SQL_NULL_DATA) sMaYeuCauGoc = CleanString((char*)cGoc);
                }
                SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
            }

            // B2. Update Trạng Thái
            if (!sMaYeuCauGoc.empty()) {
                SQLSetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, 0);
                try {
                    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
                        // Update DonVanChuyen
                        std::wstring sql1 = L"UPDATE DonVanChuyen SET TrangThai = N'Đang Vận Chuyển' WHERE MaDonVC = ?";
                        SQLPrepare(hStmt, (SQLWCHAR*)sql1.c_str(), SQL_NTS);
                        SQLCHAR cMa[51]; strcpy_s((char*)cMa, 51, sMaDon.c_str());
                        SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMa, 0, NULL);
                        SQLExecute(hStmt);
                        SQLFreeStmt(hStmt, SQL_CLOSE);

                        // Update YeuCau [FIX: Dùng LTRIM/RTRIM để tìm chính xác]
                        std::wstring sql2 = L"UPDATE YeuCau SET HoanThanh = N'Đang Vận Chuyển' "
                            L"WHERE LTRIM(RTRIM(MaYeuCau)) = LTRIM(RTRIM(?))";
                        SQLPrepare(hStmt, (SQLWCHAR*)sql2.c_str(), SQL_NTS);

                        SQLCHAR cGoc[256]; strcpy_s((char*)cGoc, 256, sMaYeuCauGoc.c_str());
                        // Bind 255 ký tự để khớp với DB
                        SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, cGoc, 0, NULL);

                        SQLExecute(hStmt);
                        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
                    }
                    SQLEndTran(SQL_HANDLE_DBC, hDbc, SQL_COMMIT);
                }
                catch (...) {
                    SQLEndTran(SQL_HANDLE_DBC, hDbc, SQL_ROLLBACK);
                }
                SQLSetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_ON, 0);
            }
            SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        }
    };
}