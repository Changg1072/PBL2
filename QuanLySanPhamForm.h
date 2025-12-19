#pragma once
#include "DataModels.h" // Để dùng struct ProductInfo
#include <IO.h>         // Để check file ảnh
#include "VietNamData.h"
namespace PBL2QuanLyKho {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Collections::Generic; // Dùng Dictionary
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Data::Odbc;
	using namespace System::IO;

	public ref class QuanLySanPhamForm : public System::Windows::Forms::Form
	{
	public:
		QuanLySanPhamForm(void)
		{
			InitializeComponent();

			// Khởi tạo map ảnh
			imageMap = gcnew Dictionary<String^, String^>();
			LoadImageDatabase("ProductImages.txt"); // Load link ảnh từ file txt nếu có

			LoadDSNhaCungCap();
			mapDiaChinh = VietnamData::GetFullData();
			LoadTinhThanhVaoComboBox();
		}

	protected:
		~QuanLySanPhamForm()
		{
			if (components) delete components;
		}

	private:
		// --- CÁC BIẾN KẾT NỐI DATABASE ---
		String^ strConn = "Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;";

		Dictionary<String^, String^>^ imageMap; // Map lưu đường dẫn ảnh

		System::Windows::Forms::SplitContainer^ splitContainer1;

		// --- TRÁI: NHÀ CUNG CẤP ---
		System::Windows::Forms::GroupBox^ grpNCC;
		System::Windows::Forms::DataGridView^ dgvNCC;
		System::Windows::Forms::Panel^ pnlNCCAction;
		System::Windows::Forms::Button^ btnXoaNCC;
		System::Windows::Forms::Button^ btnThemNCC;
		System::Windows::Forms::Panel^ pnlAddNCCInput;
		System::Windows::Forms::TextBox^ txtAddTenNCC;
		System::Windows::Forms::TextBox^ txtAddMaNCC;
		System::Windows::Forms::Button^ btnLuuNCC;
		System::Windows::Forms::Button^ btnHuyNCC;

		// --- PHẢI: SẢN PHẨM (CARD VIEW) ---
		System::Windows::Forms::GroupBox^ grpSP;
		System::Windows::Forms::FlowLayoutPanel^ flowPnlSP; // <--- THAY GRID BẰNG FLOW LAYOUT
		System::Windows::Forms::Panel^ pnlSPAction;
		System::Windows::Forms::Button^ btnThemSPVaoNCC;
		System::Windows::Forms::Label^ lblCurrentNCC;

		// --- PANEL THÊM SẢN PHẨM ---
		System::Windows::Forms::Panel^ pnlAddSPInput;
		System::Windows::Forms::RadioButton^ radChonCoSan;
		System::Windows::Forms::RadioButton^ radTaoMoi;
		System::Windows::Forms::Panel^ pnlModeExisting;
		System::Windows::Forms::ComboBox^ cboGoiYSanPham;
		System::Windows::Forms::Label^ lblGoiY;
		System::Windows::Forms::Panel^ pnlModeNew;
		System::Windows::Forms::TextBox^ txtNewMaSP;
		System::Windows::Forms::TextBox^ txtNewTenSP;
		System::Windows::Forms::TextBox^ txtNewGiaVon;
		System::Windows::Forms::TextBox^ txtNewLoai;
		System::Windows::Forms::TextBox^ txtNewNhietDo;
		System::Windows::Forms::TextBox^ txtNewTheTich;
		System::Windows::Forms::Button^ btnLuuSP;
		System::Windows::Forms::Button^ btnHuySP;

		System::Windows::Forms::GroupBox^ grpBatchInfo;
		System::Windows::Forms::NumericUpDown^ numSoLuong;
		System::Windows::Forms::TextBox^ txtDonVi;
		System::Windows::Forms::TextBox^ txtGiaNhap;
		System::Windows::Forms::Label^ lblThanhTien;
		System::Windows::Forms::DateTimePicker^ dtpNgaySX;
		System::Windows::Forms::DateTimePicker^ dtpHanSD;

		System::Windows::Forms::GroupBox^ grpInfoChiTiet; // Group bao quanh
		System::Windows::Forms::Label^ lblInfoTen;
		System::Windows::Forms::Label^ lblInfoMa;
		System::Windows::Forms::Label^ lblInfoGia;
		System::Windows::Forms::Label^ lblInfoHSD;     // Hiển thị "365 ngày"
		System::Windows::Forms::Label^ lblInfoNhietDo; // Hiển thị "25 độ C"
		System::Windows::Forms::Label^ lblInfoLoai;
		System::Windows::Forms::PictureBox^ picInfoAnh;

		System::Windows::Forms::PictureBox^ picPreview; // Khung hiển thị ảnh
		System::Windows::Forms::Button^ btnChonAnh;     // Nút bấm chọn file
		System::Windows::Forms::TextBox^ txtPathAnh;    // Biến ẩn lưu đường dẫn ảnh

		System::Windows::Forms::ComboBox^ cboNewLoai;
		System::Windows::Forms::TextBox^ txtNewNhom;       // NhomPhanLoai
		System::Windows::Forms::TextBox^ txtNewGiaBan;     // GiaBan
		System::Windows::Forms::TextBox^ txtNewHSDTieuChuan; // HanSuDungTieuChuan (int)
		System::Windows::Forms::TextBox^ txtNewRuiRoLanh;  // RuiRoLanh
		System::Windows::Forms::TextBox^ txtNewRuiRoNong;  // RuiRoNong
		
		System::Windows::Forms::TextBox^ txtAddSDT;
		System::Windows::Forms::TextBox^ txtAddDiaChi;
		ComboBox^ cboAddTinh;
		ComboBox^ cboAddHuyen;
		Dictionary<String^, List<String^>^>^ mapDiaChinh;
		System::Windows::Forms::TextBox^ txtAddChiPhi;   // ChiPhiDat
		System::Windows::Forms::TextBox^ txtAddThoiGian; // ThoiGianCho

		System::ComponentModel::Container^ components;
		String^ currentMaNCC = "";

		bool isEditing = false;
		bool isEditingNCC = false;
		System::Windows::Forms::Button^ btnSuaNCC;

#pragma region Windows Form Designer generated code
		void InitializeComponent(void)
		{
			this->splitContainer1 = (gcnew System::Windows::Forms::SplitContainer());
			this->grpNCC = (gcnew System::Windows::Forms::GroupBox());
			this->dgvNCC = (gcnew System::Windows::Forms::DataGridView());
			this->pnlNCCAction = (gcnew System::Windows::Forms::Panel());
			this->btnThemNCC = (gcnew System::Windows::Forms::Button());
			this->btnXoaNCC = (gcnew System::Windows::Forms::Button());
			this->btnSuaNCC = (gcnew System::Windows::Forms::Button()); // [MỚI] Khởi tạo nút Sửa

			// Form Init
			this->Size = System::Drawing::Size(1280, 720);
			this->BackColor = System::Drawing::Color::White;

			// SplitContainer
			this->splitContainer1->Dock = System::Windows::Forms::DockStyle::Fill;
			this->splitContainer1->SplitterDistance = 450;
			this->Controls->Add(this->splitContainer1);

			// ==========================================
			// LEFT: NHÀ CUNG CẤP
			// ==========================================
			this->splitContainer1->Panel1->Controls->Add(this->grpNCC);
			this->grpNCC->Dock = DockStyle::Fill;
			this->grpNCC->Text = L" DANH SÁCH NHÀ CUNG CẤP";
			this->grpNCC->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, FontStyle::Bold));

