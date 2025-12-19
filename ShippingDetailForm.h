#pragma once
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <msclr/marshal_cppstd.h>
#include "DataModels.h" 

namespace PBL2QuanLyKho {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	// [ĐỔI TÊN CLASS] Đổi từ OrderDetailForm -> ShippingDetailForm
	public ref class ShippingDetailForm : public System::Windows::Forms::Form
	{
	public:
		String^ MaDonHang;

		// Constructor cũng phải đổi tên
		ShippingDetailForm(String^ maDon) {
			this->MaDonHang = maDon;
			InitializeComponent();
			LoadData();
		}

	private:
		DataGridView^ dgvDetails;
		Label^ lblTitle;
		Button^ btnClose;
		Label^ lblTongTien;

		void InitializeComponent() {
			this->Text = L"Chi Tiết Đơn Vận Chuyển: " + MaDonHang;
			this->Size = System::Drawing::Size(900, 600);
			this->StartPosition = FormStartPosition::CenterParent;
			this->BackColor = Color::White;
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			this->MaximizeBox = false;

			// 1. Tiêu đề
			lblTitle = gcnew Label();
			lblTitle->Text = L"📦 CHI TIẾT ĐƠN: " + MaDonHang;
			lblTitle->Dock = DockStyle::Top;
			lblTitle->Height = 70;
			lblTitle->TextAlign = ContentAlignment::MiddleCenter;
			lblTitle->Font = gcnew System::Drawing::Font(L"Segoe UI", 16, FontStyle::Bold);
			lblTitle->ForeColor = Color::FromArgb(23, 42, 69);
			lblTitle->BackColor = Color::WhiteSmoke;

			// 2. Panel Đáy
			Panel^ pnlBottom = gcnew Panel();
			pnlBottom->Dock = DockStyle::Bottom;
			pnlBottom->Height = 60;
			pnlBottom->BackColor = Color::WhiteSmoke;

			btnClose = gcnew Button();
			btnClose->Text = L"Đóng";
			btnClose->Location = Point(750, 10);
			btnClose->Size = System::Drawing::Size(100, 40);
			btnClose->BackColor = Color::DimGray;
			btnClose->ForeColor = Color::White;
			btnClose->FlatStyle = FlatStyle::Flat;
			btnClose->Click += gcnew EventHandler(this, &ShippingDetailForm::OnCloseClick);

			lblTongTien = gcnew Label();
			lblTongTien->Text = L"Đang tải...";
			lblTongTien->AutoSize = true;
			lblTongTien->Location = Point(20, 20);
			lblTongTien->Font = gcnew System::Drawing::Font(L"Segoe UI", 12, FontStyle::Bold);
			lblTongTien->ForeColor = Color::DarkRed;

			pnlBottom->Controls->Add(lblTongTien);
			pnlBottom->Controls->Add(btnClose);

			// 3. Bảng dữ liệu
			dgvDetails = gcnew DataGridView();
			dgvDetails->Dock = DockStyle::Fill;
			dgvDetails->BackgroundColor = Color::White;
			dgvDetails->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;
			dgvDetails->ReadOnly = true;
			dgvDetails->RowTemplate->Height = 40;
			dgvDetails->AllowUserToAddRows = false;
			dgvDetails->ColumnHeadersHeight = 40;
			dgvDetails->EnableHeadersVisualStyles = false;

			dgvDetails->ColumnHeadersDefaultCellStyle->BackColor = Color::FromArgb(0, 114, 188);
			dgvDetails->ColumnHeadersDefaultCellStyle->ForeColor = Color::White;
			dgvDetails->ColumnHeadersDefaultCellStyle->Font = gcnew System::Drawing::Font(L"Segoe UI", 10, FontStyle::Bold);

			dgvDetails->Columns->Add("MaSP", L"Mã SP");
			dgvDetails->Columns->Add("TenSP", L"Tên Sản Phẩm");
			dgvDetails->Columns->Add("SL", L"Số Lượng (Tấn)");
			dgvDetails->Columns->Add("DonGia", L"Đơn Giá (VNĐ)");
			dgvDetails->Columns->Add("ThanhTien", L"Thành Tiền");

			dgvDetails->Columns["DonGia"]->DefaultCellStyle->Format = "N0";
			dgvDetails->Columns["ThanhTien"]->DefaultCellStyle->Format = "N0";
			dgvDetails->Columns["SL"]->DefaultCellStyle->Format = "N2";
			dgvDetails->Columns["TenSP"]->Width = 250;

			this->Controls->Add(dgvDetails);
			this->Controls->Add(pnlBottom);
			this->Controls->Add(lblTitle);
		}

		void LoadData() {
			SQLHENV hEnv; SQLHDBC hDbc; SQLHSTMT hStmt;
			if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) != SQL_SUCCESS) return;
			SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
			if (SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc) != SQL_SUCCESS) return;

			SQLWCHAR* connStr = (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;";
			SQLDriverConnect(hDbc, NULL, connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

			if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
				std::wstring sql = L"SELECT c.MaSanPham, s.TenSanPham, c.SoLuong, c.DonGiaVon, c.ThanhTien "
					L"FROM ChiTietDonDatHang c "
					L"JOIN SanPham s ON c.MaSanPham = s.MaSanPham "
					L"WHERE c.MaDonHang = ?";

				SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);
				msclr::interop::marshal_context ctx;
				std::string sMaDon = ctx.marshal_as<std::string>(this->MaDonHang);
				SQLCHAR cMaDon[51];
				strcpy_s((char*)cMaDon, 51, sMaDon.c_str());
				SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMaDon, 0, NULL);

				if (SQLExecute(hStmt) == SQL_SUCCESS) {
					SQLCHAR cMaSP[51]; SQLWCHAR cTenSP[256];
					SQLDOUBLE dSL, dDonGia, dThanhTien;
					double totalVal = 0;

					while (SQLFetch(hStmt) == SQL_SUCCESS) {
						SQLGetData(hStmt, 1, SQL_C_CHAR, cMaSP, 51, NULL);
						SQLGetData(hStmt, 2, SQL_C_WCHAR, cTenSP, 256, NULL);
						SQLGetData(hStmt, 3, SQL_C_DOUBLE, &dSL, 0, NULL);
						SQLGetData(hStmt, 4, SQL_C_DOUBLE, &dDonGia, 0, NULL);
						SQLGetData(hStmt, 5, SQL_C_DOUBLE, &dThanhTien, 0, NULL);

						dgvDetails->Rows->Add(gcnew String((char*)cMaSP), gcnew String(cTenSP), dSL, dDonGia, dThanhTien);
						totalVal += dThanhTien;
					}
					lblTongTien->Text = L"Tổng giá trị đơn: " + totalVal.ToString("N0") + L" VNĐ";
				}
			}
			SQLFreeHandle(SQL_HANDLE_STMT, hStmt); SQLDisconnect(hDbc);
			SQLFreeHandle(SQL_HANDLE_DBC, hDbc); SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
		}

		void OnCloseClick(Object^ sender, EventArgs^ e) { this->Close(); }
	};
}