#pragma once
#pragma comment(lib, "odbc32.lib")
#include "StockDetailForm.h"
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm> // Cho sort
#include <msclr/marshal_cppstd.h>

#include "DataModels.h"
#include "SaoLuu.h" 
#include "RestockAlertForm.h"
#include "ChinhSachTon.h"
#include "TransportTab.h"
#include "TemperatureTab.h"
#include "ImportLogic.h"
#include "ImportGoodsForm.h"
#include "ShippingDetailForm.h"

namespace PBL2QuanLyKho {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections::Generic;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::IO;

	// Struct hỗ trợ hiển thị Tồn kho chi tiết
	public ref struct StockViewModel {
		String^ MaSP;
		String^ TenSP;
		String^ Loai;
		String^ MaLo;
		DateTime HSD;
		double SoLuong;
		double GiaTri;
	};

	// Helper so sánh để sắp xếp
	public ref class StockComparer : System::Collections::Generic::IComparer<StockViewModel^>
	{
		int _mode; // 0: Mặc định (HSD), 1: Tên A-Z, 2: Giá Tăng, 3: Giá Giảm
	public:
		StockComparer(int mode) { _mode = mode; }
		virtual int Compare(StockViewModel^ x, StockViewModel^ y) {
			if (_mode == 1) return String::Compare(x->TenSP, y->TenSP);
			if (_mode == 2) return x->GiaTri.CompareTo(y->GiaTri);
			if (_mode == 3) return y->GiaTri.CompareTo(x->GiaTri);

			// Mặc định (0): Sắp xếp theo Hạn Sử Dụng tăng dần (Gần hết hạn lên đầu)
			return x->HSD.CompareTo(y->HSD);
		}
	};
	public ref class PickDateForm : public System::Windows::Forms::Form {
	public:
		DateTime SelectedDate;
		PickDateForm(String^ tenSP) {
			this->Text = L"Nhập HSD";
			this->Size = System::Drawing::Size(400, 200);
			this->StartPosition = FormStartPosition::CenterScreen;
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			this->MaximizeBox = false; this->MinimizeBox = false;

			Label^ lbl = gcnew Label();
			lbl->Text = L"Hạn sử dụng của: " + tenSP;
			lbl->Location = Point(20, 20);
			lbl->AutoSize = true;
			lbl->Font = gcnew System::Drawing::Font(L"Segoe UI", 10, FontStyle::Bold);

			DateTimePicker^ dtp = gcnew DateTimePicker();
			dtp->Location = Point(20, 60);
			dtp->Width = 340;
			dtp->Format = DateTimePickerFormat::Short;
			dtp->Value = DateTime::Now.AddMonths(6); // Mặc định +6 tháng cho nhanh

			Button^ btnOK = gcnew Button();
			btnOK->Text = L"Xác Nhận";
			btnOK->DialogResult = System::Windows::Forms::DialogResult::OK;
			btnOK->Location = Point(130, 100);
			btnOK->Size = System::Drawing::Size(120, 40);
			btnOK->BackColor = Color::SeaGreen;
			btnOK->ForeColor = Color::White;

			// Sự kiện lấy ngày khi bấm OK
			btnOK->Click += gcnew EventHandler(this, &PickDateForm::OnOK);
			this->Controls->Add(lbl);
			this->Controls->Add(dtp);
			this->Controls->Add(btnOK);

			// Lưu tham chiếu để lấy giá trị sau này
			this->Tag = dtp;
		}
		void OnOK(Object^ sender, EventArgs^ e) {
			DateTimePicker^ dtp = (DateTimePicker^)this->Tag;
			this->SelectedDate = dtp->Value;
			this->Close();
		}
	};
	// ====================================================================
	// FORM POPUP: CẬP NHẬT THÔNG TIN KHO (GIỮ NGUYÊN)
	// ====================================================================
	public ref class UpdateKhoForm : public System::Windows::Forms::Form {
	public:
		String^ NewTen; String^ NewSDT; String^ NewDiaChi; String^ NewQuan; String^ NewTinh;
	private:
		TextBox^ txtTen; TextBox^ txtSDT; TextBox^ txtDiaChi; TextBox^ txtID;
		ComboBox^ cboTinh; ComboBox^ cboQuan;
		Dictionary<String^, List<String^>^>^ dataVN;
	public:
		UpdateKhoForm(String^ ma, String^ ten, String^ sdt, String^ dc, String^ quan, String^ tinh) {
			InitializeComponent();
			dataVN = VietnamData::GetFullData();
			for each (String ^ t in dataVN->Keys) cboTinh->Items->Add(t);
			txtID->Text = ma; txtTen->Text = ten; txtSDT->Text = sdt; txtDiaChi->Text = dc;
			if (cboTinh->Items->Contains(tinh)) {
				cboTinh->SelectedItem = tinh;
				if (cboQuan->Items->Contains(quan)) cboQuan->SelectedItem = quan;
			}
		}
	private:
		void InitializeComponent() {
			this->Text = L"Cập Nhật Thông Tin Kho"; this->Size = System::Drawing::Size(500, 550);
			this->StartPosition = FormStartPosition::CenterParent; this->BackColor = Color::White;
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog; this->MaximizeBox = false;
			System::Drawing::Font^ f = gcnew System::Drawing::Font(L"Segoe UI", 10);
			System::Drawing::Font^ ft = gcnew System::Drawing::Font(L"Segoe UI", 11);

			Label^ l1 = gcnew Label(); l1->Text = L"Mã Kho (Cố định):"; l1->Location = Point(30, 20); l1->AutoSize = true; l1->Font = f; l1->ForeColor = Color::Gray;
			txtID = gcnew TextBox(); txtID->Location = Point(30, 45); txtID->Size = System::Drawing::Size(420, 30); txtID->Font = ft; txtID->ReadOnly = true;

			Label^ l2 = gcnew Label(); l2->Text = L"Tên Kho:"; l2->Location = Point(30, 85); l2->AutoSize = true; l2->Font = f;
			txtTen = gcnew TextBox(); txtTen->Location = Point(30, 110); txtTen->Size = System::Drawing::Size(420, 30); txtTen->Font = ft;

			Label^ l3 = gcnew Label(); l3->Text = L"Số điện thoại:"; l3->Location = Point(30, 150); l3->AutoSize = true; l3->Font = f;
			txtSDT = gcnew TextBox(); txtSDT->Location = Point(30, 175); txtSDT->Size = System::Drawing::Size(420, 30); txtSDT->Font = ft;

			Label^ l4 = gcnew Label(); l4->Text = L"Tỉnh / Thành phố:"; l4->Location = Point(30, 215); l4->AutoSize = true; l4->Font = f;
			cboTinh = gcnew ComboBox(); cboTinh->Location = Point(30, 240); cboTinh->Size = System::Drawing::Size(200, 30); cboTinh->Font = ft; cboTinh->DropDownStyle = ComboBoxStyle::DropDownList;
			cboTinh->SelectedIndexChanged += gcnew EventHandler(this, &UpdateKhoForm::OnTinhChanged);

			Label^ l5 = gcnew Label(); l5->Text = L"Quận / Huyện:"; l5->Location = Point(250, 215); l5->AutoSize = true; l5->Font = f;
			cboQuan = gcnew ComboBox(); cboQuan->Location = Point(250, 240); cboQuan->Size = System::Drawing::Size(200, 30); cboQuan->Font = ft; cboQuan->DropDownStyle = ComboBoxStyle::DropDownList;

			Label^ l6 = gcnew Label(); l6->Text = L"Địa chỉ chi tiết:"; l6->Location = Point(30, 280); l6->AutoSize = true; l6->Font = f;
			txtDiaChi = gcnew TextBox(); txtDiaChi->Location = Point(30, 305); txtDiaChi->Size = System::Drawing::Size(420, 30); txtDiaChi->Font = ft;

			Button^ btnSave = gcnew Button(); btnSave->Text = L"Lưu Thay Đổi"; btnSave->BackColor = Color::FromArgb(0, 114, 188); btnSave->ForeColor = Color::White;
			btnSave->FlatStyle = FlatStyle::Flat; btnSave->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11, FontStyle::Bold));
			btnSave->Size = System::Drawing::Size(200, 45); btnSave->Location = Point(30, 380);
			btnSave->Click += gcnew EventHandler(this, &UpdateKhoForm::OnSaveClick);