			this->dgvNCC->Dock = DockStyle::Fill;
			this->dgvNCC->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;
			this->dgvNCC->SelectionMode = DataGridViewSelectionMode::FullRowSelect;
			this->dgvNCC->ReadOnly = true;
			this->dgvNCC->BackgroundColor = Color::White;
			this->dgvNCC->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, FontStyle::Regular));
			this->dgvNCC->CellClick += gcnew DataGridViewCellEventHandler(this, &QuanLySanPhamForm::OnNCC_Click);

			// --- PANEL ACTION (THÊM / SỬA / XÓA) ---
			this->pnlNCCAction->Dock = DockStyle::Bottom; this->pnlNCCAction->Height = 60;

			// 1. Nút Thêm
			this->btnThemNCC->Text = L"Thêm NCC"; this->btnThemNCC->Location = Point(10, 10); this->btnThemNCC->Size = System::Drawing::Size(120, 40);
			this->btnThemNCC->BackColor = Color::SeaGreen; this->btnThemNCC->ForeColor = Color::White;
			this->btnThemNCC->Click += gcnew EventHandler(this, &QuanLySanPhamForm::OnBtnThemNCC_Click);

			// 2. Nút Sửa [MỚI THÊM]
			this->btnSuaNCC->Text = L"Sửa TT";
			this->btnSuaNCC->Location = Point(140, 10); // Nằm giữa
			this->btnSuaNCC->Size = System::Drawing::Size(100, 40);
			this->btnSuaNCC->BackColor = Color::Orange;
			this->btnSuaNCC->ForeColor = Color::White;
			this->btnSuaNCC->Click += gcnew EventHandler(this, &QuanLySanPhamForm::OnBtnSuaNCC_Click);

			// 3. Nút Xóa [ĐÃ DỜI SANG PHẢI]
			this->btnXoaNCC->Text = L"Xóa NCC";
			this->btnXoaNCC->Location = Point(250, 10); // Dời từ 140 sang 250
			this->btnXoaNCC->Size = System::Drawing::Size(120, 40);
			this->btnXoaNCC->BackColor = Color::IndianRed; this->btnXoaNCC->ForeColor = Color::White;
			this->btnXoaNCC->Click += gcnew EventHandler(this, &QuanLySanPhamForm::OnBtnXoaNCC_Click);

			this->pnlNCCAction->Controls->Add(this->btnThemNCC);
			this->pnlNCCAction->Controls->Add(this->btnSuaNCC); // Add nút Sửa
			this->pnlNCCAction->Controls->Add(this->btnXoaNCC);

			// --- PANEL INPUT NCC (FULL 8 TRƯỜNG THÔNG TIN) ---
			this->pnlAddNCCInput = (gcnew System::Windows::Forms::Panel());
			this->pnlAddNCCInput->Size = System::Drawing::Size(400, 420);
			this->pnlAddNCCInput->Location = Point(25, 80);
			this->pnlAddNCCInput->BackColor = Color::WhiteSmoke;
			this->pnlAddNCCInput->BorderStyle = BorderStyle::FixedSingle;
			this->pnlAddNCCInput->Visible = false;

			// Tiêu đề Form
			Label^ lblTitleNCC = gcnew Label(); lblTitleNCC->Text = L"THÔNG TIN NHÀ CUNG CẤP";
			lblTitleNCC->Dock = DockStyle::Top; lblTitleNCC->Height = 40;
			lblTitleNCC->TextAlign = ContentAlignment::MiddleCenter;
			lblTitleNCC->BackColor = Color::SeaGreen; lblTitleNCC->ForeColor = Color::White;
			lblTitleNCC->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, FontStyle::Bold));
			this->pnlAddNCCInput->Controls->Add(lblTitleNCC);

			// Hàng 1: Mã & Tên
			Label^ lblMa = gcnew Label(); lblMa->Text = L"Mã NCC:"; lblMa->Location = Point(20, 50); lblMa->AutoSize = true;
			this->txtAddMaNCC = gcnew TextBox(); this->txtAddMaNCC->Location = Point(20, 70); this->txtAddMaNCC->Width = 150;

			Label^ lblTen = gcnew Label(); lblTen->Text = L"Tên NCC:"; lblTen->Location = Point(190, 50); lblTen->AutoSize = true;
			this->txtAddTenNCC = gcnew TextBox(); this->txtAddTenNCC->Location = Point(190, 70); this->txtAddTenNCC->Width = 180;

			// Hàng 2: SĐT & Tỉnh (COMBOBOX)
			Label^ lblSDT = gcnew Label(); lblSDT->Text = L"Số điện thoại:"; lblSDT->Location = Point(20, 110); lblSDT->AutoSize = true;
			this->txtAddSDT = gcnew TextBox(); this->txtAddSDT->Location = Point(20, 130); this->txtAddSDT->Width = 150;

			Label^ lblTinh = gcnew Label(); lblTinh->Text = L"Tỉnh/TP:"; lblTinh->Location = Point(190, 110); lblTinh->AutoSize = true;
			this->cboAddTinh = gcnew ComboBox();
			this->cboAddTinh->Location = Point(190, 130);
			this->cboAddTinh->Width = 180;
			this->cboAddTinh->DropDownStyle = ComboBoxStyle::DropDownList;
			this->cboAddTinh->SelectedIndexChanged += gcnew EventHandler(this, &QuanLySanPhamForm::OnTinhChanged);

			// Hàng 3: Quận/Huyện (COMBOBOX) & Địa chỉ
			Label^ lblHuyen = gcnew Label(); lblHuyen->Text = L"Quận/Huyện:"; lblHuyen->Location = Point(20, 170); lblHuyen->AutoSize = true;
			this->cboAddHuyen = gcnew ComboBox();
			this->cboAddHuyen->Location = Point(20, 190);
			this->cboAddHuyen->Width = 150;
			this->cboAddHuyen->DropDownStyle = ComboBoxStyle::DropDownList;

			Label^ lblDC = gcnew Label(); lblDC->Text = L"Địa chỉ chi tiết:"; lblDC->Location = Point(190, 170); lblDC->AutoSize = true;
			this->txtAddDiaChi = gcnew TextBox(); this->txtAddDiaChi->Location = Point(190, 190); this->txtAddDiaChi->Width = 180;

			// Hàng 4: Chi phí & Thời gian
			Label^ lblCP = gcnew Label(); lblCP->Text = L"Chi phí đặt (VNĐ):"; lblCP->Location = Point(20, 230); lblCP->AutoSize = true;
			this->txtAddChiPhi = gcnew TextBox(); this->txtAddChiPhi->Location = Point(20, 250); this->txtAddChiPhi->Width = 150; this->txtAddChiPhi->Text = "0";

			Label^ lblTG = gcnew Label(); lblTG->Text = L"Thời gian chờ (Ngày):"; lblTG->Location = Point(190, 230); lblTG->AutoSize = true;
			this->txtAddThoiGian = gcnew TextBox(); this->txtAddThoiGian->Location = Point(190, 250); this->txtAddThoiGian->Width = 180; this->txtAddThoiGian->Text = "0";

			// Nút bấm
			this->btnLuuNCC = gcnew Button(); this->btnLuuNCC->Text = L"LƯU THÔNG TIN";
			this->btnLuuNCC->Location = Point(60, 310); this->btnLuuNCC->Size = System::Drawing::Size(120, 40);
			this->btnLuuNCC->BackColor = Color::SeaGreen; this->btnLuuNCC->ForeColor = Color::White;
			this->btnLuuNCC->Click += gcnew EventHandler(this, &QuanLySanPhamForm::OnSaveNewNCC);

			this->btnHuyNCC = gcnew Button(); this->btnHuyNCC->Text = L"HỦY BỎ";
			this->btnHuyNCC->Location = Point(200, 310); this->btnHuyNCC->Size = System::Drawing::Size(120, 40);
			this->btnHuyNCC->Click += gcnew EventHandler(this, &QuanLySanPhamForm::OnCancelNewNCC);

			// Add controls vào Panel NCC
			this->pnlAddNCCInput->Controls->Add(lblMa); this->pnlAddNCCInput->Controls->Add(this->txtAddMaNCC);
			this->pnlAddNCCInput->Controls->Add(lblTen); this->pnlAddNCCInput->Controls->Add(this->txtAddTenNCC);
			this->pnlAddNCCInput->Controls->Add(lblSDT); this->pnlAddNCCInput->Controls->Add(this->txtAddSDT);

			this->pnlAddNCCInput->Controls->Add(lblTinh); this->pnlAddNCCInput->Controls->Add(this->cboAddTinh);
			this->pnlAddNCCInput->Controls->Add(lblHuyen); this->pnlAddNCCInput->Controls->Add(this->cboAddHuyen);

			this->pnlAddNCCInput->Controls->Add(lblDC); this->pnlAddNCCInput->Controls->Add(this->txtAddDiaChi);
			this->pnlAddNCCInput->Controls->Add(lblCP); this->pnlAddNCCInput->Controls->Add(this->txtAddChiPhi);
			this->pnlAddNCCInput->Controls->Add(lblTG); this->pnlAddNCCInput->Controls->Add(this->txtAddThoiGian);
			this->pnlAddNCCInput->Controls->Add(this->btnLuuNCC); this->pnlAddNCCInput->Controls->Add(this->btnHuyNCC);

			this->grpNCC->Controls->Add(this->dgvNCC);
			this->grpNCC->Controls->Add(this->pnlAddNCCInput);
			this->pnlAddNCCInput->BringToFront(); // Hiện lên trên Grid
			this->grpNCC->Controls->Add(this->pnlNCCAction);

			// ==========================================
			// RIGHT: SẢN PHẨM (CARD LAYOUT)
			// ==========================================
			this->grpSP = (gcnew System::Windows::Forms::GroupBox());
			this->flowPnlSP = (gcnew System::Windows::Forms::FlowLayoutPanel());
			this->pnlSPAction = (gcnew System::Windows::Forms::Panel());
			this->lblCurrentNCC = (gcnew System::Windows::Forms::Label());
			this->btnThemSPVaoNCC = (gcnew System::Windows::Forms::Button());

			// Gọi hàm khởi tạo Panel SP
			InitializePanelAddSP();

			this->splitContainer1->Panel2->Controls->Add(this->grpSP);
			this->grpSP->Dock = DockStyle::Fill;
			this->grpSP->Text = L" SẢN PHẨM CUNG CẤP";
			this->grpSP->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, FontStyle::Bold));

			// Setup Flow Layout Panel (Thay cho Grid)
			this->flowPnlSP->Dock = DockStyle::Fill;
			this->flowPnlSP->AutoScroll = true;
			this->flowPnlSP->BackColor = Color::FromArgb(245, 245, 250);
			this->flowPnlSP->Padding = System::Windows::Forms::Padding(10);

			this->pnlSPAction->Dock = DockStyle::Bottom; this->pnlSPAction->Height = 80;
			this->lblCurrentNCC->Text = L"Vui lòng chọn NCC..."; this->lblCurrentNCC->Dock = DockStyle::Top; this->lblCurrentNCC->ForeColor = Color::Blue;

			this->btnThemSPVaoNCC->Text = L"➕ Thêm / Tạo Sản Phẩm"; this->btnThemSPVaoNCC->Location = Point(10, 30); this->btnThemSPVaoNCC->Size = System::Drawing::Size(250, 40);
			this->btnThemSPVaoNCC->BackColor = Color::FromArgb(0, 122, 204); this->btnThemSPVaoNCC->ForeColor = Color::White;
			this->btnThemSPVaoNCC->Click += gcnew EventHandler(this, &QuanLySanPhamForm::OnBtnThemSP_Click);

			this->pnlSPAction->Controls->Add(this->btnThemSPVaoNCC);
			this->pnlSPAction->Controls->Add(this->lblCurrentNCC);

			this->grpSP->Controls->Add(this->pnlAddSPInput);
			this->grpSP->Controls->Add(this->flowPnlSP);
			this->grpSP->Controls->Add(this->pnlSPAction);
		}
		void InitializePanelAddSP() {
			this->pnlAddSPInput = (gcnew System::Windows::Forms::Panel());
			this->pnlAddSPInput->Size = System::Drawing::Size(560, 680);
			this->pnlAddSPInput->Location = Point(40, 10);
			this->pnlAddSPInput->BackColor = Color::White;
			this->pnlAddSPInput->BorderStyle = BorderStyle::FixedSingle;
			this->pnlAddSPInput->Visible = false;

			// Header
			Label^ lblHeader = gcnew Label(); lblHeader->Text = L"LIÊN KẾT SẢN PHẨM VỚI NHÀ CUNG CẤP";
			lblHeader->Dock = DockStyle::Top; lblHeader->Height = 40;
			lblHeader->BackColor = Color::FromArgb(23, 42, 69); lblHeader->ForeColor = Color::White;
			lblHeader->TextAlign = ContentAlignment::MiddleCenter; lblHeader->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, FontStyle::Bold));
			this->pnlAddSPInput->Controls->Add(lblHeader);

			// Mode Selection
			GroupBox^ grpMode = gcnew GroupBox(); grpMode->Text = L"Bước 1: Chọn nguồn dữ liệu";
			grpMode->Size = System::Drawing::Size(520, 60); grpMode->Location = Point(20, 50);

			this->radChonCoSan = gcnew RadioButton(); this->radChonCoSan->Text = L"Chọn từ hệ thống (Có sẵn)";
			this->radChonCoSan->Location = Point(20, 25); this->radChonCoSan->AutoSize = true; this->radChonCoSan->Checked = true;
			this->radChonCoSan->CheckedChanged += gcnew EventHandler(this, &QuanLySanPhamForm::OnModeChanged);

			this->radTaoMoi = gcnew RadioButton(); this->radTaoMoi->Text = L"Định nghĩa sản phẩm mới";
			this->radTaoMoi->Location = Point(250, 25); this->radTaoMoi->AutoSize = true;
			this->radTaoMoi->CheckedChanged += gcnew EventHandler(this, &QuanLySanPhamForm::OnModeChanged);
			grpMode->Controls->Add(this->radChonCoSan); grpMode->Controls->Add(this->radTaoMoi);

			// Panel Existing
			this->pnlModeExisting = gcnew Panel(); this->pnlModeExisting->Location = Point(20, 115); this->pnlModeExisting->Size = System::Drawing::Size(520, 60);
			this->lblGoiY = gcnew Label(); this->lblGoiY->Text = L"Tìm kiếm:"; this->lblGoiY->Location = Point(5, 5); this->lblGoiY->AutoSize = true;
			this->cboGoiYSanPham = gcnew ComboBox(); this->cboGoiYSanPham->Location = Point(5, 25); this->cboGoiYSanPham->Width = 480;
			this->cboGoiYSanPham->DropDownStyle = ComboBoxStyle::DropDown; this->cboGoiYSanPham->AutoCompleteMode = AutoCompleteMode::SuggestAppend;
			this->cboGoiYSanPham->AutoCompleteSource = AutoCompleteSource::ListItems;
			this->cboGoiYSanPham->SelectedIndexChanged += gcnew EventHandler(this, &QuanLySanPhamForm::OnExistingProductSelected);
			this->pnlModeExisting->Controls->Add(this->lblGoiY); this->pnlModeExisting->Controls->Add(this->cboGoiYSanPham);

			// Panel New
			this->pnlModeNew = gcnew Panel(); this->pnlModeNew->Location = Point(20, 115); this->pnlModeNew->Size = System::Drawing::Size(520, 270); this->pnlModeNew->Visible = false;

			// Cột 1: Mã (Auto) - Tên - Loại - Nhóm
			this->txtNewMaSP = CreateInputBox(pnlModeNew, L"Mã SP (Auto):", 0, 0);
			this->txtNewMaSP->ReadOnly = true; this->txtNewMaSP->BackColor = Color::WhiteSmoke; this->txtNewMaSP->ForeColor = Color::Red;

			this->txtNewTenSP = CreateInputBox(pnlModeNew, L"Tên SP:", 0, 1);

			Label^ lblLoai = gcnew Label(); lblLoai->Text = L"Loại hàng:"; lblLoai->Location = Point(10, 120); lblLoai->AutoSize = true;
			this->cboNewLoai = gcnew ComboBox(); this->cboNewLoai->Location = Point(10, 140); this->cboNewLoai->Width = 120;
			this->cboNewLoai->Leave += gcnew EventHandler(this, &QuanLySanPhamForm::OnLoaiChanged);
			this->pnlModeNew->Controls->Add(lblLoai); this->pnlModeNew->Controls->Add(this->cboNewLoai);

			//this->txtNewNhom = CreateInputBox(pnlModeNew, L"Nhóm (Auto):", 0, 3);
			//this->txtNewNhom->ReadOnly = true; this->txtNewNhom->BackColor = Color::WhiteSmoke; this->txtNewNhom->Text = L"Hàng Đông Lạnh";

			// Cột 2: Giá & Quy cách
			this->txtNewGiaVon = CreateInputBox(pnlModeNew, L"Giá vốn:", 135, 0);
			this->txtNewGiaBan = CreateInputBox(pnlModeNew, L"Giá bán:", 135, 1);
			this->txtNewTheTich = CreateInputBox(pnlModeNew, L"Thể tích:", 135, 2);
			this->txtNewHSDTieuChuan = CreateInputBox(pnlModeNew, L"HSD (ngày):", 135, 3);

			// Cột 3: Nhiệt độ (TextBox) & Rủi ro
			// [QUAY VỀ] Dùng TextBox nhập số như cũ
			this->txtNewNhietDo = CreateInputBox(pnlModeNew, L"Nhiệt độ (°C):", 270, 0);
			this->txtNewRuiRoLanh = CreateInputBox(pnlModeNew, L"RR Lạnh:", 270, 1);
			this->txtNewRuiRoNong = CreateInputBox(pnlModeNew, L"RR Nóng:", 270, 2);

			// Cột 4: Ảnh
			GroupBox^ grpAnhInput = gcnew GroupBox(); grpAnhInput->Text = L"Ảnh SP"; grpAnhInput->Location = Point(405, 5); grpAnhInput->Size = System::Drawing::Size(110, 160);
			this->picPreview = gcnew PictureBox(); this->picPreview->Size = System::Drawing::Size(90, 80); this->picPreview->Location = Point(10, 20); this->picPreview->SizeMode = PictureBoxSizeMode::Zoom; this->picPreview->BorderStyle = BorderStyle::FixedSingle;
			this->btnChonAnh = gcnew Button(); this->btnChonAnh->Text = L"Chọn"; this->btnChonAnh->Location = Point(15, 110); this->btnChonAnh->Size = System::Drawing::Size(80, 35);
			this->btnChonAnh->Click += gcnew EventHandler(this, &QuanLySanPhamForm::OnBtnChonAnh_Click);
			this->txtPathAnh = gcnew TextBox(); this->txtPathAnh->Visible = false;
			grpAnhInput->Controls->Add(this->picPreview); grpAnhInput->Controls->Add(this->btnChonAnh);
			this->pnlModeNew->Controls->Add(grpAnhInput); this->pnlModeNew->Controls->Add(this->txtPathAnh);

			// Group Info (Read Only)
			this->grpInfoChiTiet = gcnew GroupBox(); this->grpInfoChiTiet->Text = L"Thông tin chi tiết"; this->grpInfoChiTiet->Size = System::Drawing::Size(520, 180); this->grpInfoChiTiet->Location = Point(20, 200);

			this->picInfoAnh = gcnew PictureBox(); this->picInfoAnh->Location = Point(20, 30); this->picInfoAnh->Size = System::Drawing::Size(100, 100); this->picInfoAnh->BorderStyle = BorderStyle::FixedSingle; this->picInfoAnh->SizeMode = PictureBoxSizeMode::Zoom;
			this->lblInfoTen = gcnew Label(); this->lblInfoTen->Location = Point(140, 30); this->lblInfoTen->AutoSize = true; this->lblInfoTen->Text = "---";
			this->lblInfoMa = gcnew Label(); this->lblInfoMa->Location = Point(140, 60); this->lblInfoMa->AutoSize = true; this->lblInfoMa->Text = "---";
			this->lblInfoGia = gcnew Label(); this->lblInfoGia->Location = Point(140, 90); this->lblInfoGia->AutoSize = true; this->lblInfoGia->Text = "---";
			this->lblInfoLoai = gcnew Label(); this->lblInfoLoai->Location = Point(300, 60); this->lblInfoLoai->AutoSize = true; this->lblInfoLoai->Text = "---";
			this->lblInfoHSD = gcnew Label(); this->lblInfoHSD->Location = Point(300, 90); this->lblInfoHSD->AutoSize = true; this->lblInfoHSD->Text = "---";
			this->lblInfoNhietDo = gcnew Label(); this->lblInfoNhietDo->Location = Point(140, 120); this->lblInfoNhietDo->AutoSize = true; this->lblInfoNhietDo->Text = "---";

			this->grpInfoChiTiet->Controls->Add(this->picInfoAnh); this->grpInfoChiTiet->Controls->Add(this->lblInfoTen);
			this->grpInfoChiTiet->Controls->Add(this->lblInfoMa); this->grpInfoChiTiet->Controls->Add(this->lblInfoGia);
			this->grpInfoChiTiet->Controls->Add(this->lblInfoLoai); this->grpInfoChiTiet->Controls->Add(this->lblInfoHSD);
			this->grpInfoChiTiet->Controls->Add(this->lblInfoNhietDo);

			// Buttons
			this->btnLuuSP = gcnew Button(); this->btnLuuSP->Text = L"LIÊN KẾT"; this->btnLuuSP->Location = Point(120, 420); this->btnLuuSP->Size = System::Drawing::Size(180, 45);
			this->btnLuuSP->BackColor = Color::FromArgb(0, 114, 188); this->btnLuuSP->ForeColor = Color::White;
			this->btnLuuSP->Click += gcnew EventHandler(this, &QuanLySanPhamForm::OnSaveNewSP);

			this->btnHuySP = gcnew Button(); this->btnHuySP->Text = L"Thoát"; this->btnHuySP->Location = Point(320, 420); this->btnHuySP->Size = System::Drawing::Size(100, 45);
			this->btnHuySP->Click += gcnew EventHandler(this, &QuanLySanPhamForm::OnCancelNewSP);

			this->pnlAddSPInput->Controls->Add(grpMode); this->pnlAddSPInput->Controls->Add(this->pnlModeExisting);
			this->pnlAddSPInput->Controls->Add(this->pnlModeNew); this->pnlAddSPInput->Controls->Add(this->grpInfoChiTiet);
			this->pnlAddSPInput->Controls->Add(this->btnLuuSP); this->pnlAddSPInput->Controls->Add(this->btnHuySP);
		}
		TextBox^ CreateInputBox(Panel^ p, String^ label, int x, int row) {
			Label^ l = gcnew Label(); l->Text = label; l->AutoSize = true;
			l->Location = Point(x + 10, 10 + row * 55);

			TextBox^ t = gcnew TextBox();
			t->Width = 120; // <--- SỬA THÀNH 130
			t->Location = Point(x + 10, 30 + row * 55);

			p->Controls->Add(l); p->Controls->Add(t);
			return t;
		}
