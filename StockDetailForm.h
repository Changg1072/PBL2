#pragma once
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <string>
#include <iostream>

namespace PBL2QuanLyKho {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::IO;

	public ref class StockDetailForm : public System::Windows::Forms::Form
	{
	public:
		// Constructor nhận các thông tin từ Grid (Kho) + MaSP để tra cứu thêm
		StockDetailForm(String^ maSP, String^ tenSP_Grid, String^ maLo, DateTime hsd, int sl, double gia, String^ imagePath)
		{
			InitializeComponent();

			// 1. Load các thông tin cơ bản từ lưới (WarehouseForm truyền sang)
			// Đây là thông tin cụ thể của Lô hàng này
			LoadBatchData(maSP, maLo, hsd, sl, gia);

			// 2. Load ảnh (nếu có)
			LoadImage(imagePath);

			// 3. QUAN TRỌNG: Kết nối SQL để lấy thông tin gốc từ bảng SanPham
			GetProductInfoFromDB(maSP);
		}

	protected:
		~StockDetailForm() {
			if (components) delete components;
		}

	private:
		System::Windows::Forms::PictureBox^ pictureBox1;
		System::Windows::Forms::Label^ lblTenSP;
		System::Windows::Forms::Label^ lblChiTiet; // Thông tin lô hàng (Kho)
		System::Windows::Forms::Label^ lblThongTinGoc; // Thông tin gốc (SQL SanPham)
		System::Windows::Forms::Button^ btnClose;
		System::ComponentModel::Container^ components;

		void InitializeComponent(void)
		{
			this->pictureBox1 = (gcnew System::Windows::Forms::PictureBox());
			this->lblTenSP = (gcnew System::Windows::Forms::Label());
			this->lblChiTiet = (gcnew System::Windows::Forms::Label());
			this->lblThongTinGoc = (gcnew System::Windows::Forms::Label()); // Label mới
			this->btnClose = (gcnew System::Windows::Forms::Button());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox1))->BeginInit();
			this->SuspendLayout();

			// Form setup
			this->AutoScaleDimensions = System::Drawing::SizeF(8, 16);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(700, 500); // Tăng kích thước
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"Chi tiết sản phẩm & Lô hàng";
			this->BackColor = System::Drawing::Color::White;

			// PictureBox
			this->pictureBox1->Location = System::Drawing::Point(20, 20);
			this->pictureBox1->Size = System::Drawing::Size(220, 220);
			this->pictureBox1->SizeMode = System::Windows::Forms::PictureBoxSizeMode::Zoom;
			this->pictureBox1->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;

			// Label Tên SP
			this->lblTenSP->Location = System::Drawing::Point(260, 20);
			this->lblTenSP->Size = System::Drawing::Size(400, 50);
			this->lblTenSP->Font = (gcnew System::Drawing::Font(L"Segoe UI", 16, System::Drawing::FontStyle::Bold));
			this->lblTenSP->ForeColor = System::Drawing::Color::Navy;
			this->lblTenSP->Text = L"Đang tải tên...";

