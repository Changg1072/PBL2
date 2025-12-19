#pragma once
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <string>
#include <msclr/marshal_cppstd.h>

namespace PBL2QuanLyKho {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	public ref class AdminProfileForm : public System::Windows::Forms::Form
	{
	public:
		String^ CurrentUser; // Tên đăng nhập hiện tại

		AdminProfileForm(String^ user)
		{
			this->CurrentUser = user;
			InitializeComponent();

			// Setup giao diện ban đầu
			txtCurrentUser->Text = this->CurrentUser;
			this->Text = L"Hồ Sơ Quản Trị Viên - " + user;
		}

	protected:
		~AdminProfileForm()
		{
			if (components) delete components;
		}

	private:
		System::Windows::Forms::TabControl^ tabControl1;
		System::Windows::Forms::TabPage^ tabInfo;
		System::Windows::Forms::TabPage^ tabSecurity;

		// Tab Info
		System::Windows::Forms::GroupBox^ grpInfo;
		System::Windows::Forms::Label^ label1;
		System::Windows::Forms::TextBox^ txtCurrentUser;
		System::Windows::Forms::Label^ label2;
		System::Windows::Forms::TextBox^ txtNewUser;
		System::Windows::Forms::Button^ btnSaveInfo;

		// Tab Security
		System::Windows::Forms::GroupBox^ grpPass;
		System::Windows::Forms::Label^ label3;
		System::Windows::Forms::TextBox^ txtOldPass;
		System::Windows::Forms::Label^ label4;
		System::Windows::Forms::TextBox^ txtNewPass;
		System::Windows::Forms::Label^ label5;
		System::Windows::Forms::TextBox^ txtConfirmPass;
		System::Windows::Forms::Button^ btnChangePass;
		System::Windows::Forms::CheckBox^ chkShowPass; // Nút hiện mật khẩu

		System::ComponentModel::Container^ components;

		void InitializeComponent(void)
		{
			this->tabControl1 = (gcnew System::Windows::Forms::TabControl());
			this->tabInfo = (gcnew System::Windows::Forms::TabPage());
			this->tabSecurity = (gcnew System::Windows::Forms::TabPage());
			this->grpInfo = (gcnew System::Windows::Forms::GroupBox());
			this->grpPass = (gcnew System::Windows::Forms::GroupBox());

			// Form Setup
			this->Size = System::Drawing::Size(500, 400);
			this->StartPosition = FormStartPosition::CenterParent;
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			this->MaximizeBox = false;

			// Tab Control
			this->tabControl1->Dock = DockStyle::Fill;
			this->tabControl1->Controls->Add(this->tabInfo);
			this->tabControl1->Controls->Add(this->tabSecurity);

			// --- TAB 1: THÔNG TIN ---
			this->tabInfo->Text = L"Thông tin cá nhân";
			this->tabInfo->Padding = System::Windows::Forms::Padding(3);
			this->tabInfo->BackColor = Color::White;

			this->grpInfo->Text = L"Cập nhật tên hiển thị";
			this->grpInfo->Location = Point(20, 20);
			this->grpInfo->Size = System::Drawing::Size(430, 250);
			this->grpInfo->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, FontStyle::Bold));

