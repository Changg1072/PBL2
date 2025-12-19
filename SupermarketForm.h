#pragma once
// ====================================================================
// 1. THƯ VIỆN HỆ THỐNG & SQL
// ====================================================================
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <vector>
#include <string>
#include <iostream>
#include <cctype>
#include <algorithm>
#include <vcclr.h>
#include <msclr/marshal_cppstd.h>

// ====================================================================
// 2. CÁC HEADER DỮ LIỆU & LOGIC
// ====================================================================
#include "DataModels.h"       
#include "QuanLyVanChuyen.h" 
#include "HeThongQuanLy.h"
#include "SaoLuu.h"
#include "SanPham.h"
#include "ChiTietYeuCau.h"
#include "DinhGiaDong.h"
#include "VietNamData.h"

// ====================================================================
// 3. HEADER FORM CON
// ====================================================================
#include "OrderConfirmationForm.h" 

// ====================================================================
// 4. NAMESPACE .NET
// ====================================================================
using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections::Generic;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::Data::Odbc;
using namespace System::Runtime::InteropServices;
using namespace System::IO;

#include <algorithm> 
#include <cctype>

void DocfileSanPham(SQLHDBC hDbc, std::vector<SanPham>& spList);
void DocfileKho(SQLHDBC hDbc, std::vector<Kho>& khoList);
void DocfilePhuongTien(SQLHDBC hDbc, std::vector<PhuongTien>& ptList);
void DocfileTuyenVanChuyen(SQLHDBC hDbc, std::vector<TuyenVanChuyen>& dsTVC);
void DocfileSieuThi(SQLHDBC hDbc, std::vector<SieuThi>& stList);
void GhiFileYeuCau(SQLHDBC hDbc, const ChiTietYeuCau& ctyc, const std::string& maSieuThi);
// Tìm dòng này và sửa lại thêm double chiPhi ở cuối
void GhiNhatKyNguonHang(SQLHDBC hDbc, std::string maYeuCau, std::string maKho, std::wstring loaiNguon, std::wstring ghiChu, double chiPhi);

static std::string FormTrim(const std::string& source) {
	std::string s(source);
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
		}).base(), s.end());
	return s;
}
static std::string CleanStdString(std::string s) {
	std::string res = "";
	for (size_t i = 0; i < s.length(); ++i) {
		if (!std::isspace((unsigned char)s[i])) {
			res += s[i];
		}
	}
	return res;
}
static std::string StdTrim(const std::string& str) {
	size_t first = str.find_first_not_of(' ');
	if (std::string::npos == first) return str;
	size_t last = str.find_last_not_of(' ');
	return str.substr(first, (last - first + 1));
}
inline std::string FromSQLWChar(SQLWCHAR* wstr) {
            if (!wstr) return "";
            std::wstring ws((wchar_t*)wstr);
            if (ws.empty()) return "";

            // Chuyển đổi sang UTF-8
            int size_needed = WideCharToMultiByte(CP_UTF8, 0, &ws[0], (int)ws.size(), NULL, 0, NULL, NULL);
            std::string strTo(size_needed, 0);
            WideCharToMultiByte(CP_UTF8, 0, &ws[0], (int)ws.size(), &strTo[0], size_needed, NULL, NULL);
            return strTo;
}
namespace PBL2QuanLyKho {
	public ref struct BatchInfo {
		String^ MaLo;
		String^ MaKho;
		DateTime NgayNhap;
		DateTime HanSuDung;     // Quan trọng
		double SoLuongTrongLo;
	};
	public ref struct OrderHeader {
		String^ MaDon;
		String^ TrangThai;
		DateTime NgayDat;
		double TongGiaTriHang;  // Tiền hàng
		double PhiVanChuyen;    // Phí ship
		double TongThanhToan;   // Tổng cộng
		String^ MaSieuThi;
		String^ TenSieuThi;
		String^ SDT;
		String^ DiaChiFull;     // Địa chỉ ghép

		// Timeline Vận chuyển
		String^ KhoDi;
		String^ KhoTC;
		String^ SieuThiNhan;
		DateTime NgayVanChuyen;
		DateTime NgayNhanDuKien;
		bool CoVanChuyen;       // Kiểm tra có đơn vận chuyển không

		OrderHeader() {
			MaDon = ""; TrangThai = ""; MaSieuThi = ""; TenSieuThi = ""; SDT = ""; DiaChiFull = "";
			KhoDi = ""; KhoTC = ""; SieuThiNhan = "";
			NgayDat = DateTime::Now; NgayVanChuyen = DateTime::Now; NgayNhanDuKien = DateTime::Now;
			TongGiaTriHang = 0; PhiVanChuyen = 0; TongThanhToan = 0; CoVanChuyen = false;
		}
	};
	public ref struct OrderDetail {
		String^ MaSP;
		String^ TenSP;
		double SoLuong;         // Tấn hoặc Kg tùy hiển thị
		double ThanhTien;
		String^ HinhAnh;        // Link ảnh (nếu có)
	};
	inline Color GetStatusColor(String^ status) {
		if (status == L"Hoàn thành" || status == "R" || status == "C") return Color::FromArgb(220, 255, 220); // Xanh lá nhạt
		if (status == L"Đang vận chuyển" || status == "S") return Color::FromArgb(220, 240, 255); // Xanh dương nhạt
		if (status == L"Chờ xác nhận" || status == "P") return Color::FromArgb(255, 250, 220); // Vàng nhạt
		return Color::White;
	}
	// --- GLOBAL HELPERS ---
	// --- GLOBAL HELPERS (ĐỒNG BỘ UTF-8 VỚI SAOLUU.CPP) ---
	static std::string CleanStdString(std::string s) {
		std::string res = "";
		for (size_t i = 0; i < s.length(); ++i) {
			if (!std::isspace((unsigned char)s[i])) {
				res += s[i];
			}
		}
		return res;
	}

	// Chuyển std::string (UTF-8) -> System::String^ (Unicode)
	static String^ ToSysString(std::string str) {
		if (str.empty()) return "";
		// Cách tối ưu: Dùng Constructor của String nhận trực tiếp con trỏ char* và Encoding
		// Cấu trúc: gcnew String(char_pointer, start_index, length, encoding)
		return gcnew String(str.c_str(), 0, (int)str.length(), System::Text::Encoding::UTF8);
	}

	// Chuyển System::String^ (Unicode) -> std::string (UTF-8)
	static std::string ToStdString(String^ sysString) {
		if (String::IsNullOrEmpty(sysString)) return "";

		// SỬA: Thêm "cli::" trước "array" để tránh nhầm với std::array
		cli::array<unsigned char>^ bytes = System::Text::Encoding::UTF8->GetBytes(sysString);

		pin_ptr<unsigned char> p = &bytes[0];

		// Ép kiểu từ unsigned char* sang char* để tạo std::string
		return std::string((char*)p, bytes->Length);
	}

	// Hàm này giữ lại để tương thích code cũ nếu có gọi, trỏ về ToStdString
	static std::string ToStringStandard(String^ sysString) {
		return ToStdString(sysString);
	}

	// ######################################################################################
	// CLASS 1: CART FORM (GIỎ HÀNG)
	// ######################################################################################
	public ref class CartForm : public System::Windows::Forms::Form {
	public:
		List<CartItem^>^ _cartItems;
		String^ _maSieuThi;
		Dictionary<String^, String^>^ _imageMap;

		CartForm(List<CartItem^>^ cartItems, String^ maST, Dictionary<String^, String^>^ imgMap) {
			_cartItems = cartItems; _maSieuThi = maST; _imageMap = imgMap;
			InitializeComponent();
			LoadDataToGrid();
		}

	private:
		DataGridView^ dgvCart;
		Label^ lblTotalLabel;
		Label^ lblTotalValue;
		Button^ btnOrder;
		std::string CleanStdString(std::string s) {
			std::string res = "";
			for (size_t i = 0; i < s.length(); ++i) {
				if (!isspace((unsigned char)s[i])) {
					res += s[i];
				}
			}
			return res;
		}
		void InitializeComponent() {
			this->Text = L"Giỏ Hàng Của Bạn";
			this->Size = System::Drawing::Size(950, 600);
			this->StartPosition = FormStartPosition::CenterParent;
			this->BackColor = Color::White;

			Label^ lblTitle = gcnew Label();
			lblTitle->Text = L"Danh Sách Đơn Hàng";
			lblTitle->Font = (gcnew System::Drawing::Font(L"Segoe UI", 14, FontStyle::Bold));
			lblTitle->Dock = DockStyle::Top;
			lblTitle->Height = 50;
			lblTitle->TextAlign = ContentAlignment::MiddleCenter;

			// --- FOOTER ---
			Panel^ pnlFooter = gcnew Panel();
			pnlFooter->Dock = DockStyle::Bottom;
			pnlFooter->Height = 80;
			pnlFooter->BackColor = Color::WhiteSmoke;
			pnlFooter->Padding = System::Windows::Forms::Padding(15);

			btnOrder = gcnew Button();
			btnOrder->Text = L"Đặt Hàng";
			btnOrder->BackColor = Color::FromArgb(238, 77, 45);
			btnOrder->ForeColor = Color::White;
			btnOrder->Dock = DockStyle::Right;
			btnOrder->Width = 160;
			btnOrder->FlatStyle = FlatStyle::Flat;
			btnOrder->FlatAppearance->BorderSize = 0;
			btnOrder->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, FontStyle::Bold));
			btnOrder->Cursor = Cursors::Hand;
			btnOrder->Click += gcnew EventHandler(this, &CartForm::btnOrder_Click);

			Panel^ pnlTextInfo = gcnew Panel();
			pnlTextInfo->Dock = DockStyle::Fill;
			pnlTextInfo->Padding = System::Windows::Forms::Padding(0, 0, 20, 0);

