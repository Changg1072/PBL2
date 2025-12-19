#pragma once

// --- 1. NHÚNG CÁC HEADER CẦN THIẾT ---
#include "HeThongQuanLy.h"
#include "QuanLyTaiKhoanForm.h"
#include "SaoLuu.h"
#include "QuanLySanPhamForm.h"
#include "QuanLyLuuChuyenForm.h"
#include <msclr\marshal_cppstd.h> 

namespace PBL2QuanLyKho {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Data::Odbc;
	using namespace System::IO; // Bắt buộc có để check File

	public ref class MyForm : public System::Windows::Forms::Form
	{
	public:
		MyForm(void)
		{
			// [SỬA TẠI ĐÂY] 
			// 1. Phải load ảnh vào bộ nhớ trước!
			LoadImageResources();

			// 2. Sau đó mới vẽ giao diện (lúc này biến ảnh đã có dữ liệu để dùng)
			InitializeComponent();

			this->CurrentUsername = "ADMIN";
		}

		// ========================================================================
		// PHẦN 1: CÁC BIẾN LƯU TRỮ
		// ========================================================================
		String^ CurrentRole;
		String^ CurrentLinkedID;
		String^ CurrentUsername;

		// ========================================================================
		// PHẦN 2: LOGIC PHÂN QUYỀN
		// ========================================================================
		void SetUserContext(String^ role, String^ linkedID) {
			this->CurrentRole = role;
			this->CurrentLinkedID = linkedID;
			this->CurrentUsername = (role == "ADMIN" || role == "QUANTRI") ? "ADMIN" : role;

			if (role == "ADMIN") {
				this->Text = L"ADMINISTRATOR - TOÀN QUYỀN HỆ THỐNG";
				lblAppName->Text = L"Quản Trị Viên";
			}
			else {
				this->Text = L"Xin chào, " + role;
			}

			// Cập nhật hiển thị tên vào ô text
			txtCurrentUser->Text = this->CurrentUsername;
		}

	protected:
		~MyForm() { if (components) delete components; }

	private:
		// --- UI COMPONENTS ---
		System::Windows::Forms::TabControl^ tabControl1;
		System::Windows::Forms::TabPage^ tabDashboard;
		System::Windows::Forms::TabPage^ tabAdminInfo;
		System::Windows::Forms::TabPage^ tabAdminPass;
		System::Windows::Forms::TabPage^ tabAccount; // Tab chứa giao diện
		PBL2QuanLyKho::QuanLyTaiKhoanForm^ frmAccountEmbed;

		System::Windows::Forms::TabPage^ tabQLSP; // Tab mới
		PBL2QuanLyKho::QuanLySanPhamForm^ frmQLSPEmbed;

		// --- SIDEBAR ---
		System::Windows::Forms::Panel^ pnlSidebar;
		System::Windows::Forms::Label^ lblAppName;

		// Menu Cá nhân
		System::Windows::Forms::Button^ btnMenuPersonal;
		System::Windows::Forms::Panel^ pnlSubMenuPersonal;
		System::Windows::Forms::Button^ btnSubInfo;
		System::Windows::Forms::Button^ btnSubPass;
		System::Windows::Forms::Button^ btnSubLogout;
		bool isPersonalMenuOpen = false;

		System::Windows::Forms::TabPage^ tabQLLC;
		PBL2QuanLyKho::QuanLyLuuChuyenForm^ frmQLLCEmbed;

		// --- CÁC CONTROL TRONG TAB ADMIN INFO ---
		TextBox^ txtCurrentUser;
		TextBox^ txtNewUser;
		Button^ btnConfirmChangeUser;

		// --- CÁC CONTROL TRONG TAB ADMIN PASS ---
		TextBox^ txtOldPass;
		TextBox^ txtNewPass;
		TextBox^ txtConfirmPass;
		Button^ btnConfirmChangePass;
		Button^ btnEyeNew;
		Button^ btnEyeConfirm;

		// --- [QUAN TRỌNG] BIẾN LƯU ẢNH (GIỐNG WAREHOUSE) ---
		System::Drawing::Image^ imgShowPass;
		System::Drawing::Image^ imgHidePass;

		System::ComponentModel::Container^ components;

		// ========================================================================
		// PHẦN 3: CÁC HÀM TẠO UI (COPY STYLE TỪ WAREHOUSEFORM)
		// ========================================================================

		// 1. Hàm Load Ảnh (Copy y hệt từ WarehouseForm)
		void LoadImageResources() {
			try {
				// ĐƯỜNG DẪN 1: MẮT MỞ (TachMoMat.png)
				String^ pathMo = "D:\\code\\ChanggWithOOP\\PBL2_QuanLyKho\\PBL2_QuanLyKho\\Kho_anh\\TachMoMat.png";

				if (System::IO::File::Exists(pathMo)) {
					imgShowPass = System::Drawing::Image::FromFile(pathMo);
				}
				else {
					MessageBox::Show(L"Không tìm thấy ảnh Mắt Mở:\n" + pathMo);
				}

				// ĐƯỜNG DẪN 2: MẮT NHẮM (TachNhamMat.png)
				String^ pathDong = "D:\\code\\ChanggWithOOP\\PBL2_QuanLyKho\\PBL2_QuanLyKho\\Kho_anh\\TachNhamMat.png";

				if (System::IO::File::Exists(pathDong)) {
					imgHidePass = System::Drawing::Image::FromFile(pathDong);
				}
				else {
					MessageBox::Show(L"Không tìm thấy ảnh Mắt Nhắm:\n" + pathDong);
				}
			}
			catch (Exception^ ex) {
				MessageBox::Show(L"Lỗi load ảnh MyForm: " + ex->Message);
				imgShowPass = nullptr;
				imgHidePass = nullptr;
			}
		}

		Button^ CreateMenuButton(String^ text) {
			Button^ btn = gcnew Button(); btn->Text = text; btn->Height = 60; btn->Dock = DockStyle::Top;
			btn->FlatStyle = FlatStyle::Flat; btn->FlatAppearance->BorderSize = 0;
			btn->TextAlign = ContentAlignment::MiddleLeft; btn->Padding = System::Windows::Forms::Padding(20, 0, 0, 0);
			btn->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, FontStyle::Regular));
			btn->Cursor = Cursors::Hand;
			btn->BackColor = Color::FromArgb(10, 25, 47); btn->ForeColor = Color::FromArgb(204, 214, 246);
			return btn;
		}

		Button^ CreateSubMenuButton(String^ text, Color txtColor) {
			Button^ btn = gcnew Button(); btn->Text = text; btn->Height = 45; btn->Dock = DockStyle::Top;
			btn->FlatStyle = FlatStyle::Flat; btn->FlatAppearance->BorderSize = 0;
			btn->TextAlign = ContentAlignment::MiddleLeft; btn->Padding = System::Windows::Forms::Padding(40, 0, 0, 0);
			btn->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, FontStyle::Regular));
			btn->BackColor = Color::FromArgb(15, 30, 50); btn->ForeColor = txtColor;
			btn->Cursor = Cursors::Hand; return btn;
		}

		GroupBox^ CreateGroupBox(String^ title, int x, int y) {
			GroupBox^ g = gcnew GroupBox(); g->Text = title;
			g->Font = (gcnew System::Drawing::Font(L"Segoe UI", 14, FontStyle::Bold));
			g->Size = System::Drawing::Size(450, 450); g->Location = Point(x, y);
			g->ForeColor = Color::DimGray; return g;
		}

		TextBox^ CreateInput(GroupBox^ g, String^ lbl, int y, bool isPass) {
			Label^ l = gcnew Label(); l->Text = lbl; l->Location = Point(35, y); l->AutoSize = true;
			l->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10));
			TextBox^ t = gcnew TextBox(); t->Location = Point(35, y + 30); t->Size = System::Drawing::Size(320, 30);
			t->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11));
			t->BackColor = Color::FromArgb(240, 242, 245); t->BorderStyle = BorderStyle::None;
			if (isPass) t->UseSystemPasswordChar = true;
			Label^ line = gcnew Label(); line->BorderStyle = BorderStyle::Fixed3D;
			line->Size = System::Drawing::Size(320, 2); line->Location = Point(35, y + 65);
			g->Controls->Add(l); g->Controls->Add(t); g->Controls->Add(line); return t;
		}

		Button^ CreateButton(GroupBox^ g, String^ txt, int y, Color c) {
			Button^ b = gcnew Button(); b->Text = txt; b->BackColor = c; b->ForeColor = Color::White;
			b->Size = System::Drawing::Size(320, 45); b->Location = Point(35, y); b->FlatStyle = FlatStyle::Flat;
			g->Controls->Add(b); return b;
		}

		// 2. Hàm Tạo Nút Mắt (Copy logic từ WarehouseForm)
		Button^ CreateEyeBtn(GroupBox^ g, int y) {
			Button^ b = gcnew Button();
			b->Size = System::Drawing::Size(35, 30);
			b->Location = Point(355, y);
			b->FlatStyle = FlatStyle::Flat;
			b->FlatAppearance->BorderSize = 0;
			b->BackColor = Color::FromArgb(240, 242, 245);

			// [THÊM DÒNG NÀY] Luôn set chế độ Zoom để ảnh không bị vỡ/mất
			b->BackgroundImageLayout = ImageLayout::Zoom;

			// Logic gán ảnh
			if (imgHidePass != nullptr) {
				b->BackgroundImage = imgHidePass;
				b->Text = L"";
			}
			else {
				b->Text = L"👁"; // Hiện tạm chữ nếu ảnh lỗi
			}

			g->Controls->Add(b);
			return b;
		}

		void InitializeComponent(void)
		{
			this->components = gcnew System::ComponentModel::Container();
			this->Size = System::Drawing::Size(1280, 720);
			this->Text = L"Hệ Thống Quản Lý Chuỗi Cung Ứng Lạnh - ADMIN";
			this->WindowState = FormWindowState::Maximized;
			this->StartPosition = FormStartPosition::CenterScreen;

			// ========================================================================
			// 1. SIDEBAR
			// ========================================================================
			this->pnlSidebar = (gcnew System::Windows::Forms::Panel());
			this->pnlSidebar->Dock = System::Windows::Forms::DockStyle::Left;
			this->pnlSidebar->Width = 260;
			this->pnlSidebar->BackColor = Color::FromArgb(10, 25, 47);

			// --- Label Logo ---
			this->lblAppName = (gcnew System::Windows::Forms::Label());
			this->lblAppName->Text = L"Quản Trị Viên";
			this->lblAppName->Dock = DockStyle::Top;
			this->lblAppName->Height = 100;
			this->lblAppName->TextAlign = ContentAlignment::MiddleCenter;
			this->lblAppName->Font = (gcnew System::Drawing::Font(L"Segoe UI", 20, FontStyle::Bold));
			this->lblAppName->ForeColor = Color::FromArgb(100, 255, 218);

			// --- CÁC NÚT MENU ---

			// Nút Tài Khoản
			System::Windows::Forms::Button^ btnManageAccount = CreateMenuButton(L"👥  Quản Lý Tài Khoản");
			btnManageAccount->Click += gcnew EventHandler(this, &MyForm::OnManageAccountClick);

			// Nút Sản Phẩm
			System::Windows::Forms::Button^ btnMenuSP = CreateMenuButton(L"📦  Quản Lý Sản Phẩm");
			btnMenuSP->Click += gcnew EventHandler(this, &MyForm::OnMenuSPClick);

			// --- [MỚI] NÚT QUẢN LÝ LƯU CHUYỂN ---
			System::Windows::Forms::Button^ btnMenuLC = CreateMenuButton(L"🚚  Quản Lý Lưu Chuyển");
			btnMenuLC->Click += gcnew EventHandler(this, &MyForm::OnMenuLCClick);

			// Nút Cá Nhân
			this->btnMenuPersonal = CreateMenuButton(L"👤  Cá Nhân");
			this->btnMenuPersonal->Click += gcnew EventHandler(this, &MyForm::btnMenuPersonal_Click);

			// --- SubMenu Cá Nhân ---
			this->pnlSubMenuPersonal = (gcnew System::Windows::Forms::Panel());
			this->pnlSubMenuPersonal->Dock = DockStyle::Top;
			this->pnlSubMenuPersonal->Height = 135;
			this->pnlSubMenuPersonal->Visible = false;
			this->pnlSubMenuPersonal->BackColor = Color::FromArgb(15, 30, 50);

			this->btnSubLogout = CreateSubMenuButton(L"•  Đăng xuất", Color::FromArgb(238, 77, 45));
			this->btnSubLogout->Click += gcnew EventHandler(this, &MyForm::OnLogoutClick);
			this->btnSubPass = CreateSubMenuButton(L"•  Đổi mật khẩu", Color::WhiteSmoke);
			this->btnSubPass->Click += gcnew EventHandler(this, &MyForm::OnMenuPassClick);
			this->btnSubInfo = CreateSubMenuButton(L"•  Thông tin tài khoản", Color::WhiteSmoke);
			this->btnSubInfo->Click += gcnew EventHandler(this, &MyForm::OnMenuInfoClick);

			this->pnlSubMenuPersonal->Controls->Add(this->btnSubLogout);
			this->pnlSubMenuPersonal->Controls->Add(this->btnSubPass);
			this->pnlSubMenuPersonal->Controls->Add(this->btnSubInfo);

			// --- Add vào Sidebar (Dock Top: Add sau cùng nằm trên cùng) ---
			this->pnlSidebar->Controls->Add(this->pnlSubMenuPersonal);
			this->pnlSidebar->Controls->Add(this->btnMenuPersonal);

			// [MỚI] Thêm nút Lưu Chuyển vào menu
			this->pnlSidebar->Controls->Add(btnMenuLC);

			this->pnlSidebar->Controls->Add(btnMenuSP);
			this->pnlSidebar->Controls->Add(btnManageAccount);
			this->pnlSidebar->Controls->Add(this->lblAppName);

			// ========================================================================
			// 2. TAB CONTROL
			// ========================================================================
			this->tabControl1 = (gcnew System::Windows::Forms::TabControl());
			this->tabControl1->Dock = System::Windows::Forms::DockStyle::Fill;
			this->tabControl1->Appearance = TabAppearance::FlatButtons;
			this->tabControl1->ItemSize = System::Drawing::Size(0, 1);
			this->tabControl1->SizeMode = TabSizeMode::Fixed;

			// Tab 1: Dashboard
			this->tabDashboard = (gcnew System::Windows::Forms::TabPage());
			this->tabDashboard->Text = L"Trang Chủ";
			this->tabDashboard->BackColor = Color::White;
			Label^ lblDemo = gcnew Label();
			lblDemo->Text = L"Chào mừng đến với trang Quản Trị";
			lblDemo->Font = (gcnew System::Drawing::Font(L"Segoe UI", 20));
			lblDemo->AutoSize = true;
			lblDemo->Location = Point(50, 50);
			this->tabDashboard->Controls->Add(lblDemo);

			// Tab 2 & 3: Info & Pass
			this->tabAdminInfo = (gcnew System::Windows::Forms::TabPage());
			this->tabAdminInfo->BackColor = Color::FromArgb(240, 242, 245);
			InitializeTabAdminInfo();

			this->tabAdminPass = (gcnew System::Windows::Forms::TabPage());
			this->tabAdminPass->BackColor = Color::FromArgb(240, 242, 245);
			InitializeTabAdminPass();

			// Tab 4: Sản Phẩm
			this->tabQLSP = (gcnew System::Windows::Forms::TabPage());
			this->tabQLSP->Text = L"Quản Lý Sản Phẩm";
			this->tabQLSP->BackColor = System::Drawing::Color::White;
			this->frmQLSPEmbed = (gcnew PBL2QuanLyKho::QuanLySanPhamForm());
			this->frmQLSPEmbed->TopLevel = false;
			this->frmQLSPEmbed->FormBorderStyle = System::Windows::Forms::FormBorderStyle::None;
			this->frmQLSPEmbed->Dock = System::Windows::Forms::DockStyle::Fill;
			this->frmQLSPEmbed->Visible = true;
			this->tabQLSP->Controls->Add(this->frmQLSPEmbed);

			// Tab 5: Tài Khoản
			this->tabAccount = (gcnew System::Windows::Forms::TabPage());
			this->tabAccount->Text = L"Quản Lý Tài Khoản";
			this->tabAccount->BackColor = System::Drawing::Color::White;
			this->frmAccountEmbed = (gcnew PBL2QuanLyKho::QuanLyTaiKhoanForm());
			this->frmAccountEmbed->TopLevel = false;
			this->frmAccountEmbed->FormBorderStyle = System::Windows::Forms::FormBorderStyle::None;
			this->frmAccountEmbed->Dock = System::Windows::Forms::DockStyle::Fill;
			this->frmAccountEmbed->Visible = true;
			this->tabAccount->Controls->Add(this->frmAccountEmbed);

			// --- [MỚI] TAB 6: QUẢN LÝ LƯU CHUYỂN ---
			this->tabQLLC = (gcnew System::Windows::Forms::TabPage());
			this->tabQLLC->Text = L"Quản Lý Lưu Chuyển";
			this->tabQLLC->BackColor = System::Drawing::Color::White;

			// Nhúng Form
			this->frmQLLCEmbed = (gcnew PBL2QuanLyKho::QuanLyLuuChuyenForm());
			this->frmQLLCEmbed->TopLevel = false;
			this->frmQLLCEmbed->FormBorderStyle = System::Windows::Forms::FormBorderStyle::None;
			this->frmQLLCEmbed->Dock = System::Windows::Forms::DockStyle::Fill;
			this->frmQLLCEmbed->Visible = true;

			this->tabQLLC->Controls->Add(this->frmQLLCEmbed);

			// --- Add All Tabs ---
			this->tabControl1->Controls->Add(this->tabDashboard);
			this->tabControl1->Controls->Add(this->tabAdminInfo);
			this->tabControl1->Controls->Add(this->tabAdminPass);
			this->tabControl1->Controls->Add(this->tabQLSP);
			this->tabControl1->Controls->Add(this->tabAccount);
			this->tabControl1->Controls->Add(this->tabQLLC); // [MỚI] Add tab LC vào control

			this->Controls->Add(this->tabControl1);
			this->Controls->Add(this->pnlSidebar);
		}
		void InitializeTabAdminInfo() {
			int startX = 350; int startY = 100;
			GroupBox^ grpUser = CreateGroupBox(L" CẬP NHẬT TÊN HIỂN THỊ ", startX, startY);

			txtCurrentUser = CreateInput(grpUser, L"Tên hiện tại:", 60, false);
			txtCurrentUser->ReadOnly = true;
			txtCurrentUser->Text = this->CurrentUsername;

			txtNewUser = CreateInput(grpUser, L"Tên mới mong muốn:", 160, false);

			btnConfirmChangeUser = CreateButton(grpUser, L"LƯU THAY ĐỔI", 280, Color::FromArgb(0, 114, 188));
			btnConfirmChangeUser->Click += gcnew EventHandler(this, &MyForm::OnConfirmChangeUserClick);

			tabAdminInfo->Controls->Add(grpUser);
		}

		void InitializeTabAdminPass() {
			int startX = 350; int startY = 80;
			GroupBox^ grpPass = CreateGroupBox(L" ĐỔI MẬT KHẨU QUẢN TRỊ ", startX, startY);

			txtOldPass = CreateInput(grpPass, L"Mật khẩu cũ:", 50, true);
			txtNewPass = CreateInput(grpPass, L"Mật khẩu mới:", 140, true);
			txtConfirmPass = CreateInput(grpPass, L"Nhập lại mật khẩu mới:", 230, true);

			btnConfirmChangePass = CreateButton(grpPass, L"XÁC NHẬN ĐỔI", 340, Color::SeaGreen);
			btnConfirmChangePass->Click += gcnew EventHandler(this, &MyForm::OnConfirmChangePassClick);

			// Tạo nút mắt
			btnEyeNew = CreateEyeBtn(grpPass, 170);
			btnEyeNew->Click += gcnew EventHandler(this, &MyForm::OnToggleNewPass);

			btnEyeConfirm = CreateEyeBtn(grpPass, 260);
			btnEyeConfirm->Click += gcnew EventHandler(this, &MyForm::OnToggleConfirmPass);

			tabAdminPass->Controls->Add(grpPass);
		}

		// ========================================================================
		// PHẦN 5: XỬ LÝ SỰ KIỆN
		// ========================================================================

		void btnMenuPersonal_Click(Object^ sender, EventArgs^ e) {
			isPersonalMenuOpen = !isPersonalMenuOpen;
			pnlSubMenuPersonal->Visible = isPersonalMenuOpen;
			if (isPersonalMenuOpen) btnMenuPersonal->BackColor = Color::FromArgb(23, 42, 69);
			else btnMenuPersonal->BackColor = Color::FromArgb(10, 25, 47);
		}

		void OnMenuInfoClick(Object^ sender, EventArgs^ e) {
			tabControl1->SelectedTab = tabAdminInfo;
		}

		void OnMenuPassClick(Object^ sender, EventArgs^ e) {
			tabControl1->SelectedTab = tabAdminPass;
		}

		void OnMenuSPClick(Object^ sender, EventArgs^ e) {
			tabControl1->SelectedTab = tabQLSP;
		}
		void OnMenuLCClick(Object^ sender, EventArgs^ e) {
			tabControl1->SelectedTab = tabQLLC;
		}
		void OnLogoutClick(Object^ sender, EventArgs^ e) {
			if (MessageBox::Show(L"Bạn chắc chắn muốn đăng xuất?", L"Xác nhận", MessageBoxButtons::YesNo, MessageBoxIcon::Question) == System::Windows::Forms::DialogResult::Yes) {
				this->Close();
			}
		}
		void OnManageAccountClick(Object^ sender, EventArgs^ e) {
			// Kiểm tra quyền (Admin mới được vào)
			if (this->CurrentRole == "ADMIN" || this->CurrentRole == "QUANTRI") {

				// Thay vì bật cửa sổ, ta chuyển người dùng đến Tab vừa nhúng
				tabControl1->SelectedTab = tabAccount;

			}
			else {
				MessageBox::Show(L"Bạn không có quyền truy cập mục này!", L"Cảnh báo");
			}
		}
		// --- LOGIC ĐỔI TÊN ---
		void OnConfirmChangeUserClick(Object^ sender, EventArgs^ e) {
			String^ newName = txtNewUser->Text->Trim();
			String^ currentName = this->CurrentUsername;

			if (String::IsNullOrEmpty(newName) || newName == currentName) {
				MessageBox::Show(L"Vui lòng nhập tên mới khác tên hiện tại!"); return;
			}

			if (ExecuteSQL("SELECT COUNT(*) FROM TaiKhoan WHERE TenDangNhap = '" + newName + "'", true)) {
				MessageBox::Show(L"Tên đăng nhập này đã tồn tại!"); return;
			}

			String^ sql = "UPDATE TaiKhoan SET TenDangNhap = '" + newName + "' WHERE TenDangNhap = '" + currentName + "'";

			if (ExecuteSQL(sql, false)) {
				MessageBox::Show(L"Đổi tên thành công!");
				this->CurrentUsername = newName;
				txtCurrentUser->Text = newName;
				txtNewUser->Text = "";
			}
		}

		// --- LOGIC ĐỔI PASS ---
		void OnConfirmChangePassClick(Object^ sender, EventArgs^ e) {
			String^ oldP = txtOldPass->Text;
			String^ newP = txtNewPass->Text;
			String^ cfP = txtConfirmPass->Text;

			if (String::IsNullOrEmpty(oldP) || String::IsNullOrEmpty(newP)) {
				MessageBox::Show(L"Vui lòng nhập đầy đủ!"); return;
			}
			if (newP != cfP) { MessageBox::Show(L"Mật khẩu mới không khớp!"); return; }

			if (!CheckOldPass(oldP)) {
				MessageBox::Show(L"Mật khẩu cũ không đúng!", L"Lỗi", MessageBoxButtons::OK, MessageBoxIcon::Error);
				return;
			}

			String^ sql = "UPDATE TaiKhoan SET MatKhau = '" + newP + "' WHERE TenDangNhap = '" + this->CurrentUsername + "'";
			if (ExecuteSQL(sql, false)) {
				MessageBox::Show(L"Đổi mật khẩu thành công!");
				txtOldPass->Clear(); txtNewPass->Clear(); txtConfirmPass->Clear();
			}
		}

		// --- 3. LOGIC ẨN/HIỆN PASS (Copy từ WarehouseForm) ---
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

		// --- HÀM SQL HELPER ---
		bool ExecuteSQL(String^ sql, bool isCheckCount) {
			SQLHENV hEnv; SQLHDBC hDbc; SQLHSTMT hStmt;
			bool result = false;
			SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
			SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
			SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
			SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

			if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
				pin_ptr<const wchar_t> wch = PtrToStringChars(sql);
				if (SQLExecDirect(hStmt, (SQLWCHAR*)wch, SQL_NTS) == SQL_SUCCESS) {
					if (isCheckCount) {
						SQLLEN count = 0;
						if (SQLFetch(hStmt) == SQL_SUCCESS) {
							SQLGetData(hStmt, 1, SQL_C_LONG, &count, 0, NULL);
							if (count > 0) result = true;
						}
					}
					else result = true;
				}
			}
			SQLFreeHandle(SQL_HANDLE_STMT, hStmt); SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
			return result;
		}

		bool CheckOldPass(String^ oldPassInput) {
			SQLHENV hEnv; SQLHDBC hDbc; SQLHSTMT hStmt;
			bool isCorrect = false;
			SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
			SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
			SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
			SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

			if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
				String^ query = "SELECT MatKhau FROM TaiKhoan WHERE TenDangNhap = '" + this->CurrentUsername + "'";
				pin_ptr<const wchar_t> wch = PtrToStringChars(query);
				if (SQLExecDirect(hStmt, (SQLWCHAR*)wch, SQL_NTS) == SQL_SUCCESS) {
					if (SQLFetch(hStmt) == SQL_SUCCESS) {
						SQLCHAR dbPass[51]; SQLGetData(hStmt, 1, SQL_C_CHAR, dbPass, 51, NULL);
						String^ sDB = gcnew String((char*)dbPass);
						if (sDB->Trim() == oldPassInput->Trim()) isCorrect = true;
					}
				}
			}
			SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
			return isCorrect;
		}
	};
}