			// Label & Textbox User cũ
			this->label1 = gcnew Label(); this->label1->Text = L"Tên hiện tại:"; this->label1->Location = Point(20, 40); this->label1->AutoSize = true; this->label1->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9));
			this->txtCurrentUser = gcnew TextBox(); this->txtCurrentUser->Location = Point(20, 65); this->txtCurrentUser->Size = System::Drawing::Size(380, 25); this->txtCurrentUser->ReadOnly = true;

			// Label & Textbox User mới
			this->label2 = gcnew Label(); this->label2->Text = L"Tên mới mong muốn:"; this->label2->Location = Point(20, 110); this->label2->AutoSize = true; this->label2->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9));
			this->txtNewUser = gcnew TextBox(); this->txtNewUser->Location = Point(20, 135); this->txtNewUser->Size = System::Drawing::Size(380, 25);

			// Button Save
			this->btnSaveInfo = gcnew Button(); this->btnSaveInfo->Text = L"Lưu Thay Đổi"; this->btnSaveInfo->Location = Point(20, 190); this->btnSaveInfo->Size = System::Drawing::Size(380, 40);
			this->btnSaveInfo->BackColor = Color::FromArgb(0, 114, 188); this->btnSaveInfo->ForeColor = Color::White; this->btnSaveInfo->FlatStyle = FlatStyle::Flat;
			this->btnSaveInfo->Click += gcnew System::EventHandler(this, &AdminProfileForm::btnSaveInfo_Click);

			this->grpInfo->Controls->Add(this->label1); this->grpInfo->Controls->Add(this->txtCurrentUser);
			this->grpInfo->Controls->Add(this->label2); this->grpInfo->Controls->Add(this->txtNewUser);
			this->grpInfo->Controls->Add(this->btnSaveInfo);
			this->tabInfo->Controls->Add(this->grpInfo);


			// --- TAB 2: BẢO MẬT ---
			this->tabSecurity->Text = L"Đổi mật khẩu";
			this->tabSecurity->Padding = System::Windows::Forms::Padding(3);
			this->tabSecurity->BackColor = Color::White;

			this->grpPass->Text = L"Bảo mật tài khoản";
			this->grpPass->Location = Point(20, 20);
			this->grpPass->Size = System::Drawing::Size(430, 300);
			this->grpPass->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, FontStyle::Bold));

			// Mật khẩu cũ
			this->label3 = gcnew Label(); this->label3->Text = L"Mật khẩu cũ:"; this->label3->Location = Point(20, 30); this->label3->AutoSize = true; this->label3->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9));
			this->txtOldPass = gcnew TextBox(); this->txtOldPass->Location = Point(20, 50); this->txtOldPass->Size = System::Drawing::Size(380, 25); this->txtOldPass->UseSystemPasswordChar = true;

			// Mật khẩu mới
			this->label4 = gcnew Label(); this->label4->Text = L"Mật khẩu mới:"; this->label4->Location = Point(20, 90); this->label4->AutoSize = true; this->label4->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9));
			this->txtNewPass = gcnew TextBox(); this->txtNewPass->Location = Point(20, 110); this->txtNewPass->Size = System::Drawing::Size(380, 25); this->txtNewPass->UseSystemPasswordChar = true;

			// Nhập lại
			this->label5 = gcnew Label(); this->label5->Text = L"Nhập lại mật khẩu mới:"; this->label5->Location = Point(20, 150); this->label5->AutoSize = true; this->label5->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9));
			this->txtConfirmPass = gcnew TextBox(); this->txtConfirmPass->Location = Point(20, 170); this->txtConfirmPass->Size = System::Drawing::Size(380, 25); this->txtConfirmPass->UseSystemPasswordChar = true;

			// Checkbox hiện pass
			this->chkShowPass = gcnew CheckBox(); this->chkShowPass->Text = L"Hiện mật khẩu"; this->chkShowPass->Location = Point(20, 205); this->chkShowPass->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9));
			this->chkShowPass->CheckedChanged += gcnew EventHandler(this, &AdminProfileForm::OnTogglePass);

			// Button Change
			this->btnChangePass = gcnew Button(); this->btnChangePass->Text = L"Xác Nhận Đổi"; this->btnChangePass->Location = Point(20, 240); this->btnChangePass->Size = System::Drawing::Size(380, 40);
			this->btnChangePass->BackColor = Color::SeaGreen; this->btnChangePass->ForeColor = Color::White; this->btnChangePass->FlatStyle = FlatStyle::Flat;
			this->btnChangePass->Click += gcnew System::EventHandler(this, &AdminProfileForm::btnChangePass_Click);

			this->grpPass->Controls->Add(this->label3); this->grpPass->Controls->Add(this->txtOldPass);
			this->grpPass->Controls->Add(this->label4); this->grpPass->Controls->Add(this->txtNewPass);
			this->grpPass->Controls->Add(this->label5); this->grpPass->Controls->Add(this->txtConfirmPass);
			this->grpPass->Controls->Add(this->chkShowPass); this->grpPass->Controls->Add(this->btnChangePass);
			this->tabSecurity->Controls->Add(this->grpPass);

			this->Controls->Add(this->tabControl1);
		}

		// --- LOGIC HIỆN PASS ---
		void OnTogglePass(Object^ sender, EventArgs^ e) {
			bool show = !chkShowPass->Checked; // Ngược lại vì UseSystemPasswordChar = true là ẩn
			txtOldPass->UseSystemPasswordChar = show;
			txtNewPass->UseSystemPasswordChar = show;
			txtConfirmPass->UseSystemPasswordChar = show;
		}

		// --- LOGIC SQL ĐỔI TÊN ---
		void btnSaveInfo_Click(System::Object^ sender, System::EventArgs^ e) {
			String^ newUser = txtNewUser->Text->Trim();
			if (String::IsNullOrEmpty(newUser)) { MessageBox::Show(L"Vui lòng nhập tên mới!"); return; }
			if (newUser == this->CurrentUser) { MessageBox::Show(L"Tên mới phải khác tên cũ!"); return; }

			// Check xem tên đã tồn tại chưa
			if (ExecuteSQL("SELECT COUNT(*) FROM TaiKhoan WHERE TenDangNhap = '" + newUser + "'", true)) {
				MessageBox::Show(L"Tên đăng nhập này đã tồn tại!");
				return;
			}

			// Update
			String^ sql = "UPDATE TaiKhoan SET TenDangNhap = '" + newUser + "' WHERE TenDangNhap = '" + this->CurrentUser + "'";
			if (ExecuteSQL(sql, false)) {
				MessageBox::Show(L"Đổi tên thành công! Vui lòng đăng nhập lại để cập nhật.");
				this->CurrentUser = newUser;
				this->DialogResult = System::Windows::Forms::DialogResult::OK; // Báo cho Form cha biết đã OK
				this->Close();
			}
		}

		// --- LOGIC SQL ĐỔI PASS ---
		void btnChangePass_Click(System::Object^ sender, System::EventArgs^ e) {
			String^ oldP = txtOldPass->Text;
			String^ newP = txtNewPass->Text;
			String^ cfP = txtConfirmPass->Text;

			if (String::IsNullOrEmpty(oldP) || String::IsNullOrEmpty(newP)) { MessageBox::Show(L"Vui lòng nhập đủ thông tin!"); return; }
			if (newP != cfP) { MessageBox::Show(L"Mật khẩu xác nhận không khớp!"); return; }

			// 1. Kiểm tra pass cũ có đúng không
			if (!CheckOldPass(oldP)) {
				MessageBox::Show(L"Mật khẩu cũ không chính xác!", L"Lỗi", MessageBoxButtons::OK, MessageBoxIcon::Error);
				return;
			}

			// 2. Cập nhật pass mới
			String^ sql = "UPDATE TaiKhoan SET MatKhau = '" + newP + "' WHERE TenDangNhap = '" + this->CurrentUser + "'";
			if (ExecuteSQL(sql, false)) {
				MessageBox::Show(L"Đổi mật khẩu thành công!");
				txtOldPass->Clear(); txtNewPass->Clear(); txtConfirmPass->Clear();
			}
		}

		// --- HELPER SQL ---
		// Hàm trả về True nếu Update thành công hoặc Count > 0
		bool ExecuteSQL(String^ sql, bool isCheckCount) {
			SQLHENV hEnv; SQLHDBC hDbc; SQLHSTMT hStmt;
			bool result = false;
			SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
			SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
			SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
			SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

			if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
				// Convert String^ to std::wstring for SQL
				pin_ptr<const wchar_t> wch = PtrToStringChars(sql);
				if (SQLExecute(hStmt) == SQL_SUCCESS || SQLExecDirect(hStmt, (SQLWCHAR*)wch, SQL_NTS) == SQL_SUCCESS) {
					if (isCheckCount) {
						// Nếu đang check Count (xem tên tồn tại chưa)
						SQLLEN count = 0;
						if (SQLFetch(hStmt) == SQL_SUCCESS) {
							SQLGetData(hStmt, 1, SQL_C_LONG, &count, 0, NULL);
							if (count > 0) result = true;
						}
					}
					else {
						// Nếu là Update
						result = true;
					}
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
				String^ query = "SELECT MatKhau FROM TaiKhoan WHERE TenDangNhap = '" + this->CurrentUser + "'";
				pin_ptr<const wchar_t> wch = PtrToStringChars(query);

				if (SQLExecDirect(hStmt, (SQLWCHAR*)wch, SQL_NTS) == SQL_SUCCESS) {
					if (SQLFetch(hStmt) == SQL_SUCCESS) {
						SQLCHAR dbPass[51];
						SQLGetData(hStmt, 1, SQL_C_CHAR, dbPass, 51, NULL);

						// Trim chuỗi từ DB để so sánh
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