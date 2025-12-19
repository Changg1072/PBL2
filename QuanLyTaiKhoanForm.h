#pragma once
#include "HeThongQuanLy.h"
#include "VietNamData.h"
#include <msclr\marshal_cppstd.h>

namespace PBL2QuanLyKho {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Collections::Generic;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Data::Odbc;
	using namespace System::IO;

	public ref class QuanLyTaiKhoanForm : public System::Windows::Forms::Form
	{
	public:
		QuanLyTaiKhoanForm(void)
		{
			LoadVietnamData();
			LoadImageResources();
			InitializeComponent();
			FillTinhToComboBox();
			LoadData("");
		}

	protected:
		~QuanLyTaiKhoanForm() { if (components) delete components; }

	private:
		// --- UI COMPONENTS ---
		Panel^ pnlTop; Panel^ pnlRight; DataGridView^ dgvTaiKhoan;
		ComboBox^ cbbFilter;
		TextBox^ txtUser; TextBox^ txtPass; Button^ btnEye; ComboBox^ cbbRole;
		Label^ lblLink; ComboBox^ cbbLinkID; CheckBox^ chkCreateNew;

		// --- PANEL TẠO MỚI ---
		Panel^ pnlNewEntity;
		TextBox^ txtNewID; TextBox^ txtNewName;
		ComboBox^ cbbTinh; ComboBox^ cbbHuyen;
		TextBox^ txtDiaChi; TextBox^ txtSDT;

		// [KHO] Sức chứa & Nhiệt độ
		Label^ lblSucChua; TextBox^ txtSucChua;

		Label^ lblTempMin; TextBox^ txtTempMin; TrackBar^ tbTempMin; // Thanh trượt Min
		Label^ lblTempMax; TextBox^ txtTempMax; TrackBar^ tbTempMax; // Thanh trượt Max

		// [SIÊU THỊ] Mức phạt
		Label^ lblMucPhat; TextBox^ txtMucPhat;

		Button^ btnAdd; Button^ btnUpdate; Button^ btnDelete; Button^ btnReset;

		Image^ imgShow; Image^ imgHide; bool isPassVisible = false;
		String^ connStr = "Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;";
		Dictionary<String^, List<String^>^>^ vnData;
		System::ComponentModel::Container^ components;

		void LoadImageResources() {
			try {
				String^ pathMo = "D:\\code\\ChanggWithOOP\\PBL2_QuanLyKho\\PBL2_QuanLyKho\\Kho_anh\\TachMoMat.png";
				String^ pathDong = "D:\\code\\ChanggWithOOP\\PBL2_QuanLyKho\\PBL2_QuanLyKho\\Kho_anh\\TachNhamMat.png";
				if (File::Exists(pathMo)) imgShow = Image::FromFile(pathMo);
				if (File::Exists(pathDong)) imgHide = Image::FromFile(pathDong);
			}
			catch (...) {}
		}

		void LoadVietnamData() { vnData = VietnamData::GetFullData(); }
		void FillTinhToComboBox() {
			cbbTinh->Items->Clear();
			if (vnData != nullptr) for each (String ^ tinh in vnData->Keys) cbbTinh->Items->Add(tinh);
		}

#pragma region Windows Form Designer generated code
		void InitializeComponent(void)
		{
			this->Size = System::Drawing::Size(1350, 800);
			this->Text = L"Quản Lý Tài Khoản & Đơn Vị";
			this->BackColor = Color::White;

			// 1. TOP BAR
			pnlTop = gcnew Panel(); pnlTop->Dock = DockStyle::Top; pnlTop->Height = 50; pnlTop->BackColor = Color::AliceBlue;
			this->Controls->Add(pnlTop);

			Label^ lblF = gcnew Label(); lblF->Text = L"Lọc theo quyền:"; lblF->Location = Point(20, 15); lblF->AutoSize = true;
			lblF->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, FontStyle::Bold));
			pnlTop->Controls->Add(lblF);

			cbbFilter = gcnew ComboBox(); cbbFilter->Location = Point(140, 12); cbbFilter->Size = System::Drawing::Size(150, 25);
			cbbFilter->DropDownStyle = ComboBoxStyle::DropDownList;
			cbbFilter->Items->AddRange(gcnew cli::array<Object^>{L"Tất cả", L"ADMIN", L"KHO", L"SIEUTHI"});
			cbbFilter->SelectedIndex = 0;
			cbbFilter->SelectedIndexChanged += gcnew EventHandler(this, &QuanLyTaiKhoanForm::cbbFilter_SelectedIndexChanged);
			pnlTop->Controls->Add(cbbFilter);