			// Label Thông tin gốc (Từ SQL SanPham)
			this->lblThongTinGoc->Location = System::Drawing::Point(260, 80);
			this->lblThongTinGoc->Size = System::Drawing::Size(400, 150);
			this->lblThongTinGoc->AutoSize = false;
			this->lblThongTinGoc->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11, System::Drawing::FontStyle::Regular));
			this->lblThongTinGoc->ForeColor = System::Drawing::Color::Black;
			this->lblThongTinGoc->Text = L"Đang tra cứu dữ liệu sản phẩm...";

			// Label Chi tiết Lô hàng (Từ Kho)
			this->lblChiTiet->Location = System::Drawing::Point(260, 240);
			this->lblChiTiet->Size = System::Drawing::Size(400, 150);
			this->lblChiTiet->AutoSize = false;
			this->lblChiTiet->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11, System::Drawing::FontStyle::Italic));
			this->lblChiTiet->ForeColor = System::Drawing::Color::DimGray;
			this->lblChiTiet->Text = L"...";

			// Button Đóng
			this->btnClose->Location = System::Drawing::Point(550, 400);
			this->btnClose->Size = System::Drawing::Size(120, 40);
			this->btnClose->Text = L"Đóng";
			this->btnClose->BackColor = System::Drawing::Color::FromArgb(238, 77, 45);
			this->btnClose->ForeColor = System::Drawing::Color::White;
			this->btnClose->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->btnClose->Click += gcnew System::EventHandler(this, &StockDetailForm::OnCloseClick);

			this->Controls->Add(this->btnClose);
			this->Controls->Add(this->lblThongTinGoc);
			this->Controls->Add(this->lblChiTiet);
			this->Controls->Add(this->lblTenSP);
			this->Controls->Add(this->pictureBox1);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox1))->EndInit();
			this->ResumeLayout(false);
		}

		void OnCloseClick(System::Object^ sender, System::EventArgs^ e) {
			this->Close();
		}

		// Hàm 1: Hiển thị dữ liệu Lô hàng (Lấy từ tham số truyền vào)
		void LoadBatchData(String^ maSP, String^ maLo, DateTime hsd, int sl, double gia) {
			this->lblChiTiet->Text =
				L"--- THÔNG TIN LÔ HÀNG TRONG KHO ---\n" +
				L"Mã Lô: " + maLo + L"\n" +
				L"Hạn sử dụng: " + hsd.ToString("dd/MM/yyyy") + L"\n" +
				L"Số lượng tồn: " + sl.ToString("N0") + L" (Tấn)\n" +
				L"Giá trị lô hàng: " + gia.ToString("N0") + L" VNĐ";
		}

		// Hàm 2: Hiển thị ảnh
		void LoadImage(String^ imagePath) {
			if (!String::IsNullOrEmpty(imagePath) && File::Exists(imagePath)) {
				try {
					// Dùng FileStream để không bị khóa file ảnh
					FileStream^ fs = gcnew FileStream(imagePath, FileMode::Open, FileAccess::Read);
					this->pictureBox1->Image = System::Drawing::Image::FromStream(fs);
					fs->Close();
				}
				catch (...) {}
			}
		}

		// Hàm 3: Tra cứu SQL lấy cả Giá Gốc và Giá Động từ bảng DinhGiaDong
		void GetProductInfoFromDB(String^ maSP) {
			SQLHENV hEnv = SQL_NULL_HENV;
			SQLHDBC hDbc = SQL_NULL_HDBC;
			SQLHSTMT hStmt = SQL_NULL_HSTMT;

			if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) == SQL_SUCCESS) {
				SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
				if (SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc) == SQL_SUCCESS) {
					SQLWCHAR* connStr = (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;";
					SQLDriverConnect(hDbc, NULL, connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

					if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {

						// --- CÂU SQL MỚI: LẤY 2 CỘT GIÁ RIÊNG BIỆT ---
						// Cột 4: S.GiaBan (Giá gốc niêm yết)
						// Cột 5: Sub-query lấy giá mới nhất từ DinhGiaDong
						std::wstring sql =
							L"SELECT S.TenSanPham, S.Loai, S.NhietDoThichHop, "
							L"       S.GiaBan, "
							L"       (SELECT TOP 1 D.GiaBanDeXuat FROM DinhGiaDong D WHERE D.MaSanPham = S.MaSanPham ORDER BY D.NgayTinh DESC), "
							L"       S.NhomPhanLoai "
							L"FROM SanPham S "
							L"WHERE S.MaSanPham = ?";

						SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);

						// Chuyển đổi mã SP an toàn
						std::string sMa = "";
						if (!String::IsNullOrEmpty(maSP)) {
							cli::array<unsigned char>^ bytes = System::Text::Encoding::UTF8->GetBytes(maSP);
							pin_ptr<unsigned char> p = &bytes[0];
							sMa = std::string((char*)p, bytes->Length);
						}

						SQLCHAR cMa[51]; strcpy_s((char*)cMa, 51, sMa.c_str());
						SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMa, 0, NULL);

						if (SQLExecute(hStmt) == SQL_SUCCESS) {
							if (SQLFetch(hStmt) == SQL_SUCCESS) {
								SQLWCHAR wTen[256], wLoai[101], wNhom[51];
								SQLDOUBLE dNhietDo, dGiaGoc, dGiaDong;
								SQLLEN len;

								// 1. Tên
								SQLGetData(hStmt, 1, SQL_C_WCHAR, wTen, 256, &len);
								String^ dbTen = (len != SQL_NULL_DATA) ? gcnew String(wTen) : L"Không xác định";

								// 2. Loại
								SQLGetData(hStmt, 2, SQL_C_WCHAR, wLoai, 101, &len);
								String^ dbLoai = (len != SQL_NULL_DATA) ? gcnew String(wLoai) : L"";

								// 3. Nhiệt độ
								SQLGetData(hStmt, 3, SQL_C_DOUBLE, &dNhietDo, 0, &len);
								double dbNhiet = (len != SQL_NULL_DATA) ? dNhietDo : 0;

								// 4. Giá Niêm Yết (Giá Gốc từ bảng SanPham)
								SQLGetData(hStmt, 4, SQL_C_DOUBLE, &dGiaGoc, 0, &len);
								double dbGiaNiemYet = (len != SQL_NULL_DATA) ? dGiaGoc : 0;

								// 5. Giá Động (Giá từ bảng DinhGiaDong)
								SQLGetData(hStmt, 5, SQL_C_DOUBLE, &dGiaDong, 0, &len);

								// LOGIC QUAN TRỌNG:
								// Nếu tìm thấy giá động (len != NULL) -> Giá hiện tại = Giá động
								// Nếu chưa có giá động (len == NULL) -> Giá hiện tại = Giá niêm yết
								double dbGiaHienTai = (len != SQL_NULL_DATA) ? dGiaDong : dbGiaNiemYet;

								// 6. Nhóm
								SQLGetData(hStmt, 6, SQL_C_WCHAR, wNhom, 51, &len);
								String^ dbNhom = (len != SQL_NULL_DATA) ? gcnew String(wNhom) : L"C";

								// --- CẬP NHẬT GIAO DIỆN ---
								this->lblTenSP->Text = dbTen;

								// Format tiền tệ
								String^ txtGiaGoc = dbGiaNiemYet.ToString("N0") + L" đ";
								String^ txtGiaBan = dbGiaHienTai.ToString("N0") + L" đ";

								// Tạo chuỗi trạng thái giá (So sánh giá động vs giá gốc)
								String^ statusGia = L"";
								if (dbGiaHienTai < dbGiaNiemYet) {
									// Giá động thấp hơn -> Đang giảm giá
									statusGia = L" (📉 Đang giảm giá)";
								}
								else if (dbGiaHienTai > dbGiaNiemYet) {
									// Giá động cao hơn -> Đang tăng giá (giờ cao điểm/khan hiếm)
									statusGia = L" (📈 Giá tăng theo cầu)";
								}
								else {
									statusGia = L" (Bằng giá niêm yết)";
								}

								// Hiển thị đầy đủ thông tin
								this->lblThongTinGoc->Text =
									L"--- THÔNG TIN SẢN PHẨM ---\n" +
									L"Mã SP: " + maSP + L"\n" +
									L"Loại: " + dbLoai + L"\n" +
									L"Nhiệt độ: " + dbNhiet.ToString("N1") + L" °C\n" +
									L"Giá niêm yết: " + txtGiaGoc + L"\n" +  // Dòng hiển thị giá gốc
									L"Giá bán hiện tại: " + txtGiaBan + statusGia + L"\n" + // Dòng hiển thị giá động
									L"Phân loại ABC: " + dbNhom;
							}
						}
					}
					SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
					SQLDisconnect(hDbc);
				}
				SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
				SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
			}
		}
	};
}