			this->Controls->Add(btnSave); this->Controls->Add(txtDiaChi); this->Controls->Add(l6);
			this->Controls->Add(cboQuan); this->Controls->Add(l5); this->Controls->Add(cboTinh); this->Controls->Add(l4);
			this->Controls->Add(txtSDT); this->Controls->Add(l3); this->Controls->Add(txtTen); this->Controls->Add(l2); this->Controls->Add(txtID); this->Controls->Add(l1);
		}
		void OnTinhChanged(Object^ sender, EventArgs^ e) {
			cboQuan->Items->Clear();
			if (cboTinh->SelectedItem != nullptr) {
				String^ tinh = cboTinh->SelectedItem->ToString();
				if (dataVN->ContainsKey(tinh)) {
					for each (String ^ q in dataVN[tinh]) cboQuan->Items->Add(q);
					if (cboQuan->Items->Count > 0) cboQuan->SelectedIndex = 0;
				}
			}
		}
		void OnSaveClick(Object^ sender, EventArgs^ e) {
			if (String::IsNullOrWhiteSpace(txtTen->Text)) return;
			NewTen = txtTen->Text; NewSDT = txtSDT->Text; NewDiaChi = txtDiaChi->Text;
			NewTinh = cboTinh->SelectedItem ? cboTinh->SelectedItem->ToString() : "";
			NewQuan = cboQuan->SelectedItem ? cboQuan->SelectedItem->ToString() : "";
			this->DialogResult = System::Windows::Forms::DialogResult::OK; this->Close();
		}
	};

	// ====================================================================
	// MAIN FORM: WAREHOUSE FORM (KHO)
	// ====================================================================
	public ref class WarehouseForm : public System::Windows::Forms::Form
	{
	public:
		String^ MaKhoHienTai;
		String^ CurrentUsername;
		WarehouseForm(void)
		{
			InitializeComponent();

			LoadProductImageMapping();
		}
		WarehouseForm(String^ userDangNhap, String^ maKho) {
			this->MaKhoHienTai = maKho;

			// Load tài nguyên và User
			LoadImageResources();
			UpdateRealUsernameFromDB();

			InitializeComponent();
			LoadProductImageMapping();
			this->Text = L"HỆ THỐNG QUẢN LÝ KHO - " + MaKhoHienTai;
			lblUserInfo->Text = this->CurrentUsername + L"\n(Quản Lý Kho)";

			// Load dữ liệu ban đầu
			LoadStockDataFromDB();
			btnMenuStock_Click(nullptr, nullptr);
			SetupAlertButton();
			KiemTraChinhSachTon();
		}

	protected:
		~WarehouseForm() { if (components) delete components; }

	private:
		System::ComponentModel::Container^ components;

		// Sidebar
		Panel^ pnlSidebar; Label^ lblAppName; Panel^ pnlAccount; Label^ lblAccountName;
		Button^ btnMenuStock; Button^ btnMenuRequests; Button^ btnMenuImport; Button^ btnMenuPersonal;
		Panel^ pnlSubMenuPersonal; Button^ btnSubInfo; Button^ btnSubPass; Button^ btnSubLogout;
		bool isPersonalMenuOpen = false;

		// Content
		TabControl^ tabContent;
		TabPage^ tabStock; TabPage^ tabRequests; TabPage^ tabInfo; TabPage^ tabChangePass;

		// Toolbar
		Panel^ pnlToolbar; Label^ lblUserInfo; Label^ lblPageTitle;
		TabPage^ tabShipping;       // Tab mới
		DataGridView^ dgvShipping;  // Grid hiển thị đơn
		// --- TAB 1: STOCK UI (ĐÃ NÂNG CẤP) ---
		DataGridView^ dgvStock;
		Panel^ pnlStockFilter; // Panel chứa bộ lọc
		TextBox^ txtStockSearch;
		ComboBox^ cboStockSort;
		ComboBox^ cboStockCategory;
		DateTimePicker^ dtpStockFrom;
		DateTimePicker^ dtpStockTo;
		Button^ btnStockFilter;
		Button^ btnStockRefresh;

		// Dữ liệu Tồn Kho
		List<StockViewModel^>^ originalStockList; // Danh sách gốc từ DB

		// --- TAB 2, 3, 4 ---
		DataGridView^ dgvRequests;
		GroupBox^ grpCardInfo; Label^ lblInfoID; Label^ lblInfoName; Label^ lblInfoPhone; Label^ lblInfoFullAddress; Button^ btnUpdateInfo;
		TextBox^ txtCurrentUser; TextBox^ txtNewUser; Button^ btnConfirmChangeUser;
		TextBox^ txtOldPass; TextBox^ txtNewPass; TextBox^ txtConfirmPass;
		Button^ btnConfirmChangePass; Button^ btnEyeNew; Button^ btnEyeConfirm;
		Button^ btnCanhBao; List<CanhBaoItem^>^ dsCanhBao;

		// Image Resources
		System::Drawing::Image^ imgShowPass;
		System::Drawing::Image^ imgHidePass;
		System::Windows::Forms::DataGridView^ dataGridView1;
		System::Collections::Generic::Dictionary<String^, String^>^ imageMap;
		// --- TAB QUẢN LÝ NHIỆT ĐỘ ---
		TabPage^ tabTemp;
		Button^ btnMenuTemp;
		// --- [THÊM MỚI] MENU CON CHO TỒN KHO ---
		//Button^ btnMenuStock;       // Nút cha
		Panel^ pnlSubMenuStock;     // Panel chứa menu con
		Button^ btnSubImport;       // 1. Nhập hàng
		Button^ btnSubShipping;     // 2. Đơn đang vận chuyển
		Button^ btnSubReceived;     // 3. Đơn đã nhận / Lịch sử
		bool isStockMenuOpen = false; // Trạng thái đóng/mở

		// --- HÀM 1: KHỞI TẠO NÚT CẢNH BÁO (Gọi trong InitializeComponent hoặc Constructor) ---
		void SetupAlertButton() {
			btnCanhBao = gcnew Button();
			btnCanhBao->Text = L"⚠️ CẦN NHẬP HÀNG";
			btnCanhBao->BackColor = Color::Red;
			btnCanhBao->ForeColor = Color::White;
			btnCanhBao->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, FontStyle::Bold));
			btnCanhBao->Height = 45;
			btnCanhBao->Dock = DockStyle::Bottom; // Nằm dưới cùng Sidebar
			btnCanhBao->Cursor = Cursors::Hand;
			btnCanhBao->Visible = false; // Mặc định ẩn

			btnCanhBao->Click += gcnew EventHandler(this, &WarehouseForm::OnBtnCanhBaoClick);

			if (this->pnlSidebar != nullptr) {
				this->pnlSidebar->Controls->Add(btnCanhBao);

				this->pnlSidebar->Controls->SetChildIndex(this->pnlAccount, 0);

				// Đưa nút Cảnh báo lên trên panel Account
				this->pnlSidebar->Controls->SetChildIndex(btnCanhBao, 1);
				// ------------------------------------
			}
		}
		// Hàm kiểm tra xem Kho này có quản lý sản phẩm này không
		bool IsProductInWarehouse(SQLHDBC hDbc, std::string maKho, std::string maSP) {
			SQLHSTMT hStmt;
			bool exists = false;

			if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
				// Truy vấn xem có bất kỳ lô hàng nào của SP này trong Kho không (kể cả số lượng 0)
				std::wstring sql = L"SELECT TOP 1 1 FROM TonKho t "
					L"JOIN ChiTietTonKho c ON t.MaLoHang = c.MaLoHang "
					L"WHERE t.MaKho = ? AND c.MaSanPham = ?";

				SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);

				SQLCHAR cKho[51], cSP[51];
				strcpy_s((char*)cKho, 51, maKho.c_str());
				strcpy_s((char*)cSP, 51, maSP.c_str());

				SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cKho, 0, NULL);
				SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cSP, 0, NULL);

				if (SQLExecute(hStmt) == SQL_SUCCESS) {
					if (SQLFetch(hStmt) == SQL_SUCCESS) {
						exists = true; // Tìm thấy ít nhất 1 dòng -> Kho có quản lý SP này
					}
				}
			}
			SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
			return exists;
		}
		// Hàm Logic: Kiểm tra tồn kho của Chính kho này
		void KiemTraChinhSachTon() {
			dsCanhBao = gcnew List<CanhBaoItem^>();

			// 1. Kết nối SQL
			SQLHENV hEnv = SQL_NULL_HENV; SQLHDBC hDbc = SQL_NULL_HDBC;
			SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
			SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
			SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
			SQLWCHAR* connStr = (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;";
			SQLDriverConnect(hDbc, NULL, connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

			// 2. Lấy danh sách sản phẩm & Chính sách tồn
			std::vector<SanPham> dsSP;
			DocfileSanPham(hDbc, dsSP);

			std::vector<ChinhSachTon> dsCS;
			DocFileChinhSachTon(hDbc, dsCS, dsSP);

			// 3. DUYỆT TỪNG CHÍNH SÁCH VÀ SO SÁNH VỚI KHO HIỆN TẠI
			std::string sMaKho = ToStdString(this->MaKhoHienTai); // Convert 1 lần dùng cho nhanh
			for (const auto& cs : dsCS) {
				// 1. Lấy tồn kho thực tế
				float tonTaiKho = LayTonKhoTaiKho(hDbc, sMaKho, cs.getMaSP());

				// 2. LOGIC LỌC THÔNG MINH
				bool isRelevant = false;

				if (tonTaiKho > 0) {
					// Nếu đang có hàng -> Chắc chắn là sản phẩm của kho này
					isRelevant = true;
				}
				else {
					// Nếu hết sạch hàng (ton = 0) -> Kiểm tra lịch sử xem kho này CÓ TỪNG BÁN món này không?
					// Nếu từng bán thì mới báo nhập, còn chưa từng bán thì bỏ qua.
					if (IsProductInWarehouse(hDbc, sMaKho, cs.getMaSP())) {
						isRelevant = true;
					}
				}

				// Nếu không liên quan đến kho này thì bỏ qua (Continue)
				if (!isRelevant) continue;

				// 3. Kiểm tra ngưỡng đặt hàng (ROP)
				if (tonTaiKho <= cs.getDiemDatHang()) {
					CanhBaoItem^ item = gcnew CanhBaoItem();
					item->MaSP = ToSysString(cs.getMaSP());
					item->TenSP = ToSysString(cs.getTenSP());
					item->TonTaiKho = tonTaiKho;
					item->DiemDatHang = cs.getDiemDatHang();
					item->EOQ = cs.getEOQ();

					dsCanhBao->Add(item);
				}
			}

			// 4. HIỂN THỊ NÚT (Có xử lý Z-Order để không bị che)
			// Code hiển thị cho debug và thực tế
			if (dsCanhBao->Count > 0) {
				btnCanhBao->Text = L"⚠️ CẦN NHẬP HÀNG (" + dsCanhBao->Count + ")";
				btnCanhBao->BackColor = Color::Red;
				btnCanhBao->Visible = true;
			}
			else {
				// Để nút hiện màu xanh -> Xác nhận code đã chạy đúng, đã lọc, và kho đang ổn
				btnCanhBao->Text = L"✅ KHO ỔN ĐỊNH";
				btnCanhBao->BackColor = Color::SeaGreen;
				btnCanhBao->Visible = true;
			}

			// [QUAN TRỌNG] Đẩy nút lên trên cùng để không bị Panel khác che mất
			if (this->pnlSidebar != nullptr && btnCanhBao->Parent == this->pnlSidebar) {
				// Đặt thứ tự nút cảnh báo nằm trên Panel Account
				this->pnlSidebar->Controls->SetChildIndex(btnCanhBao, 0);
			}

			SQLDisconnect(hDbc);
			SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
			SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
		}
		// Sự kiện bấm nút
		void OnBtnCanhBaoClick(Object^ sender, EventArgs^ e) {
			if (dsCanhBao == nullptr || dsCanhBao->Count == 0) {
				MessageBox::Show(L"Hiện tại không có sản phẩm nào cần nhập hàng.", L"Thông báo");
				return;
			}

			RestockAlertForm^ frm = gcnew RestockAlertForm(dsCanhBao);
			frm->ShowDialog();
		}

		// --- LOAD ẢNH ---
		void LoadImageResources() {
			try {
				if (System::IO::File::Exists("D:\\code\\ChanggWithOOP\\PBL2_QuanLyKho\\PBL2_QuanLyKho\\Kho_anh\\TachMoMat.png"))
					imgShowPass = System::Drawing::Image::FromFile("D:\\code\\ChanggWithOOP\\PBL2_QuanLyKho\\PBL2_QuanLyKho\\Kho_anh\\TachMoMat.png");
				if (System::IO::File::Exists("D:\\code\\ChanggWithOOP\\PBL2_QuanLyKho\\PBL2_QuanLyKho\\Kho_anh\\TachNhamMat.png"))
					imgHidePass = System::Drawing::Image::FromFile("D:\\code\\ChanggWithOOP\\PBL2_QuanLyKho\\PBL2_QuanLyKho\\Kho_anh\\TachNhamMat.png");
			}
			catch (...) { imgShowPass = nullptr; imgHidePass = nullptr; }
		}
		void LoadProductImageMapping() {
			imageMap = gcnew System::Collections::Generic::Dictionary<String^, String^>();
			String^ fileName = "ProductImages.txt";

			if (System::IO::File::Exists(fileName)) {
				// Dùng cli::array để tránh xung đột với std::array
				cli::array<String^>^ lines = System::IO::File::ReadAllLines(fileName);

				for each (String ^ line in lines) {
					cli::array<String^>^ parts = line->Split('|');
					if (parts->Length >= 2) {
						String^ key = parts[0]->Trim();
						String^ path = parts[1]->Trim();

						// Xử lý đường dẫn trong file text (Sửa \\ thành \)
						path = path->Replace("\\\\", "\\");

						if (!imageMap->ContainsKey(key)) {
							imageMap->Add(key, path);
						}
					}
				}
			}
		}

		// 4. Sự kiện Click (Sửa lỗi logic)
		// Sửa tên hàm cho dễ hiểu và khớp logic
		void OnStockCellClick(System::Object^ sender, System::Windows::Forms::DataGridViewCellEventArgs^ e) {
			// 1. Kiểm tra nếu click vào tiêu đề hoặc vùng trống
			if (e->RowIndex < 0) return;

			DataGridViewRow^ row = this->dgvStock->Rows[e->RowIndex];

			// Kiểm tra ô đầu tiên có null không để tránh lỗi
			if (row->Cells[0]->Value == nullptr) return;
			String^ maSP = row->Cells[0]->Value->ToString();
			String^ tenSP = row->Cells[1]->Value->ToString();
			String^ maLo = row->Cells[3]->Value->ToString();

			// Xử lý HSD (Cột 4)
			DateTime hsd = DateTime::Now;
			String^ hsdStr = row->Cells[4]->Value->ToString();
			// Vì cột 4 bạn format dd/MM/yyyy nên parse cẩn thận, hoặc DataGridView đã lưu DateTime object
			try {
				hsd = DateTime::ParseExact(hsdStr, "dd/MM/yyyy", nullptr);
			}
			catch (...) {
				// Nếu parse lỗi thì thử parse thường
				DateTime::TryParse(hsdStr, hsd);
			}

			// Xử lý Số lượng (Cột 5)
			int sl = 0;
			if (row->Cells[5]->Value != nullptr)
				Int32::TryParse(row->Cells[5]->Value->ToString()->Replace(",", ""), sl); // Replace dấu phẩy nếu định dạng N2

			// Xử lý Giá (Cột 6)
			double gia = 0;
			if (row->Cells[6]->Value != nullptr)
				Double::TryParse(row->Cells[6]->Value->ToString()->Replace(",", ""), gia); // Replace dấu phẩy nếu định dạng N0

			// 4. Tìm ảnh
			String^ imagePath = "";
			if (imageMap != nullptr && imageMap->ContainsKey(maSP)) {
				imagePath = imageMap[maSP];
			}

			// 5. Mở Form
			PBL2QuanLyKho::StockDetailForm^ frm = gcnew PBL2QuanLyKho::StockDetailForm(maSP, tenSP, maLo, hsd, sl, gia, imagePath);
			frm->ShowDialog();
		}
		// --- UI HELPERS ---
		GroupBox^ CreateGroupBox(String^ title, int x, int y) {
			GroupBox^ g = gcnew GroupBox(); g->Text = title; g->Font = (gcnew System::Drawing::Font(L"Segoe UI", 14, FontStyle::Bold));
			g->Size = System::Drawing::Size(450, 480); g->Location = Point(x, y); g->ForeColor = Color::DimGray; return g;
		}
		TextBox^ CreateInput(GroupBox^ g, String^ lbl, int y, bool isPass) {
			Label^ l = gcnew Label(); l->Text = lbl; l->Location = Point(35, y); l->AutoSize = true; l->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10));
			TextBox^ t = gcnew TextBox(); t->Location = Point(35, y + 30); t->Size = System::Drawing::Size(320, 30); t->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11));
			t->BackColor = Color::FromArgb(240, 242, 245); t->BorderStyle = BorderStyle::None;
			if (isPass) t->UseSystemPasswordChar = true;
			Label^ line = gcnew Label(); line->BorderStyle = BorderStyle::Fixed3D; line->Size = System::Drawing::Size(320, 2); line->Location = Point(35, y + 65);
			g->Controls->Add(l); g->Controls->Add(t); g->Controls->Add(line); return t;
		}
		Button^ CreateButton(GroupBox^ g, String^ txt, int y, Color c) {
			Button^ b = gcnew Button(); b->Text = txt; b->BackColor = c; b->ForeColor = Color::White;
			b->Size = System::Drawing::Size(320, 45); b->Location = Point(35, y); b->FlatStyle = FlatStyle::Flat;
			g->Controls->Add(b); return b;
		}
		Button^ CreateEyeBtn(GroupBox^ g, int y) {
			Button^ b = gcnew Button(); b->Size = System::Drawing::Size(35, 30); b->Location = Point(355, y);
			b->FlatStyle = FlatStyle::Flat; b->FlatAppearance->BorderSize = 0; b->Text = L""; b->BackColor = Color::FromArgb(240, 242, 245);
			if (imgHidePass != nullptr) { b->BackgroundImage = imgHidePass; b->BackgroundImageLayout = ImageLayout::Zoom; }
			else b->Text = L"👁";
			g->Controls->Add(b); return b;
		}
		Button^ CreateMenuButton(String^ text) {
			Button^ btn = gcnew Button(); btn->Text = text; btn->Height = 60; btn->Dock = DockStyle::Top;
			btn->FlatStyle = FlatStyle::Flat; btn->FlatAppearance->BorderSize = 0; btn->TextAlign = ContentAlignment::MiddleLeft; btn->Padding = System::Windows::Forms::Padding(20, 0, 0, 0);
			btn->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, FontStyle::Regular)); btn->Cursor = Cursors::Hand;
			btn->BackColor = Color::FromArgb(10, 25, 47); btn->ForeColor = Color::FromArgb(204, 214, 246); return btn;
		}
		Button^ CreateSubMenuButton(String^ text, Color txtColor) {
			Button^ btn = gcnew Button(); btn->Text = text; btn->Height = 45; btn->Dock = DockStyle::Top;
			btn->FlatStyle = FlatStyle::Flat; btn->FlatAppearance->BorderSize = 0; btn->TextAlign = ContentAlignment::MiddleLeft; btn->Padding = System::Windows::Forms::Padding(40, 0, 0, 0);
			btn->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, FontStyle::Regular));
			btn->BackColor = Color::FromArgb(15, 30, 50); btn->ForeColor = txtColor; btn->Cursor = Cursors::Hand; return btn;
		}

		void InitializeComponent() {
			this->Size = System::Drawing::Size(1280, 720);
			this->StartPosition = FormStartPosition::CenterScreen;
			this->WindowState = FormWindowState::Maximized;
			this->BackColor = Color::WhiteSmoke;

			// --- SIDEBAR ---
			pnlSidebar = gcnew Panel(); pnlSidebar->Dock = DockStyle::Left; pnlSidebar->Width = 260; pnlSidebar->BackColor = Color::FromArgb(10, 25, 47);

			lblAppName = gcnew Label(); lblAppName->Text = L"Kho Tổng"; lblAppName->Font = (gcnew System::Drawing::Font(L"Segoe UI", 24, FontStyle::Bold));
			lblAppName->ForeColor = Color::FromArgb(100, 255, 218); lblAppName->Dock = DockStyle::Top; lblAppName->Height = 100; lblAppName->TextAlign = ContentAlignment::MiddleCenter;

			pnlAccount = gcnew Panel(); pnlAccount->Dock = DockStyle::Bottom; pnlAccount->Height = 60; pnlAccount->BackColor = Color::FromArgb(5, 15, 30);
			lblAccountName = gcnew Label(); lblAccountName->Text = L"ID: " + MaKhoHienTai; lblAccountName->ForeColor = Color::Gray; lblAccountName->Dock = DockStyle::Fill; lblAccountName->TextAlign = ContentAlignment::MiddleCenter;
			pnlAccount->Controls->Add(lblAccountName);

			// 1. MENU CON CÁ NHÂN
			pnlSubMenuPersonal = gcnew Panel(); pnlSubMenuPersonal->Dock = DockStyle::Top; pnlSubMenuPersonal->Height = 135; pnlSubMenuPersonal->Visible = false; pnlSubMenuPersonal->BackColor = Color::FromArgb(15, 30, 50);
			btnSubLogout = CreateSubMenuButton(L"•  Đăng xuất", Color::FromArgb(238, 77, 45)); btnSubLogout->Click += gcnew EventHandler(this, &WarehouseForm::OnLogoutClick);
			btnSubPass = CreateSubMenuButton(L"•  Đổi mật khẩu", Color::FromArgb(180, 180, 180)); btnSubPass->Click += gcnew EventHandler(this, &WarehouseForm::OnChangePassClick);
			btnSubInfo = CreateSubMenuButton(L"•  Thông tin tài khoản", Color::FromArgb(180, 180, 180)); btnSubInfo->Click += gcnew EventHandler(this, &WarehouseForm::OnSubInfoClick);
			pnlSubMenuPersonal->Controls->Add(btnSubLogout); pnlSubMenuPersonal->Controls->Add(btnSubPass); pnlSubMenuPersonal->Controls->Add(btnSubInfo);

			// 2. KHỞI TẠO CÁC NÚT MENU CHÍNH
			btnMenuPersonal = CreateMenuButton(L"👤  Cá Nhân"); btnMenuPersonal->Click += gcnew EventHandler(this, &WarehouseForm::btnMenuPersonal_Click);

			btnMenuTemp = CreateMenuButton(L"❄️  Quản Lý Nhiệt Độ");
			btnMenuTemp->Click += gcnew EventHandler(this, &WarehouseForm::btnMenuTemp_Click);

			btnMenuRequests = CreateMenuButton(L"🚚  Xuất Kho (Yêu Cầu)");
			btnMenuRequests->Click += gcnew EventHandler(this, &WarehouseForm::btnMenuRequests_Click);

			// Nút Tồn Kho (Cha)
			btnMenuStock = CreateMenuButton(L"📦  Quản Lý Tồn Kho");
			btnMenuStock->Click += gcnew EventHandler(this, &WarehouseForm::btnMenuStock_Click);

			// MENU CON TỒN KHO
			pnlSubMenuStock = gcnew Panel();
			pnlSubMenuStock->Dock = DockStyle::Top;
			pnlSubMenuStock->Height = 135;
			pnlSubMenuStock->Visible = false;
			pnlSubMenuStock->BackColor = Color::FromArgb(15, 30, 50);

			// Nút Nhập hàng
			btnSubImport = CreateSubMenuButton(L"•  Nhập Hàng Mới", Color::FromArgb(100, 255, 218));
			btnSubImport->Click += gcnew EventHandler(this, &WarehouseForm::OnSubImportClick);

			// Nút Đơn Vận Chuyển
			btnSubShipping = CreateSubMenuButton(L"•  Đơn Vận Chuyển", Color::FromArgb(255, 165, 0));
			btnSubShipping->Click += gcnew EventHandler(this, &WarehouseForm::OnSubShippingClick);

			// Nút Đơn Đã Nhận
			btnSubReceived = CreateSubMenuButton(L"•  Lịch Sử Đã Nhận", Color::FromArgb(180, 180, 180));
			btnSubReceived->Click += gcnew EventHandler(this, &WarehouseForm::OnSubReceivedClick);

			pnlSubMenuStock->Controls->Add(btnSubReceived);
			pnlSubMenuStock->Controls->Add(btnSubShipping);
			pnlSubMenuStock->Controls->Add(btnSubImport);

			// --- THÊM VÀO SIDEBAR ---
			pnlSidebar->Controls->Add(pnlSubMenuPersonal);
			pnlSidebar->Controls->Add(btnMenuPersonal);
			pnlSidebar->Controls->Add(btnMenuRequests);
			pnlSidebar->Controls->Add(btnMenuTemp);
			pnlSidebar->Controls->Add(pnlSubMenuStock);
			pnlSidebar->Controls->Add(btnMenuStock);
			pnlSidebar->Controls->Add(lblAppName);
			pnlSidebar->Controls->Add(pnlAccount);

			// --- TOOLBAR ---
			pnlToolbar = gcnew Panel(); pnlToolbar->Dock = DockStyle::Top; pnlToolbar->Height = 80; pnlToolbar->BackColor = Color::White; pnlToolbar->Padding = System::Windows::Forms::Padding(20);
			lblPageTitle = gcnew Label(); lblPageTitle->Text = L"Danh Sách Tồn Kho"; lblPageTitle->Font = (gcnew System::Drawing::Font(L"Segoe UI", 18, FontStyle::Bold));
			lblPageTitle->AutoSize = true; lblPageTitle->Location = Point(20, 20);
			lblUserInfo = gcnew Label(); lblUserInfo->Dock = DockStyle::Right; lblUserInfo->TextAlign = ContentAlignment::MiddleRight; lblUserInfo->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11, FontStyle::Bold));
			pnlToolbar->Controls->Add(lblPageTitle); pnlToolbar->Controls->Add(lblUserInfo);

			// --- CONTENT TABS ---
			tabContent = gcnew TabControl(); tabContent->Dock = DockStyle::Fill;
			tabContent->Appearance = TabAppearance::FlatButtons; tabContent->ItemSize = System::Drawing::Size(0, 1); tabContent->SizeMode = TabSizeMode::Fixed;

			// TAB 1: STOCK
			tabStock = gcnew TabPage(); tabStock->BackColor = Color::FromArgb(245, 245, 250);
			InitializeTabStock();

			// TAB 1.5: TEMP
			tabTemp = gcnew TabPage(); tabTemp->BackColor = Color::White;
			PBL2QuanLyKho::TemperatureTab^ tempModule = gcnew PBL2QuanLyKho::TemperatureTab(this->MaKhoHienTai);
			tabTemp->Controls->Add(tempModule);

			// TAB 2: REQUESTS
			tabRequests = gcnew TabPage(); tabRequests->BackColor = Color::White;
			PBL2QuanLyKho::TransportTab^ transportModule = gcnew PBL2QuanLyKho::TransportTab(this->MaKhoHienTai);
			tabRequests->Controls->Add(transportModule);

			// =========================================================
			// [QUAN TRỌNG] THÊM MỚI TAB SHIPPING (ĐƠN VẬN CHUYỂN)
			// =========================================================
			tabShipping = gcnew TabPage(L"Đơn Vận Chuyển");
			tabShipping->BackColor = Color::White;

			dgvShipping = gcnew DataGridView();
			dgvShipping->Dock = DockStyle::Fill;
			dgvShipping->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;
			dgvShipping->BackgroundColor = Color::White;
			dgvShipping->ReadOnly = true; // Chỉ đọc (trừ nút bấm)
			dgvShipping->RowTemplate->Height = 40;

			// Cột dữ liệu
			dgvShipping->Columns->Add("MaDon", L"Mã Đơn");
			dgvShipping->Columns->Add("Ngay", L"Ngày Đặt");
			dgvShipping->Columns->Add("TongTien", L"Tổng Tiền Hàng");
			dgvShipping->Columns->Add("PhiShip", L"Phí Vận Chuyển");
			dgvShipping->Columns->Add("TongCong", L"Tổng Thanh Toán");
			dgvShipping->Columns->Add("TrangThai", L"Trạng Thái");

			// [MỚI] THÊM CỘT NÚT BẤM "ĐÃ NHẬN"
			DataGridViewButtonColumn^ btnCol = gcnew DataGridViewButtonColumn();
			btnCol->Name = "btnConfirmRec";
			btnCol->HeaderText = L"Hành Động";
			btnCol->Text = L"Đã Nhận Hàng";
			btnCol->UseColumnTextForButtonValue = true; // Hiển thị chữ trên nút
			dgvShipping->Columns->Add(btnCol);

			// Đăng ký sự kiện Click
			dgvShipping->CellContentClick += gcnew DataGridViewCellEventHandler(this, &WarehouseForm::OnShippingCellClick);

			tabShipping->Controls->Add(dgvShipping);
			// =========================================================

			// TAB 3: INFO
			tabInfo = gcnew TabPage(); tabInfo->BackColor = Color::FromArgb(240, 242, 245); InitializeTabInfo();

			// TAB 4: CHANGE PASS
			tabChangePass = gcnew TabPage(); tabChangePass->BackColor = Color::FromArgb(240, 242, 245); InitializeTabChangePass();

			// Add Tabs (Thứ tự thêm vào TabControl)
			tabContent->Controls->Add(tabStock);
			tabContent->Controls->Add(tabTemp);
			tabContent->Controls->Add(tabRequests);

			// [MỚI] Add tab shipping vào đây
			tabContent->Controls->Add(tabShipping);

			tabContent->Controls->Add(tabInfo);
			tabContent->Controls->Add(tabChangePass);

			this->Controls->Add(tabContent); this->Controls->Add(pnlToolbar); this->Controls->Add(pnlSidebar);
		}
		// --- TAB 1: STOCK UI ---
		void InitializeTabStock() {
			// 1. FILTER PANEL (DOCK TOP)
			pnlStockFilter = gcnew Panel(); pnlStockFilter->Dock = DockStyle::Top; pnlStockFilter->Height = 60;
			pnlStockFilter->BackColor = Color::WhiteSmoke; pnlStockFilter->Padding = System::Windows::Forms::Padding(10);

			// Search Box (Gõ là tìm ngay)
			Label^ lSearch = gcnew Label(); lSearch->Text = L"Tìm tên:"; lSearch->Location = Point(10, 20); lSearch->AutoSize = true;
			txtStockSearch = gcnew TextBox(); txtStockSearch->Location = Point(70, 18); txtStockSearch->Width = 150;
			// [MỚI] Sự kiện gõ phím
			txtStockSearch->TextChanged += gcnew EventHandler(this, &WarehouseForm::OnStockFilterChanged);

			// Sort ComboBox (Chọn là sắp xếp ngay)
			Label^ lSort = gcnew Label(); lSort->Text = L"Sắp xếp:"; lSort->Location = Point(240, 20); lSort->AutoSize = true;
			cboStockSort = gcnew ComboBox(); cboStockSort->Location = Point(300, 18); cboStockSort->Width = 120; cboStockSort->DropDownStyle = ComboBoxStyle::DropDownList;
			cboStockSort->Items->Add(L"HSD: Gần nhất"); cboStockSort->Items->Add(L"Tên: A -> Z"); cboStockSort->Items->Add(L"Giá: Tăng"); cboStockSort->Items->Add(L"Giá: Giảm");
			cboStockSort->SelectedIndex = 0;
			// [MỚI] Sự kiện chọn item
			cboStockSort->SelectedIndexChanged += gcnew EventHandler(this, &WarehouseForm::OnStockFilterChanged);

			// Category ComboBox (Chọn là lọc ngay)
			Label^ lCat = gcnew Label(); lCat->Text = L"Loại:"; lCat->Location = Point(440, 20); lCat->AutoSize = true;
			cboStockCategory = gcnew ComboBox(); cboStockCategory->Location = Point(480, 18); cboStockCategory->Width = 120; cboStockCategory->DropDownStyle = ComboBoxStyle::DropDownList;
			cboStockCategory->Items->Add(L"Tất cả"); cboStockCategory->SelectedIndex = 0;
			// [MỚI] Sự kiện chọn loại
			cboStockCategory->SelectedIndexChanged += gcnew EventHandler(this, &WarehouseForm::OnStockFilterChanged);

			// Date Range (Chọn ngày là lọc ngay)
			Label^ lFrom = gcnew Label(); lFrom->Text = L"HSD từ:"; lFrom->Location = Point(620, 20); lFrom->AutoSize = true;
			dtpStockFrom = gcnew DateTimePicker(); dtpStockFrom->Format = DateTimePickerFormat::Short; dtpStockFrom->Location = Point(675, 18); dtpStockFrom->Width = 100;
			dtpStockFrom->Value = DateTime::Now.AddYears(-5);
			// [MỚI] Sự kiện đổi ngày
			dtpStockFrom->ValueChanged += gcnew EventHandler(this, &WarehouseForm::OnStockFilterChanged);

			Label^ lTo = gcnew Label(); lTo->Text = L"đến:"; lTo->Location = Point(785, 20); lTo->AutoSize = true;
			dtpStockTo = gcnew DateTimePicker(); dtpStockTo->Format = DateTimePickerFormat::Short; dtpStockTo->Location = Point(820, 18); dtpStockTo->Width = 100;
			dtpStockTo->Value = DateTime::Now.AddYears(5);
			// [MỚI] Sự kiện đổi ngày
			dtpStockTo->ValueChanged += gcnew EventHandler(this, &WarehouseForm::OnStockFilterChanged);

			// Buttons (Giữ lại nút Lọc thủ công nếu muốn, hoặc chỉ để nút Reset)
			btnStockRefresh = gcnew Button(); btnStockRefresh->Text = L"Reset"; btnStockRefresh->Location = Point(1010, 16); btnStockRefresh->Size = System::Drawing::Size(70, 30);
			btnStockRefresh->BackColor = Color::White; btnStockRefresh->FlatStyle = FlatStyle::Flat;
			btnStockRefresh->Click += gcnew EventHandler(this, &WarehouseForm::OnStockRefreshClick);

			pnlStockFilter->Controls->Add(btnStockRefresh); pnlStockFilter->Controls->Add(btnStockFilter);
			pnlStockFilter->Controls->Add(dtpStockTo); pnlStockFilter->Controls->Add(lTo);
			pnlStockFilter->Controls->Add(dtpStockFrom); pnlStockFilter->Controls->Add(lFrom);
			pnlStockFilter->Controls->Add(cboStockCategory); pnlStockFilter->Controls->Add(lCat);
			pnlStockFilter->Controls->Add(cboStockSort); pnlStockFilter->Controls->Add(lSort);
			pnlStockFilter->Controls->Add(txtStockSearch); pnlStockFilter->Controls->Add(lSearch);

			// 2. GRID VIEW (GIỮ NGUYÊN)
			dgvStock = gcnew DataGridView();
			dgvStock->Dock = DockStyle::Fill;
			dgvStock->BackgroundColor = Color::White;
			dgvStock->BorderStyle = BorderStyle::None;
			dgvStock->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;
			dgvStock->RowTemplate->Height = 40;
			dgvStock->AllowUserToAddRows = false; dgvStock->ReadOnly = true;
			dgvStock->SelectionMode = DataGridViewSelectionMode::FullRowSelect;

			dgvStock->EnableHeadersVisualStyles = false;
			dgvStock->ColumnHeadersDefaultCellStyle->BackColor = Color::FromArgb(23, 42, 69);
			dgvStock->ColumnHeadersDefaultCellStyle->ForeColor = Color::White;
			dgvStock->ColumnHeadersDefaultCellStyle->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11, FontStyle::Bold));
			dgvStock->ColumnHeadersHeight = 45;

			dgvStock->Columns->Add("MaSP", L"Mã SP");
			dgvStock->Columns->Add("TenSP", L"Tên Sản Phẩm");
			dgvStock->Columns->Add("Loai", L"Loại");
			dgvStock->Columns->Add("MaLo", L"Mã Lô Hàng");
			dgvStock->Columns->Add("HSD", L"Hạn Sử Dụng");
			dgvStock->Columns->Add("SL", L"Số Lượng (Tấn)");
			dgvStock->Columns->Add("Gia", L"Giá Trị (VNĐ)");

			dgvStock->Columns["TenSP"]->Width = 200;
			dgvStock->CellClick += gcnew DataGridViewCellEventHandler(this, &WarehouseForm::OnStockCellClick);
			tabStock->Controls->Add(dgvStock);
			tabStock->Controls->Add(pnlStockFilter);
		}
		void LoadShippingData(String^ trangThaiFilter) {
			dgvShipping->Rows->Clear();
			if (dgvShipping->Columns["btnConfirmRec"] != nullptr) {
				dgvShipping->Columns["btnConfirmRec"]->Visible = (trangThaiFilter == "Dang Van Chuyen" || trangThaiFilter == L"Đang Vận Chuyển");
			}

			SQLHENV hEnv; SQLHDBC hDbc; SQLHSTMT hStmt;
			SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
			SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
			SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
			SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

			if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
				std::wstring sql = L"SELECT MaDonHang, NgayDat, TongTienHang, TongPhiVanChuyen, TongThanhToan, TrangThai FROM DonDatHang WHERE MaKho = ? ";

				// Nếu có lọc trạng thái thì thêm điều kiện
				if (!String::IsNullOrEmpty(trangThaiFilter)) {
					sql += L"AND TrangThai = ? ";
				}
				sql += L"ORDER BY NgayDat DESC";

				SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);

				std::string sKho = ToStdString(this->MaKhoHienTai);
				SQLCHAR cKho[51]; strcpy_s((char*)cKho, 51, sKho.c_str());
				SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cKho, 0, NULL);

				// Bind tham số thứ 2 nếu có lọc
				SQLWCHAR cTT[51];
				if (!String::IsNullOrEmpty(trangThaiFilter)) {
					// Chuyển String^ sang Wide String an toàn
					pin_ptr<const wchar_t> wch = PtrToStringChars(trangThaiFilter);
					wcsncpy_s(cTT, 51, wch, _TRUNCATE);
					SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 50, 0, cTT, 0, NULL);
				}

				if (SQLExecute(hStmt) == SQL_SUCCESS) {
					SQLCHAR md[51];
					SQL_TIMESTAMP_STRUCT nd;
					double tth, pvc, tong;

					// [SỬA QUAN TRỌNG]: Dùng SQLWCHAR cho cột trạng thái
					SQLWCHAR wTrangThai[101];

					while (SQLFetch(hStmt) == SQL_SUCCESS) {
						SQLGetData(hStmt, 1, SQL_C_CHAR, md, 51, NULL);
						SQLGetData(hStmt, 2, SQL_C_TYPE_TIMESTAMP, &nd, 0, NULL);
						SQLGetData(hStmt, 3, SQL_C_DOUBLE, &tth, 0, NULL);
						SQLGetData(hStmt, 4, SQL_C_DOUBLE, &pvc, 0, NULL);
						SQLGetData(hStmt, 5, SQL_C_DOUBLE, &tong, 0, NULL);

						// [SỬA QUAN TRỌNG]: Lấy dữ liệu dạng Unicode (SQL_C_WCHAR)
						SQLGetData(hStmt, 6, SQL_C_WCHAR, wTrangThai, 101, NULL);

						String^ sNgay = nd.day + "/" + nd.month + "/" + nd.year;

						// Tạo String từ mảng wchar_t để hiển thị đúng tiếng Việt
						String^ sHienThiTrangThai = gcnew String(wTrangThai);

						dgvShipping->Rows->Add(
							ToSysString((char*)md),
							sNgay,
							tth.ToString("N0"),
							pvc.ToString("N0"),
							tong.ToString("N0"),
							sHienThiTrangThai // Dùng biến chuỗi đã sửa
						);
					}
				}
			}
			SQLFreeHandle(SQL_HANDLE_STMT, hStmt); SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
		}
		void LoadStockDataFromDB() {
			originalStockList = gcnew List<StockViewModel^>();

			SQLHENV hEnv = SQL_NULL_HENV; SQLHDBC hDbc = SQL_NULL_HDBC; SQLHSTMT hStmt = SQL_NULL_HSTMT;
			SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv); SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0); SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
			SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

			if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
				std::wstring sql = L"SELECT c.MaSanPham, s.TenSanPham, s.Loai, c.MaLoHang, c.SoLuongTan, c.HanSuDung, c.GiaTriSanPham "
					L"FROM ChiTietTonKho c "
					L"JOIN TonKho t ON c.MaLoHang = t.MaLoHang "
					L"JOIN SanPham s ON c.MaSanPham = s.MaSanPham "
					L"WHERE t.MaKho = ? AND c.SoLuongTan > 0";

				SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);
				std::string sKho = ToStdString(this->MaKhoHienTai);
				SQLCHAR cKho[51]; strcpy_s((char*)cKho, 51, sKho.c_str());
				SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cKho, 0, NULL);

				if (SQLExecute(hStmt) == SQL_SUCCESS) {
					// [SỬA] Đổi sLoai sang SQLWCHAR để nhận tiếng Việt
					SQLCHAR sMaSP[51], sMaLo[51];
					SQLWCHAR sTen[256], sLoai[101]; // <--- QUAN TRỌNG: sLoai phải là WCHAR
					SQLDOUBLE dSL, dGia; SQL_DATE_STRUCT dHSD;
					SQLLEN len;

					List<String^>^ categories = gcnew List<String^>();

					while (SQLFetch(hStmt) == SQL_SUCCESS) {
						SQLGetData(hStmt, 1, SQL_C_CHAR, sMaSP, 51, &len);
						SQLGetData(hStmt, 2, SQL_C_WCHAR, sTen, 256, &len);
						// [SỬA] Lấy dữ liệu bằng SQL_C_WCHAR
						SQLGetData(hStmt, 3, SQL_C_WCHAR, sLoai, 101, &len);
						SQLGetData(hStmt, 4, SQL_C_CHAR, sMaLo, 51, &len);
						SQLGetData(hStmt, 5, SQL_C_DOUBLE, &dSL, 0, &len);
						SQLGetData(hStmt, 6, SQL_C_TYPE_DATE, &dHSD, 0, &len);
						SQLGetData(hStmt, 7, SQL_C_DOUBLE, &dGia, 0, &len);

						StockViewModel^ item = gcnew StockViewModel();
						item->MaSP = ToSysString(StdTrim((char*)sMaSP));
						item->TenSP = gcnew String(sTen);
						// [SỬA] Chuyển đổi trực tiếp từ WideChar sang System String
						item->Loai = gcnew String(sLoai);
						item->MaLo = ToSysString(StdTrim((char*)sMaLo));
						item->SoLuong = dSL;
						item->GiaTri = dGia;
						item->HSD = DateTime(dHSD.year, dHSD.month, dHSD.day);

						originalStockList->Add(item);

						if (!categories->Contains(item->Loai)) {
							categories->Add(item->Loai);
						}
					}

					cboStockCategory->Items->Clear();
					cboStockCategory->Items->Add(L"Tất cả");
					for each (String ^ cat in categories) cboStockCategory->Items->Add(cat);
					cboStockCategory->SelectedIndex = 0;
				}
			}
			SQLFreeHandle(SQL_HANDLE_STMT, hStmt); SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

			ApplyStockFilter();
		}
		void ApplyStockFilter() {
			if (originalStockList == nullptr) return;

			String^ keyword = txtStockSearch->Text->Trim()->ToLower();
			String^ selectedCat = cboStockCategory->SelectedItem->ToString();
			DateTime dFrom = dtpStockFrom->Value.Date;
			DateTime dTo = dtpStockTo->Value.Date;
			int sortMode = cboStockSort->SelectedIndex;

			List<StockViewModel^>^ filtered = gcnew List<StockViewModel^>();

			for each (StockViewModel ^ item in originalStockList) {
				bool matchName = String::IsNullOrEmpty(keyword) || item->TenSP->ToLower()->Contains(keyword);
				bool matchCat = (selectedCat == L"Tất cả") || (item->Loai == selectedCat);
				bool matchDate = (item->HSD.Date >= dFrom && item->HSD.Date <= dTo);

				if (matchName && matchCat && matchDate) {
					filtered->Add(item);
				}
			}

			// Sort
			filtered->Sort(gcnew StockComparer(sortMode));

			// Display
			dgvStock->Rows->Clear();
			for (int i = 0; i < dgvStock->Rows->Count; i++) {
				String^ hsdStr = dgvStock->Rows[i]->Cells[4]->Value->ToString();
				DateTime hsd;
				if (DateTime::TryParseExact(hsdStr, "dd/MM/yyyy", nullptr, System::Globalization::DateTimeStyles::None, hsd)) {
					TimeSpan span = hsd - DateTime::Now;
					if (span.TotalDays <= 0) {
						dgvStock->Rows[i]->DefaultCellStyle->BackColor = Color::MistyRose; // Hết hạn -> Đỏ nhạt
						dgvStock->Rows[i]->DefaultCellStyle->ForeColor = Color::Red;
					}
					else if (span.TotalDays <= 30) {
						dgvStock->Rows[i]->DefaultCellStyle->BackColor = Color::LightYellow; // Sắp hết (30 ngày) -> Vàng
					}
				}
			}
			for each (StockViewModel ^ item in filtered) {
				dgvStock->Rows->Add(
					item->MaSP, item->TenSP, item->Loai, item->MaLo,
					item->HSD.ToString("dd/MM/yyyy"),
					item->SoLuong.ToString("N2"),
					item->GiaTri.ToString("N0")
				);
			}
		}
		void OnStockFilterChanged(Object^ sender, EventArgs^ e) {
			ApplyStockFilter();
		}
		void OnStockFilterClick(Object^ sender, EventArgs^ e) { ApplyStockFilter(); }
		void OnStockRefreshClick(Object^ sender, EventArgs^ e) {
			txtStockSearch->Text = ""; cboStockSort->SelectedIndex = 0; cboStockCategory->SelectedIndex = 0;
			dtpStockFrom->Value = DateTime::Now.AddYears(-5); dtpStockTo->Value = DateTime::Now.AddYears(5);
			LoadStockDataFromDB();
		}

		// --- TAB INIT UI ---
		void InitializeTabInfo() {
			grpCardInfo = gcnew GroupBox(); grpCardInfo->Text = L" THÔNG TIN KHO ";
			grpCardInfo->Font = (gcnew System::Drawing::Font(L"Segoe UI", 14, FontStyle::Bold)); grpCardInfo->ForeColor = Color::Black;
			grpCardInfo->Size = System::Drawing::Size(800, 580); grpCardInfo->Location = System::Drawing::Point(370, 120);

			Panel^ p = gcnew Panel(); p->Dock = DockStyle::Fill; p->Padding = System::Windows::Forms::Padding(50, 60, 50, 30);
			p->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, FontStyle::Regular)); p->ForeColor = Color::Black;

			lblInfoID = CreateInfoLabel(p, L"Mã Kho:", 50);
			lblInfoName = CreateInfoLabel(p, L"Tên Kho:", 130);
			lblInfoPhone = CreateInfoLabel(p, L"Số điện thoại:", 210);
			lblInfoFullAddress = CreateInfoLabel(p, L"Địa chỉ:", 290);

			btnUpdateInfo = gcnew Button(); btnUpdateInfo->Text = L"CẬP NHẬT THÔNG TIN"; btnUpdateInfo->BackColor = Color::FromArgb(0, 114, 188);
			btnUpdateInfo->ForeColor = Color::White; btnUpdateInfo->FlatStyle = FlatStyle::Flat; btnUpdateInfo->Size = System::Drawing::Size(700, 50);
			btnUpdateInfo->Location = Point(50, 450); btnUpdateInfo->Click += gcnew EventHandler(this, &WarehouseForm::OnUpdateInfoClick);
			p->Controls->Add(btnUpdateInfo);

			grpCardInfo->Controls->Add(p); tabInfo->Controls->Add(grpCardInfo);
		}

		Label^ CreateInfoLabel(Panel^ p, String^ title, int y) {
			Label^ lTitle = gcnew Label(); lTitle->Text = title; lTitle->AutoSize = true; lTitle->Location = Point(50, y);
			Label^ lVal = gcnew Label(); lVal->Text = L"..."; lVal->AutoSize = true; lVal->Font = (gcnew System::Drawing::Font(L"Segoe UI", 13, FontStyle::Bold)); lVal->Location = Point(50, y + 25);
			Label^ line = gcnew Label(); line->BorderStyle = BorderStyle::Fixed3D; line->Size = System::Drawing::Size(700, 3); line->Location = Point(50, y + 55); line->ForeColor = Color::Black;
			p->Controls->Add(lTitle); p->Controls->Add(lVal); p->Controls->Add(line); return lVal;
		}

		void InitializeTabChangePass() {
			int boxWidth = 450; int gap = 40; int startX = 200; int startY = 80;
			GroupBox^ grpUser = CreateGroupBox(L" ĐỔI TÊN TÀI KHOẢN ", startX, startY);
			txtCurrentUser = CreateInput(grpUser, L"Tên hiện tại:", 40, false); txtCurrentUser->ReadOnly = true; txtCurrentUser->Text = this->CurrentUsername;
			txtNewUser = CreateInput(grpUser, L"Tên mới:", 135, false);
			btnConfirmChangeUser = CreateButton(grpUser, L"LƯU TÊN MỚI", 360, Color::FromArgb(0, 114, 188));
			btnConfirmChangeUser->Click += gcnew EventHandler(this, &WarehouseForm::OnConfirmChangeUserClick);

			GroupBox^ grpPass = CreateGroupBox(L" ĐỔI MẬT KHẨU ", startX + boxWidth + gap, startY);
			txtOldPass = CreateInput(grpPass, L"Mật khẩu cũ:", 40, true);
			txtNewPass = CreateInput(grpPass, L"Mật khẩu mới:", 135, true);
			txtConfirmPass = CreateInput(grpPass, L"Nhập lại mật khẩu:", 230, true);
			btnConfirmChangePass = CreateButton(grpPass, L"XÁC NHẬN ĐỔI", 360, Color::FromArgb(34, 139, 34));
			btnConfirmChangePass->Click += gcnew EventHandler(this, &WarehouseForm::OnConfirmChangePassClick);

			btnEyeNew = CreateEyeBtn(grpPass, 165); btnEyeNew->Click += gcnew EventHandler(this, &WarehouseForm::OnToggleNewPass);
			btnEyeConfirm = CreateEyeBtn(grpPass, 260); btnEyeConfirm->Click += gcnew EventHandler(this, &WarehouseForm::OnToggleConfirmPass);

			tabChangePass->Controls->Add(grpUser); tabChangePass->Controls->Add(grpPass);
		}

		// EVENTS
		void btnMenuStock_Click(Object^ sender, EventArgs^ e) {
			// 1. Xử lý đóng/mở menu con (Giữ nguyên)
			isStockMenuOpen = !isStockMenuOpen;
			pnlSubMenuStock->Visible = isStockMenuOpen;

			// 2. Đổi màu nút (Giữ nguyên)
			ResetMenuColors();
			btnMenuStock->BackColor = Color::FromArgb(23, 42, 69);

			// 3. [THÊM MỚI] Chuyển trang và Load dữ liệu tồn kho
			lblPageTitle->Text = L"📦 Danh Sách Tồn Kho"; // Đổi tiêu đề
			tabContent->SelectedTab = tabStock;           // Chuyển sang Tab Tồn Kho
			LoadStockDataFromDB();                        // Load lại dữ liệu mới nhất
		}
		void btnMenuRequests_Click(Object^ sender, EventArgs^ e) {
			ResetMenuColors();
			btnMenuRequests->BackColor = Color::FromArgb(23, 42, 69);
			lblPageTitle->Text = L"🚚 Yêu Cầu Xuất Kho";
			tabContent->SelectedTab = tabRequests; // <--- Gọi thẳng tên tab cho chắc chắn
		}
		void btnMenuImport_Click(Object^ sender, EventArgs^ e) {
			ResetMenuColors();
			btnMenuImport->BackColor = Color::FromArgb(23, 42, 69);

			// Mở Form Nhập Hàng mới (Truyền mã kho hiện tại)
			ImportGoodsForm^ frm = gcnew ImportGoodsForm(this->MaKhoHienTai);
			frm->ShowDialog();

			// Sau khi nhập xong, load lại tồn kho để thấy số lượng tăng lên
			LoadStockDataFromDB();
		}
		// ---------------------------------------------------------
	// 1. Hàm xử lý nút "Nhập Mới" (OnSubImportClick)
	// ---------------------------------------------------------
		void OnSubImportClick(Object^ sender, EventArgs^ e) {
			ResetMenuColors();

			btnMenuStock->BackColor = Color::FromArgb(23, 42, 69);
			PBL2QuanLyKho::ImportGoodsForm^ frm = gcnew PBL2QuanLyKho::ImportGoodsForm(this->MaKhoHienTai);
			frm->ShowDialog();
			LoadStockDataFromDB();
			KiemTraChinhSachTon();
		}
		// [MỚI] XỬ LÝ KHI BẤM NÚT TRONG BẢNG
		void OnShippingCellClick(Object^ sender, DataGridViewCellEventArgs^ e) {
			if (e->RowIndex < 0) return;
			String^ maDon = dgvShipping->Rows[e->RowIndex]->Cells[0]->Value->ToString();

			// Nếu bấm nút "Đã Nhận Hàng"
			if (dgvShipping->Columns[e->ColumnIndex]->Name == "btnConfirmRec") {
				if (MessageBox::Show(L"Xác nhận đã nhận hàng và nhập kho cho đơn " + maDon + L"?", L"Xác nhận", MessageBoxButtons::YesNo, MessageBoxIcon::Question) == System::Windows::Forms::DialogResult::Yes) {
					UpdateOrderStatus(maDon, L"Đã Giao Hàng");
					NhapKhoTuDonHang(maDon);
					OnSubReceivedClick(nullptr, nullptr);
					LoadStockDataFromDB();
					MessageBox::Show(L"Đã cập nhật trạng thái và nhập hàng vào kho thành công!", L"Thành công");
				}
			}
			// Nếu bấm vào ô khác -> Hiện chi tiết bằng form MỚI
			else {
				// [SỬA] Dùng ShippingDetailForm thay vì OrderDetailForm
				ShippingDetailForm^ frm = gcnew ShippingDetailForm(maDon);
				frm->ShowDialog();
			}
		}
		// --- HÀM NHẬP KHO (CÓ HỎI HẠN SỬ DỤNG) ---
		void NhapKhoTuDonHang(String^ maDon) {
			SQLHENV hEnv; SQLHDBC hDbc; SQLHSTMT hStmt = SQL_NULL_HSTMT;

			if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) != SQL_SUCCESS) return;
			SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
			if (SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc) != SQL_SUCCESS) return;

			// Driver "Bất tử"
			const wchar_t* drivers[] = {
				L"Driver={ODBC Driver 17 for SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;",
				L"Driver={SQL Server Native Client 11.0};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;",
				L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;"
			};

			SQLRETURN ret = SQL_ERROR;
			for (int i = 0; i < 3; i++) {
				ret = SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)drivers[i], SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
				if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) break;
			}

			if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
				MessageBox::Show(L"Lỗi kết nối SQL!", L"Lỗi");
				return;
			}

			// List dữ liệu
			System::Collections::Generic::List<String^>^ listMaSP = gcnew System::Collections::Generic::List<String^>();
			System::Collections::Generic::List<double>^ listSL = gcnew System::Collections::Generic::List<double>();
			System::Collections::Generic::List<double>^ listGia = gcnew System::Collections::Generic::List<double>();
			// [MỚI] List chứa HSD người dùng nhập
			System::Collections::Generic::List<DateTime>^ listHSD = gcnew System::Collections::Generic::List<DateTime>();

			String^ maKhoNhan = "";
			double tongTan = 0, tongGiaTri = 0;

			// --- BƯỚC 1: LẤY DỮ LIỆU TỪ DB TRƯỚC ---
			if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
				std::wstring sql = L"SELECT d.MaKho, c.MaSanPham, c.SoLuong, c.ThanhTien FROM ChiTietDonDatHang c JOIN DonDatHang d ON c.MaDonHang = d.MaDonHang WHERE c.MaDonHang = ?";
				SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);

				msclr::interop::marshal_context ctx;
				std::string sMD = ctx.marshal_as<std::string>(maDon);
				SQLCHAR cMD[51]; strcpy_s((char*)cMD, 51, sMD.c_str());
				SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMD, 0, NULL);

				if (SQLExecute(hStmt) == SQL_SUCCESS) {
					SQLCHAR cKho[51], cSP[51]; SQLDOUBLE dSL, dTT;
					while (SQLFetch(hStmt) == SQL_SUCCESS) {
						SQLGetData(hStmt, 1, SQL_C_CHAR, cKho, 51, NULL);
						SQLGetData(hStmt, 2, SQL_C_CHAR, cSP, 51, NULL);
						SQLGetData(hStmt, 3, SQL_C_DOUBLE, &dSL, 0, NULL);
						SQLGetData(hStmt, 4, SQL_C_DOUBLE, &dTT, 0, NULL);

						maKhoNhan = gcnew String((char*)cKho);
						String^ strSP = gcnew String((char*)cSP);

						listMaSP->Add(strSP->Trim());
						listSL->Add(dSL);
						listGia->Add(dTT);
						tongTan += dSL; tongGiaTri += dTT;
					}
				}
			}
			SQLFreeHandle(SQL_HANDLE_STMT, hStmt);

			if (listMaSP->Count == 0) {
				SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
				return;
			}

			// --- BƯỚC 2: [QUAN TRỌNG] HIỆN BẢNG HỎI NGÀY CHO TỪNG MÓN ---
			// Phải hỏi xong xuôi hết rồi mới mở Transaction Insert
			for (int i = 0; i < listMaSP->Count; i++) {
				PickDateForm^ frm = gcnew PickDateForm(listMaSP[i]);
				frm->ShowDialog(); // Hiện bảng lên và chờ người dùng chọn
				listHSD->Add(frm->SelectedDate); // Lưu ngày đã chọn vào List
			}

			// --- BƯỚC 3: BẮT ĐẦU INSERT VÀO DB ---
			SQLSetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, 0);
			try {
				msclr::interop::marshal_context context;
				String^ ticks = DateTime::Now.Ticks.ToString();
				String^ strMaLo = "LO" + (ticks->Length > 10 ? ticks->Substring(ticks->Length - 10) : ticks);
				std::string sMaLo = context.marshal_as<std::string>(strMaLo);
				std::string sMaKho = context.marshal_as<std::string>(maKhoNhan);
				std::string sMaNCC = "UNKNOWN";

				// 3.1 Insert LoHang
				if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
					std::wstring sql = L"INSERT INTO LoHang (MaLoHang, MaKho, MaNCC, NgayNhap, SoLuongTanTong, GiaTriTong) VALUES (?, ?, ?, GETDATE(), ?, ?)";
					SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);
					SQLCHAR cLo[51], cKho[51], cNCC[51];
					strcpy_s((char*)cLo, 51, sMaLo.c_str()); strcpy_s((char*)cKho, 51, sMaKho.c_str()); strcpy_s((char*)cNCC, 51, sMaNCC.c_str());
					SQLDOUBLE dTan = tongTan, dVal = tongGiaTri;
					SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cLo, 0, NULL);
					SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cKho, 0, NULL);
					SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cNCC, 0, NULL);
					SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dTan, 0, NULL);
					SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dVal, 0, NULL);
					SQLExecute(hStmt); SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
				}

				// 3.2 Insert TonKho
				if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
					std::wstring sql = L"INSERT INTO TonKho (MaLoHang, MaKho, SoLuongTanTong) VALUES (?, ?, ?)";
					SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);
					SQLCHAR cLo[51], cKho[51]; strcpy_s((char*)cLo, 51, sMaLo.c_str()); strcpy_s((char*)cKho, 51, sMaKho.c_str());
					SQLDOUBLE dTan = tongTan;
					SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cLo, 0, NULL);
					SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cKho, 0, NULL);
					SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_FLOAT, 0, 0, &dTan, 0, NULL);
					SQLExecute(hStmt); SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
				}

				// 3.3 Insert Chi Tiết (Dùng ngày HSD từ ListHSD)
				for (int i = 0; i < listMaSP->Count; i++) {
					std::string sSP = context.marshal_as<std::string>(listMaSP[i]);
					SQLCHAR cLo[51], cSP[51]; strcpy_s((char*)cLo, 51, sMaLo.c_str()); strcpy_s((char*)cSP, 51, sSP.c_str());
					SQLDOUBLE dSL = listSL[i], dGia = listGia[i], dVol = listSL[i];

					// [MỚI] Lấy ngày từ List người dùng nhập
					DateTime dt = listHSD[i];
					SQL_DATE_STRUCT dHSD = { dt.Year, dt.Month, dt.Day };

					// Insert ChiTietLoHang
					if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
						std::wstring sql = L"INSERT INTO ChiTietLoHang (MaLoHang, MaSanPham, SoLuongTan, HanSuDung, GiaTriSP, TheTich) VALUES (?, ?, ?, ?, ?, ?)";
						SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);
						SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cLo, 0, NULL);
						SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cSP, 0, NULL);
						SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dSL, 0, NULL);
						SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_TYPE_DATE, SQL_DATE, 0, 0, &dHSD, 0, NULL);
						SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dGia, 0, NULL);
						SQLBindParameter(hStmt, 6, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dVol, 0, NULL);
						SQLExecute(hStmt); SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
					}

					// Insert ChiTietTonKho
					if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
						std::wstring sql = L"INSERT INTO ChiTietTonKho (MaLoHang, MaSanPham, SoLuongTan, HanSuDung, GiaTriSanPham, TheTich) VALUES (?, ?, ?, ?, ?, ?)";
						SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);
						SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cLo, 0, NULL);
						SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cSP, 0, NULL);
						SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_FLOAT, 0, 0, &dSL, 0, NULL);
						SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_TYPE_DATE, SQL_DATE, 0, 0, &dHSD, 0, NULL);
						SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &dGia, 0, NULL);
						SQLBindParameter(hStmt, 6, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_FLOAT, 0, 0, &dVol, 0, NULL);
						SQLExecute(hStmt); SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
					}
				}

				SQLEndTran(SQL_HANDLE_DBC, hDbc, SQL_COMMIT);

			}
			catch (Exception^ ex) {
				SQLEndTran(SQL_HANDLE_DBC, hDbc, SQL_ROLLBACK);
				MessageBox::Show(L"Lỗi: " + ex->Message, L"Lỗi Nhập Kho");
			}

			SQLSetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_ON, 0);
			SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
		}
		void UpdateOrderStatus(String^ maDon, String^ trangThaiMoi) {
			SQLHENV hEnv; SQLHDBC hDbc; SQLHSTMT hStmt;
			SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
			SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
			SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
			SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

			if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
				std::wstring sql = L"UPDATE DonDatHang SET TrangThai = ? WHERE MaDonHang = ?";
				SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);

				// [SỬA QUAN TRỌNG] Xử lý chuỗi Unicode cho Trạng Thái Mới
				SQLWCHAR wTrangThai[51];
				pin_ptr<const wchar_t> pStatus = PtrToStringChars(trangThaiMoi);
				wcsncpy_s(wTrangThai, 51, pStatus, _TRUNCATE);

				// Xử lý Mã Đơn (ANSI hay Unicode đều được vì mã thường không dấu, nhưng nên chuẩn)
				std::string sMD = ToStdString(maDon);
				SQLCHAR cMD[51];
				strcpy_s((char*)cMD, 51, sMD.c_str());

				// Bind tham số: Cột 1 là Trạng Thái (Unicode), Cột 2 là Mã Đơn
				SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 50, 0, wTrangThai, 0, NULL);
				SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cMD, 0, NULL);

				SQLExecute(hStmt);
			}
			SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
			SQLDisconnect(hDbc);
			SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
			SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
		}
		// ---------------------------------------------------------
		// 2. Hàm xử lý nút "Danh Sách Tồn" (OnSubStockListClick)
		// ---------------------------------------------------------
		void OnSubShippingClick(Object^ sender, EventArgs^ e) {
			ResetMenuColors();
			btnMenuStock->BackColor = Color::FromArgb(23, 42, 69);

			lblPageTitle->Text = L"🚚 Đơn Hàng Đang Vận Chuyển";
			tabContent->SelectedTab = tabShipping;

			// Load các đơn đang chạy
			LoadShippingData(L"Đang Vận Chuyển");
		}
		void OnSubReceivedClick(Object^ sender, EventArgs^ e) {
			ResetMenuColors();
			btnMenuStock->BackColor = Color::FromArgb(23, 42, 69);

			lblPageTitle->Text = L"📜 Lịch Sử Đơn Hàng Đã Nhận";
			tabContent->SelectedTab = tabShipping; // Vẫn dùng tabShipping nhưng data khác

			// Load các đơn đã xong (và hàm Load sẽ tự ẩn nút bấm)
			LoadShippingData(L"Đã Giao Hàng");
		}
		void ResetMenuColors() {
			Color c = Color::FromArgb(10, 25, 47);
			btnMenuStock->BackColor = c;
			btnMenuRequests->BackColor = c;
			//btnMenuImport->BackColor = c;
			btnMenuPersonal->BackColor = c;
			btnMenuTemp->BackColor = c;
		}

		void btnMenuPersonal_Click(Object^ sender, EventArgs^ e) {
			isPersonalMenuOpen = !isPersonalMenuOpen; pnlSubMenuPersonal->Visible = isPersonalMenuOpen;
			tabContent->SelectedTab = tabInfo; LoadThongTinCaNhan();
			ResetMenuColors(); btnMenuPersonal->BackColor = Color::FromArgb(23, 42, 69);
		}
		void btnMenuTemp_Click(Object^ sender, EventArgs^ e) {
			ResetMenuColors();
			btnMenuTemp->BackColor = Color::FromArgb(23, 42, 69); // Highlight
			lblPageTitle->Text = L"❄️ Quản Lý Nhiệt Độ Kho";
			tabContent->SelectedTab = tabTemp;
		}
		void OnSubInfoClick(Object^ sender, EventArgs^ e) { tabContent->SelectedTab = tabInfo; LoadThongTinCaNhan(); }
		void OnChangePassClick(Object^ sender, EventArgs^ e) { tabContent->SelectedTab = tabChangePass; }
		void OnLogoutClick(Object^ sender, EventArgs^ e) { if (MessageBox::Show(L"Đăng xuất?", L"Xác nhận", MessageBoxButtons::YesNo) == System::Windows::Forms::DialogResult::Yes) this->Close(); }

		void LoadThongTinCaNhan() {
			if (lblInfoID == nullptr) return;
			lblInfoID->Text = this->MaKhoHienTai;
			SQLHENV hEnv = SQL_NULL_HENV; SQLHDBC hDbc = SQL_NULL_HDBC; SQLHSTMT hStmt = SQL_NULL_HSTMT;
			SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv); SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0); SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
			SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
			if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
				std::wstring sql = L"SELECT TenKho, SDT, DiaChi, QuanHuyen, Tinh FROM Kho WHERE MaKho = ?";
				SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);
				std::string sID = ToStdString(this->MaKhoHienTai); SQLCHAR cID[51]; strcpy_s((char*)cID, 51, sID.c_str());
				SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cID, 0, NULL);
				if (SQLExecute(hStmt) == SQL_SUCCESS) {
					if (SQLFetch(hStmt) == SQL_SUCCESS) {
						SQLWCHAR wTen[256], wSDT[50], wDC[256], wQuan[101], wTinh[101]; SQLLEN len;
						SQLGetData(hStmt, 1, SQL_C_WCHAR, wTen, 256, &len); lblInfoName->Text = (len != SQL_NULL_DATA) ? gcnew String(wTen) : L"";
						SQLGetData(hStmt, 2, SQL_C_WCHAR, wSDT, 50, &len); lblInfoPhone->Text = (len != SQL_NULL_DATA) ? gcnew String(wSDT) : L"";
						SQLGetData(hStmt, 3, SQL_C_WCHAR, wDC, 256, &len); String^ dc = (len != SQL_NULL_DATA) ? gcnew String(wDC) : L"";
						SQLGetData(hStmt, 4, SQL_C_WCHAR, wQuan, 101, &len); String^ q = (len != SQL_NULL_DATA) ? gcnew String(wQuan) : L"";
						SQLGetData(hStmt, 5, SQL_C_WCHAR, wTinh, 101, &len); String^ t = (len != SQL_NULL_DATA) ? gcnew String(wTinh) : L"";
						lblInfoFullAddress->Text = dc + L", " + q + L", " + t;
						lblInfoFullAddress->Tag = dc; lblInfoPhone->Tag = q; lblInfoName->Tag = t;
					}
				}
			}
			SQLFreeHandle(SQL_HANDLE_STMT, hStmt); SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
		}

		void OnUpdateInfoClick(Object^ sender, EventArgs^ e) {
			String^ currTen = lblInfoName->Text; String^ currSDT = lblInfoPhone->Text;
			String^ currDC = (lblInfoFullAddress->Tag != nullptr) ? lblInfoFullAddress->Tag->ToString() : "";
			String^ currQuan = (lblInfoPhone->Tag != nullptr) ? lblInfoPhone->Tag->ToString() : "";
			String^ currTinh = (lblInfoName->Tag != nullptr) ? lblInfoName->Tag->ToString() : "";
			UpdateKhoForm^ frm = gcnew UpdateKhoForm(this->MaKhoHienTai, currTen, currSDT, currDC, currQuan, currTinh);
			if (frm->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
				UpdateToSQL(frm->NewTen, frm->NewSDT, frm->NewDiaChi, frm->NewQuan, frm->NewTinh);
				LoadThongTinCaNhan(); MessageBox::Show(L"Cập nhật thành công!", L"Thông báo");
			}
		}

		void UpdateToSQL(String^ ten, String^ sdt, String^ dc, String^ quan, String^ tinh) {
			SQLHENV hEnv = SQL_NULL_HENV; SQLHDBC hDbc = SQL_NULL_HDBC; SQLHSTMT hStmt = SQL_NULL_HSTMT;
			SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv); SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0); SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
			SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
			if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
				std::wstring sql = L"UPDATE Kho SET TenKho=?, SDT=?, DiaChi=?, QuanHuyen=?, Tinh=? WHERE MaKho=?";
				SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);
				pin_ptr<const wchar_t> pTen = PtrToStringChars(ten); pin_ptr<const wchar_t> pDC = PtrToStringChars(dc);
				pin_ptr<const wchar_t> pQuan = PtrToStringChars(quan); pin_ptr<const wchar_t> pTinh = PtrToStringChars(tinh);
				std::string sPhone = ToStdString(sdt); SQLCHAR cPhone[21]; strcpy_s((char*)cPhone, 21, sPhone.c_str());
				std::string sID = ToStdString(this->MaKhoHienTai); SQLCHAR cID[51]; strcpy_s((char*)cID, 51, sID.c_str());
				SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 255, 0, (SQLPOINTER)pTen, 0, NULL);
				SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 20, 0, cPhone, 0, NULL);
				SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 255, 0, (SQLPOINTER)pDC, 0, NULL);
				SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)pQuan, 0, NULL);
				SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)pTinh, 0, NULL);
				SQLBindParameter(hStmt, 6, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cID, 0, NULL);
				SQLExecute(hStmt);
			}
			SQLFreeHandle(SQL_HANDLE_STMT, hStmt); SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
		}

		void UpdateRealUsernameFromDB() {
			SQLHENV hEnv = SQL_NULL_HENV; SQLHDBC hDbc = SQL_NULL_HDBC; SQLHSTMT hStmt = SQL_NULL_HSTMT;
			SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv); SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0); SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
			SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
			std::wstring sql = L"SELECT TenDangNhap FROM TaiKhoan WHERE MaLienKet = ?";
			if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
				SQLPrepare(hStmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);
				std::string sID = ToStdString(this->MaKhoHienTai); SQLCHAR cID[51]; strcpy_s((char*)cID, 51, sID.c_str());
				SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cID, 0, NULL);
				if (SQLExecute(hStmt) == SQL_SUCCESS && SQLFetch(hStmt) == SQL_SUCCESS) {
					SQLCHAR cUser[51]; SQLLEN len; SQLGetData(hStmt, 1, SQL_C_CHAR, cUser, 51, &len);
					if (len != SQL_NULL_DATA) this->CurrentUsername = ToSysString(StdTrim((char*)cUser));
				}
			}
			SQLFreeHandle(SQL_HANDLE_STMT, hStmt); SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
		}

		void OnConfirmChangeUserClick(Object^ sender, EventArgs^ e) {
			String^ newName = txtNewUser->Text->Trim(); String^ currentName = this->CurrentUsername;
			if (String::IsNullOrEmpty(newName) || newName == currentName) { MessageBox::Show(L"Vui lòng nhập tên mới khác tên hiện tại!"); return; }
			SQLHENV hEnv = SQL_NULL_HENV; SQLHDBC hDbc = SQL_NULL_HDBC; SQLHSTMT hStmt = SQL_NULL_HSTMT;
			SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv); SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0); SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
			SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
			if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
				SQLPrepare(hStmt, (SQLWCHAR*)L"SELECT COUNT(*) FROM TaiKhoan WHERE TenDangNhap = ?", SQL_NTS);
				std::string sNew = ToStdString(newName); SQLCHAR cNew[51]; strcpy_s((char*)cNew, 51, sNew.c_str());
				SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cNew, 0, NULL);
				if (SQLExecute(hStmt) == SQL_SUCCESS) { SQLLEN count = 0; SQLFetch(hStmt); SQLGetData(hStmt, 1, SQL_C_LONG, &count, 0, NULL); if (count > 0) { MessageBox::Show(L"Tên đã tồn tại!"); return; } }
			}
			SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
			if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
				SQLPrepare(hStmt, (SQLWCHAR*)L"UPDATE TaiKhoan SET TenDangNhap = ? WHERE MaLienKet = ?", SQL_NTS);
				std::string sNew = ToStdString(newName); SQLCHAR cNew[51]; strcpy_s((char*)cNew, 51, sNew.c_str());
				std::string sID = ToStdString(this->MaKhoHienTai); SQLCHAR cID[51]; strcpy_s((char*)cID, 51, sID.c_str());
				SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cNew, 0, NULL);
				SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cID, 0, NULL);
				if (SQLExecute(hStmt) == SQL_SUCCESS) {
					SQLLEN rows = 0; SQLRowCount(hStmt, &rows);
					if (rows > 0) { MessageBox::Show(L"Đổi tên thành công!"); this->CurrentUsername = newName; txtCurrentUser->Text = newName; txtNewUser->Text = ""; lblUserInfo->Text = newName + L"\n(Quản Lý Kho)"; }
				}
			}
			SQLFreeHandle(SQL_HANDLE_STMT, hStmt); SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
		}

		void OnConfirmChangePassClick(Object^ sender, EventArgs^ e) {
			String^ oldP = txtOldPass->Text; String^ newP = txtNewPass->Text; String^ cfP = txtConfirmPass->Text;
			if (newP != cfP) { MessageBox::Show(L"Mật khẩu không khớp!"); return; }
			SQLHENV hEnv = SQL_NULL_HENV; SQLHDBC hDbc = SQL_NULL_HDBC; SQLHSTMT hStmt = SQL_NULL_HSTMT;
			SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv); SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0); SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
			SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
			bool ok = false; std::string sID = ToStdString(this->MaKhoHienTai); SQLCHAR cID[51]; strcpy_s((char*)cID, 51, sID.c_str());
			if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
				SQLPrepare(hStmt, (SQLWCHAR*)L"SELECT MatKhau FROM TaiKhoan WHERE MaLienKet = ?", SQL_NTS);
				SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cID, 0, NULL);
				if (SQLExecute(hStmt) == SQL_SUCCESS && SQLFetch(hStmt) == SQL_SUCCESS) {
					SQLCHAR dbP[51]; SQLLEN len; SQLGetData(hStmt, 1, SQL_C_CHAR, dbP, 51, &len);
					if (len != SQL_NULL_DATA) { String^ sDbP = gcnew String((char*)dbP); if (sDbP->Trim() == oldP->Trim()) ok = true; }
				}
			}
			SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
			if (!ok) { MessageBox::Show(L"Mật khẩu cũ sai!"); return; }
			if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
				SQLPrepare(hStmt, (SQLWCHAR*)L"UPDATE TaiKhoan SET MatKhau = ? WHERE MaLienKet = ?", SQL_NTS);
				std::string sNewP = ToStdString(newP); SQLCHAR cNewP[51]; strcpy_s((char*)cNewP, 51, sNewP.c_str());
				SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cNewP, 0, NULL);
				SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, cID, 0, NULL);
				if (SQLExecute(hStmt) == SQL_SUCCESS) { MessageBox::Show(L"Đổi mật khẩu thành công!"); txtOldPass->Clear(); txtNewPass->Clear(); txtConfirmPass->Clear(); }
			}
			SQLFreeHandle(SQL_HANDLE_STMT, hStmt); SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
		}

		void OnToggleNewPass(Object^ sender, EventArgs^ e) {
			txtNewPass->UseSystemPasswordChar = !txtNewPass->UseSystemPasswordChar;
			if (imgShowPass != nullptr && imgHidePass != nullptr) {
				btnEyeNew->BackgroundImage = txtNewPass->UseSystemPasswordChar ? imgHidePass : imgShowPass;
			}
		}
		void OnToggleConfirmPass(Object^ sender, EventArgs^ e) {
			txtConfirmPass->UseSystemPasswordChar = !txtConfirmPass->UseSystemPasswordChar;
			if (imgShowPass != nullptr && imgHidePass != nullptr) {
				btnEyeConfirm->BackgroundImage = txtConfirmPass->UseSystemPasswordChar ? imgHidePass : imgShowPass;
			}
		}
		/*
		public:
		static std::string ToStdString(String^ s) { if (String::IsNullOrEmpty(s)) return ""; cli::array<unsigned char>^ bytes = System::Text::Encoding::UTF8->GetBytes(s); pin_ptr<unsigned char> p = &bytes[0]; return std::string((char*)p, bytes->Length); }
		static String^ ToSysString(std::string s) { return gcnew String(s.c_str(), 0, (int)s.length(), System::Text::Encoding::UTF8); }
		static std::string StdTrim(const std::string& str) { size_t first = str.find_first_not_of(' '); if (std::string::npos == first) return str; size_t last = str.find_last_not_of(' '); return str.substr(first, (last - first + 1)); }
		*/
	};
}