			// 2. RIGHT PANEL
			pnlRight = gcnew Panel(); pnlRight->Dock = DockStyle::Right; pnlRight->Width = 460;
			pnlRight->BackColor = Color::WhiteSmoke; pnlRight->Padding = System::Windows::Forms::Padding(10);
			this->Controls->Add(pnlRight);

			GroupBox^ grp = gcnew GroupBox(); grp->Text = L"Thông tin chi tiết"; grp->Dock = DockStyle::Fill;
			grp->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11, FontStyle::Regular));
			pnlRight->Controls->Add(grp);

			// -- Inputs --
			CreateLabel(grp, L"Tên Đăng Nhập:", 20, 30);
			txtUser = CreateTextBox(grp, 20, 55, 390);

			CreateLabel(grp, L"Mật Khẩu:", 20, 90);
			txtPass = CreateTextBox(grp, 20, 115, 340); txtPass->UseSystemPasswordChar = true;
			btnEye = gcnew Button(); btnEye->Location = Point(370, 114); btnEye->Size = System::Drawing::Size(40, 30);
			btnEye->FlatStyle = FlatStyle::Flat; btnEye->FlatAppearance->BorderSize = 0; btnEye->BackgroundImageLayout = ImageLayout::Zoom;
			if (imgHide) btnEye->BackgroundImage = imgHide;
			btnEye->Click += gcnew EventHandler(this, &QuanLyTaiKhoanForm::btnEye_Click);
			grp->Controls->Add(btnEye);

			CreateLabel(grp, L"Quyền Hạn:", 20, 150);
			cbbRole = gcnew ComboBox(); cbbRole->Location = Point(20, 175); cbbRole->Size = System::Drawing::Size(390, 30);
			cbbRole->DropDownStyle = ComboBoxStyle::DropDownList;
			cbbRole->Items->AddRange(gcnew cli::array<Object^>{L"ADMIN", L"KHO", L"SIEUTHI"});
			cbbRole->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11));
			cbbRole->SelectedIndexChanged += gcnew EventHandler(this, &QuanLyTaiKhoanForm::cbbRole_SelectedIndexChanged);
			grp->Controls->Add(cbbRole);

			// -- Liên kết --
			lblLink = gcnew Label(); lblLink->Text = L"Liên Kết Đơn Vị:"; lblLink->Location = Point(20, 215);
			lblLink->AutoSize = true; lblLink->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10)); lblLink->ForeColor = Color::DimGray;
			grp->Controls->Add(lblLink);

			cbbLinkID = gcnew ComboBox(); cbbLinkID->Location = Point(20, 240); cbbLinkID->Size = System::Drawing::Size(390, 30);
			cbbLinkID->DropDownStyle = ComboBoxStyle::DropDownList; cbbLinkID->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11));
			grp->Controls->Add(cbbLinkID);

			chkCreateNew = gcnew CheckBox(); chkCreateNew->Text = L"➕ Tạo Kho/Siêu Thị Mới";
			chkCreateNew->Location = Point(20, 275); chkCreateNew->AutoSize = true;
			chkCreateNew->ForeColor = Color::Blue; chkCreateNew->Visible = false;
			chkCreateNew->CheckedChanged += gcnew EventHandler(this, &QuanLyTaiKhoanForm::chkCreateNew_CheckedChanged);
			grp->Controls->Add(chkCreateNew);

			// -- PANEL TẠO MỚI (MỞ RỘNG CAO HƠN ĐỂ CHỨA THANH TRƯỢT) --
			pnlNewEntity = gcnew Panel(); pnlNewEntity->Location = Point(15, 305);
			pnlNewEntity->Size = System::Drawing::Size(420, 360); // Tăng chiều cao lên 360
			pnlNewEntity->BorderStyle = BorderStyle::FixedSingle; pnlNewEntity->BackColor = Color::FromArgb(240, 255, 240);
			pnlNewEntity->Visible = false;
			grp->Controls->Add(pnlNewEntity);

			Label^ lblTitleNew = gcnew Label(); lblTitleNew->Text = L"Nhập thông tin đơn vị mới:";
			lblTitleNew->Location = Point(5, 5); lblTitleNew->AutoSize = true; lblTitleNew->ForeColor = Color::SeaGreen;
			lblTitleNew->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, FontStyle::Regular));
			pnlNewEntity->Controls->Add(lblTitleNew);

			CreateLabelMin(pnlNewEntity, L"Mã (KHO/ST):", 5, 30);
			txtNewID = CreateTextMin(pnlNewEntity, 5, 50, 100);

			CreateLabelMin(pnlNewEntity, L"Tên Hiển Thị:", 120, 30);
			txtNewName = CreateTextMin(pnlNewEntity, 120, 50, 280);

			CreateLabelMin(pnlNewEntity, L"Tỉnh/TP:", 5, 85);
			cbbTinh = gcnew ComboBox(); cbbTinh->Location = Point(5, 105); cbbTinh->Size = System::Drawing::Size(180, 25);
			cbbTinh->DropDownStyle = ComboBoxStyle::DropDownList;
			cbbTinh->SelectedIndexChanged += gcnew EventHandler(this, &QuanLyTaiKhoanForm::cbbTinh_SelectedIndexChanged);
			pnlNewEntity->Controls->Add(cbbTinh);

			CreateLabelMin(pnlNewEntity, L"Quận/Huyện:", 200, 85);
			cbbHuyen = gcnew ComboBox(); cbbHuyen->Location = Point(200, 105); cbbHuyen->Size = System::Drawing::Size(200, 25);
			cbbHuyen->DropDownStyle = ComboBoxStyle::DropDownList;
			pnlNewEntity->Controls->Add(cbbHuyen);

			CreateLabelMin(pnlNewEntity, L"Địa Chỉ Cụ Thể:", 5, 135);
			txtDiaChi = CreateTextMin(pnlNewEntity, 5, 155, 395);

			CreateLabelMin(pnlNewEntity, L"Số Điện Thoại:", 5, 185);
			txtSDT = CreateTextMin(pnlNewEntity, 5, 205, 150);

			// === [CẬP NHẬT] THANH TRƯỢT NHIỆT ĐỘ ===
			lblSucChua = CreateLabelMin(pnlNewEntity, L"Sức Chứa (Tấn):", 200, 185);
			txtSucChua = CreateTextMin(pnlNewEntity, 200, 205, 100);

			// 1. Nhiệt độ Min
			lblTempMin = CreateLabelMin(pnlNewEntity, L"Nhiệt Độ Min (°C):", 5, 255);
			txtTempMin = CreateTextMin(pnlNewEntity, 120, 252, 60); txtTempMin->Text = "-10";
			txtTempMin->TextChanged += gcnew EventHandler(this, &QuanLyTaiKhoanForm::OnTextTempMinChanged);

			tbTempMin = gcnew TrackBar();
			tbTempMin->Location = Point(5, 280); tbTempMin->Size = System::Drawing::Size(180, 45);
			tbTempMin->Minimum = -50; tbTempMin->Maximum = 50; tbTempMin->Value = -10; // Mặc định -10 độ
			tbTempMin->TickFrequency = 10;
			tbTempMin->Scroll += gcnew EventHandler(this, &QuanLyTaiKhoanForm::OnTrackMinScroll);
			pnlNewEntity->Controls->Add(tbTempMin);

			// 2. Nhiệt độ Max
			lblTempMax = CreateLabelMin(pnlNewEntity, L"Nhiệt Độ Max (°C):", 220, 255);
			txtTempMax = CreateTextMin(pnlNewEntity, 335, 252, 60); txtTempMax->Text = "20";
			txtTempMax->TextChanged += gcnew EventHandler(this, &QuanLyTaiKhoanForm::OnTextTempMaxChanged);

			tbTempMax = gcnew TrackBar();
			tbTempMax->Location = Point(220, 280); tbTempMax->Size = System::Drawing::Size(180, 45);
			tbTempMax->Minimum = -50; tbTempMax->Maximum = 50; tbTempMax->Value = 20; // Mặc định 20 độ
			tbTempMax->TickFrequency = 10;
			tbTempMax->Scroll += gcnew EventHandler(this, &QuanLyTaiKhoanForm::OnTrackMaxScroll);
			pnlNewEntity->Controls->Add(tbTempMax);

			// 3. Mức phạt (Siêu thị)
			lblMucPhat = CreateLabelMin(pnlNewEntity, L"Mức Phạt Thiếu (VNĐ):", 5, 255);
			txtMucPhat = CreateTextMin(pnlNewEntity, 5, 280, 250);

			// Ẩn mặc định
			lblSucChua->Visible = false; txtSucChua->Visible = false;
			lblTempMin->Visible = false; txtTempMin->Visible = false; tbTempMin->Visible = false;
			lblTempMax->Visible = false; txtTempMax->Visible = false; tbTempMax->Visible = false;
			lblMucPhat->Visible = false; txtMucPhat->Visible = false;

			// -- Buttons --
			int btnY = 675; // Đẩy nút xuống dưới cùng
			btnAdd = CreateButton(grp, L"LƯU & TẠO MỚI", 20, btnY, Color::SeaGreen);
			btnAdd->Click += gcnew EventHandler(this, &QuanLyTaiKhoanForm::btnAdd_Click);

			btnUpdate = CreateButton(grp, L"CẬP NHẬT", 230, btnY, Color::Orange);
			btnUpdate->Click += gcnew EventHandler(this, &QuanLyTaiKhoanForm::btnUpdate_Click);

			btnDelete = CreateButton(grp, L"XÓA TK", 20, btnY + 50, Color::Firebrick);
			btnDelete->Click += gcnew EventHandler(this, &QuanLyTaiKhoanForm::btnDelete_Click);

			btnReset = CreateButton(grp, L"LÀM MỚI", 230, btnY + 50, Color::Gray);
			btnReset->Click += gcnew EventHandler(this, &QuanLyTaiKhoanForm::btnReset_Click);

			// 3. GRID
			dgvTaiKhoan = gcnew DataGridView(); dgvTaiKhoan->Dock = DockStyle::Fill;
			dgvTaiKhoan->BackgroundColor = Color::White; dgvTaiKhoan->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;
			dgvTaiKhoan->SelectionMode = DataGridViewSelectionMode::FullRowSelect; dgvTaiKhoan->ReadOnly = true;
			dgvTaiKhoan->ColumnHeadersHeight = 40; dgvTaiKhoan->RowTemplate->Height = 35;
			dgvTaiKhoan->CellClick += gcnew DataGridViewCellEventHandler(this, &QuanLyTaiKhoanForm::dgvTaiKhoan_CellClick);
			dgvTaiKhoan->CellFormatting += gcnew DataGridViewCellFormattingEventHandler(this, &QuanLyTaiKhoanForm::dgvTaiKhoan_CellFormatting);
			this->Controls->Add(dgvTaiKhoan);

			this->pnlTop->SendToBack(); this->pnlRight->SendToBack(); this->pnlTop->SendToBack();
			this->dgvTaiKhoan->BringToFront();
		}