			lblTotalLabel = gcnew Label();
			lblTotalLabel->Text = L"Tổng thanh toán";
			lblTotalLabel->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, FontStyle::Regular));
			lblTotalLabel->ForeColor = Color::DimGray;
			lblTotalLabel->Dock = DockStyle::Top;
			lblTotalLabel->Height = 20;
			lblTotalLabel->TextAlign = ContentAlignment::BottomRight;

			lblTotalValue = gcnew Label();
			lblTotalValue->Text = L"0 đ";
			lblTotalValue->Font = (gcnew System::Drawing::Font(L"Segoe UI", 14, FontStyle::Bold));
			lblTotalValue->ForeColor = Color::FromArgb(238, 77, 45);
			lblTotalValue->Dock = DockStyle::Top;
			lblTotalValue->Height = 30;
			lblTotalValue->TextAlign = ContentAlignment::TopRight;

			pnlTextInfo->Controls->Add(lblTotalValue);
			pnlTextInfo->Controls->Add(lblTotalLabel);

			pnlFooter->Controls->Add(btnOrder);
			pnlFooter->Controls->Add(pnlTextInfo);
			btnOrder->SendToBack();
			pnlTextInfo->BringToFront();

			// --- GRID ---
			dgvCart = gcnew DataGridView();
			dgvCart->Dock = DockStyle::Fill;
			dgvCart->BackgroundColor = Color::White;
			dgvCart->BorderStyle = System::Windows::Forms::BorderStyle::None;
			dgvCart->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;
			dgvCart->RowTemplate->Height = 40;
			dgvCart->AllowUserToAddRows = false;
			dgvCart->ReadOnly = true;

			dgvCart->Columns->Add("TenSP", L"Tên SP");
			dgvCart->Columns->Add("Gia", L"Giá/Kg");
			dgvCart->Columns->Add("SL", L"SL (Kg)");
			dgvCart->Columns->Add("ThanhTien", L"Thành Tiền");

			DataGridViewButtonColumn^ btnEdit = gcnew DataGridViewButtonColumn();
			btnEdit->Name = "ColEdit"; btnEdit->HeaderText = L""; btnEdit->Text = L"Sửa SL";
			btnEdit->UseColumnTextForButtonValue = true; btnEdit->Width = 80; btnEdit->FlatStyle = FlatStyle::Flat;
			btnEdit->DefaultCellStyle->BackColor = Color::FromArgb(255, 224, 178); btnEdit->DefaultCellStyle->ForeColor = Color::DarkOrange;
			dgvCart->Columns->Add(btnEdit);

			DataGridViewButtonColumn^ btnDel = gcnew DataGridViewButtonColumn();
			btnDel->Name = "ColDel"; btnDel->HeaderText = L""; btnDel->Text = L"Xóa";
			btnDel->UseColumnTextForButtonValue = true; btnDel->Width = 60; btnDel->FlatStyle = FlatStyle::Flat;
			btnDel->DefaultCellStyle->BackColor = Color::LightPink; btnDel->DefaultCellStyle->ForeColor = Color::Red;
			dgvCart->Columns->Add(btnDel);

			dgvCart->CellContentClick += gcnew DataGridViewCellEventHandler(this, &CartForm::dgvCart_CellContentClick);

			this->Controls->Add(pnlFooter);
			this->Controls->Add(lblTitle);
			this->Controls->Add(dgvCart);
			pnlFooter->SendToBack();
			lblTitle->SendToBack();
			dgvCart->BringToFront();
		}

		void LoadDataToGrid() {
			dgvCart->Rows->Clear();
			double tongTienDon = 0;
			for each (CartItem ^ item in _cartItems) {
				dgvCart->Rows->Add(item->TenSP, item->GiaKg.ToString("N0"), item->SoLuong, item->ThanhTien.ToString("N0"));
				tongTienDon += item->ThanhTien;
			}
			lblTotalValue->Text = tongTienDon.ToString("N0") + " đ";
		}

		// =================================================================================
		// HÀM XỬ LÝ ĐẶT HÀNG (ĐÃ FIX TÊN SIÊU THỊ & LOGIC TÍNH GIÁ GIẢ ĐỊNH)
		// =================================================================================

		// --- HÀM HỖ TRỢ: Lấy thông tin chi tiết 1 siêu thị (Tối ưu) ---
		SieuThi LayChiTietSieuThi(SQLHDBC hDbc, std::string maST) {
			SieuThi stResult;
			stResult.setMaSieuThi(maST);

			SQLHSTMT hStmt;
			if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
				// Sửa: Dùng LIKE ?
				std::wstring sql = L"SELECT TenSieuThi, SDT, DiaChi, QuanHuyen, Tinh FROM SieuThi WHERE MaSieuThi LIKE ?";

				if (SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS) == SQL_SUCCESS) {
					// Thêm dấu % vào sau mã để tìm kiếm (VD: "ST001%")
					std::string sMa = CleanStdString(maST); // Chỉ lấy mã sạch, không cộng %
					SQLCHAR cMa[51]; strcpy_s((char*)cMa, 51, sMa.c_str());
					SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMa, 0, NULL);

					if (SQLExecute(hStmt) == SQL_SUCCESS) {
						if (SQLFetch(hStmt) == SQL_SUCCESS) {
							SQLWCHAR sTen[256], sSDT[50], sDC[256], sQuan[101], sTinh[101];
							SQLLEN len;

							SQLGetData(hStmt, 1, SQL_C_WCHAR, sTen, 256, &len);
							if (len != SQL_NULL_DATA) stResult.setTenSieuThi(FromSQLWChar(sTen));

							SQLGetData(hStmt, 2, SQL_C_WCHAR, sSDT, 50, &len);
							if (len != SQL_NULL_DATA) stResult.setSDT(FromSQLWChar(sSDT));

							SQLGetData(hStmt, 3, SQL_C_WCHAR, sDC, 256, &len);
							if (len != SQL_NULL_DATA) stResult.setDiaChi(FromSQLWChar(sDC));

							SQLGetData(hStmt, 4, SQL_C_WCHAR, sQuan, 101, &len);
							if (len != SQL_NULL_DATA) stResult.setQuanHuyen(FromSQLWChar(sQuan));

							SQLGetData(hStmt, 5, SQL_C_WCHAR, sTinh, 101, &len);
							if (len != SQL_NULL_DATA) stResult.setTinh(FromSQLWChar(sTinh));
						}
					}
				}
				SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
			}
			return stResult;
		}
		std::vector<PhuongTien> LoadPhuongTienToiUu(SQLHDBC hDbc, std::string maSieuThiDich) {
			std::vector<PhuongTien> ds;
			SQLHSTMT hStmt;

			if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
				std::wstring sql =
					L"SELECT DISTINCT P.MaXe, P.LoaiXe, P.SoXe, P.SucChuaTan, P.ChiPhiMoiChuyen, "
					L"P.Tinh, P.QuanHuyen, P.DiaChi, P.SDT, "
					L"P.PhuThemKhacTinh, P.PhuThemKhacQuanHuyen, "
					L"P.VanTocTB, P.GioCong, P.Lrate, P.Urate, P.Tsetup "
					L"FROM PhuongTien P "
					L"JOIN TuyenVanChuyen T ON P.MaXe = T.MaXe " // Bỏ LTRIM ở JOIN để tận dụng Index nếu có
					L"WHERE T.SieuThiNhan LIKE ?"; // Dùng LIKE

				if (SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS) == SQL_SUCCESS) {
					std::string sMa = maSieuThiDich + "%";
					SQLCHAR cMaST[51]; strcpy_s((char*)cMaST, 51, sMa.c_str());
					SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMaST, 0, NULL);

					if (SQLExecute(hStmt) == SQL_SUCCESS) {
						// Khai báo biến đệm
						SQLCHAR sMa[51]; SQLWCHAR sLoai[101], sTinh[101], sQuan[101], sDiaChi[256]; SQLCHAR sSDT[21];
						SQLINTEGER iSoXe;
						SQLDOUBLE dSucChua, dChiPhi, dPhuTinh, dPhuQuan, dVanToc, dGioCong, dLrate, dUrate, dTsetup;
						SQLLEN len;

						while (SQLFetch(hStmt) == SQL_SUCCESS) {
							SQLGetData(hStmt, 1, SQL_C_CHAR, sMa, 51, &len);
							SQLGetData(hStmt, 2, SQL_C_WCHAR, sLoai, 101, &len);
							SQLGetData(hStmt, 3, SQL_C_LONG, &iSoXe, 0, &len);
							SQLGetData(hStmt, 4, SQL_C_DOUBLE, &dSucChua, 0, &len);
							SQLGetData(hStmt, 5, SQL_C_DOUBLE, &dChiPhi, 0, &len);
							SQLGetData(hStmt, 6, SQL_C_WCHAR, sTinh, 101, &len);
							SQLGetData(hStmt, 7, SQL_C_WCHAR, sQuan, 101, &len);
							SQLGetData(hStmt, 8, SQL_C_WCHAR, sDiaChi, 256, &len);
							SQLGetData(hStmt, 9, SQL_C_CHAR, sSDT, 21, &len);
							SQLGetData(hStmt, 10, SQL_C_DOUBLE, &dPhuTinh, 0, &len);
							SQLGetData(hStmt, 11, SQL_C_DOUBLE, &dPhuQuan, 0, &len);
							SQLGetData(hStmt, 12, SQL_C_DOUBLE, &dVanToc, 0, &len);
							SQLGetData(hStmt, 13, SQL_C_DOUBLE, &dGioCong, 0, &len);
							SQLGetData(hStmt, 14, SQL_C_DOUBLE, &dLrate, 0, &len);
							SQLGetData(hStmt, 15, SQL_C_DOUBLE, &dUrate, 0, &len);
							SQLGetData(hStmt, 16, SQL_C_DOUBLE, &dTsetup, 0, &len);

							PhuongTien pt(
								FormTrim((char*)sMa), FromSQLWChar(sLoai), (int)iSoXe, (float)dSucChua, (float)dChiPhi,
								FromSQLWChar(sTinh), FromSQLWChar(sQuan), FromSQLWChar(sDiaChi), FormTrim((char*)sSDT),
								(float)dPhuTinh, (float)dPhuQuan, (float)dVanToc, (float)dGioCong, (float)dLrate, (float)dUrate, (float)dTsetup
							);
							ds.push_back(pt);
						}
					}
				}
				SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
			}
			return ds;
		}
		std::vector<TuyenVanChuyen> LoadTuyenVanChuyenToiUu(SQLHDBC hDbc, std::string maSieuThiDich) {
			std::vector<TuyenVanChuyen> ds;
			SQLHSTMT hStmt;

			if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
				std::wstring sql = L"SELECT MaTuyen, KhoDi, KhoTrungChuyen, SieuThiNhan, MaXe, ChiPhiTuyenCoDinh "
					L"FROM TuyenVanChuyen "
					L"WHERE SieuThiNhan LIKE ?"; // Bỏ LTRIM/RTRIM, dùng LIKE

				if (SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS) == SQL_SUCCESS) {
					std::string sMa = maSieuThiDich + "%";
					SQLCHAR cMaST[51]; strcpy_s((char*)cMaST, 51, sMa.c_str());
					SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMaST, 0, NULL);

					if (SQLExecute(hStmt) == SQL_SUCCESS) {
						SQLCHAR sMa[51], sKhoDi[256], sKhoTC[256], sSieuThi[256], sXe[51];
						SQLDOUBLE dChiPhi;
						SQLLEN lenMa, lenKhoDi, lenKhoTC, lenSieuThi, lenXe, lenChiPhi;

						while (SQLFetch(hStmt) == SQL_SUCCESS) {
							SQLGetData(hStmt, 1, SQL_C_CHAR, sMa, 51, &lenMa);
							SQLGetData(hStmt, 2, SQL_C_CHAR, sKhoDi, 256, &lenKhoDi);
							SQLGetData(hStmt, 3, SQL_C_CHAR, sKhoTC, 256, &lenKhoTC);
							SQLGetData(hStmt, 4, SQL_C_CHAR, sSieuThi, 256, &lenSieuThi);
							SQLGetData(hStmt, 5, SQL_C_CHAR, sXe, 51, &lenXe);
							SQLGetData(hStmt, 6, SQL_C_DOUBLE, &dChiPhi, sizeof(dChiPhi), &lenChiPhi);

							std::string ma = (lenMa == SQL_NULL_DATA) ? "" : FormTrim((char*)sMa);
							std::string kDi = (lenKhoDi == SQL_NULL_DATA) ? "" : FormTrim((char*)sKhoDi);
							std::string st = (lenSieuThi == SQL_NULL_DATA) ? "" : FormTrim((char*)sSieuThi);
							std::string xe = (lenXe == SQL_NULL_DATA) ? "" : FormTrim((char*)sXe);
							std::string kTC = (lenKhoTC == SQL_NULL_DATA) ? "" : FormTrim((char*)sKhoTC);
							float chiPhi = (lenChiPhi == SQL_NULL_DATA) ? 0.0f : (float)dChiPhi;

							ds.push_back(TuyenVanChuyen(ma, kDi, kTC, st, xe, chiPhi));
						}
					}
				}
				SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
			}
			return ds;
		}
		// [TỐI ƯU KHO] Chỉ tải các Kho có tuyến đường đi đến Siêu thị này
		std::vector<Kho> LoadKhoToiUu(SQLHDBC hDbc, std::string maSieuThiDich) {
			std::vector<Kho> ds;
			SQLHSTMT hStmt;

			if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
				std::wstring sql =
					L"SELECT DISTINCT K.MaKho, K.TenKho, K.SucChua, K.NhietDoMin, K.NhietDoMax, "
					L"K.DiaChi, K.Tinh, K.QuanHuyen, K.SDT, "
					L"K.CongSuatCoBan, K.HeSoCongSuat, K.ChiPhiHaNhiet, K.ChiPhiTangNhiet "
					L"FROM Kho K "
					L"JOIN TuyenVanChuyen T ON K.MaKho = T.KhoDi " // Bỏ LTRIM ở JOIN
					L"WHERE T.SieuThiNhan LIKE ?"; // Dùng LIKE

				if (SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS) == SQL_SUCCESS) {
					std::string sMa = maSieuThiDich + "%";
					SQLCHAR cMaST[51]; strcpy_s((char*)cMaST, 51, sMa.c_str());
					SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMaST, 0, NULL);

					if (SQLExecute(hStmt) == SQL_SUCCESS) {
						// Khai báo biến đệm (giữ nguyên code cũ)
						SQLCHAR sMa[51], sSDT[21];
						SQLWCHAR sTen[256], sDC[501], sTinh[101], sQuan[101];
						SQLDOUBLE dSuc, dMin, dMax, dCongSuat, dHeSo, dChiPhiHa, dChiPhiTang;
						SQLLEN len;

						while (SQLFetch(hStmt) == SQL_SUCCESS) {
							SQLGetData(hStmt, 1, SQL_C_CHAR, sMa, 51, &len);
							SQLGetData(hStmt, 2, SQL_C_WCHAR, sTen, 256, &len);
							SQLGetData(hStmt, 3, SQL_C_DOUBLE, &dSuc, 0, &len);
							SQLGetData(hStmt, 4, SQL_C_DOUBLE, &dMin, 0, &len);
							SQLGetData(hStmt, 5, SQL_C_DOUBLE, &dMax, 0, &len);
							SQLGetData(hStmt, 6, SQL_C_WCHAR, sDC, 501, &len);
							SQLGetData(hStmt, 7, SQL_C_WCHAR, sTinh, 101, &len);
							SQLGetData(hStmt, 8, SQL_C_WCHAR, sQuan, 101, &len);
							SQLGetData(hStmt, 9, SQL_C_CHAR, sSDT, 21, &len);
							SQLGetData(hStmt, 10, SQL_C_DOUBLE, &dCongSuat, 0, &len);
							SQLGetData(hStmt, 11, SQL_C_DOUBLE, &dHeSo, 0, &len);
							SQLGetData(hStmt, 12, SQL_C_DOUBLE, &dChiPhiHa, 0, &len);
							SQLGetData(hStmt, 13, SQL_C_DOUBLE, &dChiPhiTang, 0, &len);

							float congSuat = (len == SQL_NULL_DATA) ? 500.0f : (float)dCongSuat;
							float heSo = (len == SQL_NULL_DATA) ? 20.0f : (float)dHeSo;
							float cpHa = (len == SQL_NULL_DATA) ? 5000.0f : (float)dChiPhiHa;
							float cpTang = (len == SQL_NULL_DATA) ? 0.0f : (float)dChiPhiTang;

							Kho k(
								FormTrim((char*)sMa), FromSQLWChar(sTen), (float)dSuc, (float)dMin, (float)dMax,
								FromSQLWChar(sDC), FromSQLWChar(sTinh), FromSQLWChar(sQuan), FormTrim((char*)sSDT),
								congSuat, heSo, cpHa, cpTang
							);
							ds.push_back(k);
						}
					}
				}
				SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
			}
			return ds;
		}
		void btnOrder_Click(Object^ sender, EventArgs^ e) {
			if (_cartItems->Count == 0) {
				MessageBox::Show(L"Giỏ hàng đang trống!", L"Thông báo", MessageBoxButtons::OK, MessageBoxIcon::Warning);
				return;
			}

			SQLHENV hEnv = SQL_NULL_HENV; SQLHDBC hDbc = SQL_NULL_HDBC; SQLRETURN ret = SQL_ERROR;
			ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
			ret = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
			ret = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);

			SQLWCHAR* connStr = (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;";
			ret = SQLDriverConnect(hDbc, NULL, connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

			if (!SQL_SUCCEEDED(ret)) {
				MessageBox::Show(L"Lỗi kết nối CSDL!", L"Lỗi", MessageBoxButtons::OK, MessageBoxIcon::Error);
				if (hDbc) SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
				if (hEnv) SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
				return;
			}

			try {
				std::string stdMaST = "";
				if (_maSieuThi != nullptr) stdMaST = ToStringStandard(_maSieuThi);

				// --- BƯỚC 1: CHUẨN BỊ DỮ LIỆU TRONG BỘ NHỚ (CHƯA GHI DB) ---
				std::vector<SanPhamYeuCau> dsSPYC;
				float tongTanRequest = 0;
				double tongGiaTriThuc = 0;
				SQLHSTMT hStmtSP;
				// Cấp phát Statement để dùng lặp đi lặp lại
				if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmtSP) == SQL_SUCCESS) {

					for each (CartItem ^ item in _cartItems) {
						SanPhamYeuCau spyc;

						// Truy vấn nhanh: Lấy chi tiết đúng 1 sản phẩm theo ID
						std::wstring sql = L"SELECT TenSanPham, Loai, GiaVon, TheTich1Tan, NhietDoThichHop, NhomPhanLoai FROM SanPham WHERE MaSanPham = ?";
						SQLPrepare(hStmtSP, (SQLWCHAR*)sql.c_str(), SQL_NTS);

						std::string sMaSP = ToStringStandard(item->MaSP);
						SQLCHAR cMaSP[51]; strcpy_s((char*)cMaSP, 51, sMaSP.c_str());
						SQLBindParameter(hStmtSP, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMaSP, 0, NULL);

						// Mặc định tạo sản phẩm rỗng phòng hờ
						SanPham* spMoi = new SanPham();
						spMoi->setMaSP(sMaSP);

						if (SQLExecute(hStmtSP) == SQL_SUCCESS) {
							if (SQLFetch(hStmtSP) == SQL_SUCCESS) {
								SQLWCHAR sTen[256], sLoai[101], sNhom[51];
								SQLDOUBLE dVon, dVol, dTemp;
								SQLLEN len;

								SQLGetData(hStmtSP, 1, SQL_C_WCHAR, sTen, 256, &len);
								SQLGetData(hStmtSP, 2, SQL_C_WCHAR, sLoai, 101, &len);
								SQLGetData(hStmtSP, 3, SQL_C_DOUBLE, &dVon, 0, &len);
								SQLGetData(hStmtSP, 4, SQL_C_DOUBLE, &dVol, 0, &len);
								SQLGetData(hStmtSP, 5, SQL_C_DOUBLE, &dTemp, 0, &len);
								SQLGetData(hStmtSP, 6, SQL_C_WCHAR, sNhom, 51, &len);

								// Tạo đối tượng SanPham đầy đủ
								// Lưu ý: Các tham số 0,0,0,0 cuối là các trường phụ (Rủi ro, HSD...) không cần thiết cho vận chuyển
								delete spMoi; // Xóa cái cũ
								spMoi = new SanPham(
									sMaSP,
									FromSQLWChar(sTen),
									FromSQLWChar(sLoai),
									(float)dVon,
									(float)dVol,
									(float)dTemp,
									FromSQLWChar(sNhom),
									0, 0, 0, 0
								);
							}
						}
						SQLFreeStmt(hStmtSP, SQL_CLOSE); // Đóng cursor để tái sử dụng cho vòng lặp sau

						spyc.sp = spMoi;
						spyc.soLuongTan = (float)item->SoLuong / 1000.0f; // Đổi Kg -> Tấn
						spyc.ThanhTienRieng = (float)item->ThanhTien;

						dsSPYC.push_back(spyc);
						tongTanRequest += spyc.soLuongTan;
						tongGiaTriThuc += item->ThanhTien;
					}
					SQLFreeHandle(SQL_HANDLE_STMT, hStmtSP);
				}

				// Tạo đối tượng Yêu Cầu (Chỉ tạo Object, CHƯA GHI FILE)
				SieuThi tempST;
				std::string maYC = tempST.SinhMaYeuCau(hDbc, stdMaST, "Sieu Thi"); // Chỉ sinh mã ID
				ChiTietYeuCau ctyc(maYC, dsSPYC);

				time_t now = time(0); tm* ltm = localtime(&now);
				Date today(ltm->tm_mday, 1 + ltm->tm_mon, 1900 + ltm->tm_year);
				ctyc.setNgayNhap(today);
				ctyc.setGiaTri((float)tongGiaTriThuc);
				ctyc.setHoanThanh("Đang Chờ Xử Lý");

				// [FIX] KHÔNG GỌI GhiFileYeuCau Ở ĐÂY NỮA!

				// --- BƯỚC 2: TÍNH TOÁN VẬN CHUYỂN (TÍNH TOÁN THUẦN TÚY) ---
				// 1. Chỉ tải Kho (Số lượng thường ít < 100, tải hết cũng được)
				std::vector<Kho> dsKho = LoadKhoToiUu(hDbc, stdMaST);
				std::vector<PhuongTien> dsXe = LoadPhuongTienToiUu(hDbc, stdMaST);

				// 3. [TỐI ƯU] THAY VÌ TẢI HẾT TUYẾN, CHỈ TẢI TUYẾN LIÊN QUAN ĐẾN MÌNH
				std::vector<TuyenVanChuyen> dsTuyen = LoadTuyenVanChuyenToiUu(hDbc, stdMaST);

				// 4. [TỐI ƯU] KHÔNG TẢI LIST SIÊU THỊ, DÙNG HÀM LẤY CHI TIẾT TRỰC TIẾP
				SieuThi stDich = LayChiTietSieuThi(hDbc, stdMaST);

				// Kiểm tra nếu chưa có tên thì gán tạm
				if (stDich.getTenSieuThi().empty()) {
					stDich.setMaSieuThi(stdMaST);
					stDich.setTenSieuThi("Sieu Thi " + stdMaST);
				}

				// Gọi thuật toán tính toán (Giờ dữ liệu đầu vào đã nhỏ gọn, chạy rất nhanh)
				KetQuaSoSanh kq = QuanLyVanChuyen::TimHaiPhuongAn(tongTanRequest, stDich, dsKho, dsXe, dsTuyen);

				// Biến tạm để lưu thông tin nguồn hàng (Dùng để ghi sau này)
				std::string maKhoNguon = "";
				std::wstring loaiNguon = L"";
				double chiPhiVanChuyenLuu = 0;
				std::string ghiChuVanChuyen = "";

				// Xử lý Giả định nếu không tìm thấy tuyến
				double maxVal = std::numeric_limits<float>::max();
				if (kq.paReNhat.tongChiPhi >= maxVal || kq.paReNhat.tongChiPhi <= 1000) {
					kq = QuanLyVanChuyen::TimPhuongAnGiaDinh(tongTanRequest, stDich, dsKho, dsXe);
					if (kq.paReNhat.tongChiPhi < maxVal) {
						maKhoNguon = kq.paReNhat.maKhoXuat;
						loaiNguon = L"DieuPhoiTuDong";
						chiPhiVanChuyenLuu = kq.paReNhat.tongChiPhi;
						ghiChuVanChuyen = kq.paReNhat.ghiChu;
					}
					else {
						// Lỗi toàn tập
						kq.paReNhat.tongChiPhi = 0; kq.paNhanhNhat.tongChiPhi = 0;
						maKhoNguon = "UNKNOWN"; loaiNguon = L"Loi";
					}
				}
				else {
					maKhoNguon = kq.paReNhat.maKhoXuat;
					loaiNguon = L"CoSan";
					chiPhiVanChuyenLuu = kq.paReNhat.tongChiPhi;
					ghiChuVanChuyen = "Tuyen co san";
				}

				// [FIX] KHÔNG GHI NHẬT KÝ NGUỒN HÀNG Ở ĐÂY NỮA!

				// --- BƯỚC 3: HIỂN THỊ FORM XÁC NHẬN ---
				double fastCost = kq.paNhanhNhat.tongChiPhi;
				float fastTime = kq.paNhanhNhat.tongThoiGian;
				double stdCost = kq.paReNhat.tongChiPhi;
				float stdTime = kq.paReNhat.tongThoiGian;

				// Ngắt kết nối tạm thời để Form con sử dụng nếu cần (hoặc giữ nguyên)
				SQLDisconnect(hDbc);

				OrderConfirmationForm^ confirmForm = gcnew OrderConfirmationForm(
					_cartItems,
					_maSieuThi,
					ToSysString(stDich.getTenSieuThi()),
					ToSysString(stDich.getDiaChi()) + L"\n(" + ToSysString(ghiChuVanChuyen) + L")",
					ToSysString(stDich.getSDT()),
					_imageMap,
					IntPtr::Zero, // Không truyền DB handle để tránh conflict
					fastCost, fastTime,
					stdCost, stdTime,
					ToSysString(maYC)
				);

				// --- BƯỚC 4: NGƯỜI DÙNG BẤM OK -> MỚI BẮT ĐẦU GHI DB ---
				if (confirmForm->ShowDialog() == System::Windows::Forms::DialogResult::OK) {

					// Kết nối lại DB để ghi
					SQLDriverConnect(hDbc, NULL, connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

					// 1. GHI BẢNG YÊU CẦU (YeuCau + ChiTietYeuCau) - GIỜ MỚI GHI
					GhiFileYeuCau(hDbc, ctyc, stdMaST);

					// 2. GHI BẢNG PHÂN BỔ NGUỒN HÀNG (Log Supply Source) - GIỜ MỚI GHI
					if (!maKhoNguon.empty()) {
						// Cập nhật lại chi phí nếu người dùng chọn gói Nhanh
						double chiPhiFinal = (confirmForm->SelectedShippingMethod == "GiaoNhanh") ? fastCost : stdCost;
						GhiNhatKyNguonHang(hDbc, maYC, maKhoNguon, loaiNguon, L"Xac nhan boi nguoi dung", chiPhiFinal);
					}

					// 3. GHI ĐƠN VẬN CHUYỂN & TRỪ KHO
					PhuongAnToiUu selectedPA;
					if (confirmForm->SelectedShippingMethod == "GiaoNhanh") selectedPA = kq.paNhanhNhat;
					else selectedPA = kq.paReNhat;

					if (selectedPA.maPhuongTien.empty()) {
						selectedPA.maPhuongTien = (dsXe.size() > 0) ? dsXe[0].getMaXe() : "XE_AO";
						selectedPA.maKhoXuat = maKhoNguon;
						selectedPA.tongChiPhi = confirmForm->FinalShippingCost;
					}

					std::vector<HangCanGiao> dsHangGiao;
					for each (CartItem ^ item in _cartItems) {
						HangCanGiao hcg;
						hcg.maYeuCauGoc = maYC;
						hcg.maSP = ToStringStandard(item->MaSP);
						hcg.soLuong = (float)item->SoLuong / 1000.0f;
						hcg.thanhTien = (float)item->ThanhTien;

						// [QUAN TRỌNG] ĐỂ TRỐNG MÃ LÔ HÀNG LÚC TẠO ĐƠN
						// Để hệ thống bên Kho tự quyết định lúc duyệt
						hcg.maLoHang = "";

						dsHangGiao.push_back(hcg);
					}

					// Hàm này sẽ insert DonVanChuyen, ChiTietDonVC và cập nhật trạng thái YeuCau -> "ChoXacNhan"
					QuanLyVanChuyen::LuuDonHangVaoDB(hDbc, selectedPA, stdMaST, tongTanRequest, dsHangGiao);

					MessageBox::Show(L"Đặt hàng thành công! Mã đơn: " + ToSysString(maYC), L"Thành công", MessageBoxButtons::OK, MessageBoxIcon::Information);
					_cartItems->Clear();
					this->DialogResult = System::Windows::Forms::DialogResult::OK;
					this->Close();
				}
			}
			catch (const std::exception& ex) { MessageBox::Show(L"Lỗi: " + gcnew String(ex.what()), L"Lỗi", MessageBoxButtons::OK, MessageBoxIcon::Error); }
		cleanup:
			if (hDbc) { SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); }
			if (hEnv) { SQLFreeHandle(SQL_HANDLE_ENV, hEnv); }
		}
		void dgvCart_CellContentClick(Object^ sender, DataGridViewCellEventArgs^ e) {
			if (e->RowIndex < 0 || e->RowIndex >= _cartItems->Count) return;
			if (dgvCart->Columns[e->ColumnIndex]->Name == "ColEdit") {
				int oldQty = _cartItems[e->RowIndex]->SoLuong;
				Form^ inputForm = gcnew Form(); inputForm->Size = System::Drawing::Size(300, 180); inputForm->Text = L"Sửa số lượng";
				inputForm->StartPosition = FormStartPosition::CenterParent; inputForm->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
				Label^ lbl = gcnew Label(); lbl->Text = L"Nhập số lượng mới (Kg):"; lbl->Location = System::Drawing::Point(20, 20); lbl->AutoSize = true;
				NumericUpDown^ num = gcnew NumericUpDown(); num->Minimum = 1; num->Maximum = 1000000; num->Value = oldQty; num->Location = System::Drawing::Point(20, 50); num->Width = 240; num->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12));
				Button^ btnOk = gcnew Button(); btnOk->Text = L"Lưu"; btnOk->DialogResult = System::Windows::Forms::DialogResult::OK; btnOk->Location = System::Drawing::Point(100, 90); btnOk->Width = 80; btnOk->BackColor = Color::FromArgb(73, 129, 207); btnOk->ForeColor = Color::White;
				inputForm->Controls->Add(lbl); inputForm->Controls->Add(num); inputForm->Controls->Add(btnOk); inputForm->AcceptButton = btnOk;
				if (inputForm->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
					_cartItems[e->RowIndex]->SoLuong = (int)num->Value; _cartItems[e->RowIndex]->UpdateThanhTien(); LoadDataToGrid();
				}
			}
			if (dgvCart->Columns[e->ColumnIndex]->Name == "ColDel") {
				if (MessageBox::Show(L"Bạn có chắc muốn xóa?", L"Xác nhận", MessageBoxButtons::YesNo, MessageBoxIcon::Warning) == System::Windows::Forms::DialogResult::Yes) {
					_cartItems->RemoveAt(e->RowIndex); LoadDataToGrid();
				}
			}
		}
	}; // END CLASS CartForm

	// ######################################################################################
	// CLASS 2: PRODUCT DETAIL FORM
	// ######################################################################################
	public ref class ProductDetailForm : public System::Windows::Forms::Form
	{
	public:
		CartItem^ SelectedItem;
		bool IsAddedToCart = false;
		String^ ImageLink;
		ProductInfo^ currentInfo;
		ProductDetailForm(ProductInfo^ info, String^ imageLink) {
			this->ImageLink = imageLink; InitializeComponent(info);
		}

	private:
		System::Windows::Forms::Panel^ pnlMainCard;
		System::Windows::Forms::NumericUpDown^ numQuantity;
		void InitializeComponent(ProductInfo^ info)
		{
			this->currentInfo = info; this->WindowState = FormWindowState::Maximized; this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::None;
			this->BackColor = Color::FromArgb(50, 50, 50); this->Opacity = 0.95; this->StartPosition = FormStartPosition::CenterScreen;
			this->Click += gcnew EventHandler(this, &ProductDetailForm::Background_Click);

			pnlMainCard = gcnew Panel(); pnlMainCard->Size = System::Drawing::Size(400, 550);
			int x = (Screen::PrimaryScreen->Bounds.Width - pnlMainCard->Width) / 2;
			int y = (Screen::PrimaryScreen->Bounds.Height - pnlMainCard->Height) / 2;
			pnlMainCard->Location = System::Drawing::Point(x, y); pnlMainCard->BackColor = Color::White; pnlMainCard->Click += gcnew EventHandler(this, &ProductDetailForm::Card_Click);

			PictureBox^ picProduct = gcnew PictureBox(); picProduct->Dock = DockStyle::Top; picProduct->Height = 200;
			picProduct->BackColor = Color::FromArgb(240, 240, 240); picProduct->Tag = info->TenSP; picProduct->SizeMode = PictureBoxSizeMode::Zoom;
			if (!String::IsNullOrEmpty(this->ImageLink)) {
				try { picProduct->ImageLocation = this->ImageLink; }
				catch (...) { picProduct->Paint += gcnew PaintEventHandler(this, &ProductDetailForm::DrawPlaceholder); }
			}
			else { picProduct->Paint += gcnew PaintEventHandler(this, &ProductDetailForm::DrawPlaceholder); }

			Panel^ pnlInfo = gcnew Panel(); pnlInfo->Dock = DockStyle::Fill; pnlInfo->Padding = System::Windows::Forms::Padding(20);
			Label^ lblName = gcnew Label(); lblName->Text = info->TenSP; lblName->Font = (gcnew System::Drawing::Font(L"Segoe UI", 16, FontStyle::Bold)); lblName->AutoSize = true; lblName->Dock = DockStyle::Top;
			Label^ lblPrice = gcnew Label(); lblPrice->Text = (info->GiaHienTai / 1000.0).ToString("N0") + L" đ / Kg"; lblPrice->ForeColor = Color::Red; lblPrice->Font = (gcnew System::Drawing::Font(L"Segoe UI", 18, FontStyle::Bold)); lblPrice->AutoSize = true; lblPrice->Dock = DockStyle::Top; lblPrice->Padding = System::Windows::Forms::Padding(0, 5, 0, 15);

			Label^ lblDetail = gcnew Label();
			lblDetail->Text = L"🔖 Mã SP: " + info->MaSP + L"\n📂 Loại: " + info->Loai + L"\n🏭 Nhà SX: " + info->TenNCC + L"\n❄️ Bảo quản: " + info->NhietDo.ToString("N1") + L" độ C" + L"\n📦 Quy cách: Bán theo Kg";
			lblDetail->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11)); lblDetail->Dock = DockStyle::Top; lblDetail->AutoSize = true;

			Panel^ pnlAction = gcnew Panel(); pnlAction->Dock = DockStyle::Bottom; pnlAction->Height = 70; pnlAction->BackColor = Color::WhiteSmoke; pnlAction->Padding = System::Windows::Forms::Padding(10);
			Button^ btnAdd = gcnew Button(); btnAdd->Text = L"Thêm Vào Giỏ"; btnAdd->BackColor = Color::FromArgb(238, 77, 45); btnAdd->ForeColor = Color::White; btnAdd->Dock = DockStyle::Right; btnAdd->Width = 150; btnAdd->FlatStyle = FlatStyle::Flat; btnAdd->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, FontStyle::Bold)); btnAdd->Click += gcnew EventHandler(this, &ProductDetailForm::btnAdd_Click);
			numQuantity = gcnew NumericUpDown(); numQuantity->Minimum = 1; numQuantity->Maximum = 1000000; numQuantity->Increment = 1; numQuantity->Value = 1; numQuantity->Location = System::Drawing::Point(10, 30); numQuantity->Width = 120; numQuantity->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12));
			Label^ lblQ = gcnew Label(); lblQ->Text = "Số lượng (Kg):"; lblQ->Location = System::Drawing::Point(10, 5); lblQ->AutoSize = true; lblQ->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10));

			pnlAction->Controls->Add(lblQ); pnlAction->Controls->Add(numQuantity); pnlAction->Controls->Add(btnAdd);
			pnlInfo->Controls->Add(lblDetail); pnlInfo->Controls->Add(lblPrice); pnlInfo->Controls->Add(lblName);
			pnlMainCard->Controls->Add(pnlInfo); pnlMainCard->Controls->Add(picProduct); pnlMainCard->Controls->Add(pnlAction); this->Controls->Add(pnlMainCard);
		}
		void Background_Click(Object^ sender, EventArgs^ e) { this->Close(); }
		void Card_Click(Object^ sender, EventArgs^ e) {}
		void DrawPlaceholder(Object^ sender, PaintEventArgs^ e) {
			PictureBox^ p = (PictureBox^)sender; String^ name = (String^)p->Tag; if (String::IsNullOrEmpty(name)) return;
			int hash = name->GetHashCode(); Random^ rnd = gcnew Random(hash); Color bgCol = Color::FromArgb(215 + rnd->Next(40), 215 + rnd->Next(40), 215 + rnd->Next(40));
			p->BackColor = bgCol; String^ firstLetter = name->Substring(0, 1)->ToUpper(); System::Drawing::Font^ f = gcnew System::Drawing::Font("Segoe UI", 100, FontStyle::Bold);
			StringFormat^ sf = gcnew StringFormat(); sf->Alignment = StringAlignment::Center; sf->LineAlignment = StringAlignment::Center; e->Graphics->DrawString(firstLetter, f, Brushes::DarkSlateGray, p->ClientRectangle, sf);
		}
		void btnAdd_Click(Object^ sender, EventArgs^ e) {
			SelectedItem = gcnew CartItem(); SelectedItem->MaSP = currentInfo->MaSP; SelectedItem->TenSP = currentInfo->TenSP; SelectedItem->GiaKg = currentInfo->GiaHienTai / 1000.0;
			SelectedItem->SoLuong = (int)numQuantity->Value; SelectedItem->UpdateThanhTien(); IsAddedToCart = true; this->Close();
		}
	}; // END CLASS ProductDetailForm

	// ######################################################################################
	// CLASS PHỤ: FORM CẬP NHẬT THÔNG TIN (POPUP)
	// ######################################################################################
	public ref class UpdateInfoForm : public System::Windows::Forms::Form {
	public:
		// Các thuộc tính để Form cha lấy dữ liệu sau khi sửa
		String^ NewTen;
		String^ NewSDT;
		String^ NewDiaChi;
		String^ NewQuan;
		String^ NewTinh;

	private:
		TextBox^ txtTen;
		TextBox^ txtSDT;
		TextBox^ txtDiaChi;
		TextBox^ txtID; // Readonly
		ComboBox^ cboTinh;
		ComboBox^ cboQuan;
		Dictionary<String^, List<String^>^>^ dataVN;

	public:
		UpdateInfoForm(String^ ma, String^ ten, String^ sdt, String^ dc, String^ quan, String^ tinh) {
			InitializeComponent();

			// Load dữ liệu Tỉnh/Huyện từ VietnamData
			dataVN = VietnamData::GetFullData();
			for each (String ^ t in dataVN->Keys) {
				cboTinh->Items->Add(t);
			}

			// Điền dữ liệu cũ
			txtID->Text = ma;
			txtTen->Text = ten;
			txtSDT->Text = sdt;
			txtDiaChi->Text = dc;

			// Xử lý chọn lại Tỉnh/Huyện cũ
			if (cboTinh->Items->Contains(tinh)) {
				cboTinh->SelectedItem = tinh;
				// Sau khi chọn tỉnh, list Quận sẽ tự nạp, giờ ta chọn Quận
				if (cboQuan->Items->Contains(quan)) {
					cboQuan->SelectedItem = quan;
				}
			}
		}

	private:
		void InitializeComponent() {
			this->Text = L"Cập Nhật Thông Tin";
			this->Size = System::Drawing::Size(500, 550);
			this->StartPosition = FormStartPosition::CenterParent;
			this->BackColor = Color::White;
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			this->MaximizeBox = false;

			System::Drawing::Font^ fontLbl = gcnew System::Drawing::Font(L"Segoe UI", 10);
			System::Drawing::Font^ fontTxt = gcnew System::Drawing::Font(L"Segoe UI", 11);

			// ID (CỐ ĐỊNH)
			Label^ lblID = gcnew Label(); lblID->Text = L"Mã Siêu Thị (Cố định):"; lblID->Location = Point(30, 20); lblID->AutoSize = true; lblID->Font = fontLbl; lblID->ForeColor = Color::Gray;
			txtID = gcnew TextBox(); txtID->Location = Point(30, 45); txtID->Size = System::Drawing::Size(420, 30); txtID->Font = fontTxt;
			txtID->ReadOnly = true; txtID->BackColor = Color::WhiteSmoke;

			// TÊN
			Label^ lblTen = gcnew Label(); lblTen->Text = L"Tên hiển thị:"; lblTen->Location = Point(30, 85); lblTen->AutoSize = true; lblTen->Font = fontLbl;
			txtTen = gcnew TextBox(); txtTen->Location = Point(30, 110); txtTen->Size = System::Drawing::Size(420, 30); txtTen->Font = fontTxt;

			// SDT
			Label^ lblSDT = gcnew Label(); lblSDT->Text = L"Số điện thoại:"; lblSDT->Location = Point(30, 150); lblSDT->AutoSize = true; lblSDT->Font = fontLbl;
			txtSDT = gcnew TextBox(); txtSDT->Location = Point(30, 175); txtSDT->Size = System::Drawing::Size(420, 30); txtSDT->Font = fontTxt;

			// TỈNH / THÀNH PHỐ (COMBOBOX)
			Label^ lblTinh = gcnew Label(); lblTinh->Text = L"Tỉnh / Thành phố:"; lblTinh->Location = Point(30, 215); lblTinh->AutoSize = true; lblTinh->Font = fontLbl;
			cboTinh = gcnew ComboBox(); cboTinh->Location = Point(30, 240); cboTinh->Size = System::Drawing::Size(200, 30); cboTinh->Font = fontTxt;
			cboTinh->DropDownStyle = ComboBoxStyle::DropDownList;
			cboTinh->SelectedIndexChanged += gcnew EventHandler(this, &UpdateInfoForm::OnTinhChanged);

			// QUẬN / HUYỆN (COMBOBOX)
			Label^ lblQuan = gcnew Label(); lblQuan->Text = L"Quận / Huyện:"; lblQuan->Location = Point(250, 215); lblQuan->AutoSize = true; lblQuan->Font = fontLbl;
			cboQuan = gcnew ComboBox(); cboQuan->Location = Point(250, 240); cboQuan->Size = System::Drawing::Size(200, 30); cboQuan->Font = fontTxt;
			cboQuan->DropDownStyle = ComboBoxStyle::DropDownList;

			// ĐỊA CHỈ
			Label^ lblDC = gcnew Label(); lblDC->Text = L"Số nhà / Đường:"; lblDC->Location = Point(30, 280); lblDC->AutoSize = true; lblDC->Font = fontLbl;
			txtDiaChi = gcnew TextBox(); txtDiaChi->Location = Point(30, 305); txtDiaChi->Size = System::Drawing::Size(420, 30); txtDiaChi->Font = fontTxt;

			// BUTTONS
			Button^ btnSave = gcnew Button();
			btnSave->Text = L"Hoàn thành";
			btnSave->BackColor = Color::FromArgb(0, 114, 188);
			btnSave->ForeColor = Color::White;
			btnSave->FlatStyle = FlatStyle::Flat; btnSave->FlatAppearance->BorderSize = 0;
			btnSave->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11, FontStyle::Bold));
			btnSave->Size = System::Drawing::Size(200, 45);
			btnSave->Location = Point(30, 380);
			btnSave->Click += gcnew EventHandler(this, &UpdateInfoForm::OnSaveClick);

			Button^ btnCancel = gcnew Button();
			btnCancel->Text = L"Hủy bỏ";
			btnCancel->BackColor = Color::WhiteSmoke;
			btnCancel->FlatStyle = FlatStyle::Flat; btnCancel->FlatAppearance->BorderSize = 0;
			btnCancel->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11));
			btnCancel->Size = System::Drawing::Size(100, 45);
			btnCancel->Location = Point(250, 380);
			btnCancel->Click += gcnew EventHandler(this, &UpdateInfoForm::OnCancelClick);

			this->Controls->Add(btnCancel); this->Controls->Add(btnSave);
			this->Controls->Add(txtDiaChi); this->Controls->Add(lblDC);
			this->Controls->Add(cboQuan); this->Controls->Add(lblQuan);
			this->Controls->Add(cboTinh); this->Controls->Add(lblTinh);
			this->Controls->Add(txtSDT); this->Controls->Add(lblSDT);
			this->Controls->Add(txtTen); this->Controls->Add(lblTen);
			this->Controls->Add(txtID); this->Controls->Add(lblID);
		}

		// Sự kiện khi chọn Tỉnh -> Load lại Quận
		void OnTinhChanged(Object^ sender, EventArgs^ e) {
			cboQuan->Items->Clear();
			if (cboTinh->SelectedItem != nullptr) {
				String^ tinh = cboTinh->SelectedItem->ToString();
				if (dataVN->ContainsKey(tinh)) {
					List<String^>^ quanList = dataVN[tinh];
					for each (String ^ q in quanList) {
						cboQuan->Items->Add(q);
					}
					if (cboQuan->Items->Count > 0) cboQuan->SelectedIndex = 0;
				}
			}
		}

		void OnSaveClick(Object^ sender, EventArgs^ e) {
			// Validate sơ bộ
			if (String::IsNullOrWhiteSpace(txtTen->Text) || String::IsNullOrWhiteSpace(txtSDT->Text)) {
				MessageBox::Show(L"Vui lòng nhập Tên và SĐT!", L"Thiếu thông tin");
				return;
			}
			if (cboTinh->SelectedItem == nullptr || cboQuan->SelectedItem == nullptr) {
				MessageBox::Show(L"Vui lòng chọn Tỉnh và Quận/Huyện!", L"Thiếu thông tin");
				return;
			}

			NewTen = txtTen->Text;
			NewSDT = txtSDT->Text;
			NewDiaChi = txtDiaChi->Text;
			NewTinh = cboTinh->SelectedItem->ToString();
			NewQuan = cboQuan->SelectedItem->ToString();

			this->DialogResult = System::Windows::Forms::DialogResult::OK;
			this->Close();
		}

		void OnCancelClick(Object^ sender, EventArgs^ e) {
			this->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->Close();
		}
	};

	// ====================================================================
