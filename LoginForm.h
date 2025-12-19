#pragma once
#include "HeThongQuanLy.h"
#include "MyForm.h"          // Để gọi Form Kho
#include "SupermarketForm.h"
#include "DinhGiaDong.h"
#include "WarehouseForm.h"
#include <msclr\marshal_cppstd.h>

namespace PBL2QuanLyKho {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Drawing::Drawing2D;
	using namespace System::Data::Odbc;
	using namespace System::IO; // Thư viện để kiểm tra file ảnh

	public ref class LoginForm : public System::Windows::Forms::Form
	{
	public:
		String^ UserRole;
		String^ LinkedID;
		bool IsAuthenticated = false;

		LoginForm(void)
		{
			InitializeComponent();
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::None;
			this->StartPosition = FormStartPosition::CenterScreen;
			this->DoubleBuffered = true;
		}

	protected:
		~LoginForm()
		{
			if (components) delete components;
		}

	private:
		System::Windows::Forms::Panel^ panelCard;
		System::Windows::Forms::Label^ lblTitle;
		System::Windows::Forms::Label^ lblUser;
		System::Windows::Forms::TextBox^ txtUser;
		System::Windows::Forms::Label^ lblPass;
		System::Windows::Forms::TextBox^ txtPass;
		System::Windows::Forms::Button^ btnLogin;
		System::Windows::Forms::Label^ lblClose;

		// [THAY ĐỔI] Dùng PictureBox thay cho Label
		System::Windows::Forms::PictureBox^ pbTogglePass;
		bool isPassVisible = false;

		System::ComponentModel::Container^ components;

#pragma region Windows Form Designer generated code
		void InitializeComponent(void)
		{
			this->panelCard = (gcnew System::Windows::Forms::Panel());
			this->btnLogin = (gcnew System::Windows::Forms::Button());
			this->txtPass = (gcnew System::Windows::Forms::TextBox());
			this->lblPass = (gcnew System::Windows::Forms::Label());
			this->txtUser = (gcnew System::Windows::Forms::TextBox());
			this->lblUser = (gcnew System::Windows::Forms::Label());
			this->lblTitle = (gcnew System::Windows::Forms::Label());
			this->lblClose = (gcnew System::Windows::Forms::Label());

			// Khởi tạo PictureBox cho con mắt
			this->pbTogglePass = (gcnew System::Windows::Forms::PictureBox());

			this->panelCard->SuspendLayout();
			((System::ComponentModel::ISupportInitialize^)(this->pbTogglePass))->BeginInit();
			this->SuspendLayout();

			// Main Form Setup
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(800, 500);
			this->Controls->Add(this->lblClose);
			this->Controls->Add(this->panelCard);
			this->Name = L"LoginForm";
			this->Text = L"LoginForm";
			this->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &LoginForm::LoginForm_Paint);

			// Nút Tắt (X)
			this->lblClose->AutoSize = true;
			this->lblClose->BackColor = System::Drawing::Color::Transparent;
			this->lblClose->Cursor = System::Windows::Forms::Cursors::Hand;
			this->lblClose->Font = (gcnew System::Drawing::Font(L"Arial", 15.75F, System::Drawing::FontStyle::Bold));
			this->lblClose->ForeColor = System::Drawing::Color::White;
			this->lblClose->Location = System::Drawing::Point(760, 10);
			this->lblClose->Text = L"X";
			this->lblClose->Click += gcnew System::EventHandler(this, &LoginForm::lblClose_Click);

			// Panel Trắng
			this->panelCard->BackColor = System::Drawing::Color::White;
			this->panelCard->Controls->Add(this->pbTogglePass); // Thêm ảnh vào đây
			this->panelCard->Controls->Add(this->btnLogin);
			this->panelCard->Controls->Add(this->txtPass);
			this->panelCard->Controls->Add(this->lblPass);
			this->panelCard->Controls->Add(this->txtUser);
			this->panelCard->Controls->Add(this->lblUser);
			this->panelCard->Controls->Add(this->lblTitle);
			this->panelCard->Location = System::Drawing::Point(250, 75);
			this->panelCard->Size = System::Drawing::Size(300, 350);
			this->panelCard->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &LoginForm::panelCard_Paint);

			// Title
			this->lblTitle->AutoSize = true;
			this->lblTitle->Font = (gcnew System::Drawing::Font(L"Segoe UI", 20.25F, System::Drawing::FontStyle::Bold));
			this->lblTitle->ForeColor = System::Drawing::Color::RoyalBlue;
			this->lblTitle->Location = System::Drawing::Point(75, 30);
			this->lblTitle->Text = L"Đăng Nhập";

			// User Input
			this->lblUser->AutoSize = true;
			this->lblUser->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
			this->lblUser->ForeColor = System::Drawing::Color::Gray;
			this->lblUser->Location = System::Drawing::Point(30, 90);
			this->lblUser->Text = L"Tài khoản";