#pragma endregion

		// ==========================================
		// LOGIC XỬ LÝ
		// ==========================================

		// --- 1. Load Ảnh (Giống Siêu Thị) ---
		void LoadImageDatabase(String^ filePath) {
			try {
				if (!File::Exists(filePath)) return;
				cli::array<String^>^ lines = File::ReadAllLines(filePath);
				for each (String ^ line in lines) {
					if (String::IsNullOrWhiteSpace(line)) continue;
					cli::array<wchar_t>^ separator = gcnew cli::array<wchar_t>{ '|' };
					cli::array<String^>^ parts = line->Split(separator);
					if (parts->Length == 2) {
						String^ ma = parts[0]->Trim();
						String^ link = parts[1]->Trim();
						if (!imageMap->ContainsKey(ma)) imageMap->Add(ma, link);
					}
				}
			}
			catch (...) {}
		}
		DataTable^ GetDataTable(String^ sql) {
			DataTable^ dt = gcnew DataTable();
			try {
				OdbcConnection^ conn = gcnew OdbcConnection(strConn);
				conn->Open();
				OdbcDataAdapter^ da = gcnew OdbcDataAdapter(sql, conn);
				da->Fill(dt);
				conn->Close();
			}
			catch (Exception^ ex) { MessageBox::Show(L"Lỗi DB: " + ex->Message); }
			return dt;
		}

		void ExecuteQuery(String^ sql) {
			try {
				OdbcConnection^ conn = gcnew OdbcConnection(strConn);
				conn->Open();
				OdbcCommand^ cmd = gcnew OdbcCommand(sql, conn);
				cmd->ExecuteNonQuery();
				conn->Close();
			}
			catch (Exception^ ex) { MessageBox::Show(L"Lỗi thực thi: " + ex->Message); }
		}

		void LoadDSNhaCungCap() {
			String^ sql = "SELECT MaNCC, TenNCC, Tinh, SDT FROM NhaCungCap";
			dgvNCC->DataSource = GetDataTable(sql);
		}

		// --- HÀM LOAD SẢN PHẨM MỚI (DẠNG THẺ) ---
		void LoadDSSanPhamCuaNCC(String^ maNCC) {
			// Xóa các thẻ cũ
			flowPnlSP->Controls->Clear();

			String^ sql = "SELECT SP.MaSanPham, SP.TenSanPham, SP.GiaVon, SP.Loai " +
				"FROM SanPham SP JOIN CungCapSanPham CC ON SP.MaSanPham = CC.MaSanPham " +
				"WHERE CC.MaNCC = '" + maNCC + "'";

			try {
				OdbcConnection^ conn = gcnew OdbcConnection(strConn);
				conn->Open();
				OdbcCommand^ cmd = gcnew OdbcCommand(sql, conn);
				OdbcDataReader^ reader = cmd->ExecuteReader();

				while (reader->Read()) {
					// Lấy thông tin
					String^ ma = reader["MaSanPham"]->ToString();
					String^ ten = reader["TenSanPham"]->ToString();

					// --- SỬA LỖI Ở ĐÂY ---
					double gia = 0;
					if (reader["GiaVon"] != DBNull::Value) {
						gia = Convert::ToDouble(reader["GiaVon"]);
					}
					// ---------------------

					String^ loai = "";
					if (reader["Loai"] != DBNull::Value) {
						loai = reader["Loai"]->ToString();
					}

					// Tạo thẻ
					Panel^ card = CreateProductCard(ma, ten, gia, loai);
					flowPnlSP->Controls->Add(card);
				}
				conn->Close();
			}
			catch (Exception^ ex) {
				MessageBox::Show(L"Lỗi load SP: " + ex->Message);
			}
		}
		// 1. Hàm nạp danh sách Tỉnh vào ComboBox
		void LoadTinhThanhVaoComboBox() {
			cboAddTinh->Items->Clear();
			// Duyệt qua Key của Dictionary (Tên các tỉnh)
			for each (String ^ tinh in mapDiaChinh->Keys) {
				cboAddTinh->Items->Add(tinh);
			}
		}

		// 2. Sự kiện: Khi chọn Tỉnh -> Tự động nạp Huyện tương ứng
		void OnTinhChanged(Object^ sender, EventArgs^ e) {
			cboAddHuyen->Items->Clear();
			cboAddHuyen->Text = ""; // Reset text huyện

			if (cboAddTinh->SelectedItem != nullptr) {
				String^ tenTinh = cboAddTinh->SelectedItem->ToString();
				if (mapDiaChinh->ContainsKey(tenTinh)) {
					List<String^>^ dsHuyen = mapDiaChinh[tenTinh];
					// Nạp huyện vào combobox 2
					for each (String ^ huyen in dsHuyen) {
						cboAddHuyen->Items->Add(huyen);
					}
				}
			}
		}
		// --- HÀM TẠO THẺ SẢN PHẨM (CÓ TÍNH NĂNG CLICK ĐỂ SỬA) ---
		Panel^ CreateProductCard(String^ maSP, String^ tenSP, double gia, String^ loai) {
			int cardW = 180; int cardH = 260;
			Panel^ pnl = gcnew Panel();
			pnl->Size = System::Drawing::Size(cardW, cardH);
			pnl->BackColor = Color::FromArgb(235, 248, 255); // Xanh nhạt
			pnl->Margin = System::Windows::Forms::Padding(10);
			pnl->Tag = maSP; // Lưu mã để xử lý click

			// [MỚI] Gắn sự kiện Click vào Panel nền + Đổi con trỏ chuột
			pnl->Cursor = Cursors::Hand;
			pnl->Click += gcnew EventHandler(this, &QuanLySanPhamForm::OnProductCardClick);

			// 1. Ảnh / Placeholder
			PictureBox^ pic = gcnew PictureBox();
			pic->Size = System::Drawing::Size(140, 120);
			pic->Location = Point(20, 10);
			pic->BackColor = Color::White;
			pic->SizeMode = PictureBoxSizeMode::Zoom;
			pic->Tag = tenSP; // Để vẽ chữ cái đầu (Placeholder)

			// [MỚI] Gắn sự kiện Click vào Ảnh + Đổi con trỏ chuột
			pic->Cursor = Cursors::Hand;
			pic->Click += gcnew EventHandler(this, &QuanLySanPhamForm::OnProductCardClick);

			if (imageMap->ContainsKey(maSP)) {
				try { pic->ImageLocation = imageMap[maSP]; }
				catch (...) { pic->Paint += gcnew PaintEventHandler(this, &QuanLySanPhamForm::DrawPlaceholderImage); }
			}
			else {
				pic->Paint += gcnew PaintEventHandler(this, &QuanLySanPhamForm::DrawPlaceholderImage);
			}
			pnl->Controls->Add(pic);

			// 2. Nút XÓA (X) - Đặc quyền Admin (KHÔNG GẮN SỰ KIỆN SỬA VÀO NÚT NÀY)
			Button^ btnDel = gcnew Button();
			btnDel->Text = L"X";
			btnDel->Size = System::Drawing::Size(25, 25);
			btnDel->Location = Point(cardW - 25, 0); // Góc trên phải
			btnDel->FlatStyle = FlatStyle::Flat;
			btnDel->FlatAppearance->BorderSize = 0;
			btnDel->BackColor = Color::Transparent;
			btnDel->ForeColor = Color::Red;
			btnDel->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9, FontStyle::Bold));
			btnDel->Cursor = Cursors::Hand;
			btnDel->Tag = maSP; // Lưu mã vào nút để biết xóa cái nào
			btnDel->Click += gcnew EventHandler(this, &QuanLySanPhamForm::OnCardDeleteClick);
			pnl->Controls->Add(btnDel);
			btnDel->BringToFront(); // Đảm bảo nút xóa nằm trên cùng

			// 3. Tên SP
			Label^ lblName = gcnew Label();
			lblName->Text = tenSP;
			lblName->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, FontStyle::Bold));
			lblName->ForeColor = Color::Black;
			lblName->Location = Point(5, 140);
			lblName->Size = System::Drawing::Size(cardW - 10, 45); // Cao 2 dòng
			lblName->TextAlign = ContentAlignment::TopCenter;

			// [MỚI] Click vào tên cũng mở form sửa
			lblName->Cursor = Cursors::Hand;
			lblName->Click += gcnew EventHandler(this, &QuanLySanPhamForm::OnProductCardClick);

			pnl->Controls->Add(lblName);

			// 4. Giá
			Label^ lblPrice = gcnew Label();
			lblPrice->Text = gia.ToString("N0") + L" đ";
			lblPrice->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, FontStyle::Bold));
			lblPrice->ForeColor = Color::Red;
			lblPrice->AutoSize = false;
			lblPrice->TextAlign = ContentAlignment::MiddleCenter;
			lblPrice->Location = Point(0, 190);
			lblPrice->Size = System::Drawing::Size(cardW, 25);

			// [MỚI] Click vào giá cũng mở form sửa
			lblPrice->Cursor = Cursors::Hand;
			lblPrice->Click += gcnew EventHandler(this, &QuanLySanPhamForm::OnProductCardClick);

			pnl->Controls->Add(lblPrice);

			// 5. Chữ cái đại diện (Loại)
			Label^ lblTag = gcnew Label();
			lblTag->Text = loai;
			lblTag->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8, FontStyle::Regular));
			lblTag->ForeColor = Color::Gray;
			lblTag->AutoSize = false;
			lblTag->TextAlign = ContentAlignment::MiddleCenter;
			lblTag->Location = Point(0, 220);
			lblTag->Size = System::Drawing::Size(cardW, 20);

			// [MỚI] Click vào loại cũng mở form sửa
			lblTag->Cursor = Cursors::Hand;
			lblTag->Click += gcnew EventHandler(this, &QuanLySanPhamForm::OnProductCardClick);

			pnl->Controls->Add(lblTag);

			return pnl;
		}

		// Hàm vẽ chữ cái đầu (Placeholder)
		void DrawPlaceholderImage(Object^ sender, PaintEventArgs^ e) {
			PictureBox^ p = (PictureBox^)sender;
			String^ name = (String^)p->Tag;
			if (String::IsNullOrEmpty(name)) return;

			int hash = name->GetHashCode();
			Random^ rnd = gcnew Random(hash);
			Color bgCol = Color::FromArgb(200 + rnd->Next(55), 200 + rnd->Next(55), 200 + rnd->Next(55)); // Màu pastel ngẫu nhiên
			p->BackColor = bgCol;

			String^ firstLetter = name->Substring(0, 1)->ToUpper();
			System::Drawing::Font^ f = gcnew System::Drawing::Font("Segoe UI", 36, FontStyle::Bold);
			SizeF s = e->Graphics->MeasureString(firstLetter, f);
			e->Graphics->DrawString(firstLetter, f, Brushes::DarkSlateGray, (p->Width - s.Width) / 2, (p->Height - s.Height) / 2);
		}
		// --- SỰ KIỆN NÚT XÓA TRÊN CARD (Đã rút gọn - An toàn dữ liệu) ---
		void OnCardDeleteClick(Object^ sender, EventArgs^ e) {
			Button^ btn = (Button^)sender;
			String^ maSP = (String^)btn->Tag;

			// Chỉ hỏi xác nhận gỡ liên kết
			if (MessageBox::Show(L"Bạn muốn gỡ sản phẩm [" + maSP + L"] khỏi NCC này?",
				L"Xác nhận", MessageBoxButtons::YesNo, MessageBoxIcon::Question) == System::Windows::Forms::DialogResult::Yes) {

				// 1. CHỈ XÓA LIÊN KẾT (CungCapSanPham)
				// Việc này an toàn 100%, không bao giờ bị lỗi khóa ngoại (Foreign Key)
				String^ sqlDelLink = "DELETE FROM CungCapSanPham WHERE MaNCC='" + currentMaNCC + "' AND MaSanPham='" + maSP + "'";
				ExecuteQuery(sqlDelLink);

				// --- ĐÃ BỎ ĐOẠN CODE XÓA VĨNH VIỄN Ở ĐÂY ---
				// (Sản phẩm vẫn nằm trong bảng SanPham, chỉ là không thuộc NCC này nữa thôi)

				// Load lại giao diện để thấy thẻ biến mất
				LoadDSSanPhamCuaNCC(currentMaNCC);
			}
		}
		void OnBtnChonAnh_Click(Object^ sender, EventArgs^ e) {
			OpenFileDialog^ ofd = gcnew OpenFileDialog();
			ofd->Filter = "Image Files|*.jpg;*.jpeg;*.png;*.bmp";
			if (ofd->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
				try {
					this->picPreview->ImageLocation = ofd->FileName;
					this->txtPathAnh->Text = ofd->FileName;
				}
				catch (...) {}
			}
		}
		String^ GenerateAutoID() {
			DateTime now = DateTime::Now;
			return "SP" + now.ToString("ddHHmm"); // VD: SP171830
		}
		
		void OnNCC_Click(Object^ sender, DataGridViewCellEventArgs^ e) {
			if (e->RowIndex >= 0) {
				String^ ma = dgvNCC->Rows[e->RowIndex]->Cells["MaNCC"]->Value->ToString();
				String^ ten = dgvNCC->Rows[e->RowIndex]->Cells["TenNCC"]->Value->ToString();
				this->currentMaNCC = ma;
				this->lblCurrentNCC->Text = L"Đang xem: " + ten + " (" + ma + ")";
				LoadDSSanPhamCuaNCC(ma);
			}
		}
		void OnCancelNewNCC(Object^ sender, EventArgs^ e) {
			pnlAddNCCInput->Visible = false;
		}
		void OnBtnXoaNCC_Click(Object^ sender, EventArgs^ e) {
			if (String::IsNullOrEmpty(currentMaNCC)) return;
			if (MessageBox::Show(L"Xóa NCC này?", "Cảnh báo", MessageBoxButtons::YesNo) == System::Windows::Forms::DialogResult::Yes) {
				ExecuteQuery("DELETE FROM CungCapSanPham WHERE MaNCC = '" + currentMaNCC + "'");
				ExecuteQuery("DELETE FROM NhaCungCap WHERE MaNCC = '" + currentMaNCC + "'");
				LoadDSNhaCungCap();
				flowPnlSP->Controls->Clear(); // Clear cards
				currentMaNCC = ""; lblCurrentNCC->Text = "";
			}
		}
		void OnModeChanged(Object^ sender, EventArgs^ e) {
			if (radChonCoSan->Checked) {
				pnlModeExisting->Visible = true; pnlModeNew->Visible = false; grpInfoChiTiet->Visible = true;
				btnLuuSP->Text = L"Liên kết NCC"; btnLuuSP->Location = Point(120, 420); btnHuySP->Location = Point(320, 420);
			}
			else {
				pnlModeExisting->Visible = false; pnlModeNew->Visible = true; grpInfoChiTiet->Visible = false;
				btnLuuSP->Text = L"Lưu & Liên kết"; btnLuuSP->Location = Point(120, 450); btnHuySP->Location = Point(320, 450);

				LoadLoaiSuggestions();

				// Sinh mã tự động
				txtNewMaSP->Text = GenerateAutoID();

				txtNewTenSP->Clear(); cboNewLoai->Text = ""; //txtNewNhom->Text = L"Hàng Đông Lạnh";

				// [FIX] Reset nhiệt độ nhập tay
				txtNewNhietDo->Text = "25";

				picPreview->Image = nullptr; txtPathAnh->Clear();
			}
		}
		void OnBtnThemSP_Click(Object^ sender, EventArgs^ e) {
			// Kiểm tra đã chọn NCC chưa
			if (String::IsNullOrEmpty(currentMaNCC)) {
				MessageBox::Show(L"Vui lòng chọn Nhà Cung Cấp ở danh sách bên trái trước!");
				return;
			}

			// --- [MỚI] RESET TRẠNG THÁI VỀ "THÊM MỚI" ---
			isEditing = false; // Đặt cờ hiệu là thêm mới
			btnLuuSP->Text = L"LIÊN KẾT / LƯU"; // Đổi lại tên nút

			// Mở khóa các nút chọn chế độ (vì khi Sửa ta đã khóa nó lại)
			radChonCoSan->Enabled = true;
			radTaoMoi->Enabled = true;

			// Mặc định chọn tab "Có sẵn"
			radChonCoSan->Checked = true;
			pnlModeExisting->Visible = true;
			pnlModeNew->Visible = false;

			// --- LOAD DỮ LIỆU GỢI Ý (Code cũ) ---
			String^ sql = "SELECT MaSanPham, TenSanPham FROM SanPham WHERE MaSanPham NOT IN (SELECT MaSanPham FROM CungCapSanPham WHERE MaNCC = '" + currentMaNCC + "')";
			DataTable^ dt = GetDataTable(sql);
			dt->Columns->Add("DisplayInfo", String::typeid, "MaSanPham + ' - ' + TenSanPham");
			cboGoiYSanPham->DataSource = dt;
			cboGoiYSanPham->DisplayMember = "DisplayInfo";
			cboGoiYSanPham->ValueMember = "MaSanPham";

			// Hiển thị Form
			pnlAddSPInput->Visible = true;
			pnlAddSPInput->BringToFront();
		}
		// --- MỞ FORM THÊM NCC ---
		void OnBtnThemNCC_Click(Object^ sender, EventArgs^ e) {
			// RESET TRẠNG THÁI VỀ THÊM MỚI
			isEditingNCC = false;
			btnLuuNCC->Text = L"LƯU THÔNG TIN";
			txtAddMaNCC->ReadOnly = false; // Cho phép nhập mã
			txtAddMaNCC->BackColor = Color::White;

			pnlAddNCCInput->Visible = true;
			pnlAddNCCInput->BringToFront();

			// Xóa trắng các ô
			txtAddMaNCC->Clear();
			txtAddTenNCC->Clear();
			txtAddSDT->Clear();
			cboAddTinh->SelectedIndex = -1;
			cboAddHuyen->Items->Clear(); cboAddHuyen->Text = "";
			txtAddDiaChi->Clear();
			txtAddChiPhi->Text = "0";
			txtAddThoiGian->Text = "0";
		}
		// --- [MỚI] HÀM XỬ LÝ KHI BẤM NÚT SỬA NCC ---
		void OnBtnSuaNCC_Click(Object^ sender, EventArgs^ e) {
			if (String::IsNullOrEmpty(currentMaNCC)) {
				MessageBox::Show(L"Vui lòng chọn Nhà cung cấp cần sửa trong danh sách!");
				return;
			}

			// 1. Chuyển sang chế độ SỬA
			isEditingNCC = true;
			pnlAddNCCInput->Visible = true;
			pnlAddNCCInput->BringToFront();

			// Khóa mã NCC (Không cho sửa khóa chính)
			txtAddMaNCC->ReadOnly = true;
			txtAddMaNCC->BackColor = Color::LightGray;

			btnLuuNCC->Text = L"CẬP NHẬT"; // Đổi tên nút

			// 2. Lấy Full thông tin từ DB
			String^ sql = "SELECT * FROM NhaCungCap WHERE MaNCC = '" + currentMaNCC + "'";
			DataTable^ dt = GetDataTable(sql);

			if (dt->Rows->Count > 0) {
				DataRow^ r = dt->Rows[0];

				// Đổ dữ liệu text
				txtAddMaNCC->Text = r["MaNCC"]->ToString();
				txtAddTenNCC->Text = r["TenNCC"]->ToString();
				txtAddSDT->Text = r["SDT"]->ToString();
				txtAddDiaChi->Text = r["DiaChi"]->ToString();

				// Xử lý số
				txtAddChiPhi->Text = (r["ChiPhiDat"] != DBNull::Value) ? Convert::ToDouble(r["ChiPhiDat"]).ToString("0.##") : "0";
				txtAddThoiGian->Text = (r["ThoiGianCho"] != DBNull::Value) ? Convert::ToDouble(r["ThoiGianCho"]).ToString("0.##") : "0";

				// Xử lý ComboBox Tỉnh/Huyện
				String^ tinhDB = r["Tinh"]->ToString();
				String^ huyenDB = r["QuanHuyen"]->ToString();

				// Chọn Tỉnh -> Code trong OnTinhChanged sẽ tự chạy để load Huyện
				cboAddTinh->Text = tinhDB;

				// Chọn Huyện (Phải đợi list huyện load xong - ComboBox Text set sẽ tự khớp nếu có item)
				cboAddHuyen->Text = huyenDB;
			}
		}
		// --- LƯU NHÀ CUNG CẤP (FULL DATA) ---
		void OnSaveNewNCC(Object^ sender, EventArgs^ e) {
			// 1. Lấy dữ liệu
			String^ ma = txtAddMaNCC->Text->Trim();
			String^ ten = txtAddTenNCC->Text->Trim();
			String^ sdt = txtAddSDT->Text->Trim();
			String^ tinh = cboAddTinh->Text->Trim();
			String^ huyen = cboAddHuyen->Text->Trim();
			String^ dc = txtAddDiaChi->Text->Trim();
			String^ chiphi = txtAddChiPhi->Text->Trim()->Replace(",", "."); // Thêm replace
			if (chiphi == "") chiphi = "0";
			String^ thoigian = txtAddThoiGian->Text->Trim()->Replace(",", "."); // Thêm replace
			if (thoigian == "") thoigian = "0";

			if (ma == "" || ten == "") { MessageBox::Show(L"Mã và Tên không được để trống!"); return; }

			// =========================================================
			// TRƯỜNG HỢP 1: ĐANG SỬA (UPDATE)
			// =========================================================
			if (isEditingNCC) {
				try {
					// Cập nhật tất cả trừ MaNCC
					String^ sqlUpdate = String::Format(
						"UPDATE NhaCungCap SET TenNCC=N'{0}', SDT='{1}', Tinh=N'{2}', QuanHuyen=N'{3}', " +
						"DiaChi=N'{4}', ChiPhiDat={5}, ThoiGianCho={6} WHERE MaNCC='{7}'",
						ten, sdt, tinh, huyen, dc, chiphi, thoigian, ma
					);
					ExecuteQuery(sqlUpdate);

					MessageBox::Show(L"Cập nhật NCC thành công!");
					LoadDSNhaCungCap(); // Refresh Grid
					pnlAddNCCInput->Visible = false;
				}
				catch (Exception^ ex) { MessageBox::Show(L"Lỗi cập nhật: " + ex->Message); }
				return; // Kết thúc
			}

			// =========================================================
			// TRƯỜNG HỢP 2: THÊM MỚI (INSERT) - Code cũ
			// =========================================================
			try {
				String^ sqlInsert = String::Format(
					"INSERT INTO NhaCungCap (MaNCC, TenNCC, SDT, Tinh, QuanHuyen, DiaChi, ChiPhiDat, ThoiGianCho) " +
					"VALUES ('{0}', N'{1}', '{2}', N'{3}', N'{4}', N'{5}', {6}, {7})",
					ma, ten, sdt, tinh, huyen, dc, chiphi, thoigian
				);
				ExecuteQuery(sqlInsert);

				MessageBox::Show(L"Thêm Nhà cung cấp thành công!");
				LoadDSNhaCungCap();
				pnlAddNCCInput->Visible = false;
			}
			catch (Exception^ ex) { MessageBox::Show(L"Lỗi thêm mới: " + ex->Message); }
		}
		// HÀM 1: Tự động load danh sách các Loại đã có để gợi ý
		void LoadLoaiSuggestions() {
			cboNewLoai->Items->Clear();
			try {
				// Lấy danh sách Loại duy nhất từ DB
				String^ sql = "SELECT DISTINCT Loai FROM SanPham WHERE Loai IS NOT NULL AND Loai <> ''";
				DataTable^ dt = GetDataTable(sql);
				for each (DataRow ^ row in dt->Rows) {
					cboNewLoai->Items->Add(row["Loai"]->ToString());
				}
			}
			catch (...) {}
		}
		void OnLoaiChanged(Object^ sender, EventArgs^ e) {
			String^ currentLoai = cboNewLoai->Text->Trim();
			if (String::IsNullOrEmpty(currentLoai)) return;

			// Kiểm tra trong DB xem loại này thuộc nhóm nào
			String^ sql = "SELECT TOP 1 NhomPhanLoai FROM SanPham WHERE Loai = N'" + currentLoai + "'";
			DataTable^ dt = GetDataTable(sql);

			if (dt->Rows->Count > 0) {
				// [SỬA LẠI ĐOẠN NÀY]
				// Tách ra biến row để tránh lỗi cú pháp
				DataRow^ row = dt->Rows[0];
				txtNewNhom->Text = row["NhomPhanLoai"]->ToString();
			}
		}
		void OnProductCardClick(Object^ sender, EventArgs^ e) {
			Control^ ctrl = (Control^)sender;
			String^ maSP = "";

			// --- SỬA LOGIC LẤY MÃ SẢN PHẨM ---

			// 1. Nếu click vào Panel nền (Cái thẻ to nhất)
			// Cách nhận biết: Panel này chứa các control con, nên nó là cha
			if (ctrl->GetType() == Panel::typeid) {
				if (ctrl->Tag != nullptr) maSP = ctrl->Tag->ToString();
			}
			// 2. Nếu click vào Ảnh, Tên, Giá... (Các thành phần con bên trong)
			// Thì phải lấy Tag của thằng CHA nó (tức là cái Panel nền)
			else {
				if (ctrl->Parent != nullptr && ctrl->Parent->Tag != nullptr) {
					maSP = ctrl->Parent->Tag->ToString();
				}
			}

			if (maSP == "") return; // Nếu không tìm thấy mã thì thoát

			// --- PHẦN DƯỚI GIỮ NGUYÊN ---
			isEditing = true;
			pnlAddSPInput->Visible = true;
			pnlAddSPInput->BringToFront();

			radChonCoSan->Checked = false;
			radTaoMoi->Checked = true;
			pnlModeExisting->Visible = false;
			pnlModeNew->Visible = true;

			radChonCoSan->Enabled = false;
			radTaoMoi->Enabled = false;

			btnLuuSP->Text = L"CẬP NHẬT";

			String^ sql = "SELECT * FROM SanPham WHERE MaSanPham = '" + maSP + "'";
			DataTable^ dt = GetDataTable(sql);

			if (dt->Rows->Count > 0) {
				DataRow^ r = dt->Rows[0];

				txtNewMaSP->Text = r["MaSanPham"]->ToString();
				txtNewTenSP->Text = r["TenSanPham"]->ToString();
				cboNewLoai->Text = r["Loai"]->ToString();

				// Xử lý dấu chấm/phẩy khi hiển thị lên lại form
				txtNewGiaVon->Text = (r["GiaVon"] != DBNull::Value) ? Convert::ToDouble(r["GiaVon"]).ToString("0.##") : "0";
				txtNewGiaBan->Text = (r["GiaBan"] != DBNull::Value) ? Convert::ToDouble(r["GiaBan"]).ToString("0.##") : "0";
				txtNewTheTich->Text = (r["TheTich1Tan"] != DBNull::Value) ? Convert::ToDouble(r["TheTich1Tan"]).ToString("0.##") : "0";
				txtNewNhietDo->Text = (r["NhietDoThichHop"] != DBNull::Value) ? Convert::ToDouble(r["NhietDoThichHop"]).ToString("0.##") : "25";

				txtNewHSDTieuChuan->Text = r["HanSuDungTieuChuan"]->ToString();
				txtNewRuiRoLanh->Text = r["RuiRoLanh"]->ToString();
				txtNewRuiRoNong->Text = r["RuiRoNong"]->ToString();

				if (imageMap->ContainsKey(maSP)) {
					try {
						picPreview->ImageLocation = imageMap[maSP];
						txtPathAnh->Text = imageMap[maSP];
					}
					catch (...) {}
				}
				else {
					picPreview->Image = nullptr;
					txtPathAnh->Clear();
				}
			}
		}
		void OnCancelNewSP(Object^ sender, EventArgs^ e) { pnlAddSPInput->Visible = false; }
		// --- [MỚI] HÀM LƯU ẢNH: XÓA CŨ - GHI MỚI (TRÁNH TRÙNG LẶP) ---
		void SaveImageToTextFile(String^ maSP, String^ path) {
			String^ filename = "ProductImages.txt";
			List<String^>^ newLines = gcnew List<String^>();

			// 1. Đọc file cũ và lọc bỏ dòng chứa ID này
			if (File::Exists(filename)) {
				cli::array<String^>^ lines = File::ReadAllLines(filename);
				for each (String ^ line in lines) {
					if (String::IsNullOrWhiteSpace(line)) continue;

					// Tách ID để kiểm tra (Format: ID|Path)
					cli::array<String^>^ parts = line->Split('|');
					if (parts->Length > 0) {
						String^ currentID = parts[0]->Trim();
						// Nếu ID khác ID đang lưu thì giữ lại. 
						// Nếu trùng thì bỏ qua (coi như xóa)
						if (currentID != maSP) {
							newLines->Add(line);
						}
					}
				}
			}

			// 2. Thêm dòng mới vào danh sách
			newLines->Add(maSP + "|" + path);

			// 3. Ghi đè lại toàn bộ file
			try {
				File::WriteAllLines(filename, newLines);
			}
			catch (Exception^ ex) {
				MessageBox::Show("Lỗi ghi file ảnh: " + ex->Message);
			}
		}
		void OnSaveNewSP(Object^ sender, EventArgs^ e) {
			String^ maSP = txtNewMaSP->Text->Trim();

			// Lấy dữ liệu từ form và CHUYỂN DẤU PHẨY THÀNH DẤU CHẤM
			String^ tenSP = txtNewTenSP->Text->Trim();
			String^ loai = cboNewLoai->Text->Trim();

			String^ nhietDo = txtNewNhietDo->Text->Trim()->Replace(",", ".");
			if (nhietDo == "") nhietDo = "25";

			String^ giaVon = txtNewGiaVon->Text->Trim()->Replace(",", ".");
			if (giaVon == "") giaVon = "0";

			String^ giaBan = txtNewGiaBan->Text->Trim()->Replace(",", ".");
			if (giaBan == "") giaBan = "0";

			String^ theTich = txtNewTheTich->Text->Trim()->Replace(",", ".");
			if (theTich == "") theTich = "0";

			String^ hsdChuan = txtNewHSDTieuChuan->Text->Trim();
			if (hsdChuan == "") hsdChuan = "365";

			String^ rrLanh = txtNewRuiRoLanh->Text->Trim()->Replace(",", ".");
			if (rrLanh == "") rrLanh = "0";

			String^ rrNong = txtNewRuiRoNong->Text->Trim()->Replace(",", ".");
			if (rrNong == "") rrNong = "0";

			// ========================================================================
			// PHẦN 1: XỬ LÝ CẬP NHẬT THÔNG TIN (EDIT MODE)
			// ========================================================================
			if (isEditing) {
				if (tenSP == "") { MessageBox::Show(L"Tên sản phẩm không được để trống!"); return; }

				try {
					// Câu lệnh UPDATE
					String^ sqlUpdate = String::Format(
						"UPDATE SanPham SET TenSanPham=N'{0}', Loai=N'{1}', GiaVon={2}, TheTich1Tan={3}, " +
						"NhietDoThichHop={4}, RuiRoLanh={5}, RuiRoNong={6}, HanSuDungTieuChuan={7}, GiaBan={8} " +
						"WHERE MaSanPham='{9}'",
						tenSP, loai, giaVon, theTich, nhietDo, rrLanh, rrNong, hsdChuan, giaBan, maSP
					);

					ExecuteQuery(sqlUpdate);

					// [SỬA] Cập nhật ảnh nếu có đường dẫn mới
					if (!String::IsNullOrEmpty(txtPathAnh->Text)) {
						// Cập nhật Map bộ nhớ
						if (imageMap->ContainsKey(maSP)) imageMap[maSP] = txtPathAnh->Text;
						else imageMap->Add(maSP, txtPathAnh->Text);

						// [GỌI HÀM MỚI] Lưu xuống file txt (Xóa cũ ghi mới)
						SaveImageToTextFile(maSP, txtPathAnh->Text);
					}

					MessageBox::Show(L"Cập nhật thông tin thành công!");
					LoadDSSanPhamCuaNCC(currentMaNCC);
					pnlAddSPInput->Visible = false;
				}
				catch (Exception^ ex) {
					MessageBox::Show(L"Lỗi cập nhật: " + ex->Message);
				}
				return;
			}

			// ========================================================================
			// PHẦN 2: XỬ LÝ THÊM MỚI (INSERT MODE)
			// ========================================================================
			if (radChonCoSan->Checked) {
				if (cboGoiYSanPham->SelectedValue == nullptr) return;
				maSP = cboGoiYSanPham->SelectedValue->ToString();
			}
			else {
				// TẠO MỚI HOÀN TOÀN
				if (tenSP == "") { MessageBox::Show(L"Thiếu tên SP!"); return; }

				try {
					String^ sqlCreate = String::Format(
						"INSERT INTO SanPham (MaSanPham, TenSanPham, Loai, GiaVon, TheTich1Tan, NhietDoThichHop, RuiRoLanh, RuiRoNong, HanSuDungTieuChuan, GiaBan) " +
						"VALUES ('{0}', N'{1}', N'{2}', {3}, {4}, {5}, {6}, {7}, {8}, {9})",
						maSP, tenSP, loai, giaVon, theTich, nhietDo, rrLanh, rrNong, hsdChuan, giaBan
					);

					ExecuteQuery(sqlCreate);

					// [SỬA] Lưu ảnh xuống file
					if (!String::IsNullOrEmpty(txtPathAnh->Text)) {
						// Gọi hàm lưu thông minh
						SaveImageToTextFile(maSP, txtPathAnh->Text);

						if (!imageMap->ContainsKey(maSP)) imageMap->Add(maSP, txtPathAnh->Text);
					}
				}
				catch (Exception^ ex) { MessageBox::Show(L"Lỗi tạo SP mới: " + ex->Message); return; }
			}

			// LIÊN KẾT VỚI NHÀ CUNG CẤP
			try {
				ExecuteQuery("INSERT INTO CungCapSanPham (MaNCC, MaSanPham) VALUES ('" + currentMaNCC + "', '" + maSP + "')");
				MessageBox::Show(L"Liên kết thành công!");

				LoadDSSanPhamCuaNCC(currentMaNCC);
				pnlAddSPInput->Visible = false;

				if (!radChonCoSan->Checked) {
					txtNewMaSP->Text = GenerateAutoID();
					txtNewTenSP->Clear();
				}
			}
			catch (Exception^ ex) { MessageBox::Show(L"Lỗi liên kết NCC: " + ex->Message); }
		}
		void UpdateTotalMoney(Object^ sender, EventArgs^ e) {
			try {
				// Lấy số lượng từ NumericUpDown
				double sl = (double)numSoLuong->Value;

				// Lấy giá từ TextBox (cần parse an toàn tránh lỗi khi để trống)
				double gia = 0;
				if (Double::TryParse(txtGiaNhap->Text, gia)) {
					double tong = sl * gia;
					// Format tiền tệ (ví dụ: 100,000 đ)
					lblThanhTien->Text = tong.ToString("N0") + L" đ";
				}
				else {
					lblThanhTien->Text = L"0 đ";
				}
			}
			catch (...) {}
		}

		// --- [2] HÀM XỬ LÝ KHI CHỌN SP CÓ SẴN (Sửa lỗi OnExistingProductSelected) ---
		void OnExistingProductSelected(Object^ sender, EventArgs^ e) {
			if (cboGoiYSanPham->SelectedValue != nullptr) {
				String^ maSelected = cboGoiYSanPham->SelectedValue->ToString();

				// 1. HIỆN ẢNH
				if (imageMap->ContainsKey(maSelected)) {
					try { picInfoAnh->ImageLocation = imageMap[maSelected]; }
					catch (...) { picInfoAnh->Image = nullptr; }
				}
				else { picInfoAnh->Image = nullptr; }

				// 2. LẤY FULL THÔNG TIN TỪ DB ĐỂ HIỆN THỊ
				String^ sql = "SELECT * FROM SanPham WHERE MaSanPham = '" + maSelected + "'";
				DataTable^ dt = GetDataTable(sql);

				if (dt->Rows->Count > 0) {
					DataRow^ r = dt->Rows[0];
					lblInfoMa->Text = L"Mã: " + r["MaSanPham"]->ToString();
					lblInfoTen->Text = r["TenSanPham"]->ToString();
					lblInfoLoai->Text = L"Loại: " + r["Loai"]->ToString();

					// Format tiền tệ
					double gia = 0;
					if (r["GiaVon"] != DBNull::Value) {
						gia = Convert::ToDouble(r["GiaVon"]);
					}
					lblInfoGia->Text = L"Giá vốn: " + gia.ToString("N0") + " VND";

					lblInfoNhietDo->Text = L"Bảo quản: " + r["NhietDoThichHop"]->ToString() + L"°C";
					lblInfoHSD->Text = L"HSD Chuẩn: " + r["HanSuDungTieuChuan"]->ToString() + L" ngày";
				}
			}
		}
	};
}