// POPUP: CHI TIẾT LÔ HÀNG & ẢNH SẢN PHẨM (ĐÃ CẬP NHẬT)
// ====================================================================
	public ref class ProductBatchForm : public System::Windows::Forms::Form {
	public:
		// Constructor nhận thêm tham số imagePath
		ProductBatchForm(String^ tenSP, String^ imagePath, List<BatchInfo^>^ batches) {
			this->Text = L"Chi tiết sản phẩm: " + tenSP;
			this->Size = System::Drawing::Size(750, 500); // Mở rộng chiều ngang để chứa ảnh
			this->StartPosition = FormStartPosition::CenterParent;
			this->BackColor = Color::White;
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedToolWindow;

			// 1. Ảnh Sản Phẩm (Bên Trái)
			PictureBox^ picBox = gcnew PictureBox();
			picBox->Size = System::Drawing::Size(280, 280);
			picBox->Location = Point(20, 20);
			picBox->SizeMode = PictureBoxSizeMode::Zoom;
			picBox->BorderStyle = BorderStyle::FixedSingle;
			picBox->Tag = tenSP; // Dùng cho hàm vẽ placeholder

			// Load ảnh an toàn
			bool loadSuccess = false;
			if (!String::IsNullOrEmpty(imagePath)) {
				try {
					if (System::IO::File::Exists(imagePath)) {
						picBox->Image = System::Drawing::Image::FromFile(imagePath);
						loadSuccess = true;
					}
				}
				catch (...) {}
			}

			if (!loadSuccess) {
				picBox->Paint += gcnew PaintEventHandler(this, &ProductBatchForm::DrawPlaceholder);
			}

			this->Controls->Add(picBox);

			// 2. Thông tin tên (Bên Phải)
			Label^ lblName = gcnew Label();
			lblName->Text = tenSP;
			lblName->Font = gcnew System::Drawing::Font("Segoe UI", 14, FontStyle::Bold);
			lblName->Location = Point(320, 20);
			lblName->AutoSize = true;
			lblName->MaximumSize = System::Drawing::Size(400, 0); // Tự xuống dòng nếu tên dài
			this->Controls->Add(lblName);

			Label^ lblTitleTable = gcnew Label();
			lblTitleTable->Text = L"📦 Lịch sử xuất kho (Lô hàng & HSD):";
			lblTitleTable->Font = gcnew System::Drawing::Font("Segoe UI", 10, FontStyle::Underline);
			lblTitleTable->Location = Point(320, 80);
			lblTitleTable->AutoSize = true;
			this->Controls->Add(lblTitleTable);

			// 3. Bảng Lô Hàng (Bên Phải, dưới tên)
			DataGridView^ dgv = gcnew DataGridView();
			dgv->Location = Point(320, 110);
			dgv->Size = System::Drawing::Size(390, 330);
			dgv->BackgroundColor = Color::WhiteSmoke;
			dgv->RowHeadersVisible = false;
			dgv->AllowUserToAddRows = false; dgv->ReadOnly = true;

			dgv->Columns->Add("MaLo", L"Mã Lô");
			dgv->Columns->Add("HSD", L"Hạn Sử Dụng");
			dgv->Columns->Add("SL", L"SL Lấy");

			dgv->Columns[0]->Width = 90;
			dgv->Columns[2]->Width = 70;
			dgv->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;

			for each (auto b in batches) {
				dgv->Rows->Add(b->MaLo, b->HanSuDung.ToString("dd/MM/yyyy"), b->SoLuongTrongLo.ToString("N2"));
			}

			this->Controls->Add(dgv);
		}

		// Hàm vẽ ảnh thay thế nếu không tìm thấy file ảnh
		void DrawPlaceholder(Object^ sender, PaintEventArgs^ e) {
			PictureBox^ p = (PictureBox^)sender;
			String^ text = (String^)p->Tag;

			e->Graphics->Clear(Color::LightGray);
			String^ letter = (String::IsNullOrEmpty(text)) ? "?" : text->Substring(0, 1);

			System::Drawing::Font^ f = gcnew System::Drawing::Font("Segoe UI", 70, FontStyle::Bold);
			SizeF s = e->Graphics->MeasureString(letter, f);

			e->Graphics->DrawString(letter, f, Brushes::DimGray,
				(p->Width - s.Width) / 2, (p->Height - s.Height) / 2);
		}
	};
	// ====================================================================