#pragma endregion

		// --- HÀM HỖ TRỢ TẠO UI ---
		Label^ CreateLabel(GroupBox^ g, String^ t, int x, int y) {
			Label^ l = gcnew Label(); l->Text = t; l->Location = Point(x, y); l->AutoSize = true;
			l->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9)); g->Controls->Add(l); return l;
		}
		TextBox^ CreateTextBox(GroupBox^ g, int x, int y, int w) {
			TextBox^ t = gcnew TextBox(); t->Location = Point(x, y); t->Size = System::Drawing::Size(w, 29);
			t->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11)); g->Controls->Add(t); return t;
		}
		Label^ CreateLabelMin(Panel^ p, String^ t, int x, int y) {
			Label^ l = gcnew Label(); l->Text = t; l->Location = Point(x, y); l->AutoSize = true;
			l->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8)); p->Controls->Add(l); return l;
		}
		TextBox^ CreateTextMin(Panel^ p, int x, int y, int w) {
			TextBox^ t = gcnew TextBox(); t->Location = Point(x, y); t->Size = System::Drawing::Size(w, 25);
			p->Controls->Add(t); return t;
		}
		Button^ CreateButton(GroupBox^ g, String^ t, int x, int y, Color c) {
			Button^ b = gcnew Button(); b->Text = t; b->Location = Point(x, y); b->Size = System::Drawing::Size(180, 40);
			b->BackColor = c; b->ForeColor = Color::White; b->FlatStyle = FlatStyle::Flat;
			b->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9, FontStyle::Bold)); g->Controls->Add(b); return b;
		}

		// --- SỰ KIỆN THANH TRƯỢT NHIỆT ĐỘ ---
		void OnTrackMinScroll(Object^ sender, EventArgs^ e) { txtTempMin->Text = tbTempMin->Value.ToString(); }
		void OnTrackMaxScroll(Object^ sender, EventArgs^ e) { txtTempMax->Text = tbTempMax->Value.ToString(); }

		// Sự kiện ngược: Nhập số -> Thanh trượt chạy theo
		void OnTextTempMinChanged(Object^ sender, EventArgs^ e) {
			try {
				int val = Convert::ToInt32(txtTempMin->Text);
				if (val >= tbTempMin->Minimum && val <= tbTempMin->Maximum) tbTempMin->Value = val;
			}
			catch (...) {}
		}
		void OnTextTempMaxChanged(Object^ sender, EventArgs^ e) {
			try {
				int val = Convert::ToInt32(txtTempMax->Text);
				if (val >= tbTempMax->Minimum && val <= tbTempMax->Maximum) tbTempMax->Value = val;
			}
			catch (...) {}
		}

		// --- LOGIC CHÍNH ---
		void LoadData(String^ filter) {
			OdbcConnection^ conn = gcnew OdbcConnection(connStr);
			try {
				conn->Open();
				String^ sql = "SELECT TenDangNhap, MatKhau, QuyenHan, MaLienKet FROM TaiKhoan";
				if (filter != "" && filter != "Tất cả") sql += " WHERE QuyenHan = '" + filter + "'";
				OdbcDataAdapter^ da = gcnew OdbcDataAdapter(sql, conn);
				DataTable^ dt = gcnew DataTable(); da->Fill(dt);
				dgvTaiKhoan->DataSource = dt;
			}
			catch (...) {}
			finally { conn->Close(); }
		}

		void cbbTinh_SelectedIndexChanged(Object^ sender, EventArgs^ e) {
			cbbHuyen->Items->Clear(); cbbHuyen->Text = "";
			if (cbbTinh->SelectedItem == nullptr) return;
			String^ tinh = cbbTinh->SelectedItem->ToString();
			if (vnData->ContainsKey(tinh)) for each (String ^ h in vnData[tinh]) cbbHuyen->Items->Add(h);
			cbbHuyen->SelectedIndex = 0;
		}

		void cbbRole_SelectedIndexChanged(Object^ sender, EventArgs^ e) {
			if (cbbRole->SelectedItem == nullptr) {
				lblLink->Visible = false; cbbLinkID->Visible = false; chkCreateNew->Visible = false; pnlNewEntity->Visible = false; return;
			}
			String^ role = cbbRole->SelectedItem->ToString();
			if (role == "ADMIN") {
				lblLink->Visible = false; cbbLinkID->Visible = false; chkCreateNew->Visible = false; pnlNewEntity->Visible = false; cbbLinkID->Items->Clear();
			}
			else {
				lblLink->Visible = true; cbbLinkID->Visible = true; chkCreateNew->Visible = true;
				chkCreateNew->Checked = false; pnlNewEntity->Visible = false; cbbLinkID->Enabled = true;
				LoadLinkIDs(role);
			}
		}

		void LoadLinkIDs(String^ role) {
			cbbLinkID->Items->Clear(); cbbLinkID->Text = "";
			String^ table = (role == "KHO") ? "Kho" : "SieuThi";
			String^ colID = (role == "KHO") ? "MaKho" : "MaSieuThi";
			String^ colName = (role == "KHO") ? "TenKho" : "TenSieuThi";
			OdbcConnection^ conn = gcnew OdbcConnection(connStr);
			try {
				conn->Open(); OdbcCommand^ cmd = gcnew OdbcCommand("SELECT " + colID + ", " + colName + " FROM " + table, conn);
				OdbcDataReader^ rd = cmd->ExecuteReader();
				bool hasData = false;
				while (rd->Read()) { cbbLinkID->Items->Add(rd[0]->ToString() + " - " + rd[1]->ToString()); hasData = true; }
				if (!hasData) cbbLinkID->Items->Add(L"(Trống - Hãy tạo mới)");
			}
			catch (...) {}
			finally { conn->Close(); }
		}

		// [ĐÃ CẬP NHẬT] Ẩn hiện thanh trượt theo quyền
		void chkCreateNew_CheckedChanged(Object^ sender, EventArgs^ e) {
			bool isNew = chkCreateNew->Checked;
			if (isNew && cbbRole->SelectedItem == nullptr) {
				chkCreateNew->Checked = false; MessageBox::Show(L"Chọn Quyền trước!"); return;
			}

			pnlNewEntity->Visible = isNew; cbbLinkID->Enabled = !isNew;

			if (isNew) {
				String^ role = cbbRole->SelectedItem->ToString();
				txtNewID->Text = (role == "KHO") ? "KHO" : "ST";

				bool isKho = (role == "KHO");

				// KHO: Hiện thanh trượt
				lblSucChua->Visible = isKho; txtSucChua->Visible = isKho;
				lblTempMin->Visible = isKho; txtTempMin->Visible = isKho; tbTempMin->Visible = isKho;
				lblTempMax->Visible = isKho; txtTempMax->Visible = isKho; tbTempMax->Visible = isKho;

				// ST: Hiện mức phạt
				lblMucPhat->Visible = !isKho; txtMucPhat->Visible = !isKho;
			}
		}

		void btnAdd_Click(Object^ sender, EventArgs^ e) {
			String^ u = txtUser->Text; String^ p = txtPass->Text; String^ r = cbbRole->Text;
			if (u == "" || p == "" || r == "") { MessageBox::Show(L"Thiếu thông tin!"); return; }
			if (r == "ADMIN") { InsertAccount(u, p, r, ""); return; }

			String^ linkID = "";
			if (chkCreateNew->Checked) {
				String^ newID = txtNewID->Text; String^ newName = txtNewName->Text;
				if (newID == "" || newName == "") { MessageBox::Show(L"Nhập Mã và Tên!"); return; }
				if (InsertNewEntity(r, newID, newName)) linkID = newID; else return;
			}
			else {
				if (cbbLinkID->SelectedItem == nullptr || cbbLinkID->Text->Contains(L"(Trống")) { MessageBox::Show(L"Chọn liên kết!"); return; }
				linkID = cbbLinkID->SelectedItem->ToString()->Split('-')[0]->Trim();
			}
			InsertAccount(u, p, r, linkID);
		}

		bool InsertNewEntity(String^ role, String^ id, String^ name) {
			String^ tinh = cbbTinh->Text; String^ huyen = cbbHuyen->Text; String^ dc = txtDiaChi->Text; String^ sdt = txtSDT->Text;
			String^ sql = "";

			String^ sucChua = (txtSucChua->Text != "") ? txtSucChua->Text : "0";
			String^ tMin = (txtTempMin->Text != "") ? txtTempMin->Text : "0";
			String^ tMax = (txtTempMax->Text != "") ? txtTempMax->Text : "0";
			String^ phat = (txtMucPhat->Text != "") ? txtMucPhat->Text : "0";

			if (role == "KHO") {
				sql = "INSERT INTO Kho (MaKho, TenKho, Tinh, QuanHuyen, DiaChi, SDT, SucChua, NhietDoMin, NhietDoMax) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)";
			}
			else {
				sql = "INSERT INTO SieuThi (MaSieuThi, TenSieuThi, Tinh, QuanHuyen, DiaChi, SDT, MucPhatThieu) VALUES (?, ?, ?, ?, ?, ?, ?)";
			}

			OdbcConnection^ conn = gcnew OdbcConnection(connStr);
			try {
				conn->Open(); OdbcCommand^ cmd = gcnew OdbcCommand(sql, conn);
				cmd->Parameters->AddWithValue("p1", id); cmd->Parameters->AddWithValue("p2", name);
				cmd->Parameters->AddWithValue("p3", tinh); cmd->Parameters->AddWithValue("p4", huyen);
				cmd->Parameters->AddWithValue("p5", dc); cmd->Parameters->AddWithValue("p6", sdt);

				if (role == "KHO") {
					cmd->Parameters->AddWithValue("p7", sucChua);
					cmd->Parameters->AddWithValue("p8", tMin);
					cmd->Parameters->AddWithValue("p9", tMax);
				}
				else {
					cmd->Parameters->AddWithValue("p7", phat);
				}
				cmd->ExecuteNonQuery(); return true;
			}
			catch (Exception^ ex) { MessageBox::Show(L"Lỗi: " + ex->Message); return false; }
			finally { conn->Close(); }
		}

		void InsertAccount(String^ u, String^ p, String^ r, String^ link) {
			String^ sql = "INSERT INTO TaiKhoan (TenDangNhap, MatKhau, QuyenHan, MaLienKet) VALUES (?, ?, ?, ?)";
			OdbcConnection^ conn = gcnew OdbcConnection(connStr);
			try {
				conn->Open(); OdbcCommand^ cmd = gcnew OdbcCommand(sql, conn);
				cmd->Parameters->AddWithValue("p1", u); cmd->Parameters->AddWithValue("p2", p);
				cmd->Parameters->AddWithValue("p3", r); cmd->Parameters->AddWithValue("p4", link);
				cmd->ExecuteNonQuery(); MessageBox::Show(L"Thêm thành công!"); LoadData(""); btnReset_Click(nullptr, nullptr);
			}
			catch (Exception^ ex) { MessageBox::Show(ex->Message); }
			finally { conn->Close(); }
		}

		void btnEye_Click(Object^ s, EventArgs^ e) { isPassVisible = !isPassVisible; txtPass->UseSystemPasswordChar = !isPassVisible; btnEye->BackgroundImage = isPassVisible ? imgShow : imgHide; }
		void cbbFilter_SelectedIndexChanged(Object^ s, EventArgs^ e) { LoadData(cbbFilter->SelectedItem->ToString()); }
		void dgvTaiKhoan_CellFormatting(Object^ s, DataGridViewCellFormattingEventArgs^ e) { if (dgvTaiKhoan->Columns[e->ColumnIndex]->Name == "MatKhau" && e->Value != nullptr) e->Value = "••••••"; }

		void btnReset_Click(Object^ s, EventArgs^ e) {
			txtUser->Text = ""; txtUser->Enabled = true; txtPass->Text = "";
			cbbRole->SelectedIndex = -1; chkCreateNew->Checked = false;
			txtNewID->Text = ""; txtNewName->Text = ""; txtDiaChi->Text = ""; txtSDT->Text = "";
			txtSucChua->Text = "";

			// Reset cả thanh trượt
			txtTempMin->Text = "-10"; tbTempMin->Value = -10;
			txtTempMax->Text = "20"; tbTempMax->Value = 20;
			txtMucPhat->Text = "";
		}

		void dgvTaiKhoan_CellClick(Object^ s, DataGridViewCellEventArgs^ e) {
			if (e->RowIndex < 0) return;
			DataGridViewRow^ r = dgvTaiKhoan->Rows[e->RowIndex];
			txtUser->Text = r->Cells["TenDangNhap"]->Value->ToString(); txtUser->Enabled = false;
			txtPass->Text = r->Cells["MatKhau"]->Value->ToString();
			String^ role = r->Cells["QuyenHan"]->Value->ToString();
			if (cbbRole->Items->Contains(role)) cbbRole->SelectedItem = role;
			chkCreateNew->Checked = false;
		}

		void btnUpdate_Click(Object^ s, EventArgs^ e) {
			if (txtUser->Enabled) return;
			try {
				OdbcConnection^ conn = gcnew OdbcConnection(connStr); conn->Open();
				OdbcCommand^ cmd = gcnew OdbcCommand("UPDATE TaiKhoan SET MatKhau = ? WHERE TenDangNhap = ?", conn);
				cmd->Parameters->AddWithValue("p1", txtPass->Text); cmd->Parameters->AddWithValue("p2", txtUser->Text);
				cmd->ExecuteNonQuery(); conn->Close(); MessageBox::Show(L"Cập nhật mật khẩu thành công!"); LoadData("");
			}
			catch (Exception^ ex) { MessageBox::Show(ex->Message); }
		}

		void btnDelete_Click(Object^ s, EventArgs^ e) {
			if (txtUser->Enabled) return;
			if (MessageBox::Show(L"Xóa tài khoản này?", L"Xác nhận", MessageBoxButtons::YesNo) == System::Windows::Forms::DialogResult::Yes) {
				try {
					OdbcConnection^ conn = gcnew OdbcConnection(connStr); conn->Open();
					OdbcCommand^ cmd = gcnew OdbcCommand("DELETE FROM TaiKhoan WHERE TenDangNhap = ?", conn);
					cmd->Parameters->AddWithValue("p1", txtUser->Text);
					cmd->ExecuteNonQuery(); conn->Close(); LoadData(""); btnReset_Click(nullptr, nullptr);
				}
				catch (Exception^ ex) { MessageBox::Show(ex->Message); }
			}
		}
	};
}