			this->txtUser->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F));
			this->txtUser->Location = System::Drawing::Point(33, 110);
			this->txtUser->Size = System::Drawing::Size(235, 27);

			// Pass Input
			this->lblPass->AutoSize = true;
			this->lblPass->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
			this->lblPass->ForeColor = System::Drawing::Color::Gray;
			this->lblPass->Location = System::Drawing::Point(30, 160);
			this->lblPass->Text = L"Mật khẩu";

			this->txtPass->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F));
			this->txtPass->Location = System::Drawing::Point(33, 180);
			this->txtPass->Size = System::Drawing::Size(205, 27);
			this->txtPass->PasswordChar = '*'; // Mặc định ẩn
			this->txtPass->TabIndex = 4;

			// [MỚI] Cấu hình PictureBox Con Mắt
			this->pbTogglePass->Cursor = System::Windows::Forms::Cursors::Hand;
			this->pbTogglePass->Location = System::Drawing::Point(245, 182); // Vị trí bên phải ô pass
			this->pbTogglePass->Name = L"pbTogglePass";
			this->pbTogglePass->Size = System::Drawing::Size(24, 24); // Kích thước icon
			this->pbTogglePass->BackColor = System::Drawing::Color::Red; // Tô đỏ để dễ thấy
			this->pbTogglePass->BringToFront();
			this->pbTogglePass->SizeMode = System::Windows::Forms::PictureBoxSizeMode::Zoom; // Co giãn ảnh cho vừa
			this->pbTogglePass->Click += gcnew System::EventHandler(this, &LoginForm::pbTogglePass_Click);

			// Load ảnh mặc định (Mắt gạch chéo - Đang ẩn)
			LoadEyeImage("D:\\code\\ChanggWithOOP\\PBL2_QuanLyKho\\PBL2_QuanLyKho\\Kho_anh\\NhamMat.jpg");

			// Login Button
			this->btnLogin->BackColor = System::Drawing::Color::RoyalBlue;
			this->btnLogin->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->btnLogin->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold));
			this->btnLogin->ForeColor = System::Drawing::Color::White;
			this->btnLogin->Location = System::Drawing::Point(33, 240);
			this->btnLogin->Size = System::Drawing::Size(235, 40);
			this->btnLogin->Text = L"Đăng Nhập";
			this->btnLogin->Click += gcnew System::EventHandler(this, &LoginForm::btnLogin_Click);

			this->panelCard->ResumeLayout(false);
			this->panelCard->PerformLayout();
			((System::ComponentModel::ISupportInitialize^)(this->pbTogglePass))->EndInit();
			this->ResumeLayout(false);
			this->PerformLayout();
		}