// HÀM HELPER: LẤY CHI TIẾT ĐƠN HÀNG (Bridge SQL -> Managed Form)
// ====================================================================
	// Lấy thông tin chi tiết Lô hàng của 1 sản phẩm (Deep Dive)
	inline List<BatchInfo^>^ LayThongTinLoHang(SQLHDBC hDbc, String^ maSP) {
		List<BatchInfo^>^ list = gcnew List<BatchInfo^>();
		SQLHSTMT hStmt;
		if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
			// Truy vấn Lô hàng + Chi tiết lô để lấy HSD
			std::wstring sql = L"SELECT L.MaLoHang, L.MaKho, L.NgayNhap, C.HanSuDung, C.SoLuongTan "
				L"FROM LoHang L JOIN ChiTietLoHang C ON L.MaLoHang = C.MaLoHang "
				L"WHERE C.MaSanPham = ? ORDER BY C.HanSuDung ASC";

			SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);
			std::string sMaSP = ToStdString(maSP);
			SQLCHAR cMaSP[51]; strcpy_s((char*)cMaSP, 51, sMaSP.c_str());
			SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMaSP, 0, NULL);

			if (SQLExecute(hStmt) == SQL_SUCCESS) {
				SQLCHAR sMaLo[51], sMaKho[51];
				SQL_DATE_STRUCT dNhap, dHSD;
				SQLDOUBLE dSL;
				SQLLEN len;
				while (SQLFetch(hStmt) == SQL_SUCCESS) {
					BatchInfo^ b = gcnew BatchInfo();
					SQLGetData(hStmt, 1, SQL_C_CHAR, sMaLo, 51, &len); b->MaLo = ToSysString(FormTrim((char*)sMaLo));
					SQLGetData(hStmt, 2, SQL_C_CHAR, sMaKho, 51, &len); b->MaKho = ToSysString(FormTrim((char*)sMaKho));
					SQLGetData(hStmt, 3, SQL_C_TYPE_DATE, &dNhap, 0, &len); b->NgayNhap = DateTime(dNhap.year, dNhap.month, dNhap.day);
					SQLGetData(hStmt, 4, SQL_C_TYPE_DATE, &dHSD, 0, &len); b->HanSuDung = DateTime(dHSD.year, dHSD.month, dHSD.day);
					SQLGetData(hStmt, 5, SQL_C_DOUBLE, &dSL, 0, &len); b->SoLuongTrongLo = (double)dSL;
					list->Add(b);
				}
			}
		}
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		return list;
	}

	// Lấy chi tiết đơn hàng (Header + Product List + Shipping Info)
	inline void LayChiTietDonHangFull(SQLHDBC hDbc, String^ maDon, [Out] OrderHeader^% header, [Out] List<OrderDetail^>^% details) {
		header = gcnew OrderHeader();
		details = gcnew List<OrderDetail^>();
		std::string sMaYC = StdTrim(ToStdString(maDon));
		SQLHSTMT hStmt;

		// --- PHẦN 1: HEADER ---
		if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
			// Chỉ lấy thông tin từ PhanBoNguonHang và SieuThi
			std::wstring sql =
				L"SELECT Y.MaYeuCau, Y.HoanThanh, Y.NgayYeuCau, Y.GiaTriTong, "
				L"ISNULL(S.TenSieuThi, 'Khach Le'), ISNULL(S.SDT, ''), ISNULL(S.DiaChi, ''), ISNULL(S.QuanHuyen, ''), ISNULL(S.Tinh, ''), "
				L"ISNULL(P.ChiPhiVanChuyen, 0), ISNULL(P.LoaiNguon, '') "
				L"FROM YeuCau Y "
				L"LEFT JOIN SieuThi S ON Y.MaSieuThi = S.MaSieuThi "
				L"LEFT JOIN PhanBoNguonHang P ON LTRIM(RTRIM(Y.MaYeuCau)) = LTRIM(RTRIM(P.MaYeuCau)) "
				L"WHERE LTRIM(RTRIM(Y.MaYeuCau)) = ?";

			SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);
			SQLCHAR cMa[51]; strcpy_s((char*)cMa, 51, sMaYC.c_str());
			SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMa, 0, NULL);

			if (SQLExecute(hStmt) == SQL_SUCCESS) {
				if (SQLFetch(hStmt) == SQL_SUCCESS) {
					SQLCHAR sMaY[51];
					SQLWCHAR wTT[51], wTenST[256], wSDT[51], wDC[256], wQuan[101], wTinh[101], wNguon[51];
					SQL_DATE_STRUCT dDat;
					SQLDOUBLE dGiaTri, dPhiVC;
					SQLLEN len;

					SQLGetData(hStmt, 1, SQL_C_CHAR, sMaY, 51, &len); header->MaDon = ToSysString(StdTrim((char*)sMaY));
					SQLGetData(hStmt, 2, SQL_C_WCHAR, wTT, 51, &len);
					String^ rawStatus = gcnew String((wchar_t*)wTT); rawStatus = rawStatus->Trim();
					if (rawStatus == "C") header->TrangThai = L"Chờ xác nhận";
					else if (rawStatus == "S") header->TrangThai = L"Đang vận chuyển";
					else if (rawStatus == "R") header->TrangThai = L"Hoàn thành";
					else header->TrangThai = rawStatus;

					SQLGetData(hStmt, 3, SQL_C_TYPE_DATE, &dDat, 0, &len); header->NgayDat = DateTime(dDat.year, dDat.month, dDat.day);
					SQLGetData(hStmt, 4, SQL_C_DOUBLE, &dGiaTri, 0, &len); header->TongGiaTriHang = dGiaTri;

					SQLGetData(hStmt, 5, SQL_C_WCHAR, wTenST, 256, &len); header->TenSieuThi = gcnew String((wchar_t*)wTenST);
					SQLGetData(hStmt, 6, SQL_C_WCHAR, wSDT, 51, &len); header->SDT = gcnew String((wchar_t*)wSDT);

					SQLGetData(hStmt, 7, SQL_C_WCHAR, wDC, 256, &len);
					SQLGetData(hStmt, 8, SQL_C_WCHAR, wQuan, 101, &len);
					SQLGetData(hStmt, 9, SQL_C_WCHAR, wTinh, 101, &len);
					header->DiaChiFull = gcnew String((wchar_t*)wDC) + ", " + gcnew String((wchar_t*)wQuan) + ", " + gcnew String((wchar_t*)wTinh);

					// Lấy phí ship từ PhanBoNguonHang
					SQLGetData(hStmt, 10, SQL_C_DOUBLE, &dPhiVC, 0, &len);
					header->PhiVanChuyen = (len == SQL_NULL_DATA) ? 0 : dPhiVC;

					SQLGetData(hStmt, 11, SQL_C_WCHAR, wNguon, 51, &len);
					if (len != SQL_NULL_DATA) {
						header->CoVanChuyen = true; // Có dữ liệu phân bổ = Có vận chuyển
						header->KhoDi = gcnew String((wchar_t*)wNguon); // Tạm lấy cột này làm ghi chú kho
					}
					else {
						header->CoVanChuyen = false;
					}

					header->TongThanhToan = header->TongGiaTriHang + header->PhiVanChuyen;

					// Set mặc định ngày (vì không lấy từ VanChuyen nữa)
					header->NgayVanChuyen = header->NgayDat.AddDays(1);
					header->NgayNhanDuKien = header->NgayDat.AddDays(3);
				}
			}
		}
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);

		// --- PHẦN 2: CHI TIẾT (GIỮ NGUYÊN) ---
		if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
			std::wstring sqlD = L"SELECT C.MaSanPham, S.TenSanPham, C.SoTanYeuCau, C.GiaTri "
				L"FROM ChiTietYeuCau C JOIN SanPham S ON C.MaSanPham = S.MaSanPham "
				L"WHERE LTRIM(RTRIM(C.MaYeuCau)) = ?";
			SQLPrepare(hStmt, (SQLWCHAR*)sqlD.c_str(), SQL_NTS);
			SQLCHAR cMa[51]; strcpy_s((char*)cMa, 51, sMaYC.c_str());
			SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMa, 0, NULL);

			if (SQLExecute(hStmt) == SQL_SUCCESS) {
				SQLCHAR sMaSP[51]; SQLWCHAR wTenSP[256]; SQLDOUBLE dSL, dTien; SQLLEN len;
				while (SQLFetch(hStmt) == SQL_SUCCESS) {
					OrderDetail^ d = gcnew OrderDetail();
					SQLGetData(hStmt, 1, SQL_C_CHAR, sMaSP, 51, &len); d->MaSP = ToSysString(StdTrim((char*)sMaSP));
					SQLGetData(hStmt, 2, SQL_C_WCHAR, wTenSP, 256, &len); d->TenSP = gcnew String((wchar_t*)wTenSP);
					SQLGetData(hStmt, 3, SQL_C_DOUBLE, &dSL, 0, &len); d->SoLuong = dSL;
					SQLGetData(hStmt, 4, SQL_C_DOUBLE, &dTien, 0, &len); d->ThanhTien = dTien;
					details->Add(d);
				}
			}
		}
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	}
	// ====================================================================
	// FORM CHI TIẾT ĐƠN HÀNG (CÓ ẢNH & POPUP LÔ HÀNG)
	// ====================================================================
	public ref class OrderDetailForm : public System::Windows::Forms::Form {
		OrderHeader^ header;
		List<OrderDetail^>^ details;
		System::IntPtr hDbcPtr;
		Dictionary<String^, String^>^ _imageMap;
		Button^ btnClose; // Nút thoát thủ công

	public:
		OrderDetailForm(OrderHeader^ h, List<OrderDetail^>^ d, System::IntPtr dbPtr, Dictionary<String^, String^>^ imgMap) {
			header = h; details = d; hDbcPtr = dbPtr; _imageMap = imgMap;
			InitializeComponent();
			// Đăng ký sự kiện
			this->Deactivate += gcnew EventHandler(this, &OrderDetailForm::OnDeactivate);
			this->Paint += gcnew PaintEventHandler(this, &OrderDetailForm::DrawBorder);
		}

	private:
		// 1. Tự động đóng khi click sang ứng dụng khác hoặc màn hình Desktop
		void OnDeactivate(Object^ sender, EventArgs^ e) {
			this->Close();
		}

		// 2. Hàm vẽ viền (Border) màu xám đậm
		void DrawBorder(Object^ sender, PaintEventArgs^ e) {
			Control^ c = (Control^)sender;
			// Vẽ viền dày 2px màu xám đậm
			Pen^ p = gcnew Pen(Color::DimGray, 2);
			System::Drawing::Rectangle rect = c->ClientRectangle;
			rect.Width -= 1; rect.Height -= 1;
			e->Graphics->DrawRectangle(p, rect);
		}

		// 3. Xử lý khi bấm nút X hoặc phím ESC
		void OnCloseClick(Object^ sender, EventArgs^ e) {
			this->Close();
		}

		void OnProductClick(Object^ sender, DataGridViewCellEventArgs^ e) {
			if (e->RowIndex < 0) return;
			if (header->TrangThai != L"Hoàn thành" && header->TrangThai != L"Đã Giao Hàng") {
				MessageBox::Show(L"Bạn chỉ có thể xem chi tiết Lô/Hạn Sử Dụng sau khi đã nhận hàng thành công!\n\n(Lý do: Kho hàng đang xử lý phân bổ lô theo nguyên tắc FEFO)",
					L"Thông tin chưa khả dụng", MessageBoxButtons::OK, MessageBoxIcon::Information);
				return; // Dừng lại, không mở Form chi tiết lô
			}

			// --- Nếu đã hoàn thành thì chạy tiếp code cũ bên dưới ---

			String^ maSP = details[e->RowIndex]->MaSP;
			String^ tenSP = details[e->RowIndex]->TenSP;

			String^ imgPath = "";
			if (_imageMap != nullptr && _imageMap->ContainsKey(maSP)) {
				imgPath = _imageMap[maSP];
			}

			// Lấy handle kết nối database từ con trỏ
			SQLHDBC hDbc = (SQLHDBC)hDbcPtr.ToPointer();

			// Lấy danh sách lô hàng thực tế của sản phẩm
			List<BatchInfo^>^ batches = LayThongTinLoHang(hDbc, maSP);

			// Mở form hiển thị chi tiết
			ProductBatchForm^ frm = gcnew ProductBatchForm(tenSP, imgPath, batches);
			frm->ShowDialog();
		}
		void InitializeComponent() {
			this->Size = System::Drawing::Size(900, 800);
			this->StartPosition = FormStartPosition::CenterParent;
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::None; // Bỏ viền Windows
			this->BackColor = Color::White;

			// [QUAN TRỌNG] Padding để nội dung không đè lên viền vẽ
			this->Padding = System::Windows::Forms::Padding(2);

			// --- TẠO NÚT "X" ĐỂ THOÁT ---
			btnClose = gcnew Button();
			btnClose->Text = L"×";
			btnClose->Font = (gcnew System::Drawing::Font(L"Arial", 12, FontStyle::Bold));
			btnClose->ForeColor = Color::Black;
			btnClose->BackColor = Color::White;
			btnClose->FlatStyle = FlatStyle::Flat;
			btnClose->FlatAppearance->BorderSize = 0;
			btnClose->Size = System::Drawing::Size(30, 30);
			btnClose->Location = System::Drawing::Point(850, 5); // Góc trên bên phải
			btnClose->Cursor = Cursors::Hand;
			btnClose->Anchor = static_cast<AnchorStyles>(AnchorStyles::Top | AnchorStyles::Right);
			btnClose->Click += gcnew EventHandler(this, &OrderDetailForm::OnCloseClick);
			this->Controls->Add(btnClose);

			// Gán nút này cho phím ESC (ấn ESC là thoát)
			this->CancelButton = btnClose;

			Panel^ mainPanel = gcnew Panel();
			mainPanel->Dock = DockStyle::Fill;
			mainPanel->BackColor = Color::White;
			// Padding top để tránh đè lên nút X
			mainPanel->Padding = System::Windows::Forms::Padding(0, 40, 0, 0);
			this->Controls->Add(mainPanel);

			FlowLayoutPanel^ flow = gcnew FlowLayoutPanel();
			flow->Dock = DockStyle::Fill;
			flow->FlowDirection = FlowDirection::TopDown;
			flow->WrapContents = false;
			flow->AutoScroll = true;
			flow->Padding = System::Windows::Forms::Padding(20, 0, 20, 20);
			mainPanel->Controls->Add(flow);

			// 1. Header
			Label^ lblTitle = gcnew Label(); lblTitle->Text = L"THÔNG TIN ĐƠN HÀNG #" + header->MaDon;
			lblTitle->Font = gcnew System::Drawing::Font("Segoe UI", 16, FontStyle::Bold); lblTitle->AutoSize = true;
			flow->Controls->Add(lblTitle);

			GroupBox^ grpInfo = gcnew GroupBox(); grpInfo->Text = L"Thông tin Siêu Thị"; grpInfo->Size = System::Drawing::Size(820, 90);
			Label^ l1 = gcnew Label(); l1->Text = L"Tên: " + header->TenSieuThi + L"\nSĐT: " + header->SDT + L"\nĐ/C: " + header->DiaChiFull;
			l1->Dock = DockStyle::Fill; l1->Padding = System::Windows::Forms::Padding(10);
			grpInfo->Controls->Add(l1); flow->Controls->Add(grpInfo);

			// 2. Danh sách sản phẩm
			Label^ lblProd = gcnew Label(); 
			if (header->TrangThai == L"Hoàn thành" || header->TrangThai == L"Đã Giao Hàng") {
				lblProd->Text = L"Danh sách sản phẩm (Bấm vào SP để xem chi tiết Lô)";
			}
			else {
				lblProd->Text = L"Danh sách sản phẩm";
			}
			lblProd->Font = gcnew System::Drawing::Font("Segoe UI", 11, FontStyle::Bold); lblProd->Margin = System::Windows::Forms::Padding(0, 20, 0, 5); lblProd->AutoSize = true;
			flow->Controls->Add(lblProd);

			DataGridView^ dgv = gcnew DataGridView();
			dgv->Width = 820; dgv->Height = 350;
			dgv->BackgroundColor = Color::White;
			dgv->AllowUserToAddRows = false; dgv->ReadOnly = true;
			dgv->RowTemplate->Height = 60;
			dgv->SelectionMode = DataGridViewSelectionMode::FullRowSelect;

			DataGridViewImageColumn^ imgCol = gcnew DataGridViewImageColumn();
			imgCol->HeaderText = L"Ảnh"; imgCol->Name = "Img"; imgCol->ImageLayout = DataGridViewImageCellLayout::Zoom; imgCol->Width = 80;
			dgv->Columns->Add(imgCol);

			DataGridViewTextBoxColumn^ colTen = gcnew DataGridViewTextBoxColumn();
			colTen->Name = "Ten"; colTen->HeaderText = L"Tên Sản Phẩm"; colTen->Width = 300;
			dgv->Columns->Add(colTen);

			DataGridViewTextBoxColumn^ colSL = gcnew DataGridViewTextBoxColumn();
			colSL->Name = "SL"; colSL->HeaderText = L"Số Lượng (Tấn)"; colSL->Width = 120;
			dgv->Columns->Add(colSL);

			DataGridViewTextBoxColumn^ colTien = gcnew DataGridViewTextBoxColumn();
			colTien->Name = "Tien"; colTien->HeaderText = L"Thành Tiền"; colTien->AutoSizeMode = DataGridViewAutoSizeColumnMode::Fill;
			dgv->Columns->Add(colTien);

			dgv->CellClick += gcnew DataGridViewCellEventHandler(this, &OrderDetailForm::OnProductClick);

			for each (auto p in details) {
				System::Drawing::Image^ img = nullptr;
				if (_imageMap != nullptr && _imageMap->ContainsKey(p->MaSP)) {
					String^ path = _imageMap[p->MaSP];
					try { if (System::IO::File::Exists(path)) img = System::Drawing::Image::FromFile(path); }
					catch (...) {}
				}
				if (img == nullptr) img = gcnew Bitmap(1, 1);
				dgv->Rows->Add(img, p->TenSP, p->SoLuong, p->ThanhTien.ToString("N0") + L" đ");
			}
			flow->Controls->Add(dgv);

			// 3. Vận chuyển & Tổng tiền
			GroupBox^ grpShip = gcnew GroupBox();
			grpShip->Text = L"Vận Chuyển & Thời Gian"; grpShip->Size = System::Drawing::Size(820, 110);
			String^ shipText = (header->CoVanChuyen) ?
				L"🚚 Tuyến: " + header->KhoDi + L" ➔ " + header->SieuThiNhan +
				L"\n🕒 Ngày đặt: " + header->NgayDat.ToString("dd/MM/yyyy") +
				L"  |  Nhận: " + header->NgayNhanDuKien.ToString("dd/MM/yyyy") : L"Không vận chuyển";
			Label^ lShip = gcnew Label(); lShip->Text = shipText; lShip->Dock = DockStyle::Fill; lShip->Padding = System::Windows::Forms::Padding(10);
			grpShip->Controls->Add(lShip); flow->Controls->Add(grpShip);

			Panel^ pnlTotal = gcnew Panel(); pnlTotal->Size = System::Drawing::Size(820, 100);
			Label^ lT3 = gcnew Label(); lT3->Text = L"TỔNG CỘNG: " + header->TongThanhToan.ToString("N0") + L" đ";
			lT3->Location = Point(500, 30); lT3->AutoSize = true; lT3->Font = gcnew System::Drawing::Font("Segoe UI", 16, FontStyle::Bold); lT3->ForeColor = Color::Red;
			pnlTotal->Controls->Add(lT3); flow->Controls->Add(pnlTotal);

			// Đảm bảo nút Close luôn nằm trên cùng
			btnClose->BringToFront();
		}
	};
	public ref class ProductComparerHelper : System::Collections::Generic::IComparer<ProductInfo^>
	{
		int _mode; // 1: Tên A-Z, 2: Giá Tăng, 3: Giá Giảm
	public:
		ProductComparerHelper(int mode) { _mode = mode; }

		virtual int Compare(ProductInfo^ p1, ProductInfo^ p2) {
			if (_mode == 1) return String::Compare(p1->TenSP, p2->TenSP);
			if (_mode == 2) {
				if (p1->GiaHienTai > p2->GiaHienTai) return 1;
				if (p1->GiaHienTai < p2->GiaHienTai) return -1;
				return 0;
			}
			if (_mode == 3) {
				if (p1->GiaHienTai < p2->GiaHienTai) return 1;
				if (p1->GiaHienTai > p2->GiaHienTai) return -1;
				return 0;
			}
			return 0;
		}
	};

	// ######################################################################################
	// CLASS 3: MAIN FORM (SUPERMARKET)
	// ######################################################################################
	inline std::string LayMaSieuThiTuTaiKhoan(String^ username) {
		std::string maSieuThi = "";
		std::string sUser = CleanStdString(ToStdString(username));

		SQLHENV hEnv = SQL_NULL_HENV; SQLHDBC hDbc = SQL_NULL_HDBC; SQLHSTMT hStmt = SQL_NULL_HSTMT;
		SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
		SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
		SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);

		// Chuỗi kết nối (Đảm bảo đúng Server của bạn)
		SQLWCHAR* connStr = (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;";

		if (SQL_SUCCEEDED(SQLDriverConnect(hDbc, NULL, connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT))) {
			if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
				// Lấy MaLienKet, nếu NULL thì trả về rỗng
				std::wstring sql = L"SELECT ISNULL(MaLienKet, '') FROM TaiKhoan WHERE TenDangNhap = ?";
				SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);

				SQLCHAR sInUser[51]; strcpy_s((char*)sInUser, 51, sUser.c_str());
				SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sInUser, 0, NULL);

				if (SQLExecute(hStmt) == SQL_SUCCESS) {
					if (SQLFetch(hStmt) == SQL_SUCCESS) {
						SQLCHAR sResult[51]; SQLLEN len;
						SQLGetData(hStmt, 1, SQL_C_CHAR, sResult, 51, &len);
						if (len != SQL_NULL_DATA) {
							sResult[50] = '\0'; // Đảm bảo null-terminated
							maSieuThi = (char*)sResult;
						}
					}
				}
			}
		}
		if (hStmt) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		if (hDbc) { SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); }
		if (hEnv) SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
		std::string maSach = "";
		for (char c : maSieuThi) {
			if (!isspace(c)) maSach += c;
		}
		return maSieuThi.empty() ? sUser : maSieuThi;
	}

	// FIX: Lấy tên siêu thị với xử lý khoảng trắng (TRIM)
	inline String^ LayTenSieuThiTuMa(String^ maST)
	{
		if (String::IsNullOrEmpty(maST)) return L"(Chưa xác định)";

		// 1. Chuẩn bị mã cần tìm (Xóa khoảng trắng)
		std::string stdMaCanTim = CleanStdString(ToStdString(maST));
		String^ tenKetQua = maST + L" (Chưa cập nhật tên)";

		// 2. Kết nối DB
		SQLHENV hEnv = SQL_NULL_HENV; SQLHDBC hDbc = SQL_NULL_HDBC;
		SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
		SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
		SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
		SQLWCHAR* connStr = (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;";

		if (SQL_SUCCEEDED(SQLDriverConnect(hDbc, NULL, connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT))) {

			// 3. Lấy toàn bộ danh sách (Hàm này đã có sẵn trim_right trong SaoLuu.cpp)
			std::vector<SieuThi> dsST;
			DocfileSieuThi(hDbc, dsST);

			// 4. Duyệt vòng lặp tìm kiếm
			for (const auto& s : dsST) {
				// So sánh chuỗi C++ sau khi đã dọn sạch
				if (CleanStdString(s.getMaSieuThi()) == stdMaCanTim) {
					// Tìm thấy!
					tenKetQua = ToSysString(s.getTenSieuThi());
					break;
				}
			}
		}

		// Dọn dẹp
		if (hDbc) { SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); }
		if (hEnv) SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

		return tenKetQua;
	}

	// ====================================================================
	// HÀM HELPER: LẤY CHI TIẾT ĐƠN HÀNG
	// [FIX] Thêm 'inline' để sửa lỗi LNK2005 và xử lý chuỗi an toàn
	// ====================================================================
	inline void LayChiTietDonHang(SQLHDBC hDbc, std::string maYC, [Out] OrderHeader^% header, [Out] List<OrderDetail^>^% details) {
		header = nullptr;
		details = gcnew List<OrderDetail^>();
		std::string sMaYC = CleanStdString(maYC);

		SQLHSTMT hStmt = SQL_NULL_HSTMT;

		// 1. LẤY HEADER
		if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
			std::wstring sql = L"SELECT TOP 1 y.MaYeuCau, y.HoanThanh, y.NgayYeuCau, y.GiaTriTong, "
				L"st.TenSieuThi, st.SDT, st.DiaChi, st.QuanHuyen, st.Tinh, "
				L"v.KhoDi, v.KhoTrungChuyen, st.TenSieuThi, v.NgayVanChuyen, v.NgayNhanDuKien, ISNULL(v.ChiPhiVanChuyen, 0) "
				L"FROM YeuCau y "
				L"LEFT JOIN SieuThi st ON y.MaSieuThi = st.MaSieuThi "
				L"LEFT JOIN VanChuyen v ON y.MaYeuCau = v.MaYeuCauGoc "
				L"WHERE LTRIM(RTRIM(y.MaYeuCau)) = ?";

			if (SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS) == SQL_SUCCESS) {
				SQLCHAR cMaYC[51]; strcpy_s((char*)cMaYC, 51, sMaYC.c_str());
				SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMaYC, 0, NULL);

				if (SQLExecute(hStmt) == SQL_SUCCESS) {
					if (SQLFetch(hStmt) == SQL_SUCCESS) {
						header = gcnew OrderHeader();
						// Khởi tạo buffer
						SQLCHAR sMa[51] = { 0 }, sKhoDi[51] = { 0 }, sKhoTC[51] = { 0 };
						SQLWCHAR wTrangThai[51] = { 0 }, wTenST[256] = { 0 }, wSDT[51] = { 0 }, wDC[256] = { 0 }, wQuan[101] = { 0 }, wTinh[101] = { 0 }, wSieuThiNhan[256] = { 0 };
						SQL_DATE_STRUCT dNgayYC, dNgayVC, dNgayNhan;
						SQLDOUBLE dTongTien, dChiPhi;
						SQLLEN len;

						// Lấy dữ liệu
						SQLGetData(hStmt, 1, SQL_C_CHAR, sMa, 51, &len);
						if (len != SQL_NULL_DATA) header->MaDon = ToSysString(FormTrim((char*)sMa));

						SQLGetData(hStmt, 2, SQL_C_WCHAR, wTrangThai, 51, &len);
						if (len != SQL_NULL_DATA) header->TrangThai = gcnew String((wchar_t*)wTrangThai);

						SQLGetData(hStmt, 3, SQL_C_TYPE_DATE, &dNgayYC, 0, &len);
						header->NgayDat = DateTime(dNgayYC.year, dNgayYC.month, dNgayYC.day);

						SQLGetData(hStmt, 4, SQL_C_DOUBLE, &dTongTien, 0, &len);
						header->TongGiaTriHang = dTongTien;

						SQLGetData(hStmt, 5, SQL_C_WCHAR, wTenST, 256, &len);
						if (len != SQL_NULL_DATA) header->TenSieuThi = gcnew String((wchar_t*)wTenST);

						SQLGetData(hStmt, 6, SQL_C_WCHAR, wSDT, 51, &len);
						if (len != SQL_NULL_DATA) header->SDT = gcnew String((wchar_t*)wSDT);

						// Ghép địa chỉ
						String^ dc = ""; String^ q = ""; String^ t = "";
						SQLGetData(hStmt, 7, SQL_C_WCHAR, wDC, 256, &len); if (len != SQL_NULL_DATA) dc = gcnew String((wchar_t*)wDC);
						SQLGetData(hStmt, 8, SQL_C_WCHAR, wQuan, 101, &len); if (len != SQL_NULL_DATA) q = gcnew String((wchar_t*)wQuan);
						SQLGetData(hStmt, 9, SQL_C_WCHAR, wTinh, 101, &len); if (len != SQL_NULL_DATA) t = gcnew String((wchar_t*)wTinh);
						header->DiaChiFull = dc + ", " + q + ", " + t;

						// Vận chuyển
						SQLGetData(hStmt, 10, SQL_C_CHAR, sKhoDi, 51, &len);
						if (len != SQL_NULL_DATA) {
							header->CoVanChuyen = true;
							header->KhoDi = ToSysString(FormTrim((char*)sKhoDi));

							SQLGetData(hStmt, 11, SQL_C_CHAR, sKhoTC, 51, &len);
							if (len != SQL_NULL_DATA) header->KhoTC = ToSysString(FormTrim((char*)sKhoTC));

							SQLGetData(hStmt, 12, SQL_C_WCHAR, wSieuThiNhan, 256, &len);
							if (len != SQL_NULL_DATA) header->SieuThiNhan = gcnew String((wchar_t*)wSieuThiNhan);

							SQLGetData(hStmt, 13, SQL_C_TYPE_DATE, &dNgayVC, 0, &len);
							header->NgayVanChuyen = DateTime(dNgayVC.year, dNgayVC.month, dNgayVC.day);

							SQLGetData(hStmt, 14, SQL_C_TYPE_DATE, &dNgayNhan, 0, &len);
							header->NgayNhanDuKien = DateTime(dNgayNhan.year, dNgayNhan.month, dNgayNhan.day);

							SQLGetData(hStmt, 15, SQL_C_DOUBLE, &dChiPhi, 0, &len);
							header->PhiVanChuyen = dChiPhi;
						}
						else {
							header->CoVanChuyen = false;
							header->PhiVanChuyen = 0;
						}
						header->TongThanhToan = header->TongGiaTriHang + header->PhiVanChuyen;
					}
				}
			}
			SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		}

		// 2. LẤY CHI TIẾT SẢN PHẨM
		if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
			std::wstring sqlD = L"SELECT C.MaSanPham, S.TenSanPham, C.SoTanYeuCau, C.GiaTri FROM ChiTietYeuCau C JOIN SanPham S ON C.MaSanPham = S.MaSanPham WHERE C.MaYeuCau = ?";
			SQLPrepare(hStmt, (SQLWCHAR*)sqlD.c_str(), SQL_NTS);
			SQLCHAR cMa[51]; strcpy_s((char*)cMa, 51, sMaYC.c_str());
			SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMa, 0, NULL);

			if (SQLExecute(hStmt) == SQL_SUCCESS) {
				SQLCHAR sMaSP[51]; SQLWCHAR wTenSP[256]; SQLDOUBLE dSL, dTien; SQLLEN len;
				while (SQLFetch(hStmt) == SQL_SUCCESS) {
					OrderDetail^ detail = gcnew OrderDetail();

					SQLGetData(hStmt, 1, SQL_C_CHAR, sMaSP, 51, &len);
					detail->MaSP = ToSysString(FormTrim((char*)sMaSP));

					SQLGetData(hStmt, 2, SQL_C_WCHAR, wTenSP, 256, &len);
					detail->TenSP = gcnew String((wchar_t*)wTenSP);

					SQLGetData(hStmt, 3, SQL_C_DOUBLE, &dSL, 0, &len);
					detail->SoLuong = dSL;

					SQLGetData(hStmt, 4, SQL_C_DOUBLE, &dTien, 0, &len);
					detail->ThanhTien = dTien;

					details->Add(detail);
				}
			}
		}
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	}
	public ref class SupermarketForm : public System::Windows::Forms::Form
	{
	public:
		delegate int ProductComparer(ProductInfo^ x, ProductInfo^ y);
		String^ MaSieuThiHienTai;
		List<CartItem^>^ ShoppingCart;
		List<ProductInfo^>^ originalProductList;
		Panel^ pnlToolbar;
		ComboBox^ cboSort;
		ComboBox^ cboCategory;
		TextBox^ txtSearch;
		Button^ btnRefresh;
		Label^ lblUserInfo;
		Button^ btnCart;
		FlowLayoutPanel^ flowPnlProducts;
		Dictionary<String^, String^>^ imageMap;

		inline void SupermarketForm::LoadImageDatabase(String^ filePath) {
			try {
				if (!File::Exists(filePath)) return;
				cli::array<String^>^ lines = File::ReadAllLines(filePath);
				for each (String ^ line in lines) {
					if (String::IsNullOrWhiteSpace(line)) continue;
					cli::array<wchar_t>^ separator = gcnew cli::array<wchar_t>{ '|' };
					cli::array<String^>^ parts = line->Split(separator);
					if (parts->Length == 2) {
						String^ ma = parts[0]->Trim(); String^ link = parts[1]->Trim();
						if (!imageMap->ContainsKey(ma)) imageMap->Add(ma, link);
					}
				}
			}
			catch (...) {}
		}

		// SỬA CONSTRUCTOR
		SupermarketForm(String^ userDangNhap) {
			// 1. Lấy MÃ SIÊU THỊ từ User (DB Account)
			this->CurrentUsername = userDangNhap;
			std::string maSieuThiThucTe = LayMaSieuThiTuTaiKhoan(userDangNhap);
			this->MaSieuThiHienTai = ToSysString(maSieuThiThucTe);

			// Khởi tạo các list
			ShoppingCart = gcnew List<CartItem^>();
			originalProductList = gcnew List<ProductInfo^>();
			imageMap = gcnew Dictionary<String^, String^>();
			LoadImageDatabase("ProductImages.txt");
			try {
				String^ pathMoMat = "D:\\code\\ChanggWithOOP\\PBL2_QuanLyKho\\PBL2_QuanLyKho\\Kho_anh\\TachMoMat.png";
				// Đường dẫn ảnh Nhắm Mắt
				String^ pathNhamMat = "D:\\code\\ChanggWithOOP\\PBL2_QuanLyKho\\PBL2_QuanLyKho\\Kho_anh\\TachNhamMat.png";

				if (System::IO::File::Exists(pathMoMat))
					imgShowPass = System::Drawing::Image::FromFile(pathMoMat);

				if (System::IO::File::Exists(pathNhamMat))
					imgHidePass = System::Drawing::Image::FromFile(pathNhamMat);
			}
			catch (...) {
				// Nếu không tìm thấy ảnh thì thôi, chương trình không lỗi
				imgShowPass = nullptr;
				imgHidePass = nullptr;
			}
			InitializeComponent();

			String^ tenHienThi = LayTenSieuThiTuMa(this->MaSieuThiHienTai);

			// 3. Setup giao diện
			this->flowPnlProducts->FlowDirection = System::Windows::Forms::FlowDirection::LeftToRight;
			this->flowPnlProducts->WrapContents = true;
			this->flowPnlProducts->AutoScroll = true;
			this->flowPnlProducts->Dock = DockStyle::Fill;

			// Cập nhật Tiêu đề cửa sổ
			this->Text = L"HỆ THỐNG QUẢN LÝ - " + tenHienThi;
			this->WindowState = FormWindowState::Maximized;

			// Truyền tên hiển thị vào Toolbar (Góc trên bên phải)
			SetupToolbar(tenHienThi);
			SetupProductPagination();
			LoadDanhSachSanPham();
			LoadLichSuDonHang();
			this->tabContent->SelectedIndex = 0;

			ResetMenuColors();
			if (btnMenuGeneral != nullptr) {
				btnMenuGeneral->BackColor = Color::FromArgb(23, 42, 69);
			}

			isPersonalMenuOpen = false;
			if (pnlSubMenuPersonal != nullptr) {
				pnlSubMenuPersonal->Visible = false;
			}
			if (btnMenuPersonal != nullptr) {
				btnMenuPersonal->BackColor = Color::FromArgb(10, 25, 47); // Màu tối (không active)
			}
			// -----------------------------------------------------
		}
	protected:
		~SupermarketForm() { if (components) delete components; }

	private:
		System::Windows::Forms::Panel^ pnlSidebar; System::Windows::Forms::Label^ lblAppName; System::Windows::Forms::Button^ btnMenuGeneral;
		System::Windows::Forms::Button^ btnMenuOrders; System::Windows::Forms::Panel^ pnlAccount; System::Windows::Forms::Label^ lblAccountName;
		System::Windows::Forms::TabControl^ tabContent; System::Windows::Forms::TabPage^ tabGeneral; System::Windows::Forms::TabPage^ tabOrders;
		System::Windows::Forms::Label^ lblTitleTab1; System::Windows::Forms::DataGridView^ dgvOrders; System::Windows::Forms::Label^ lblTitleTab2;
		System::Windows::Forms::Button^ btnMenuInfo; // Nút menu mới
		System::Windows::Forms::TabPage^ tabInfo;    // Tab trang mới
		System::Windows::Forms::Label^ lblTitleTab3; // Tiêu đề trang mới
		System::Windows::Forms::GroupBox^ grpCardInfo;
		// Tiêu đề Card
		Button^ btnMenuPersonal;
		Panel^ pnlSubMenuPersonal;
		Button^ btnSubInfo;            // "Thông tin tài khoản"
		Button^ btnSubPass;            // "Đổi mật khẩu"
		Button^ btnSubLogout;          // "Đăng xuất"
		bool isPersonalMenuOpen = true;
		System::ComponentModel::Container^ components; Color colorPrimary = Color::FromArgb(73, 129, 207);
		System::Windows::Forms::Label^ lblInfoID;
		System::Windows::Forms::Label^ lblInfoName;
		System::Windows::Forms::Label^ lblInfoPhone;
		System::Windows::Forms::Label^ lblInfoFullAddress;
		System::Windows::Forms::Button^ btnUpdateInfo;
		// --- BIẾN CHO TRANG ĐỔI MẬT KHẨU ---
		System::Windows::Forms::TabPage^ tabChangePass;
		System::Windows::Forms::TextBox^ txtOldPass;
		System::Windows::Forms::TextBox^ txtNewPass;
		System::Windows::Forms::TextBox^ txtConfirmPass;
		System::Windows::Forms::Button^ btnConfirmChangePass;
		System::Windows::Forms::Button^ btnEyeNew;
		System::Windows::Forms::Button^ btnEyeConfirm;
		System::Drawing::Image^ imgShowPass;
		System::Drawing::Image^ imgHidePass; // Ảnh đóng mắt (gạch chéo)
		// --- CÁC BIẾN CHO BẢNG ĐỔI TÊN TÀI KHOẢN (MỚI) ---
		System::Windows::Forms::TextBox^ txtCurrentUser; // Tên hiện tại (Chỉ xem)
		System::Windows::Forms::TextBox^ txtNewUser;     // Tên mới
		System::Windows::Forms::Button^ btnConfirmChangeUser;

		Panel^ pnlOrderToolbar;
		ComboBox^ cboOrderStatus;
		DateTimePicker^ dtpStartDate;
		DateTimePicker^ dtpEndDate;
		TextBox^ txtOrderSearch;
		Button^ btnOrderFilter;
		List<OrderHeader^>^ originalOrderList; // Danh sách đơn hàng gốc (để lọc)
		List<String^>^ orderIDList;

		// BIẾN LƯU TÊN ĐĂNG NHẬP ĐỂ BIẾT ĐỔI PASS CHO AI
		String^ CurrentUsername;

		int currentPage = 1;      // Trang hiện tại
		int pageSize = 15;        // Số dòng mỗi trang
		int totalPages = 1;       // Tổng số trang

		// Các control giao diện cho phân trang
		Button^ btnPagePrev;
		Button^ btnPageNext;
		Label^ lblPageInfo;

		inline void SupermarketForm::SetupToolbar(String^ userName) {
			pnlToolbar = gcnew Panel(); pnlToolbar->Height = 85; pnlToolbar->Dock = DockStyle::Top; pnlToolbar->BackColor = Color::WhiteSmoke; pnlToolbar->Padding = System::Windows::Forms::Padding(20, 15, 20, 0);
			System::Drawing::Font^ fontLabel = gcnew System::Drawing::Font(L"Segoe UI", 11, FontStyle::Regular); System::Drawing::Font^ fontBold = gcnew System::Drawing::Font(L"Segoe UI", 12, FontStyle::Bold);

			Label^ lblSort = gcnew Label(); lblSort->Text = L"Sắp xếp:"; lblSort->AutoSize = true; lblSort->Font = fontLabel; lblSort->Location = System::Drawing::Point(20, 28);
			cboSort = gcnew ComboBox(); cboSort->Items->Add(L"Mặc định"); cboSort->Items->Add(L"Tên: A -> Z"); cboSort->Items->Add(L"Giá: Tăng dần"); cboSort->Items->Add(L"Giá: Giảm dần");
			cboSort->SelectedIndex = 0; cboSort->DropDownStyle = ComboBoxStyle::DropDownList; cboSort->Font = fontLabel; cboSort->Location = System::Drawing::Point(100, 25); cboSort->Width = 140;
			cboSort->SelectedIndexChanged += gcnew EventHandler(this, &SupermarketForm::OnFilterChanged);

			Label^ lblLoai = gcnew Label(); lblLoai->Text = L"Loại:"; lblLoai->AutoSize = true; lblLoai->Font = fontLabel; lblLoai->Location = System::Drawing::Point(260, 28);
			cboCategory = gcnew ComboBox(); cboCategory->Items->Add(L"Tất cả"); cboCategory->SelectedIndex = 0; cboCategory->DropDownStyle = ComboBoxStyle::DropDownList;
			cboCategory->Font = fontLabel; cboCategory->Location = System::Drawing::Point(310, 25); cboCategory->Width = 140;
			cboCategory->SelectedIndexChanged += gcnew EventHandler(this, &SupermarketForm::OnFilterChanged);

			Label^ lblSearch = gcnew Label(); lblSearch->Text = L"🔍 Tìm tên:"; lblSearch->AutoSize = true; lblSearch->Font = fontLabel; lblSearch->Location = System::Drawing::Point(480, 28);
			txtSearch = gcnew TextBox(); txtSearch->Font = fontLabel; txtSearch->Location = System::Drawing::Point(580, 25); txtSearch->Width = 220; txtSearch->TextChanged += gcnew EventHandler(this, &SupermarketForm::OnFilterChanged);

			btnRefresh = gcnew Button(); btnRefresh->Text = L"Làm mới"; btnRefresh->BackColor = Color::White; btnRefresh->FlatStyle = FlatStyle::Standard;
			btnRefresh->Font = fontLabel; btnRefresh->Location = System::Drawing::Point(820, 24); btnRefresh->Width = 110; btnRefresh->Height = 32; btnRefresh->Click += gcnew EventHandler(this, &SupermarketForm::OnRefreshClick);

			lblUserInfo = gcnew Label(); lblUserInfo->Text = userName + L"\n(Siêu Thị)"; lblUserInfo->AutoSize = true; lblUserInfo->TextAlign = ContentAlignment::MiddleRight;
			lblUserInfo->Font = fontBold; lblUserInfo->ForeColor = Color::FromArgb(50, 50, 50); lblUserInfo->Dock = DockStyle::Right;

			pnlToolbar->Controls->Add(lblUserInfo); pnlToolbar->Controls->Add(btnRefresh); pnlToolbar->Controls->Add(txtSearch); pnlToolbar->Controls->Add(lblSearch); pnlToolbar->Controls->Add(cboCategory); pnlToolbar->Controls->Add(lblLoai); pnlToolbar->Controls->Add(cboSort); pnlToolbar->Controls->Add(lblSort);
			Panel^ pnlHeaderSection = gcnew Panel(); pnlHeaderSection->Height = 110; pnlHeaderSection->Dock = DockStyle::Top; pnlHeaderSection->BackColor = Color::Transparent;

			this->lblTitleTab1->Parent = pnlHeaderSection; this->lblTitleTab1->Location = System::Drawing::Point(20, 10); this->lblTitleTab1->Font = gcnew System::Drawing::Font(L"Segoe UI", 18, FontStyle::Bold);
			this->btnCart->Parent = pnlHeaderSection; this->btnCart->Location = System::Drawing::Point(20, 55); this->btnCart->Font = gcnew System::Drawing::Font(L"Segoe UI", 12, FontStyle::Bold);

			this->flowPnlProducts->Anchor = AnchorStyles::None; this->flowPnlProducts->Dock = DockStyle::Fill; this->flowPnlProducts->Padding = System::Windows::Forms::Padding(10);
			this->flowPnlProducts->MouseEnter += gcnew EventHandler(this, &SupermarketForm::flowPnlProducts_MouseEnter);

			this->tabGeneral->Controls->Clear(); this->tabGeneral->Controls->Add(pnlToolbar); this->tabGeneral->Controls->Add(pnlHeaderSection); this->tabGeneral->Controls->Add(this->flowPnlProducts);
			pnlToolbar->BringToFront(); pnlHeaderSection->BringToFront(); this->flowPnlProducts->BringToFront();
		}
		void flowPnlProducts_MouseEnter(Object^ sender, EventArgs^ e) { this->flowPnlProducts->Focus(); }
		void OnFilterChanged(Object^ sender, EventArgs^ e) { ApplyFilters(); }
		void OnRefreshClick(Object^ sender, EventArgs^ e) { txtSearch->Text = ""; cboSort->SelectedIndex = 0; cboCategory->SelectedIndex = 0; LoadDanhSachSanPham(); }

		void ApplyFilters() {
			List<ProductInfo^>^ filteredList = gcnew List<ProductInfo^>();
			String^ keyword = txtSearch->Text->ToLower();
			String^ selectedLoai = cboCategory->SelectedItem->ToString();
			int sortMode = cboSort->SelectedIndex;

			for each (ProductInfo ^ p in originalProductList) {
				bool matchName = String::IsNullOrEmpty(keyword) || p->TenSP->ToLower()->Contains(keyword);
				bool matchLoai = (selectedLoai == L"Tất cả") || (p->Loai == selectedLoai);
				if (matchName && matchLoai) filteredList->Add(p);
			}

			// --- SỬA LỖI SORT: Dùng Helper Class ---
			if (sortMode > 0) {
				filteredList->Sort(gcnew ProductComparerHelper(sortMode));
			}
			// ---------------------------------------

			flowPnlProducts->Controls->Clear();
			flowPnlProducts->SuspendLayout();
			for each (ProductInfo ^ pInfo in filteredList) {
				Panel^ card = CreateProductCard(pInfo);
				flowPnlProducts->Controls->Add(card);
			}
			flowPnlProducts->ResumeLayout();
		}
		// Trong class SupermarketForm (Phần private/public)
		int CompareProducts(ProductInfo^ p1, ProductInfo^ p2) {
			// So sánh theo Tên (A->Z)
			if (cboSort->SelectedIndex == 1) {
				return String::Compare(p1->TenSP, p2->TenSP);
			}
			// So sánh theo Giá (Tăng dần)
			else if (cboSort->SelectedIndex == 2) {
				if (p1->GiaHienTai > p2->GiaHienTai) return 1;
				if (p1->GiaHienTai < p2->GiaHienTai) return -1;
				return 0;
			}
			// So sánh theo Giá (Giảm dần)
			else if (cboSort->SelectedIndex == 3) {
				if (p1->GiaHienTai < p2->GiaHienTai) return 1;
				if (p1->GiaHienTai > p2->GiaHienTai) return -1;
				return 0;
			}
			return 0; // Mặc định
		}
		String^ LayTenSieuThi(String^ maST) {
			String^ tenKetQua = maST; String^ connStr = "Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;";
			OdbcConnection^ conn = gcnew OdbcConnection(connStr);
			try { conn->Open(); String^ sql = "SELECT TenSieuThi FROM SieuThi WHERE MaSieuThi = ?"; OdbcCommand^ cmd = gcnew OdbcCommand(sql, conn); cmd->Parameters->AddWithValue("@ma", maST); Object^ result = cmd->ExecuteScalar(); if (result != nullptr) tenKetQua = result->ToString(); conn->Close(); }
			catch (...) {} return tenKetQua;
		}
		std::string LayTenNhaCungCapGanNhat(SQLHDBC hDbc, std::string maSP) {
			SQLHSTMT hStmt; std::string tenNCC = "Chua xac dinh";
			if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
				std::wstring sql = L"SELECT TOP 1 n.TenNCC FROM NhaCungCap n JOIN LoHang l ON n.MaNCC = l.MaNCC JOIN ChiTietLoHang c ON l.MaLoHang = c.MaLoHang WHERE c.MaSanPham = ? ORDER BY l.NgayNhap DESC";
				SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS); SQLCHAR sMaSP[51]; strcpy_s((char*)sMaSP, 51, maSP.c_str());
				SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMaSP, 0, NULL);
				if (SQLExecute(hStmt) == SQL_SUCCESS) {
					if (SQLFetch(hStmt) == SQL_SUCCESS) {
						SQLWCHAR sTenNCC[256]; SQLLEN len; SQLGetData(hStmt, 1, SQL_C_WCHAR, sTenNCC, 256, &len);
						if (len != SQL_NULL_DATA) {
							std::wstring ws(sTenNCC); int size_needed = WideCharToMultiByte(CP_UTF8, 0, &ws[0], (int)ws.size(), NULL, 0, NULL, NULL);
							std::string strTo(size_needed, 0); WideCharToMultiByte(CP_UTF8, 0, &ws[0], (int)ws.size(), &strTo[0], size_needed, NULL, NULL); tenNCC = strTo;
						}
					}
				}
				SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
			}
			return tenNCC;
		}
		std::string FromSQLWChar(SQLWCHAR* wstr) {
			if (!wstr) return "";
			std::wstring ws((wchar_t*)wstr);
			if (ws.empty()) return "";

			// Chuyển đổi sang UTF-8
			int size_needed = WideCharToMultiByte(CP_UTF8, 0, &ws[0], (int)ws.size(), NULL, 0, NULL, NULL);
			std::string strTo(size_needed, 0);
			WideCharToMultiByte(CP_UTF8, 0, &ws[0], (int)ws.size(), &strTo[0], size_needed, NULL, NULL);
			return strTo;
		}

		// --- HÀM CHÍNH: Lấy thông tin siêu thị (Phiên bản Fix lỗi) ---
		SieuThi LayChiTietSieuThi(SQLHDBC hDbc, std::string maST) {
			SieuThi stResult;
			std::string maSach = "";
			for (char c : maST) if (!isspace(c)) maSach += c;
			stResult.setMaSieuThi(maSach);

			SQLHSTMT hStmt;
			if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
				// SỬA LỖI SQL: Dùng LTRIM(RTRIM(...)) để bỏ qua khoảng trắng thừa trong DB
				std::wstring sql = L"SELECT TenSieuThi, SDT, DiaChi, QuanHuyen, Tinh FROM SieuThi WHERE LTRIM(RTRIM(MaSieuThi)) LIKE ?";

				if (SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS) == SQL_SUCCESS) {
					std::string sMaParam = maSach + "%";
					SQLCHAR sMa[51];
					strcpy_s((char*)sMa, 51, maST.c_str());
					SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, sMa, 0, NULL);

					if (SQLExecute(hStmt) == SQL_SUCCESS) {
						if (SQLFetch(hStmt) == SQL_SUCCESS) {
							SQLWCHAR sTen[256], sSDT[50], sDC[256], sQuan[101], sTinh[101];
							SQLLEN len;

							// Lấy dữ liệu và chuyển đổi bằng hàm FromSQLWChar mới
							SQLGetData(hStmt, 1, SQL_C_WCHAR, sTen, 256, &len);
							if (len != SQL_NULL_DATA) stResult.setTenSieuThi(FromSQLWChar(sTen));

							SQLGetData(hStmt, 2, SQL_C_WCHAR, sSDT, 50, &len);
							if (len != SQL_NULL_DATA) stResult.setSDT(FromSQLWChar(sSDT));

							SQLGetData(hStmt, 3, SQL_C_WCHAR, sDC, 256, &len);
							if (len != SQL_NULL_DATA) stResult.setDiaChi(FromSQLWChar(sDC));

							SQLGetData(hStmt, 4, SQL_C_WCHAR, sQuan, 101, &len);
							if (len != SQL_NULL_DATA) stResult.setQuanHuyen(FromSQLWChar(sQuan));

							SQLGetData(hStmt, 5, SQL_C_WCHAR, sTinh, 101, &len);
							if (len != SQL_NULL_DATA) stResult.setTinh(FromSQLWChar(sTinh));
						}
					}
				}
				SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
			}
			return stResult;
		}

#pragma region Windows Form Designer generated code
		Button^ CreateMenuButton(String^ text) {
			Button^ btn = gcnew Button();
			btn->Text = text;
			btn->Height = 60;
			btn->FlatStyle = FlatStyle::Flat;
			btn->FlatAppearance->BorderSize = 0;
			btn->TextAlign = ContentAlignment::MiddleLeft;
			btn->Padding = System::Windows::Forms::Padding(20, 0, 0, 0);
			btn->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, FontStyle::Regular));
			btn->Cursor = Cursors::Hand;
			btn->BackColor = Color::FromArgb(10, 25, 47); // Màu nền mặc định (Xanh đậm)
			btn->ForeColor = Color::FromArgb(204, 214, 246); // Màu chữ mặc định
			return btn;
		}

		Label^ CreateHeaderLabel(String^ text) {
			Label^ lbl = gcnew Label();
			lbl->Text = text;
			lbl->Font = (gcnew System::Drawing::Font(L"Segoe UI", 20, FontStyle::Bold));
			lbl->ForeColor = Color::FromArgb(23, 42, 69); // Màu xanh đậm
			lbl->Dock = DockStyle::Top;
			lbl->Height = 50;
			return lbl;
		}

		DataGridView^ CreateStyledGrid() {
			DataGridView^ dgv = gcnew DataGridView();
			dgv->BackgroundColor = Color::White;
			dgv->BorderStyle = System::Windows::Forms::BorderStyle::None;
			dgv->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;
			dgv->RowTemplate->Height = 40;
			dgv->AllowUserToAddRows = false;
			dgv->ReadOnly = true;
			dgv->SelectionMode = DataGridViewSelectionMode::FullRowSelect;

			// Style Header
			dgv->EnableHeadersVisualStyles = false;
			dgv->ColumnHeadersDefaultCellStyle->BackColor = Color::FromArgb(23, 42, 69);
			dgv->ColumnHeadersDefaultCellStyle->ForeColor = Color::White;
			dgv->ColumnHeadersDefaultCellStyle->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11, FontStyle::Bold));
			dgv->ColumnHeadersHeight = 45;

			// Style Row
			dgv->DefaultCellStyle->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10));
			dgv->DefaultCellStyle->SelectionBackColor = Color::FromArgb(225, 245, 255);
			dgv->DefaultCellStyle->SelectionForeColor = Color::Black;

			return dgv;
		}
		// Hàm tạo nút con trong menu (giống style trong ảnh)
		Button^ CreateSubMenuButton(String^ text, Color textColor) {
			Button^ btn = gcnew Button();
			btn->Text = text;
			btn->Dock = DockStyle::Top;
			btn->Height = 45; // Chiều cao nhỏ hơn nút chính
			btn->FlatStyle = FlatStyle::Flat;
			btn->FlatAppearance->BorderSize = 0;
			btn->TextAlign = ContentAlignment::MiddleLeft;
			// Padding bên trái nhiều hơn để thụt đầu dòng
			btn->Padding = System::Windows::Forms::Padding(40, 0, 0, 0);
			btn->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, FontStyle::Regular));
			btn->BackColor = Color::FromArgb(15, 30, 50); // Màu nền tối hơn nút chính một chút
			btn->ForeColor = textColor;
			btn->Cursor = Cursors::Hand;
			return btn;
		}
		void dgvOrders_CellPainting(Object^ sender, DataGridViewCellPaintingEventArgs^ e) {
			if (e->RowIndex >= 0 && e->ColumnIndex >= 0) {
				if (dgvOrders->Columns[e->ColumnIndex]->Name == "TrangThai") {
					String^ status = (e->Value != nullptr) ? e->Value->ToString() : "";

					// LOGIC: Chỉ vẽ nút nếu là "Đang Vận Chuyển"
					if (status == L"Đang Vận Chuyển") {
						e->Paint(e->CellBounds, DataGridViewPaintParts::All);

						// --- [SỬA LỖI TẠI ĐÂY] ---
						// Thêm "System::Drawing::" vào trước chữ Rectangle
						System::Drawing::Rectangle btnRect = e->CellBounds;

						btnRect.Inflate(-2, -2); // Thu nhỏ xíu để tạo viền
						ControlPaint::DrawButton(e->Graphics, btnRect, ButtonState::Normal);

						// Vẽ chữ đè lên
						TextRenderer::DrawText(e->Graphics, L"Xác Nhận Đã Nhận",
							e->CellStyle->Font, btnRect, Color::Blue,
							TextFormatFlags::HorizontalCenter | TextFormatFlags::VerticalCenter);

						e->Handled = true; // Báo hệ thống: Tôi tự vẽ xong rồi
					}
					else {
						// Các trạng thái khác -> Vẽ text bình thường
						e->Paint(e->CellBounds, DataGridViewPaintParts::All & ~DataGridViewPaintParts::ContentBackground);

						Color txtColor = Color::Black;
						if (status == L"Đã Giao Hàng") txtColor = Color::Green;

						TextRenderer::DrawText(e->Graphics, status, e->CellStyle->Font, e->CellBounds,
							txtColor, TextFormatFlags::HorizontalCenter | TextFormatFlags::VerticalCenter);

						e->Handled = true;
					}
				}
			}
		}
		void dgvOrders_CellContentClick(Object^ sender, DataGridViewCellEventArgs^ e) {
			if (e->RowIndex < 0 || e->ColumnIndex < 0) return;

			if (dgvOrders->Columns[e->ColumnIndex]->Name == "TrangThai") {
				String^ currentStatus = dgvOrders->Rows[e->RowIndex]->Cells[e->ColumnIndex]->Value->ToString();

				// Lấy Mã Yêu Cầu (Giả sử cột 0 là MaDon)
				String^ maYC = dgvOrders->Rows[e->RowIndex]->Cells["MaDon"]->Value->ToString();

				// CHỈ CHO PHÉP BẤM KHI ĐANG VẬN CHUYỂN
				if (currentStatus == L"Đang Vận Chuyển") {
					if (MessageBox::Show(L"Bạn xác nhận đã nhận đủ hàng cho đơn " + maYC + L"?",
						L"Xác nhận nhận hàng", MessageBoxButtons::YesNo, MessageBoxIcon::Question) == System::Windows::Forms::DialogResult::Yes) {

						// --- GỌI SQL UPDATE ---
						SQLHENV hEnv; SQLHDBC hDbc; SQLHSTMT hStmt;
						SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
						SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
						SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
						SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

						if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
							std::string sMa = ToStringStandard(maYC);
							SQLCHAR cMa[51]; strcpy_s((char*)cMa, 51, sMa.c_str());

							// Transaction cập nhật đồng bộ 2 bảng
							SQLSetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, 0);

							try {
								// 1. Update YeuCau -> Đã Giao Hàng
								std::wstring sql1 = L"UPDATE YeuCau SET HoanThanh = N'Đã Giao Hàng' WHERE MaYeuCau = ?";
								SQLPrepare(hStmt, (SQLWCHAR*)sql1.c_str(), SQL_NTS);
								SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMa, 0, NULL);
								if (SQLExecute(hStmt) != SQL_SUCCESS) throw gcnew Exception("Lỗi YeuCau");
								SQLFreeStmt(hStmt, SQL_CLOSE); SQLFreeStmt(hStmt, SQL_UNBIND); SQLFreeStmt(hStmt, SQL_RESET_PARAMS);

								// 2. Update DonVanChuyen -> Đã Giao Hàng (Sync ngược về kho)
								std::wstring sql2 = L"UPDATE DonVanChuyen SET TrangThai = N'Đã Giao Hàng' WHERE MaYeuCauGoc = ?";
								SQLPrepare(hStmt, (SQLWCHAR*)sql2.c_str(), SQL_NTS);
								SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMa, 0, NULL);
								if (SQLExecute(hStmt) != SQL_SUCCESS) throw gcnew Exception("Lỗi DonVanChuyen");

								SQLEndTran(SQL_HANDLE_DBC, hDbc, SQL_COMMIT);
								MessageBox::Show(L"Đã cập nhật trạng thái: ĐÃ GIAO HÀNG!", L"Thành công");

								// Refresh lại lưới để thấy chữ màu xanh
								LoadLichSuDonHang();
							}
							catch (Exception^ ex) {
								SQLEndTran(SQL_HANDLE_DBC, hDbc, SQL_ROLLBACK);
								MessageBox::Show(L"Lỗi: " + ex->Message);
							}
							SQLSetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_ON, 0);
						}

						if (hStmt) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
						SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
					}
				}
			}
		}
		inline void SupermarketForm::InitializeComponent()
		{
			if (!String::IsNullOrEmpty(this->MaSieuThiHienTai)) {
				SQLHENV hEnv = SQL_NULL_HENV; SQLHDBC hDbc = SQL_NULL_HDBC; SQLHSTMT hStmt = SQL_NULL_HSTMT;
				SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
				SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
				SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
				SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

				if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
					// Lấy tên đăng nhập dựa trên ID Siêu thị
					std::wstring sql = L"SELECT TenDangNhap FROM TaiKhoan WHERE MaLienKet = ?";
					SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);

					std::string sID = ToStringStandard(this->MaSieuThiHienTai);
					SQLCHAR cID[51]; strcpy_s((char*)cID, 51, sID.c_str());
					SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cID, 0, NULL);

					if (SQLExecute(hStmt) == SQL_SUCCESS) {
						if (SQLFetch(hStmt) == SQL_SUCCESS) {
							SQLCHAR cUser[51]; SQLLEN len;
							SQLGetData(hStmt, 1, SQL_C_CHAR, cUser, 51, &len);
							if (len != SQL_NULL_DATA) {
								// CẬP NHẬT LẠI BIẾN CURRENT USERNAME BẰNG TÊN THẬT TỪ DB
								this->CurrentUsername = ToSysString(FormTrim((char*)cUser));
							}
						}
					}
				}
				if (hStmt) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
				if (hDbc) { SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); }
				if (hEnv) SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
			}
			// 1. THIẾT LẬP MÀU SẮC CHUNG
			Color themeColorDark = Color::FromArgb(10, 25, 47);
			Color accentColor = Color::FromArgb(100, 255, 218);

			this->components = gcnew System::ComponentModel::Container();

			// -----------------------------------------------------------
			// A. CẤU HÌNH FORM CHÍNH
			// -----------------------------------------------------------
			this->Size = System::Drawing::Size(1280, 720);
			this->Text = L"Hệ Thống Quản Lý Kho - Phân quyền siêu thị";
			this->StartPosition = FormStartPosition::CenterScreen;

			// -----------------------------------------------------------
			// B. KHỞI TẠO SIDEBAR (THANH BÊN TRÁI)
			// -----------------------------------------------------------
			this->pnlSidebar = (gcnew System::Windows::Forms::Panel());
			this->pnlSidebar->Dock = System::Windows::Forms::DockStyle::Left;
			this->pnlSidebar->Width = 260;
			this->pnlSidebar->BackColor = themeColorDark;

			// B1. Logo App
			this->lblAppName = (gcnew System::Windows::Forms::Label());
			this->lblAppName->Text = L"BunKem";
			this->lblAppName->Font = (gcnew System::Drawing::Font(L"Segoe UI", 24, System::Drawing::FontStyle::Bold));
			this->lblAppName->ForeColor = accentColor;
			this->lblAppName->Dock = System::Windows::Forms::DockStyle::Top;
			this->lblAppName->Height = 100;
			this->lblAppName->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;

			// B2. Nút Menu: Sản Phẩm
			this->btnMenuGeneral = CreateMenuButton(L"📦  Sản Phẩm");
			this->btnMenuGeneral->Dock = System::Windows::Forms::DockStyle::Top;
			this->btnMenuGeneral->Click += gcnew System::EventHandler(this, &SupermarketForm::btnMenuGeneral_Click);

			// B3. Nút Menu: Đơn Hàng
			this->btnMenuOrders = CreateMenuButton(L"📄  Đơn Hàng");
			this->btnMenuOrders->Dock = System::Windows::Forms::DockStyle::Top;
			this->btnMenuOrders->Click += gcnew System::EventHandler(this, &SupermarketForm::btnMenuOrders_Click);

			// B4. MENU CÁ NHÂN (DROPDOWN) - [PHẦN MỚI]

			// -- Tạo Panel chứa các nút con (SubMenu)
			this->pnlSubMenuPersonal = (gcnew System::Windows::Forms::Panel());
			this->pnlSubMenuPersonal->Dock = System::Windows::Forms::DockStyle::Top;
			this->pnlSubMenuPersonal->Height = 135; // 3 nút x 45px
			this->pnlSubMenuPersonal->BackColor = Color::FromArgb(15, 30, 50); // Màu nền tối hơn nút chính
			this->pnlSubMenuPersonal->Visible = false; // Mặc định ẩn

			// -- Nút con 1: Đăng xuất (Màu cam)
			this->btnSubLogout = CreateSubMenuButton(L"•  Đăng xuất", Color::FromArgb(238, 77, 45));
			this->btnSubLogout->Click += gcnew EventHandler(this, &SupermarketForm::OnLogoutClick);

			// -- Nút con 2: Đổi mật khẩu
			this->btnSubPass = CreateSubMenuButton(L"•  Đổi mật khẩu", Color::FromArgb(180, 180, 180));
			this->btnSubPass->Click += gcnew EventHandler(this, &SupermarketForm::OnChangePassClick);

			// -- Nút con 3: Thông tin tài khoản
			this->btnSubInfo = CreateSubMenuButton(L"•  Thông tin tài khoản", Color::FromArgb(180, 180, 180));
			this->btnSubInfo->Click += gcnew EventHandler(this, &SupermarketForm::OnSubInfoClick);

			// -- Thêm nút con vào Panel SubMenu (Thứ tự hiển thị từ trên xuống: Info -> Pass -> Logout)
			// Do Dock=Top, ta add ngược lại: Logout add trước sẽ nằm dưới cùng
			this->pnlSubMenuPersonal->Controls->Add(this->btnSubLogout);
			this->pnlSubMenuPersonal->Controls->Add(this->btnSubPass);
			this->pnlSubMenuPersonal->Controls->Add(this->btnSubInfo);

			// -- Tạo Nút Chính: Cá Nhân
			this->btnMenuPersonal = CreateMenuButton(L"👤  Cá Nhân");
			this->btnMenuPersonal->Dock = System::Windows::Forms::DockStyle::Top;
			this->btnMenuPersonal->Click += gcnew System::EventHandler(this, &SupermarketForm::btnMenuPersonal_Click);

			// B5. Panel User dưới cùng (Footer Sidebar)
			this->pnlAccount = (gcnew System::Windows::Forms::Panel());
			this->pnlAccount->Dock = System::Windows::Forms::DockStyle::Bottom;
			this->pnlAccount->Height = 60;
			this->pnlAccount->BackColor = Color::FromArgb(5, 15, 30);

			this->lblAccountName = (gcnew System::Windows::Forms::Label());
			this->lblAccountName->Text = L"PBL2 - Phạm Minh Tuấn"; // Sẽ được cập nhật khi form load
			this->lblAccountName->ForeColor = Color::Gray;
			this->lblAccountName->Dock = System::Windows::Forms::DockStyle::Fill;
			this->lblAccountName->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			this->pnlAccount->Controls->Add(this->lblAccountName);

			// B6. SẮP XẾP SIDEBAR (QUAN TRỌNG: Thứ tự Add ngược với hiển thị vì Dock=Top)
			// Hiển thị mong muốn: Logo -> SP -> Đơn -> Cá Nhân -> SubMenu -> (Khoảng trống) -> UserFooter
			this->pnlSidebar->Controls->Add(this->pnlSubMenuPersonal); // SubMenu nằm dưới nút Cá Nhân
			this->pnlSidebar->Controls->Add(this->btnMenuPersonal);    // Nút Cá Nhân
			this->pnlSidebar->Controls->Add(this->btnMenuOrders);      // Nút Đơn Hàng
			this->pnlSidebar->Controls->Add(this->btnMenuGeneral);     // Nút Sản Phẩm
			this->pnlSidebar->Controls->Add(this->lblAppName);         // Logo (Trên cùng)
			this->pnlSidebar->Controls->Add(this->pnlAccount);         // Footer (Dock Bottom tự dính dưới)

			// -----------------------------------------------------------
			// C. KHỞI TẠO TAB CONTENT (NỘI DUNG CHÍNH)
			// -----------------------------------------------------------
			this->tabContent = (gcnew System::Windows::Forms::TabControl());
			this->tabContent->Dock = System::Windows::Forms::DockStyle::Fill;
			this->tabContent->Appearance = TabAppearance::FlatButtons;
			this->tabContent->ItemSize = System::Drawing::Size(0, 1);
			this->tabContent->SizeMode = TabSizeMode::Fixed;

			// --- TAB 1: SẢN PHẨM ---
			this->tabGeneral = (gcnew System::Windows::Forms::TabPage());
			this->tabGeneral->BackColor = Color::FromArgb(245, 245, 250);
			this->tabGeneral->Padding = System::Windows::Forms::Padding(20);
			this->lblTitleTab1 = CreateHeaderLabel(L"Danh Mục Sản Phẩm");

			this->btnCart = (gcnew System::Windows::Forms::Button());
			this->btnCart->Text = L"🛒 Giỏ Hàng (0)";
			this->btnCart->BackColor = Color::FromArgb(238, 77, 45);
			this->btnCart->ForeColor = Color::White;
			this->btnCart->FlatStyle = FlatStyle::Flat;
			this->btnCart->FlatAppearance->BorderSize = 0;
			this->btnCart->Size = System::Drawing::Size(180, 50);
			this->btnCart->Location = System::Drawing::Point(20, 60);
			this->btnCart->Click += gcnew System::EventHandler(this, &SupermarketForm::btnCart_Click);

			this->flowPnlProducts = (gcnew System::Windows::Forms::FlowLayoutPanel());
			this->flowPnlProducts->BackColor = Color::Transparent;

			this->tabGeneral->Controls->Add(this->lblTitleTab1);
			this->tabGeneral->Controls->Add(this->btnCart);
			this->tabGeneral->Controls->Add(this->flowPnlProducts);
			// TÌM ĐOẠN KHỞI TẠO tabOrders (~ dòng 610)
			// --- TAB 2: ĐƠN HÀNG ---
			this->tabOrders = (gcnew System::Windows::Forms::TabPage());
			this->tabOrders->BackColor = Color::White;
			// Padding để Grid không bị dính sát lề
			this->tabOrders->Padding = System::Windows::Forms::Padding(20);
			this->lblTitleTab2 = CreateHeaderLabel(L"Lịch Sử Đơn Hàng");

			// 1. KHỞI TẠO TOOLBAR (LỌC) - DOCK TOP
			pnlOrderToolbar = gcnew Panel();
			pnlOrderToolbar->Dock = DockStyle::Top;
			pnlOrderToolbar->Height = 50;
			pnlOrderToolbar->Padding = System::Windows::Forms::Padding(0, 5, 0, 5);
			pnlOrderToolbar->BackColor = Color::FromArgb(245, 245, 250);

			// Nút Trạng Thái
			Label^ lStatus = gcnew Label(); lStatus->Text = L"Trạng Thái:"; lStatus->Location = Point(10, 15); lStatus->AutoSize = true; lStatus->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10)); pnlOrderToolbar->Controls->Add(lStatus);
			cboOrderStatus = gcnew ComboBox();
			cboOrderStatus->Items->Add(L"Tất cả"); cboOrderStatus->Items->Add(L"Hoàn thành");
			cboOrderStatus->Items->Add(L"Chờ xác nhận"); cboOrderStatus->Items->Add(L"Đang vận chuyển");
			cboOrderStatus->SelectedIndex = 0; cboOrderStatus->Location = Point(85, 12); cboOrderStatus->Width = 130;
			cboOrderStatus->DropDownStyle = ComboBoxStyle::DropDownList;
			cboOrderStatus->SelectedIndexChanged += gcnew EventHandler(this, &SupermarketForm::OnOrderFilterChanged);
			pnlOrderToolbar->Controls->Add(cboOrderStatus);

			// Lịch (Từ ngày)
			Label^ lDateFrom = gcnew Label(); lDateFrom->Text = L"Từ:"; lDateFrom->Location = Point(230, 15); lDateFrom->AutoSize = true; lDateFrom->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10)); pnlOrderToolbar->Controls->Add(lDateFrom);
			dtpStartDate = gcnew DateTimePicker(); dtpStartDate->CustomFormat = "dd/MM/yyyy"; dtpStartDate->Format = DateTimePickerFormat::Custom;
			dtpStartDate->Location = Point(260, 12); dtpStartDate->Width = 100;
			dtpStartDate->Value = dtpStartDate->Value.AddYears(-1);
			dtpStartDate->ValueChanged += gcnew EventHandler(this, &SupermarketForm::OnOrderFilterChanged);
			pnlOrderToolbar->Controls->Add(dtpStartDate);

			// Lịch (Đến ngày)
			Label^ lDateTo = gcnew Label(); lDateTo->Text = L"Đến:"; lDateTo->Location = Point(370, 15); lDateTo->AutoSize = true; lDateTo->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10)); pnlOrderToolbar->Controls->Add(lDateTo);
			dtpEndDate = gcnew DateTimePicker(); dtpEndDate->CustomFormat = "dd/MM/yyyy"; dtpEndDate->Format = DateTimePickerFormat::Custom;
			dtpEndDate->Location = Point(410, 12); dtpEndDate->Width = 100;
			dtpEndDate->ValueChanged += gcnew EventHandler(this, &SupermarketForm::OnOrderFilterChanged);
			pnlOrderToolbar->Controls->Add(dtpEndDate);

			// Tìm kiếm
			Label^ lSearch = gcnew Label(); lSearch->Text = L"Mã Đơn:"; lSearch->Location = Point(530, 15); lSearch->AutoSize = true; lSearch->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10)); pnlOrderToolbar->Controls->Add(lSearch);
			txtOrderSearch = gcnew TextBox(); txtOrderSearch->Location = Point(590, 12); txtOrderSearch->Width = 150;
			txtOrderSearch->TextChanged += gcnew EventHandler(this, &SupermarketForm::OnOrderFilterChanged);
			pnlOrderToolbar->Controls->Add(txtOrderSearch);

			// Nút Lọc
			btnOrderFilter = gcnew Button(); btnOrderFilter->Text = L"Lọc"; btnOrderFilter->Location = Point(750, 12); btnOrderFilter->Width = 80;
			btnOrderFilter->Height = 28;
			btnOrderFilter->BackColor = Color::FromArgb(0, 114, 188); btnOrderFilter->ForeColor = Color::White;
			btnOrderFilter->Click += gcnew EventHandler(this, &SupermarketForm::OnOrderFilterChanged);
			pnlOrderToolbar->Controls->Add(btnOrderFilter);

			// 2. [MỚI] KHỞI TẠO FOOTER PHÂN TRANG - DOCK BOTTOM
			Panel^ pnlOrderFooter = gcnew Panel();
			pnlOrderFooter->Dock = DockStyle::Bottom;
			pnlOrderFooter->Height = 40; // Chiều cao nhỏ gọn
			pnlOrderFooter->BackColor = Color::WhiteSmoke; // Màu nền phân biệt

			// Tính toán vị trí Giữa (Center)
			// Lưu ý: Anchor None giúp nút luôn ở giữa khi resize
			int footerCenter = this->Width / 2 - 130; // Trừ sidebar 260/2

			// Nút Trước (Prev) - Nhỏ gọn
			btnPagePrev = gcnew Button();
			btnPagePrev->Text = L"<";
			btnPagePrev->Size = System::Drawing::Size(30, 25); // [NÚT NHỎ]
			btnPagePrev->Location = Point(footerCenter - 40, 8);
			btnPagePrev->Anchor = AnchorStyles::None;
			btnPagePrev->FlatStyle = FlatStyle::Flat;
			btnPagePrev->FlatAppearance->BorderColor = Color::Silver;
			btnPagePrev->BackColor = Color::White;
			btnPagePrev->Click += gcnew EventHandler(this, &SupermarketForm::OnPagePrevClick);
			pnlOrderFooter->Controls->Add(btnPagePrev);

			// Label Số Trang
			lblPageInfo = gcnew Label();
			lblPageInfo->Text = L"1 / 1";
			lblPageInfo->AutoSize = false;
			lblPageInfo->TextAlign = ContentAlignment::MiddleCenter;
			lblPageInfo->Size = System::Drawing::Size(60, 25);
			lblPageInfo->Location = Point(footerCenter, 8);
			lblPageInfo->Anchor = AnchorStyles::None;
			lblPageInfo->Font = gcnew System::Drawing::Font(L"Segoe UI", 9, FontStyle::Bold);
			pnlOrderFooter->Controls->Add(lblPageInfo);

			// Nút Sau (Next) - Nhỏ gọn
			btnPageNext = gcnew Button();
			btnPageNext->Text = L">";
			btnPageNext->Size = System::Drawing::Size(30, 25); // [NÚT NHỎ]
			btnPageNext->Location = Point(footerCenter + 70, 8);
			btnPageNext->Anchor = AnchorStyles::None;
			btnPageNext->FlatStyle = FlatStyle::Flat;
			btnPageNext->FlatAppearance->BorderColor = Color::Silver;
			btnPageNext->BackColor = Color::White;
			btnPageNext->Click += gcnew EventHandler(this, &SupermarketForm::OnPageNextClick);
			pnlOrderFooter->Controls->Add(btnPageNext);

			// 3. DATAGRIDVIEW - DOCK FILL (TỰ LẤP ĐẦY GIỮA HEADER VÀ FOOTER)
			this->dgvOrders = CreateStyledGrid();
			this->dgvOrders->Columns->Clear();
			this->dgvOrders->Columns->Add("MaDon", L"Mã Yêu Cầu");
			this->dgvOrders->Columns->Add("Ngay", L"Ngày Yêu Cầu");
			this->dgvOrders->Columns->Add("TienHang", L"Tiền Hàng");
			this->dgvOrders->Columns->Add("PhiShip", L"Phí Vận Chuyển");
			this->dgvOrders->Columns->Add("TongTien", L"Thành Tiền");
			//this->dgvOrders->Columns->Add("TrangThai", L"Trạng Thái");
			DataGridViewButtonColumn^ btnColStatus = gcnew DataGridViewButtonColumn();
			btnColStatus->Name = "TrangThai";
			btnColStatus->HeaderText = L"Trạng Thái";
			btnColStatus->FlatStyle = FlatStyle::Popup; // Hoặc FlatStyle::Standard
			this->dgvOrders->Columns->Add(btnColStatus);
			this->dgvOrders->CellContentClick += gcnew DataGridViewCellEventHandler(this, &SupermarketForm::dgvOrders_CellContentClick);
			this->dgvOrders->CellPainting += gcnew DataGridViewCellPaintingEventHandler(this, &SupermarketForm::dgvOrders_CellPainting);
			// Cấu hình Grid
			this->dgvOrders->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill; // Tự giãn đều
			this->dgvOrders->Columns["MaDon"]->Width = 110;
			this->dgvOrders->Columns["Ngay"]->Width = 110;
			this->dgvOrders->Columns["TienHang"]->Width = 130;
			this->dgvOrders->Columns["PhiShip"]->Width = 130;
			this->dgvOrders->Columns["TongTien"]->Width = 140;
			// Cột Trạng Thái tự giãn hết phần còn lại

			this->dgvOrders->Dock = DockStyle::Fill; // [QUAN TRỌNG] Tự lấp đầy
			this->dgvOrders->CellDoubleClick += gcnew DataGridViewCellEventHandler(this, &SupermarketForm::OnOrderGridDoubleClick);

			// Wrapper Panel để tạo khoảng hở (Padding) cho Grid đỡ dính vào Toolbar/Footer
			Panel^ pnlGridWrapper = gcnew Panel();
			pnlGridWrapper->Dock = DockStyle::Fill;
			pnlGridWrapper->Padding = System::Windows::Forms::Padding(0, 10, 0, 10); // Cách trên dưới 10px
			pnlGridWrapper->Controls->Add(this->dgvOrders);

			// --- THỨ TỰ ADD CONTROL VÀO TAB ---
			this->tabOrders->Controls->Clear();
			this->tabOrders->Controls->Add(pnlGridWrapper);  // 1. Grid (Fill)
			this->tabOrders->Controls->Add(pnlOrderFooter);  // 2. Footer (Bottom)
			this->tabOrders->Controls->Add(pnlOrderToolbar); // 3. Toolbar (Top)

			// Title Tab
			this->tabOrders->Controls->Add(this->lblTitleTab2);
			this->lblTitleTab2->Dock = DockStyle::Top;
			this->lblTitleTab2->Height = 60;
			this->lblTitleTab2->SendToBack(); // Đẩy Title lên trên cùng (vì Dock Top)

			// --- TAB 3: THÔNG TIN TÀI KHOẢN (PROFILE CARD) - [PHẦN MỚI] ---
			this->tabInfo = (gcnew System::Windows::Forms::TabPage());
			Color tabBackColor = Color::FromArgb(240, 242, 245);
			this->tabInfo->BackColor = tabBackColor;
			this->tabInfo->Padding = System::Windows::Forms::Padding(0);

			// GROUPBOX
			grpCardInfo = gcnew GroupBox();
			grpCardInfo->Text = L" THÔNG TIN TÀI KHOẢN ";
			grpCardInfo->Font = (gcnew System::Drawing::Font(L"Segoe UI", 14, FontStyle::Bold));

			// --- MÀU ĐEN TUYỀN CHO KHUNG ---
			grpCardInfo->ForeColor = Color::Black;
			// -------------------------------

			grpCardInfo->Size = System::Drawing::Size(800, 580);
			grpCardInfo->BackColor = tabBackColor;

			// Căn giữa
			grpCardInfo->Location = System::Drawing::Point(370, 120);

			Panel^ pnlInner = gcnew Panel();
			pnlInner->Dock = DockStyle::Fill;
			pnlInner->Padding = System::Windows::Forms::Padding(50, 60, 50, 30);
			pnlInner->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, FontStyle::Regular));
			pnlInner->ForeColor = Color::Black;
			pnlInner->BackColor = tabBackColor;

			// --- CÁC TRƯỜNG THÔNG TIN ---
			// Màu chữ tiêu đề: Đen
			Color lblColor = Color::Black;
			// Màu đường kẻ: Đen (thay vì xám)
			Color lineColor = Color::Black;
			// Độ dày đường kẻ: 3 (thay vì 2) cho đậm hơn
			int lineThickness = 3;

			// 1. ID
			Label^ l1 = gcnew Label(); l1->Text = L"Mã siêu thị (ID):"; l1->AutoSize = true; l1->ForeColor = lblColor;
			l1->Location = Point(50, 50);
			lblInfoID = gcnew Label(); lblInfoID->Text = L"..."; lblInfoID->Font = (gcnew System::Drawing::Font(L"Segoe UI", 13, FontStyle::Bold));
			lblInfoID->Location = Point(50, 75); lblInfoID->AutoSize = true;

			Label^ line1 = gcnew Label(); line1->BorderStyle = BorderStyle::Fixed3D;
			line1->Size = System::Drawing::Size(700, lineThickness); // <-- Tăng độ dày
			line1->Location = Point(50, 105); line1->ForeColor = lineColor; // <-- Màu đen

			// 2. Tên
			Label^ l2 = gcnew Label(); l2->Text = L"Họ và tên:"; l2->AutoSize = true; l2->ForeColor = lblColor;
			l2->Location = Point(50, 130);
			lblInfoName = gcnew Label(); lblInfoName->Text = L"...";
			lblInfoName->Location = Point(50, 155); lblInfoName->AutoSize = true;

			Label^ line2 = gcnew Label(); line2->BorderStyle = BorderStyle::Fixed3D;
			line2->Size = System::Drawing::Size(700, lineThickness);
			line2->Location = Point(50, 185); line2->ForeColor = lineColor;

			// 3. SDT
			Label^ l3 = gcnew Label(); l3->Text = L"Số điện thoại:"; l3->AutoSize = true; l3->ForeColor = lblColor;
			l3->Location = Point(50, 210);
			lblInfoPhone = gcnew Label(); lblInfoPhone->Text = L"...";
			lblInfoPhone->Location = Point(50, 235); lblInfoPhone->AutoSize = true;

			Label^ line3 = gcnew Label(); line3->BorderStyle = BorderStyle::Fixed3D;
			line3->Size = System::Drawing::Size(700, lineThickness);
			line3->Location = Point(50, 265); line3->ForeColor = lineColor;

			// 4. Địa chỉ
			Label^ l4 = gcnew Label(); l4->Text = L"Địa chỉ chi tiết:"; l4->AutoSize = true; l4->ForeColor = lblColor;
			l4->Location = Point(50, 290);
			lblInfoFullAddress = gcnew Label(); lblInfoFullAddress->Text = L"...";
			lblInfoFullAddress->Location = Point(50, 315); lblInfoFullAddress->AutoSize = true; lblInfoFullAddress->MaximumSize = System::Drawing::Size(700, 0);

			Label^ line4 = gcnew Label(); line4->BorderStyle = BorderStyle::Fixed3D;
			line4->Size = System::Drawing::Size(700, lineThickness);
			line4->Location = Point(50, 360); line4->ForeColor = lineColor;

			// BUTTON UPDATE
			btnUpdateInfo = gcnew Button();
			btnUpdateInfo->Text = L"CẬP NHẬT THÔNG TIN";
			btnUpdateInfo->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11, FontStyle::Bold));
			btnUpdateInfo->ForeColor = Color::White;
			btnUpdateInfo->BackColor = Color::FromArgb(0, 114, 188);
			btnUpdateInfo->FlatStyle = FlatStyle::Flat;
			btnUpdateInfo->FlatAppearance->BorderSize = 0;
			btnUpdateInfo->Size = System::Drawing::Size(700, 50);
			btnUpdateInfo->Location = Point(50, 450);
			btnUpdateInfo->Cursor = Cursors::Hand;
			btnUpdateInfo->Click += gcnew EventHandler(this, &SupermarketForm::OnUpdateInfoClick);

			pnlInner->Controls->Add(btnUpdateInfo);
			pnlInner->Controls->Add(line4); pnlInner->Controls->Add(line3); pnlInner->Controls->Add(line2); pnlInner->Controls->Add(line1);
			pnlInner->Controls->Add(lblInfoFullAddress); pnlInner->Controls->Add(l4);
			pnlInner->Controls->Add(lblInfoPhone); pnlInner->Controls->Add(l3);
			pnlInner->Controls->Add(lblInfoName); pnlInner->Controls->Add(l2);
			pnlInner->Controls->Add(lblInfoID); pnlInner->Controls->Add(l1);

			grpCardInfo->Controls->Add(pnlInner);
			this->tabInfo->Controls->Add(grpCardInfo);

			// --- TAB 4: ĐỔI MẬT KHẨU & TÀI KHOẢN ---
			this->tabChangePass = (gcnew System::Windows::Forms::TabPage());
			Color passTabColor = Color::FromArgb(240, 242, 245);
			this->tabChangePass->BackColor = passTabColor;
			this->tabChangePass->Padding = System::Windows::Forms::Padding(0);

			// TÍNH TOÁN VỊ TRÍ ĐỂ 2 BẢNG NẰM GIỮA MÀN HÌNH
			int boxWidth = 450;
			int gap = 40; // Khoảng cách giữa 2 bảng
			int startX = (this->Width - (boxWidth * 2 + gap)) / 2;
			// Nếu startX < 20 thì gán cứng 20 để không bị mất góc
			if (startX < 20) startX = 20;
			int startY = 80;

			// =========================================================================
			// BẢNG 1: ĐỔI TÊN TÀI KHOẢN (BÊN TRÁI)
			// =========================================================================
			GroupBox^ grpUser = gcnew GroupBox();
			grpUser->Text = L" ĐỔI TÊN TÀI KHOẢN ";
			grpUser->Font = (gcnew System::Drawing::Font(L"Segoe UI", 14, FontStyle::Bold));
			grpUser->ForeColor = Color::FromArgb(64, 64, 64);
			grpUser->Size = System::Drawing::Size(boxWidth, 480);
			grpUser->BackColor = passTabColor;
			grpUser->Location = System::Drawing::Point(startX, startY); // Vị trí bên trái

			Panel^ pnlUserInner = gcnew Panel();
			pnlUserInner->Dock = DockStyle::Fill;
			pnlUserInner->Padding = System::Windows::Forms::Padding(30, 50, 30, 30);
			pnlUserInner->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, FontStyle::Regular));
			pnlUserInner->ForeColor = Color::Black;
			pnlUserInner->BackColor = passTabColor;

			// 1. Tên hiện tại
			Label^ lCurrUser = gcnew Label(); lCurrUser->Text = L"Tên tài khoản hiện tại:";
			lCurrUser->AutoSize = true; lCurrUser->ForeColor = Color::DimGray;
			lCurrUser->Location = Point(35, 40);

			txtCurrentUser = gcnew TextBox(); txtCurrentUser->Size = System::Drawing::Size(320, 35);
			txtCurrentUser->Location = Point(35, 70);
			txtCurrentUser->BorderStyle = BorderStyle::None;
			txtCurrentUser->BackColor = passTabColor;
			txtCurrentUser->ReadOnly = true; // Chỉ xem
			txtCurrentUser->Text = this->CurrentUsername; // Hiển thị tên đăng nhập hiện tại
			txtCurrentUser->ForeColor = Color::DarkSlateGray;

			Label^ lineUser1 = gcnew Label(); lineUser1->BorderStyle = BorderStyle::Fixed3D;
			lineUser1->Size = System::Drawing::Size(320, 2); lineUser1->ForeColor = Color::Silver;
			lineUser1->Location = Point(35, 105);

			// 2. Tên mới
			Label^ lNewUser = gcnew Label(); lNewUser->Text = L"Tên tài khoản mới:";
			lNewUser->AutoSize = true; lNewUser->ForeColor = Color::DimGray;
			lNewUser->Location = Point(35, 135);

			txtNewUser = gcnew TextBox(); txtNewUser->Size = System::Drawing::Size(320, 35);
			txtNewUser->Location = Point(35, 165);
			txtNewUser->BorderStyle = BorderStyle::None;
			txtNewUser->BackColor = passTabColor;

			Label^ lineUser2 = gcnew Label(); lineUser2->BorderStyle = BorderStyle::Fixed3D;
			lineUser2->Size = System::Drawing::Size(320, 2); lineUser2->ForeColor = Color::Silver;
			lineUser2->Location = Point(35, 200);

			// Nút Xác Nhận Đổi Tên
			btnConfirmChangeUser = gcnew Button();
			btnConfirmChangeUser->Text = L"LƯU TÊN MỚI";
			btnConfirmChangeUser->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11, FontStyle::Bold));
			btnConfirmChangeUser->ForeColor = Color::White;
			btnConfirmChangeUser->BackColor = Color::FromArgb(0, 114, 188); // Màu Xanh dương khác với màu xanh lá bên kia
			btnConfirmChangeUser->FlatStyle = FlatStyle::Flat;
			btnConfirmChangeUser->FlatAppearance->BorderSize = 0;
			btnConfirmChangeUser->Size = System::Drawing::Size(320, 45);
			btnConfirmChangeUser->Location = Point(35, 360); // Căn nút xuống dưới cùng giống bên kia
			btnConfirmChangeUser->Cursor = Cursors::Hand;
			btnConfirmChangeUser->Click += gcnew EventHandler(this, &SupermarketForm::OnConfirmChangeUserClick);

			pnlUserInner->Controls->Add(btnConfirmChangeUser);
			pnlUserInner->Controls->Add(lineUser2); pnlUserInner->Controls->Add(txtNewUser); pnlUserInner->Controls->Add(lNewUser);
			pnlUserInner->Controls->Add(lineUser1); pnlUserInner->Controls->Add(txtCurrentUser); pnlUserInner->Controls->Add(lCurrUser);
			grpUser->Controls->Add(pnlUserInner);


			// =========================================================================
			// BẢNG 2: ĐỔI MẬT KHẨU (BÊN PHẢI - GIỮ NGUYÊN STYLE CŨ)
			// =========================================================================
			GroupBox^ grpPass = gcnew GroupBox();
			grpPass->Text = L" ĐỔI MẬT KHẨU ";
			grpPass->Font = (gcnew System::Drawing::Font(L"Segoe UI", 14, FontStyle::Bold));
			grpPass->ForeColor = Color::FromArgb(64, 64, 64);
			grpPass->Size = System::Drawing::Size(boxWidth, 480);
			grpPass->BackColor = passTabColor;
			grpPass->Location = System::Drawing::Point(startX + boxWidth + gap, startY); // Vị trí bên phải

			Panel^ pnlPassInner = gcnew Panel();
			pnlPassInner->Dock = DockStyle::Fill;
			pnlPassInner->Padding = System::Windows::Forms::Padding(30, 50, 30, 30);
			pnlPassInner->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, FontStyle::Regular));
			pnlPassInner->ForeColor = Color::Black;
			pnlPassInner->BackColor = passTabColor;

			int passInputWidth = 320;
			int passStartX = 35;
			Color passLblColor = Color::DimGray;
			Color passLineColor = Color::Silver;

			// 1. Mật khẩu hiện tại
			Label^ lOld = gcnew Label(); lOld->Text = L"Mật khẩu hiện tại:"; lOld->AutoSize = true;
			lOld->ForeColor = passLblColor; lOld->Location = Point(passStartX, 40);

			txtOldPass = gcnew TextBox(); txtOldPass->Size = System::Drawing::Size(passInputWidth, 35);
			txtOldPass->Location = Point(passStartX, 70); txtOldPass->UseSystemPasswordChar = true;
			txtOldPass->BorderStyle = BorderStyle::None; txtOldPass->BackColor = passTabColor;

			Label^ lineOld = gcnew Label(); lineOld->BorderStyle = BorderStyle::Fixed3D;
			lineOld->Size = System::Drawing::Size(passInputWidth, 2); lineOld->ForeColor = passLineColor;
			lineOld->Location = Point(passStartX, 105);

			// 2. Mật khẩu mới
			Label^ lNew = gcnew Label(); lNew->Text = L"Mật khẩu mới:"; lNew->AutoSize = true;
			lNew->ForeColor = passLblColor; lNew->Location = Point(passStartX, 135);

			txtNewPass = gcnew TextBox(); txtNewPass->Size = System::Drawing::Size(passInputWidth, 35);
			txtNewPass->Location = Point(passStartX, 165); txtNewPass->UseSystemPasswordChar = true;
			txtNewPass->BorderStyle = BorderStyle::None; txtNewPass->BackColor = passTabColor;

			btnEyeNew = gcnew Button(); btnEyeNew->Size = System::Drawing::Size(35, 30);
			btnEyeNew->Location = Point(passStartX + passInputWidth - 35, 165);
			btnEyeNew->FlatStyle = FlatStyle::Flat; btnEyeNew->FlatAppearance->BorderSize = 0;
			btnEyeNew->BackColor = passTabColor; btnEyeNew->Cursor = Cursors::Hand;
			if (imgShowPass != nullptr) { btnEyeNew->BackgroundImage = imgShowPass; btnEyeNew->BackgroundImageLayout = ImageLayout::Zoom; }
			else { btnEyeNew->Text = L"👁"; }
			btnEyeNew->Click += gcnew EventHandler(this, &SupermarketForm::OnToggleNewPass);

			Label^ lineNew = gcnew Label(); lineNew->BorderStyle = BorderStyle::Fixed3D;
			lineNew->Size = System::Drawing::Size(passInputWidth, 2); lineNew->ForeColor = passLineColor;
			lineNew->Location = Point(passStartX, 200);

			// 3. Nhập lại mật khẩu
			Label^ lConfirm = gcnew Label(); lConfirm->Text = L"Nhập lại mật khẩu mới:"; lConfirm->AutoSize = true;
			lConfirm->ForeColor = passLblColor; lConfirm->Location = Point(passStartX, 230);

			txtConfirmPass = gcnew TextBox(); txtConfirmPass->Size = System::Drawing::Size(passInputWidth, 35);
			txtConfirmPass->Location = Point(passStartX, 260); txtConfirmPass->UseSystemPasswordChar = true;
			txtConfirmPass->BorderStyle = BorderStyle::None; txtConfirmPass->BackColor = passTabColor;

			btnEyeConfirm = gcnew Button(); btnEyeConfirm->Size = System::Drawing::Size(35, 30);
			btnEyeConfirm->Location = Point(passStartX + passInputWidth - 35, 260);
			btnEyeConfirm->FlatStyle = FlatStyle::Flat; btnEyeConfirm->FlatAppearance->BorderSize = 0;
			btnEyeConfirm->BackColor = passTabColor; btnEyeConfirm->Cursor = Cursors::Hand;
			if (imgShowPass != nullptr) { btnEyeConfirm->BackgroundImage = imgShowPass; btnEyeConfirm->BackgroundImageLayout = ImageLayout::Zoom; }
			else { btnEyeConfirm->Text = L"👁"; }
			btnEyeConfirm->Click += gcnew EventHandler(this, &SupermarketForm::OnToggleConfirmPass);

			Label^ lineConfirm = gcnew Label(); lineConfirm->BorderStyle = BorderStyle::Fixed3D;
			lineConfirm->Size = System::Drawing::Size(passInputWidth, 2); lineConfirm->ForeColor = passLineColor;
			lineConfirm->Location = Point(passStartX, 295);

			// NÚT XÁC NHẬN ĐỔI MẬT KHẨU
			btnConfirmChangePass = gcnew Button();
			btnConfirmChangePass->Text = L"XÁC NHẬN ĐỔI";
			btnConfirmChangePass->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11, FontStyle::Bold));
			btnConfirmChangePass->ForeColor = Color::White;
			btnConfirmChangePass->BackColor = Color::FromArgb(34, 139, 34);
			btnConfirmChangePass->FlatStyle = FlatStyle::Flat; btnConfirmChangePass->FlatAppearance->BorderSize = 0;
			btnConfirmChangePass->Size = System::Drawing::Size(passInputWidth, 45);
			btnConfirmChangePass->Location = Point(passStartX, 360);
			btnConfirmChangePass->Cursor = Cursors::Hand;
			btnConfirmChangePass->Click += gcnew EventHandler(this, &SupermarketForm::OnConfirmChangePassClick);

			pnlPassInner->Controls->Add(btnEyeConfirm); pnlPassInner->Controls->Add(btnEyeNew); pnlPassInner->Controls->Add(btnConfirmChangePass);
			pnlPassInner->Controls->Add(lineConfirm); pnlPassInner->Controls->Add(txtConfirmPass); pnlPassInner->Controls->Add(lConfirm);
			pnlPassInner->Controls->Add(lineNew); pnlPassInner->Controls->Add(txtNewPass); pnlPassInner->Controls->Add(lNew);
			pnlPassInner->Controls->Add(lineOld); pnlPassInner->Controls->Add(txtOldPass); pnlPassInner->Controls->Add(lOld);
			grpPass->Controls->Add(pnlPassInner);

			// Add cả 2 GroupBox vào Tab
			this->tabChangePass->Controls->Add(grpUser);
			this->tabChangePass->Controls->Add(grpPass);

			// Add tab vào danh sách (Lúc này là Tab thứ 4, Index = 3)
			this->tabContent->Controls->Add(this->tabGeneral);

			// 2. Đơn Hàng (Index 1)
			this->tabContent->Controls->Add(this->tabOrders);

			// 3. Thông Tin Cá Nhân (Index 2)
			this->tabContent->Controls->Add(this->tabInfo);

			// 4. Đổi Mật Khẩu (Index 3) - DÁN VÀO ĐÂY
			this->tabContent->Controls->Add(this->tabChangePass);
			// -----------------------------------------------------------
			// D. HOÀN TẤT
			// -----------------------------------------------------------
			this->Controls->Add(this->tabContent);
			this->Controls->Add(this->pnlSidebar);
		}
		// --- CÁC HÀM SỰ KIỆN MENU ---
		// 1. Sự kiện khi bấm nút chính "Cá Nhân" -> Đóng/Mở menu con
		inline void SupermarketForm::btnMenuPersonal_Click(Object^ sender, EventArgs^ e) {
			// 1. Xử lý đóng/mở menu con (Giữ nguyên logic cũ)
			isPersonalMenuOpen = !isPersonalMenuOpen;
			pnlSubMenuPersonal->Visible = isPersonalMenuOpen;

			// 2. [MỚI] Chuyển ngay sang Tab Thông tin (Index 2)
			tabContent->SelectedIndex = 2;

			// 3. [MỚI] Load dữ liệu từ SQL lên luôn
			LoadThongTinCaNhan();

			// 4. Đổi màu nút để báo hiệu đang chọn
			ResetMenuColors();
			btnMenuPersonal->BackColor = Color::FromArgb(23, 42, 69); // Màu Active
		}

		// 2. Sự kiện bấm "Thông tin tài khoản" -> Chuyển sang Tab Profile
		inline void SupermarketForm::OnSubInfoClick(Object^ sender, EventArgs^ e) {
			try {
				// 1. Kiểm tra xem TabInfo có tồn tại không
				if (tabInfo == nullptr || tabContent == nullptr) {
					MessageBox::Show(L"Lỗi: Tab Thông Tin chưa được khởi tạo.", L"Lỗi Code", MessageBoxButtons::OK, MessageBoxIcon::Error);
					return;
				}

				// 2. Chuyển Tab
				tabContent->SelectedTab = tabInfo;

				// 3. Highlight Text
				ResetSubMenuColors();
				if (btnSubInfo != nullptr) btnSubInfo->ForeColor = Color::White;

				// 4. Load dữ liệu (Quan trọng: Hàm này dễ gây lỗi nhất)
				LoadThongTinCaNhan();
			}
			catch (Exception^ ex) {
				MessageBox::Show(L"Lỗi khi mở thông tin: " + ex->Message, L"Crash App", MessageBoxButtons::OK, MessageBoxIcon::Error);
			}
		}
		// 3. Sự kiện bấm "Đổi mật khẩu"
		inline void SupermarketForm::OnChangePassClick(Object^ sender, EventArgs^ e) {
			ResetSubMenuColors();
			btnSubPass->ForeColor = Color::White; // Highlight nút

			// Chuyển sang Tab Đổi Mật Khẩu (Index 3)
			tabContent->SelectedIndex = 3;
		}

		// 4. Sự kiện bấm "Đăng xuất"
		inline void SupermarketForm::OnLogoutClick(Object^ sender, EventArgs^ e) {
			if (MessageBox::Show(L"Bạn có chắc chắn muốn đăng xuất?", L"Xác nhận", MessageBoxButtons::YesNo, MessageBoxIcon::Question) == System::Windows::Forms::DialogResult::Yes) {
				this->Close(); // Đóng form hiện tại -> Trở về màn hình đăng nhập (nếu có logic đó ở Program.cpp)
			}
		}
		// Hàm xử lý ẩn/hiện mật khẩu MỚI
		void OnToggleNewPass(Object^ sender, EventArgs^ e) {
			txtNewPass->UseSystemPasswordChar = !txtNewPass->UseSystemPasswordChar;

			if (imgShowPass != nullptr && imgHidePass != nullptr) {
				if (txtNewPass->UseSystemPasswordChar) {
					// Đang ẩn -> Hiện icon "Mở mắt"
					btnEyeNew->BackgroundImage = imgShowPass;
				}
				else {
					// Đang hiện -> Hiện icon "Đóng mắt" (Gạch chéo)
					btnEyeNew->BackgroundImage = imgHidePass;
				}
			}
			else {
				btnEyeNew->Text = txtNewPass->UseSystemPasswordChar ? L"👁" : L"❌";
			}
		}
		// HÀM XỬ LÝ ĐỔI TÊN TÀI KHOẢN
		void OnConfirmChangeUserClick(Object^ sender, EventArgs^ e) {
			String^ newName = txtNewUser->Text->Trim();
			String^ currentName = this->CurrentUsername;

			if (String::IsNullOrEmpty(newName)) {
				MessageBox::Show(L"Vui lòng nhập tên tài khoản mới!", L"Cảnh báo", MessageBoxButtons::OK, MessageBoxIcon::Warning);
				return;
			}
			if (newName == currentName) {
				MessageBox::Show(L"Tên mới không được trùng với tên hiện tại!", L"Cảnh báo", MessageBoxButtons::OK, MessageBoxIcon::Warning);
				return;
			}

			// Kết nối SQL
			SQLHENV hEnv = SQL_NULL_HENV; SQLHDBC hDbc = SQL_NULL_HDBC; SQLHSTMT hStmt = SQL_NULL_HSTMT;
			SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
			SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
			SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
			SQLWCHAR* connStr = (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;";

			if (SQL_SUCCEEDED(SQLDriverConnect(hDbc, NULL, connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT))) {

				// 1. CHECK TRÙNG TÊN (GIỮ NGUYÊN)
				if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
					std::wstring sqlCheck = L"SELECT COUNT(*) FROM TaiKhoan WHERE TenDangNhap = ?";
					SQLPrepare(hStmt, (SQLWCHAR*)sqlCheck.c_str(), SQL_NTS);

					std::string sNew = ToStringStandard(newName);
					SQLCHAR cNew[51]; strcpy_s((char*)cNew, 51, sNew.c_str());
					SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cNew, 0, NULL);

					if (SQLExecute(hStmt) == SQL_SUCCESS) {
						if (SQLFetch(hStmt) == SQL_SUCCESS) {
							SQLLEN count;
							SQLGetData(hStmt, 1, SQL_C_LONG, &count, 0, NULL);
							if (count > 0) {
								MessageBox::Show(L"Tên tài khoản này đã tồn tại! Vui lòng chọn tên khác.", L"Lỗi", MessageBoxButtons::OK, MessageBoxIcon::Error);
								SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
								SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
								return;
							}
						}
					}
					SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
				}

				// 2. THỰC HIỆN UPDATE (CÓ KIỂM TRA ROW COUNT)
				if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
					// Update dựa trên MaLienKet (Chắc chắn đúng)
					std::wstring sqlUpd = L"UPDATE TaiKhoan SET TenDangNhap = ? WHERE MaLienKet = ?";
					SQLPrepare(hStmt, (SQLWCHAR*)sqlUpd.c_str(), SQL_NTS);

					std::string sNew = ToStringStandard(newName);
					SQLCHAR cNew[51]; strcpy_s((char*)cNew, 51, sNew.c_str());

					std::string sID = ToStringStandard(this->MaSieuThiHienTai);
					SQLCHAR cID[51]; strcpy_s((char*)cID, 51, sID.c_str());

					SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cNew, 0, NULL);
					SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cID, 0, NULL);

					if (SQLExecute(hStmt) == SQL_SUCCESS) {
						// [QUAN TRỌNG] Kiểm tra xem có dòng nào thực sự bị thay đổi không
						SQLLEN rowsAffected = 0;
						SQLRowCount(hStmt, &rowsAffected);

						if (rowsAffected > 0) {
							MessageBox::Show(L"Đổi tên tài khoản thành công! Vui lòng đăng nhập lại.", L"Thành công", MessageBoxButtons::OK, MessageBoxIcon::Information);

							this->CurrentUsername = newName;
							txtCurrentUser->Text = newName; // Hiện tên mới lên ô cũ
							txtNewUser->Text = "";          // Xóa ô nhập mới
							lblUserInfo->Text = newName + L"\n(Siêu Thị)";
						}
						else {
							// Trường hợp chạy thành công nhưng không tìm thấy ID để sửa
							MessageBox::Show(L"Lỗi: Không tìm thấy tài khoản liên kết (Mã: " + this->MaSieuThiHienTai + L") để cập nhật.", L"Thất bại", MessageBoxButtons::OK, MessageBoxIcon::Error);
						}
					}
					else {
						ShowSQLError(hStmt);
					}
				}
			}
			if (hStmt) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
			if (hDbc) { SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); }
			if (hEnv) SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
		}
		// Hàm bật/tắt hiển thị Xác Nhận Mật Khẩu
		void OnToggleConfirmPass(Object^ sender, EventArgs^ e) {
			txtConfirmPass->UseSystemPasswordChar = !txtConfirmPass->UseSystemPasswordChar;

			if (imgShowPass != nullptr && imgHidePass != nullptr) {
				if (txtConfirmPass->UseSystemPasswordChar) {
					btnEyeConfirm->BackgroundImage = imgShowPass;
				}
				else {
					btnEyeConfirm->BackgroundImage = imgHidePass;
				}
			}
			else {
				btnEyeConfirm->Text = txtConfirmPass->UseSystemPasswordChar ? L"👁" : L"❌";
			}
		}
		// Hàm phụ: Reset màu chữ các nút con về mặc định (Xám)
		void ResetSubMenuColors() {
			btnSubInfo->ForeColor = Color::FromArgb(180, 180, 180);
			btnSubPass->ForeColor = Color::FromArgb(180, 180, 180);
			// Nút Logout giữ nguyên màu cam đặc trưng, không cần reset
		}
		// Hàm reset màu các nút về trạng thái bình thường
		void ResetMenuColors() {
			Color colorPrimary = Color::FromArgb(10, 25, 47);
			if (btnMenuGeneral != nullptr)
				btnMenuGeneral->BackColor = colorPrimary;

			if (btnMenuOrders != nullptr)
				btnMenuOrders->BackColor = colorPrimary;
			if (btnMenuPersonal != nullptr)
				btnMenuPersonal->BackColor = colorPrimary;
			if (btnMenuInfo != nullptr)
				btnMenuInfo->BackColor = colorPrimary;
		}

		void btnMenuGeneral_Click(System::Object^ sender, System::EventArgs^ e) {
			tabContent->SelectedIndex = 0;
			ResetMenuColors();
			btnMenuGeneral->BackColor = Color::FromArgb(23, 42, 69);
		}

		void btnMenuOrders_Click(System::Object^ sender, System::EventArgs^ e) {
			tabContent->SelectedIndex = 1;
			ResetMenuColors();
			btnMenuOrders->BackColor = Color::FromArgb(23, 42, 69);
		}

		// Hàm cho nút mới
		inline void SupermarketForm::btnMenuInfo_Click(System::Object^ sender, System::EventArgs^ e) {
			tabContent->SelectedIndex = 2;
			ResetMenuColors();
			btnMenuInfo->BackColor = Color::FromArgb(23, 42, 69);

			// GỌI HÀM LOAD DỮ LIỆU MỚI
			LoadThongTinCaNhan();
		}

		// Hàm xử lý nút Sửa/Lưu
		void btnEditInfo_Click(System::Object^ sender, System::EventArgs^ e) {
			Button^ btn = dynamic_cast<Button^>(sender);
			TableLayoutPanel^ tlp = dynamic_cast<TableLayoutPanel^>(btn->Parent);
			TableLayoutPanelCellPosition pos = tlp->GetPositionFromControl(btn);
			TextBox^ txtBox = dynamic_cast<TextBox^>(tlp->GetControlFromPosition(1, pos.Row));

			if (btn->Text == L"✎") {
				txtBox->ReadOnly = false;
				txtBox->BorderStyle = BorderStyle::FixedSingle;
				txtBox->Focus();
				btn->Text = L"✓";
				btn->ForeColor = Color::Green;
			}
			else {
				// Lưu dữ liệu (Viết code SQL update ở đây nếu cần)
				MessageBox::Show(L"Đã cập nhật: " + txtBox->Text, L"Thành công");

				txtBox->ReadOnly = true;
				txtBox->BorderStyle = BorderStyle::None;
				btn->Text = L"✎";
				btn->ForeColor = Color::Orange;
			}
		}

		void btnCart_Click(Object^ sender, EventArgs^ e) {
			CartForm^ frm = gcnew CartForm(ShoppingCart, MaSieuThiHienTai, imageMap);

			// Khi form Giỏ hàng đóng lại với kết quả OK (Đã đặt hàng thành công)
			if (frm->ShowDialog() == System::Windows::Forms::DialogResult::OK) {

				// 1. Cập nhật lại số lượng trên nút giỏ hàng (về 0)
				btnCart->Text = L"🛒 Giỏ Hàng (" + ShoppingCart->Count + ")";

				// 2. [THÊM DÒNG NÀY] Gọi hàm tải lại lịch sử đơn hàng từ Database
				LoadLichSuDonHang();
			}
		}
		// Thêm vào SupermarketForm.h (Trong phần private: của class SupermarketForm)