#pragma endregion

		// --- HÀM HỖ TRỢ LOAD ẢNH AN TOÀN ---
		void LoadEyeImage(String^ path) {
			// 1. Kiểm tra file có tồn tại không
			if (System::IO::File::Exists(path)) {
				try {
					// Nếu có file -> Load ảnh và xóa màu nền đỏ test
					this->pbTogglePass->Image = Image::FromFile(path);
					this->pbTogglePass->BackColor = System::Drawing::Color::Transparent;
				}
				catch (Exception^ ex) {
					MessageBox::Show(L"Tìm thấy file nhưng file bị lỗi!\n" + ex->Message);
				}
			}
			else {
				// 2. Nếu không thấy file -> Báo lỗi chi tiết
				String^ msg = L"Không tìm thấy file ảnh!\n\n" +
					L"Đường dẫn máy đang tìm: \n" + path +
					L"\n\nHãy kiểm tra kỹ:\n" +
					L"- Đuôi file là .png hay .jpg?\n" +
					L"- Có bị trùng đuôi (.png.png) không?";
				MessageBox::Show(msg, L"Lỗi đường dẫn", MessageBoxButtons::OK, MessageBoxIcon::Error);
			}
		}
		// --- SỰ KIỆN BẤM VÀO ẢNH CON MẮT ---
	private: System::Void pbTogglePass_Click(System::Object^ sender, System::EventArgs^ e) {
		if (isPassVisible) {
			// Đang hiện -> Chuyển sang Ẩn
			txtPass->PasswordChar = '*';
			LoadEyeImage("D:\\code\\ChanggWithOOP\\PBL2_QuanLyKho\\PBL2_QuanLyKho\\Kho_anh\\NhamMat.jpg"); // Hiện icon mắt gạch chéo
			isPassVisible = false;
		}
		else {
			// Đang ẩn -> Chuyển sang Hiện
			txtPass->PasswordChar = '\0';
			LoadEyeImage("D:\\code\\ChanggWithOOP\\PBL2_QuanLyKho\\PBL2_QuanLyKho\\Kho_anh\\MoMat.jpg"); // Hiện icon mắt mở to
			isPassVisible = true;
		}
	}

	private: System::Void LoginForm_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e) {
		LinearGradientBrush^ brush = gcnew LinearGradientBrush(this->ClientRectangle, Color::FromArgb(100, 200, 255), Color::FromArgb(30, 80, 200), 45.0F);
		e->Graphics->FillRectangle(brush, this->ClientRectangle);
	}

	private: System::Void panelCard_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e) {
		int cornerRadius = 20;
		GraphicsPath^ path = gcnew GraphicsPath();
		path->AddArc(0, 0, cornerRadius, cornerRadius, 180, 90);
		path->AddArc(panelCard->Width - cornerRadius, 0, cornerRadius, cornerRadius, 270, 90);
		path->AddArc(panelCard->Width - cornerRadius, panelCard->Height - cornerRadius, cornerRadius, cornerRadius, 0, 90);
		path->AddArc(0, panelCard->Height - cornerRadius, cornerRadius, cornerRadius, 90, 90);
		path->CloseAllFigures();
		panelCard->Region = gcnew System::Drawing::Region(path);
	}

	private: System::Void lblClose_Click(System::Object^ sender, System::EventArgs^ e) {
		Application::Exit();
	}

	private: System::Void btnLogin_Click(System::Object^ sender, System::EventArgs^ e) {
		String^ user = txtUser->Text;
		String^ pass = txtPass->Text;

		if (String::IsNullOrWhiteSpace(user) || String::IsNullOrWhiteSpace(pass)) {
			MessageBox::Show(L"Vui lòng nhập đầy đủ!", L"Cảnh báo", MessageBoxButtons::OK, MessageBoxIcon::Warning);
			return;
		}

		String^ connStr = "Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;";
		OdbcConnection^ conn = gcnew OdbcConnection(connStr);

		try {
			conn->Open();
			// Truy vấn lấy Quyền hạn và Mã liên kết
			String^ sql = "SELECT QuyenHan, MaLienKet FROM TaiKhoan WHERE RTRIM(TenDangNhap) = ? AND RTRIM(MatKhau) = ?";
			OdbcCommand^ cmd = gcnew OdbcCommand(sql, conn);

			// [ĐÃ SỬA] Chỉ Add tham số 1 lần và Trim() ngay tại đây
			cmd->Parameters->AddWithValue("user", user->Trim());
			cmd->Parameters->AddWithValue("pass", pass->Trim());

			OdbcDataReader^ reader = cmd->ExecuteReader();

			if (reader->Read()) {
				// Lấy thông tin từ Database
				String^ role = reader["QuyenHan"]->ToString()->Trim(); // Trim để xóa khoảng trắng thừa trong DB
				String^ linkedID = "";
				if (!reader->IsDBNull(1)) linkedID = reader["MaLienKet"]->ToString()->Trim();

				MessageBox::Show(L"Đăng nhập thành công! Xin chào " + role, L"Thông báo");

				// --- ĐOẠN NÀY CHẠY ĐỊNH GIÁ TỰ ĐỘNG (GIỮ NGUYÊN) ---
				SQLHENV hEnv = SQL_NULL_HENV;
				SQLHDBC hDbcNative = SQL_NULL_HDBC;
				SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
				SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
				SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbcNative);
				SQLWCHAR* connString = (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;";
				SQLDriverConnect(hDbcNative, NULL, connString, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
				DinhGiaDong::TuDongChayDinhGiaHomNay(hDbcNative);
				SQLDisconnect(hDbcNative);
				SQLFreeHandle(SQL_HANDLE_DBC, hDbcNative);
				SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
				// ----------------------------------------------------

				this->Hide(); // Ẩn form đăng nhập

				// === [PHẦN PHÂN QUYỀN MỚI] ===

				// 1. DÀNH CHO ADMIN
				if (role == "ADMIN" || role == "QUANTRI") {
					// Mở Form Quản Lý Chung (MyForm) với quyền cao nhất
					PBL2QuanLyKho::MyForm^ formAdmin = gcnew PBL2QuanLyKho::MyForm();
					// Set context là ADMIN, LinkedID có thể để "ALL" hoặc rỗng
					formAdmin->SetUserContext("ADMIN", "ALL");
					formAdmin->ShowDialog();
				}
				// 2. DÀNH CHO SIÊU THỊ
				else if (role == "SIEUTHI") {
					PBL2QuanLyKho::SupermarketForm^ formST = gcnew PBL2QuanLyKho::SupermarketForm(user);
					formST->ShowDialog();
				}
				// 3. DÀNH CHO KHO
				else if (role == "KHO") {
					PBL2QuanLyKho::WarehouseForm^ formKho = gcnew PBL2QuanLyKho::WarehouseForm(user, linkedID);
					formKho->ShowDialog();
				}
				// 4. TRƯỜNG HỢP KHÁC (Mặc định)
				else {
					PBL2QuanLyKho::MyForm^ formDefault = gcnew PBL2QuanLyKho::MyForm();
					formDefault->SetUserContext(role, linkedID);
					formDefault->ShowDialog();
				}

				this->Close(); // Đóng hẳn form login sau khi form con tắt
			}
			else {
				MessageBox::Show(L"Sai tài khoản hoặc mật khẩu!", L"Lỗi", MessageBoxButtons::OK, MessageBoxIcon::Error);
				this->txtPass->Text = "";
				this->txtPass->Focus();
			}
			conn->Close();
		}
		catch (Exception^ ex) {
			MessageBox::Show("Lỗi kết nối: " + ex->Message);
		}
	}
	};
}