// Xử lý sự kiện Double Click trên lưới đơn hàng
		void OnOrderGridDoubleClick(Object^ sender, DataGridViewCellEventArgs^ e) {
			if (e->RowIndex < 0 || e->RowIndex >= dgvOrders->RowCount) return;

			// Lấy Mã đơn (cột 0)
			String^ maDon = dgvOrders->Rows[e->RowIndex]->Cells[0]->Value->ToString();

			// Kết nối DB
			SQLHENV hEnv = SQL_NULL_HENV; SQLHDBC hDbc = SQL_NULL_HDBC;
			SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
			SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
			SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
			SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

			OrderHeader^ header;
			List<OrderDetail^>^ details;

			// Lấy dữ liệu
			LayChiTietDonHangFull(hDbc, maDon, header, details);

			// Mở Popup Chi tiết
			// --- CẬP NHẬT: Truyền thêm 'this->imageMap' vào tham số cuối cùng ---
			OrderDetailForm^ frm = gcnew OrderDetailForm(header, details, (System::IntPtr)hDbc, this->imageMap);
			frm->ShowDialog();

			SQLDisconnect(hDbc);
			SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
			SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
		}

		void ApplyOrderFilters() {
			if (originalOrderList == nullptr) return;

			// 1. LỌC DỮ LIỆU (FILTERING)
			List<OrderHeader^>^ filteredList = gcnew List<OrderHeader^>();
			String^ selectedStatus = cboOrderStatus->SelectedItem->ToString();
			String^ searchKeyword = txtOrderSearch->Text->Trim()->ToLower();
			DateTime startDate = dtpStartDate->Value.Date;
			DateTime endDate = dtpEndDate->Value.Date.AddDays(1).AddSeconds(-1);

			for each (OrderHeader ^ h in originalOrderList) {
				bool matchStatus = (selectedStatus == L"Tất cả") || (h->TrangThai == selectedStatus);
				bool matchDate = (h->NgayDat.Date >= startDate.Date) && (h->NgayDat.Date <= endDate.Date);
				bool matchSearch = String::IsNullOrEmpty(searchKeyword) || h->MaDon->ToLower()->Contains(searchKeyword);

				if (matchStatus && matchDate && matchSearch) {
					filteredList->Add(h);
				}
			}

			// 2. TÍNH TOÁN PHÂN TRANG (PAGING LOGIC)
			int totalRecords = filteredList->Count;

			// Tính tổng số trang (Làm tròn lên)
			this->totalPages = (int)Math::Ceiling((double)totalRecords / (double)this->pageSize);
			if (this->totalPages < 1) this->totalPages = 1;

			// Đảm bảo trang hiện tại hợp lệ
			if (this->currentPage > this->totalPages) this->currentPage = this->totalPages;
			if (this->currentPage < 1) this->currentPage = 1;

			// Cập nhật Label
			if (lblPageInfo != nullptr)
				lblPageInfo->Text = this->currentPage.ToString() + " / " + this->totalPages.ToString();

			// Tính vị trí bắt đầu và kết thúc
			int startIndex = (this->currentPage - 1) * this->pageSize;
			int count = Math::Min(this->pageSize, totalRecords - startIndex);

			// 3. HIỂN THỊ DỮ LIỆU (RENDERING)
			dgvOrders->Rows->Clear();

			// Chỉ duyệt vòng lặp trong phạm vi trang hiện tại (startIndex -> count)
			if (totalRecords > 0 && startIndex < totalRecords) {
				for (int i = startIndex; i < startIndex + count; i++) {
					OrderHeader^ h = filteredList[i];

					int idx = dgvOrders->Rows->Add(
						h->MaDon,
						h->NgayDat.ToString("dd/MM/yyyy"),
						h->TongGiaTriHang.ToString("N0") + L" đ",
						h->PhiVanChuyen.ToString("N0") + L" đ",
						h->TongThanhToan.ToString("N0") + L" đ",
						h->TrangThai
					);

					dgvOrders->Rows[idx]->DefaultCellStyle->BackColor = GetStatusColor(h->TrangThai);
					if (h->TrangThai == L"Đang Vận Chuyển") {
						// Làm cho ô này trông giống nút bấm (màu xanh dương đậm chẳng hạn)
						dgvOrders->Rows[idx]->Cells["TrangThai"]->Style->BackColor = Color::FromArgb(0, 120, 215);
						dgvOrders->Rows[idx]->Cells["TrangThai"]->Style->ForeColor = Color::White;
						dgvOrders->Rows[idx]->Cells["TrangThai"]->Style->SelectionBackColor = Color::FromArgb(0, 100, 190);
					}
					else if (h->TrangThai == L"Đã Giao Hàng" || h->TrangThai == L"Hoàn thành") {
						// Màu xanh lá nhạt cho đơn đã xong
						dgvOrders->Rows[idx]->Cells["TrangThai"]->Style->ForeColor = Color::Green;
						dgvOrders->Rows[idx]->Cells["TrangThai"]->Style->Font = gcnew System::Drawing::Font(dgvOrders->Font, FontStyle::Bold);
					}
					dgvOrders->Rows[idx]->Cells[2]->Style->ForeColor = Color::DarkSlateGray;
					dgvOrders->Rows[idx]->Cells[3]->Style->ForeColor = Color::DarkOrange;
					dgvOrders->Rows[idx]->Cells[4]->Style->ForeColor = Color::Red;
					dgvOrders->Rows[idx]->Cells[4]->Style->Font = gcnew System::Drawing::Font(dgvOrders->Font, FontStyle::Bold);
				}
			}
		}
		// Sự kiện nút "<" (Trang trước)
		void OnPagePrevClick(Object^ sender, EventArgs^ e) {
			if (this->currentPage > 1) {
				this->currentPage--;
				ApplyOrderFilters(); // Load lại grid theo trang mới
			}
		}
		void OnPageNextClick(Object^ sender, EventArgs^ e) {
			if (this->currentPage < this->totalPages) {
				this->currentPage++;
				ApplyOrderFilters(); // Load lại grid theo trang mới
			}
		}
		void OnOrderFilterChanged(Object^ sender, EventArgs^ e) {
			this->currentPage = 1; // Khi lọc thì phải về trang đầu
			ApplyOrderFilters();
		}
		
		void LoadThongTinCaNhan() {
			if (lblInfoID == nullptr) return;

			// Hiển thị ID cố định
			lblInfoID->Text = this->MaSieuThiHienTai;

			SQLHENV hEnv = SQL_NULL_HENV; SQLHDBC hDbc = SQL_NULL_HDBC; SQLHSTMT hStmt = SQL_NULL_HSTMT;
			SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
			SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
			SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);

			SQLWCHAR* connStr = (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;";

			if (SQL_SUCCEEDED(SQLDriverConnect(hDbc, NULL, connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT))) {
				if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {

					std::wstring sql = L"SELECT TenSieuThi, SDT, DiaChi, QuanHuyen, Tinh FROM SieuThi WHERE MaSieuThi LIKE ?";
					SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);

					std::string sMa = ToStringStandard(this->MaSieuThiHienTai);
					std::string sMaSach = "";
					for (char c : sMa) if (!isspace(c)) sMaSach += c;

					sMaSach += "%";
					SQLCHAR cMa[51]; strcpy_s((char*)cMa, 51, sMa.c_str());
					SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMa, 0, NULL);

					if (SQLExecute(hStmt) == SQL_SUCCESS) {
						if (SQLFetch(hStmt) == SQL_SUCCESS) {
							SQLWCHAR wTen[256], wSDT[50], wDC[256], wQuan[101], wTinh[101];
							SQLLEN len;

							SQLGetData(hStmt, 1, SQL_C_WCHAR, wTen, 256, &len);
							lblInfoName->Text = (len != SQL_NULL_DATA) ? gcnew String(wTen) : L"";

							SQLGetData(hStmt, 2, SQL_C_WCHAR, wSDT, 50, &len);
							lblInfoPhone->Text = (len != SQL_NULL_DATA) ? gcnew String(wSDT) : L"";

							// Ghép địa chỉ hiển thị cho đẹp
							String^ dc = L""; String^ q = L""; String^ t = L"";
							SQLGetData(hStmt, 3, SQL_C_WCHAR, wDC, 256, &len);
							if (len != SQL_NULL_DATA) dc = gcnew String(wDC);

							SQLGetData(hStmt, 4, SQL_C_WCHAR, wQuan, 101, &len);
							if (len != SQL_NULL_DATA) q = gcnew String(wQuan);

							SQLGetData(hStmt, 5, SQL_C_WCHAR, wTinh, 101, &len);
							if (len != SQL_NULL_DATA) t = gcnew String(wTinh);

							lblInfoFullAddress->Text = dc + L", " + q + L", " + t;

							// Lưu tạm vào Tag để lúc mở form Update lấy ra dùng
							lblInfoFullAddress->Tag = dc; // Lưu đường
							lblInfoPhone->Tag = q;        // Lưu tạm quận vào Tag của sdt (hack xíu cho gọn code)
							lblInfoName->Tag = t;         // Lưu tạm tỉnh vào Tag của tên
						}
					}
				}
			}
			if (hStmt) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
			if (hDbc) { SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); }
			if (hEnv) SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
		}
		void OnConfirmChangePassClick(Object^ sender, EventArgs^ e) {
			String^ oldP = txtOldPass->Text;
			String^ newP = txtNewPass->Text;
			String^ confirmP = txtConfirmPass->Text;

			// 1. Kiểm tra đầu vào
			if (String::IsNullOrWhiteSpace(oldP) || String::IsNullOrWhiteSpace(newP) || String::IsNullOrWhiteSpace(confirmP)) {
				MessageBox::Show(L"Vui lòng nhập đầy đủ thông tin!", L"Cảnh báo", MessageBoxButtons::OK, MessageBoxIcon::Warning);
				return;
			}
			if (newP != confirmP) {
				MessageBox::Show(L"Mật khẩu mới không khớp!", L"Lỗi", MessageBoxButtons::OK, MessageBoxIcon::Error);
				return;
			}

			// Lấy Mã Liên Kết (MaSieuThiHienTai) làm điều kiện xét
			String^ maLienKet = this->MaSieuThiHienTai;
			if (String::IsNullOrEmpty(maLienKet)) {
				MessageBox::Show(L"Lỗi hệ thống: Không tìm thấy Mã Liên Kết.", L"Lỗi", MessageBoxButtons::OK, MessageBoxIcon::Error);
				return;
			}

			// 2. Kết nối SQL
			SQLHENV hEnv = SQL_NULL_HENV; SQLHDBC hDbc = SQL_NULL_HDBC; SQLHSTMT hStmt = SQL_NULL_HSTMT;
			SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
			SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
			SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
			SQLWCHAR* connStr = (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;";

			if (SQL_SUCCEEDED(SQLDriverConnect(hDbc, NULL, connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT))) {

				// Chuẩn bị biến Mã Liên Kết cho SQL
				std::string sMaLK = ToStringStandard(maLienKet->Trim());
				SQLCHAR cMaLK[51]; strcpy_s((char*)cMaLK, 51, sMaLK.c_str());


				// 3. KIỂM TRA MẬT KHẨU CŨ (SỬ DỤNG MaLienKet)
				if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
					// LẤY MẬT KHẨU DỰA TRÊN MaLienKet
					std::wstring sqlCheck = L"SELECT MatKhau FROM TaiKhoan WHERE MaLienKet = ?";
					SQLPrepare(hStmt, (SQLWCHAR*)sqlCheck.c_str(), SQL_NTS);

					// Bind MaLienKet
					SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMaLK, 0, NULL);

					bool oldPassCorrect = false;
					bool userFound = false;

					if (SQLExecute(hStmt) == SQL_SUCCESS) {
						if (SQLFetch(hStmt) == SQL_SUCCESS) {
							userFound = true;
							SQLCHAR dbPass[51]; SQLLEN len;
							SQLGetData(hStmt, 1, SQL_C_CHAR, dbPass, 51, &len);

							String^ sDbPass = L"";
							if (len != SQL_NULL_DATA) {
								dbPass[len] = '\0';
								sDbPass = gcnew String((char*)dbPass);
							}

							// So sánh: Trim() cả 2 bên 
							if (sDbPass->Trim() == oldP->Trim()) {
								oldPassCorrect = true;
							}
						}
					}
					SQLFreeHandle(SQL_HANDLE_STMT, hStmt);

					if (!userFound) {
						MessageBox::Show(L"Không tìm thấy tài khoản liên kết để đổi mật khẩu.", L"Lỗi hệ thống", MessageBoxButtons::OK, MessageBoxIcon::Error);
						SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
						return;
					}

					if (!oldPassCorrect) {
						MessageBox::Show(L"Mật khẩu hiện tại không đúng!", L"Lỗi", MessageBoxButtons::OK, MessageBoxIcon::Error);
						SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
						return;
					}
				}

				// 4. CẬP NHẬT MẬT KHẨU MỚI (SỬ DỤNG MaLienKet)
				if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
					// UPDATE DỰA TRÊN MaLienKet
					std::wstring sqlUpdate = L"UPDATE TaiKhoan SET MatKhau = ? WHERE MaLienKet = ?";
					SQLPrepare(hStmt, (SQLWCHAR*)sqlUpdate.c_str(), SQL_NTS);

					std::string sNewPass = ToStringStandard(newP->Trim());
					SQLCHAR cNewPass[51]; strcpy_s((char*)cNewPass, 51, sNewPass.c_str());

					// Bind MaKhau mới
					SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cNewPass, 0, NULL);

					// Bind MaLienKet cho WHERE clause
					SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMaLK, 0, NULL);

					if (SQLExecute(hStmt) == SQL_SUCCESS) {
						MessageBox::Show(L"Đổi mật khẩu thành công!", L"Thông báo", MessageBoxButtons::OK, MessageBoxIcon::Information);
						txtOldPass->Clear(); txtNewPass->Clear(); txtConfirmPass->Clear();
					}
					else {
						MessageBox::Show(L"Lỗi khi cập nhật CSDL!", L"Lỗi", MessageBoxButtons::OK, MessageBoxIcon::Error);
					}
				}
			}
			if (hStmt) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
			if (hDbc) { SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); }
			if (hEnv) SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
		}
		// Sự kiện khi bấm nút CẬP NHẬT THÔNG TIN
		void OnUpdateInfoClick(Object^ sender, EventArgs^ e) {
			// Lấy dữ liệu hiện tại đang hiển thị để truyền vào form con
			String^ currTen = lblInfoName->Text;
			String^ currSDT = lblInfoPhone->Text;

			// Lấy lại dữ liệu thô từ Tag (đã lưu ở hàm Load)
			String^ currDC = (lblInfoFullAddress->Tag != nullptr) ? lblInfoFullAddress->Tag->ToString() : "";
			String^ currQuan = (lblInfoPhone->Tag != nullptr) ? lblInfoPhone->Tag->ToString() : "";
			String^ currTinh = (lblInfoName->Tag != nullptr) ? lblInfoName->Tag->ToString() : "";

			// Mở Form cập nhật
			UpdateInfoForm^ frm = gcnew UpdateInfoForm(this->MaSieuThiHienTai, currTen, currSDT, currDC, currQuan, currTinh);

			if (frm->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
				// Nếu bấm "Hoàn thành", thực hiện Update SQL
				UpdateToSQL(frm->NewTen, frm->NewSDT, frm->NewDiaChi, frm->NewQuan, frm->NewTinh);

				// Load lại giao diện chính
				LoadThongTinCaNhan();
				lblUserInfo->Text = frm->NewTen + L"\n(Siêu Thị)"; // Update toolbar
				MessageBox::Show(L"Cập nhật thành công!", L"Thông báo");
			}
		}

		// Hàm phụ tách riêng để Update SQL
		void UpdateToSQL(String^ ten, String^ sdt, String^ dc, String^ quan, String^ tinh) {
			SQLHENV hEnv = SQL_NULL_HENV; SQLHDBC hDbc = SQL_NULL_HDBC; SQLHSTMT hStmt = SQL_NULL_HSTMT;
			SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
			SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
			SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
			SQLWCHAR* connStr = (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;";

			if (SQL_SUCCEEDED(SQLDriverConnect(hDbc, NULL, connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT))) {
				if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
					std::wstring sql = L"UPDATE SieuThi SET TenSieuThi=?, SDT=?, DiaChi=?, QuanHuyen=?, Tinh=? WHERE MaSieuThi=?";
					SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);

					pin_ptr<const wchar_t> pTen = PtrToStringChars(ten);
					pin_ptr<const wchar_t> pDC = PtrToStringChars(dc);
					pin_ptr<const wchar_t> pQuan = PtrToStringChars(quan);
					pin_ptr<const wchar_t> pTinh = PtrToStringChars(tinh);

					std::string sPhone = ToStringStandard(sdt);
					SQLCHAR cPhone[21]; strcpy_s((char*)cPhone, 21, sPhone.c_str());

					std::string sID = ToStringStandard(this->MaSieuThiHienTai);
					SQLCHAR cID[51]; strcpy_s((char*)cID, 51, sID.c_str());

					SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 255, 0, (SQLPOINTER)pTen, 0, NULL);
					SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 20, 0, cPhone, 0, NULL);
					SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 255, 0, (SQLPOINTER)pDC, 0, NULL);
					SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)pQuan, 0, NULL);
					SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)pTinh, 0, NULL);
					SQLBindParameter(hStmt, 6, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cID, 0, NULL);

					SQLExecute(hStmt);
				}
			}
			if (hStmt) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
			if (hDbc) { SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); }
			if (hEnv) SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
		}

		// --- KHAI BÁO BIẾN TOÀN CỤC CHO PHÂN TRANG SẢN PHẨM (Thêm vào đầu class SupermarketForm) ---
		int productPage = 1;
		int productPageSize = 20; // Chỉ tải 20 sản phẩm mỗi trang để cực nhanh
		int totalProductPages = 1;

		// --- HÀM LOAD TỐI ƯU MỚI ---
		void LoadDanhSachSanPham() {
			// 1. Dọn dẹp giao diện trước
			flowPnlProducts->Controls->Clear();
			flowPnlProducts->SuspendLayout(); // Tạm dừng vẽ để tăng tốc

			SQLHENV hEnv = SQL_NULL_HENV; SQLHDBC hDbc = SQL_NULL_HDBC; SQLHSTMT hStmt = SQL_NULL_HSTMT;

			// Kết nối DB
			if (!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv))) return;
			SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
			if (!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc))) { SQLFreeHandle(SQL_HANDLE_ENV, hEnv); return; }

			SQLWCHAR* connStr = (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;";
			SQLDriverConnect(hDbc, NULL, connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

			if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {

				std::wstring sql =
					L"SELECT S.MaSanPham, S.TenSanPham, S.Loai, S.NhietDoThichHop, S.GiaBan, "
					L"       ISNULL(NCC_Info.TenNCC, N'Chưa xác định') AS TenNCC, "
					L"       ISNULL(DG.GiaBanDeXuat, S.GiaBan) AS GiaHienTai, "
					L"       COUNT(*) OVER() AS TotalCount " // Đếm tổng số lượng để tính phân trang
					L"FROM SanPham S "
					// JOIN 1: Lấy Tên NCC từ lần nhập hàng gần nhất
					L"OUTER APPLY ( "
					L"    SELECT TOP 1 N.TenNCC "
					L"    FROM ChiTietLoHang C "
					L"    JOIN LoHang L ON C.MaLoHang = L.MaLoHang "
					L"    JOIN NhaCungCap N ON L.MaNCC = N.MaNCC "
					L"    WHERE C.MaSanPham = S.MaSanPham "
					L"    ORDER BY L.NgayNhap DESC "
					L") NCC_Info "
					// JOIN 2: Lấy Giá định giá động mới nhất
					L"OUTER APPLY ( "
					L"    SELECT TOP 1 D.GiaBanDeXuat "
					L"    FROM DinhGiaDong D "
					L"    WHERE D.MaSanPham = S.MaSanPham "
					L"    ORDER BY D.NgayTinh DESC "
					L") DG "
					// Phân trang
					L"ORDER BY S.TenSanPham "
					L"OFFSET ? ROWS FETCH NEXT ? ROWS ONLY";

				if (SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS) == SQL_SUCCESS) {
					// Tính toán vị trí bắt đầu
					int offsetVal = (productPage - 1) * productPageSize;

					SQLINTEGER nativeOffset = (SQLINTEGER)offsetVal;
					SQLINTEGER nativeLimit = (SQLINTEGER)productPageSize;
					SQLLEN cbParam = 0; 
					SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &nativeOffset, 0, &cbParam);
					SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &nativeLimit, 0, &cbParam);

					if (SQLExecute(hStmt) == SQL_SUCCESS) {
						SQLCHAR sMa[51];
						SQLWCHAR sTen[256], sLoai[101], sTenNCC[256];
						SQLDOUBLE dNhietDo, dGiaGoc, dGiaHienTai;
						SQLLEN len, totalCountLen;
						SQLINTEGER totalCount = 0;

						while (SQLFetch(hStmt) == SQL_SUCCESS) {
							ProductInfo^ pInfo = gcnew ProductInfo();

							SQLGetData(hStmt, 1, SQL_C_CHAR, sMa, 51, &len);
							pInfo->MaSP = ToSysString(StdTrim((char*)sMa));

							SQLGetData(hStmt, 2, SQL_C_WCHAR, sTen, 256, &len);
							pInfo->TenSP = gcnew String(sTen);

							SQLGetData(hStmt, 3, SQL_C_WCHAR, sLoai, 101, &len);
							pInfo->Loai = gcnew String(sLoai);

							SQLGetData(hStmt, 4, SQL_C_DOUBLE, &dNhietDo, 0, &len);
							pInfo->NhietDo = (float)dNhietDo;

							SQLGetData(hStmt, 5, SQL_C_DOUBLE, &dGiaGoc, 0, &len);
							pInfo->GiaGoc = dGiaGoc;

							SQLGetData(hStmt, 6, SQL_C_WCHAR, sTenNCC, 256, &len);
							pInfo->TenNCC = gcnew String(sTenNCC);

							SQLGetData(hStmt, 7, SQL_C_DOUBLE, &dGiaHienTai, 0, &len);
							pInfo->GiaHienTai = (dGiaHienTai <= 1000) ? dGiaGoc : dGiaHienTai;

							// Lấy tổng số dòng để tính số trang
							SQLGetData(hStmt, 8, SQL_C_LONG, &totalCount, 0, &totalCountLen);

							Panel^ card = CreateProductCard(pInfo);
							flowPnlProducts->Controls->Add(card);
						}

						// Cập nhật lại tổng số trang
						if (totalCount > 0) {
							totalProductPages = (int)ceil((double)totalCount / productPageSize);
						}
						else {
							totalProductPages = 1;
						}
					}
				}
			}

			// Dọn dẹp
			if (hStmt) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
			if (hDbc) { SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); }
			if (hEnv) SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

			flowPnlProducts->ResumeLayout(); // Vẽ lại giao diện 1 lần
			UpdateProductPaginationUI();
		}
		// Hàm tiện ích: Hiển thị lỗi SQL chi tiết
		void ShowSQLError(SQLHANDLE hStmt) {
			SQLCHAR sqlState[6];
			SQLCHAR msg[SQL_MAX_MESSAGE_LENGTH];
			SQLINTEGER nativeError;
			SQLSMALLINT msgLen;

			// Lấy thông tin lỗi từ Driver
			if (SQLGetDiagRecA(SQL_HANDLE_STMT, hStmt, 1, sqlState, &nativeError, msg, sizeof(msg), &msgLen) == SQL_SUCCESS) {
				String^ errMsg = gcnew String((char*)msg);
				MessageBox::Show(errMsg, L"Chi tiết lỗi SQL", MessageBoxButtons::OK, MessageBoxIcon::Error);
			}
		}

		void LoadLichSuDonHang() {
			// 1. Kiểm tra
			if (String::IsNullOrEmpty(this->MaSieuThiHienTai)) return;

			// 2. Kết nối
			SQLHENV hEnv = SQL_NULL_HENV; SQLHDBC hDbc = SQL_NULL_HDBC;
			SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
			SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
			SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);

			SQLWCHAR* connStr = (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;";
			SQLDriverConnect(hDbc, NULL, connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

			// 3. Reset Grid
			if (dgvOrders != nullptr) dgvOrders->Rows->Clear();
			if (originalOrderList == nullptr) originalOrderList = gcnew List<OrderHeader^>();
			originalOrderList->Clear();

			SQLHSTMT hStmt;
			if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {

				// --- CÂU TRUY VẤN MỚI (LOẠI BỎ BANG VANCHUYEN) ---
				// Chỉ lấy PhiShip từ PhanBoNguonHang
				std::wstring sql =
					L"SELECT Y.MaYeuCau, Y.NgayYeuCau, Y.GiaTriTong, Y.HoanThanh, "
					L"ISNULL(P.ChiPhiVanChuyen, 0) AS PhiShip "
					L"FROM YeuCau Y "
					L"LEFT JOIN PhanBoNguonHang P ON LTRIM(RTRIM(Y.MaYeuCau)) = LTRIM(RTRIM(P.MaYeuCau)) "
					L"WHERE LTRIM(RTRIM(Y.MaSieuThi)) = ? "
					L"ORDER BY Y.NgayYeuCau DESC";

				SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);

				std::string sMaST = StdTrim(ToStdString(this->MaSieuThiHienTai));
				SQLCHAR cMaST[51]; strcpy_s((char*)cMaST, 51, sMaST.c_str());
				SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMaST, 0, NULL);

				if (SQLExecute(hStmt) == SQL_SUCCESS) {
					SQLCHAR sMa[51];
					SQL_DATE_STRUCT dNgay;
					SQLDOUBLE dHang, dShip;
					SQLWCHAR sTT[51];
					SQLLEN len;

					while (SQLFetch(hStmt) == SQL_SUCCESS) {
						SQLGetData(hStmt, 1, SQL_C_CHAR, sMa, 51, &len);
						SQLGetData(hStmt, 2, SQL_C_TYPE_DATE, &dNgay, 0, &len);
						SQLGetData(hStmt, 3, SQL_C_DOUBLE, &dHang, 0, &len);
						SQLGetData(hStmt, 4, SQL_C_WCHAR, sTT, 51, &len);
						SQLGetData(hStmt, 5, SQL_C_DOUBLE, &dShip, 0, &len);

						OrderHeader^ h = gcnew OrderHeader();
						h->MaDon = ToSysString(StdTrim((char*)sMa));

						if (len != SQL_NULL_DATA) h->NgayDat = DateTime(dNgay.year, dNgay.month, dNgay.day);
						else h->NgayDat = DateTime::Now;

						h->TongGiaTriHang = dHang;
						h->PhiVanChuyen = (len == SQL_NULL_DATA) ? 0 : dShip;
						h->TongThanhToan = dHang + h->PhiVanChuyen;

						String^ rawStatus = gcnew String((wchar_t*)sTT);
						rawStatus = rawStatus->Trim();
						if (rawStatus == "C") h->TrangThai = L"Chờ xác nhận";
						else if (rawStatus == "S") h->TrangThai = L"Đang vận chuyển";
						else if (rawStatus == "R") h->TrangThai = L"Hoàn thành";
						else h->TrangThai = rawStatus;

						originalOrderList->Add(h);
					}
				}
				SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
			}

			SQLDisconnect(hDbc);
			SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
			SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

			// Reset bộ lọc để chắc chắn hiển thị
			if (cboOrderStatus != nullptr) cboOrderStatus->SelectedIndex = 0;
			ApplyOrderFilters();
		}
		// --- HÀM TẠO NÚT PHÂN TRANG CHO TAB SẢN PHẨM (Gọi trong InitializeComponent hoặc SetupToolbar) ---
		void SetupProductPagination() {
			Panel^ pnlPaging = gcnew Panel();
			pnlPaging->Dock = DockStyle::Bottom;
			pnlPaging->Height = 50;
			pnlPaging->BackColor = Color::WhiteSmoke;

			Button^ btnPrev = gcnew Button();
			btnPrev->Text = L"<";
			btnPrev->Size = System::Drawing::Size(40, 30);
			btnPrev->Location = System::Drawing::Point(this->Width / 2 - 60, 10);
			btnPrev->Click += gcnew EventHandler(this, &SupermarketForm::OnProductPrevClick);

			Label^ lblPage = gcnew Label();
			lblPage->Name = "lblProductPage";
			lblPage->Text = "1";
			lblPage->AutoSize = true;
			lblPage->Location = System::Drawing::Point(this->Width / 2, 18);
			lblPage->Font = gcnew System::Drawing::Font("Segoe UI", 10, FontStyle::Bold);

			Button^ btnNext = gcnew Button();
			btnNext->Text = L">";
			btnNext->Size = System::Drawing::Size(40, 30);
			btnNext->Location = System::Drawing::Point(this->Width / 2 + 40, 10);
			btnNext->Click += gcnew EventHandler(this, &SupermarketForm::OnProductNextClick);

			pnlPaging->Controls->Add(btnPrev);
			pnlPaging->Controls->Add(lblPage);
			pnlPaging->Controls->Add(btnNext);

			// Thêm vào Tab Sản Phẩm
			this->tabGeneral->Controls->Add(pnlPaging);
			pnlPaging->BringToFront();
		}

		void UpdateProductPaginationUI() {
			// Tìm label trang để cập nhật text
			for each (Control ^ c in this->tabGeneral->Controls) {
				if (Panel^ p = dynamic_cast<Panel^>(c)) {
					if (p->Dock == DockStyle::Bottom) {
						for each (Control ^ child in p->Controls) {
							if (child->Name == "lblProductPage") {
								child->Text = productPage + " / " + totalProductPages;
								// Căn giữa lại
								child->Location = System::Drawing::Point(p->Width / 2 - (child->Width / 2), 18);
								return;
							}
						}
					}
				}
			}
		}

		void OnProductPrevClick(Object^ sender, EventArgs^ e) {
			if (productPage > 1) {
				productPage--;
				LoadDanhSachSanPham();
			}
		}

		void OnProductNextClick(Object^ sender, EventArgs^ e) {
			if (productPage < totalProductPages) {
				productPage++;
				LoadDanhSachSanPham();
			}
		}
		// Cập nhật hàm ApplyOrderFilters để thêm màu sắc
		Panel^ CreateProductCard(ProductInfo^ info) {
			int cardW = 200; int cardH = 320;
			Panel^ pnl = gcnew Panel();
			pnl->Size = System::Drawing::Size(cardW, cardH);
			pnl->BackColor = Color::FromArgb(225, 245, 255);
			pnl->Margin = System::Windows::Forms::Padding(5);
			pnl->Cursor = Cursors::Hand;
			pnl->Tag = info;
			pnl->Click += gcnew System::EventHandler(this, &SupermarketForm::OnCardClick);

			int picW = 180; int picH = 150; int picX = (cardW - picW) / 2;
			PictureBox^ pic = gcnew PictureBox();
			pic->Size = System::Drawing::Size(picW, picH);
			pic->Location = System::Drawing::Point(picX, 10);
			pic->BackColor = Color::White;
			pic->Tag = info;
			pic->SizeMode = PictureBoxSizeMode::Zoom;
			pic->Click += gcnew System::EventHandler(this, &SupermarketForm::OnCardClick);

			if (imageMap->ContainsKey(info->MaSP)) {
				try { pic->ImageLocation = imageMap[info->MaSP]; }
				catch (...) { pic->Paint += gcnew PaintEventHandler(this, &SupermarketForm::DrawPlaceholderImage); }
			}
			else { pic->Paint += gcnew PaintEventHandler(this, &SupermarketForm::DrawPlaceholderImage); }
			pnl->Controls->Add(pic);

			Label^ lblName = gcnew Label();
			lblName->Text = info->TenSP;
			lblName->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11, FontStyle::Bold));
			lblName->ForeColor = Color::FromArgb(40, 40, 40);
			lblName->Location = System::Drawing::Point(5, 170); // Ngay dưới ảnh
			lblName->Size = System::Drawing::Size(cardW - 10, 45); // Cao 2 dòng
			lblName->TextAlign = ContentAlignment::TopCenter; // Căn giữa
			lblName->Tag = info;
			lblName->Click += gcnew System::EventHandler(this, &SupermarketForm::OnCardClick);
			pnl->Controls->Add(lblName);

			int yPriceArea = 220;
			double giaHienTai = info->GiaHienTai / 1000.0;
			double giaGoc = info->GiaGoc / 1000.0;
			bool isDiscount = (giaGoc - giaHienTai) > 1.0;

			if (isDiscount) {
				String^ txtOld = giaGoc.ToString("N0") + L" đ";
				System::Drawing::Font^ fontOld = gcnew System::Drawing::Font(L"Segoe UI", 10, FontStyle::Strikeout);
				int wOld = TextRenderer::MeasureText(txtOld, fontOld).Width;
				Label^ lblOldPrice = gcnew Label();
				lblOldPrice->Text = txtOld; lblOldPrice->Font = fontOld; lblOldPrice->ForeColor = Color::Gray;
				lblOldPrice->AutoSize = true;
				lblOldPrice->Location = System::Drawing::Point((cardW - wOld) / 2, yPriceArea);

				String^ txtNew = giaHienTai.ToString("N0") + L" đ";
				System::Drawing::Font^ fontNew = gcnew System::Drawing::Font(L"Segoe UI", 14, FontStyle::Regular);
				int wNew = TextRenderer::MeasureText(txtNew, fontNew).Width;

				double phanTram = ((giaGoc - giaHienTai) / giaGoc) * 100.0;
				String^ txtTag = "-" + Math::Round(phanTram).ToString() + "%";
				System::Drawing::Font^ fontTag = gcnew System::Drawing::Font(L"Segoe UI", 9, FontStyle::Bold);

				int gap = 5; int wTag = TextRenderer::MeasureText(txtTag, fontTag).Width + 4;
				int startX = (cardW - (wNew + gap + wTag)) / 2;

				Label^ lblNewPrice = gcnew Label();
				lblNewPrice->Text = txtNew; lblNewPrice->Font = fontNew; lblNewPrice->ForeColor = Color::Red;
				lblNewPrice->AutoSize = true; lblNewPrice->Location = System::Drawing::Point(startX, yPriceArea + 20);

				Label^ lblSaleTag = gcnew Label();
				lblSaleTag->Text = txtTag; lblSaleTag->Font = fontTag; lblSaleTag->ForeColor = Color::White;
				lblSaleTag->BackColor = Color::Red; lblSaleTag->AutoSize = true;
				lblSaleTag->Location = System::Drawing::Point(startX + wNew + gap, yPriceArea + 24);

				lblOldPrice->Tag = info; lblNewPrice->Tag = info;
				lblOldPrice->Click += gcnew EventHandler(this, &SupermarketForm::OnCardClick);
				lblNewPrice->Click += gcnew EventHandler(this, &SupermarketForm::OnCardClick);

				pnl->Controls->Add(lblOldPrice); pnl->Controls->Add(lblNewPrice); pnl->Controls->Add(lblSaleTag);
			}
			else {
				String^ txtPrice = giaHienTai.ToString("N0") + L" đ/Kg";
				System::Drawing::Font^ fontPrice = gcnew System::Drawing::Font(L"Segoe UI", 14, FontStyle::Regular);
				int wPrice = TextRenderer::MeasureText(txtPrice, fontPrice).Width;
				Label^ lblPrice = gcnew Label();
				lblPrice->Text = txtPrice; lblPrice->Font = fontPrice; lblPrice->ForeColor = Color::Red;
				lblPrice->AutoSize = true;
				lblPrice->Location = System::Drawing::Point((cardW - wPrice) / 2, yPriceArea + 15);

				lblPrice->Tag = info; lblPrice->Click += gcnew EventHandler(this, &SupermarketForm::OnCardClick);
				pnl->Controls->Add(lblPrice);
			}

			Label^ lblNCC = gcnew Label();
			String^ tenHienThi = String::IsNullOrEmpty(info->TenNCC) ? "" : info->TenNCC;
			lblNCC->Text = L"🏭 " + tenHienThi;
			lblNCC->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9, FontStyle::Italic));
			lblNCC->ForeColor = Color::Gray;
			lblNCC->Location = System::Drawing::Point(5, 295);
			lblNCC->Size = System::Drawing::Size(cardW - 10, 20);
			lblNCC->TextAlign = ContentAlignment::MiddleRight;
			lblNCC->Tag = info;
			lblNCC->Click += gcnew System::EventHandler(this, &SupermarketForm::OnCardClick);
			pnl->Controls->Add(lblNCC);

			return pnl;
		}

		void DrawPlaceholderImage(Object^ sender, PaintEventArgs^ e) {
			PictureBox^ p = (PictureBox^)sender;
			ProductInfo^ info = (ProductInfo^)p->Tag;
			if (info == nullptr || String::IsNullOrEmpty(info->TenSP)) return;
			int hash = info->TenSP->GetHashCode();
			Random^ rnd = gcnew Random(hash);
			Color bgCol = Color::FromArgb(200 + rnd->Next(55), 200 + rnd->Next(55), 200 + rnd->Next(55));
			p->BackColor = bgCol;

			String^ firstLetter = info->TenSP->Substring(0, 1)->ToUpper();
			System::Drawing::Font^ f = gcnew System::Drawing::Font("Segoe UI", 36, FontStyle::Bold);
			SizeF s = e->Graphics->MeasureString(firstLetter, f);
			e->Graphics->DrawString(firstLetter, f, Brushes::DarkSlateGray, (p->Width - s.Width) / 2, (p->Height - s.Height) / 2);
		}

		void OnCardClick(Object^ sender, EventArgs^ e) {
			Control^ ctrl = (Control^)sender;
			ProductInfo^ info = (ProductInfo^)ctrl->Tag;
			String^ linkAnh = "";
			if (imageMap->ContainsKey(info->MaSP)) {
				linkAnh = imageMap[info->MaSP];
			}
			ProductDetailForm^ detailForm = gcnew ProductDetailForm(info, linkAnh);
			detailForm->ShowDialog();
			if (detailForm->IsAddedToCart) {
				ShoppingCart->Add(detailForm->SelectedItem);
				btnCart->Text = L"🛒 Giỏ Hàng (" + ShoppingCart->Count + ")";
			}
		}
	};
}