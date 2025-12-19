#pragma once

// --- CÁC THƯ VIỆN CẦN THIẾT ---
using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::Data::Odbc;
// [QUAN TRỌNG] Thư viện biểu đồ
using namespace System::Windows::Forms::DataVisualization::Charting;

namespace PBL2QuanLyKho {

	public ref class QuanLyLuuChuyenForm : public System::Windows::Forms::Form
	{
	public:
		QuanLyLuuChuyenForm(void)
		{
			InitializeComponent();

			// 1. Load dữ liệu ban đầu
			LoadDataPhuongTien();
			LoadComboboxData();
			LoadDataTuyen();
			LoadDataTuyenNhap();

			// 2. Thiết lập mặc định cho Thống Kê
			dtpTuNgay->Value = DateTime(DateTime::Now.Year, DateTime::Now.Month, 1);
			dtpDenNgay->Value = DateTime::Now;

			// 3. [MỚI] Tự động sinh mã cho lần chạy đầu tiên
			ResetAllAutoCodes();
		}

	protected:
		~QuanLyLuuChuyenForm()
		{
			if (components) delete components;
		}

	private:
		// --- KHAI BÁO BIẾN UI ---
		System::Windows::Forms::TabControl^ tabControlLuuChuyen;
		System::ComponentModel::Container^ components;

		// CHUỖI KẾT NỐI
		String^ strConn = "Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;";

		// =========================================================
		// 1. TAB PHƯƠNG TIỆN
		// =========================================================
		System::Windows::Forms::TabPage^ tabPhuongTien;
		System::Windows::Forms::DataGridView^ dgvPhuongTien;
		System::Windows::Forms::GroupBox^ grpThongTinXe;
		System::Windows::Forms::TextBox^ txtMaXe;
		System::Windows::Forms::TextBox^ txtLoaiXe;
		System::Windows::Forms::TextBox^ txtSoXe;
		System::Windows::Forms::TextBox^ txtSucChua;
		System::Windows::Forms::TextBox^ txtChiPhiXe;
		System::Windows::Forms::TextBox^ txtVanToc;
		System::Windows::Forms::Button^ btnThemXe;
		System::Windows::Forms::Button^ btnSuaXe;
		System::Windows::Forms::Button^ btnXoaXe;
		System::Windows::Forms::Button^ btnLamMoiXe;

		// =========================================================
		// 2. TAB TUYẾN VẬN CHUYỂN
		// =========================================================
		System::Windows::Forms::TabPage^ tabTuyen;
		System::Windows::Forms::DataGridView^ dgvTuyen;
		System::Windows::Forms::GroupBox^ grpThongTinTuyen;
		System::Windows::Forms::TextBox^ txtMaTuyen;
		System::Windows::Forms::ComboBox^ cbKhoDi;
		System::Windows::Forms::ComboBox^ cbKhoTrungChuyen;
		System::Windows::Forms::ComboBox^ cbSieuThiNhan;
		System::Windows::Forms::ComboBox^ cbXePhuTrach;
		System::Windows::Forms::TextBox^ txtChiPhiTuyen;
		System::Windows::Forms::Button^ btnThemTuyen;
		System::Windows::Forms::Button^ btnSuaTuyen;
		System::Windows::Forms::Button^ btnXoaTuyen;
		System::Windows::Forms::Button^ btnLamMoiTuyen;

		// =========================================================
		// 3. TAB TUYẾN NHẬP HÀNG
		// =========================================================
		System::Windows::Forms::TabPage^ tabTuyenNhap;
		System::Windows::Forms::DataGridView^ dgvTuyenNhap;
		System::Windows::Forms::GroupBox^ grpTuyenNhap;
		System::Windows::Forms::TextBox^ txtMaTuyenNhap;
		System::Windows::Forms::ComboBox^ cbNhaCungCap;
		System::Windows::Forms::ComboBox^ cbKhoTGNhap;
		System::Windows::Forms::ComboBox^ cbKhoNhan;
		System::Windows::Forms::ComboBox^ cbXeNhap;
		System::Windows::Forms::TextBox^ txtChiPhiNhap;
		System::Windows::Forms::Button^ btnThemTuyenNhap;
		System::Windows::Forms::Button^ btnSuaTuyenNhap;
		System::Windows::Forms::Button^ btnXoaTuyenNhap;
		System::Windows::Forms::Button^ btnLamMoiTuyenNhap;

		// =========================================================
		// 4. TAB THỐNG KÊ
		// =========================================================
		System::Windows::Forms::TabPage^ tabThongKe;
		System::Windows::Forms::Panel^ pnlDieuKhienTK;
		System::Windows::Forms::DateTimePicker^ dtpTuNgay;
		System::Windows::Forms::DateTimePicker^ dtpDenNgay;
		System::Windows::Forms::Button^ btnXemBieuDo;
		System::Windows::Forms::TabControl^ tabTKSub;
		System::Windows::Forms::TabPage^ tabSubCot;
		System::Windows::Forms::TabPage^ tabSubTron;
		System::Windows::Forms::DataVisualization::Charting::Chart^ chartThongKe;
		System::Windows::Forms::DataVisualization::Charting::Chart^ chartPieNhap;
		System::Windows::Forms::DataVisualization::Charting::Chart^ chartPieXuat;
		System::Windows::Forms::TableLayoutPanel^ pnlPieLayout;

		// --- HÀM KHỞI TẠO UI CHÍNH ---
		void InitializeComponent(void)
		{
			this->tabControlLuuChuyen = (gcnew System::Windows::Forms::TabControl());
			this->SuspendLayout();
			this->BackColor = Color::White;
			this->Text = L"Hệ Thống Quản Lý Lưu Chuyển & Thống Kê";
			this->Size = System::Drawing::Size(1280, 768);
			this->tabControlLuuChuyen->Dock = System::Windows::Forms::DockStyle::Fill;
			this->tabControlLuuChuyen->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10));

			InitTabPhuongTien();
			InitTabTuyen();
			InitTabTuyenNhap();
			InitTabThongKe();

			this->tabControlLuuChuyen->Controls->Add(this->tabPhuongTien);
			this->tabControlLuuChuyen->Controls->Add(this->tabTuyenNhap);
			this->tabControlLuuChuyen->Controls->Add(this->tabTuyen);
			this->tabControlLuuChuyen->Controls->Add(this->tabThongKe);
			this->Controls->Add(this->tabControlLuuChuyen);
			this->ResumeLayout(false);
		}

		// =========================================================
		// [MỚI] HÀM SINH MÃ TỰ ĐỘNG THÔNG MINH
		// =========================================================
		// Input: Table="PhuongTien", Col="MaXe", Prefix="XE" -> Output: "XE001", "XE002"...
		String^ GenerateAutoCode(String^ tableName, String^ colName, String^ prefix) {
			String^ newCode = prefix + "001"; // Mặc định nếu chưa có dữ liệu
			try {
				OdbcConnection^ conn = gcnew OdbcConnection(strConn);
				conn->Open();
				// Lấy mã lớn nhất hiện tại (Sắp xếp theo độ dài rồi đến giá trị để tránh lỗi XE10 < XE2)
				String^ sql = "SELECT TOP 1 " + colName + " FROM " + tableName +
					" WHERE " + colName + " LIKE '" + prefix + "%' " +
					" ORDER BY LEN(" + colName + ") DESC, " + colName + " DESC";

				OdbcCommand^ cmd = gcnew OdbcCommand(sql, conn);
				Object^ result = cmd->ExecuteScalar();

				if (result != nullptr) {
					String^ lastCode = result->ToString();
					// Tách phần số: "XE005" -> lấy "005"
					String^ numberPart = lastCode->Substring(prefix->Length);
					int number = 0;
					if (Int32::TryParse(numberPart, number)) {
						number++; // Tăng 1
						// Format lại thành 3 chữ số: 6 -> "006"
						newCode = prefix + number.ToString("D3");
					}
				}
				conn->Close();
			}
			catch (Exception^ ex) { /* Lặng lẽ bỏ qua lỗi để không phiền user, dùng mã mặc định */ }
			return newCode;
		}

		// Hàm gọi sinh mã cho cả 3 tab
		void ResetAllAutoCodes() {
			txtMaXe->Text = GenerateAutoCode("PhuongTien", "MaXe", "XE");
			txtMaTuyen->Text = GenerateAutoCode("TuyenVanChuyen", "MaTuyen", "TX"); // TX: Tuyến Xuất
			txtMaTuyenNhap->Text = GenerateAutoCode("TuyenNhapHang", "MaTuyen", "TN"); // TN: Tuyến Nhập
		}

		// =========================================================
		// CODE GIAO DIỆN (ĐÃ UPDATE READONLY CHO MÃ)
		// =========================================================
		void InitTabThongKe() {
			this->tabThongKe = (gcnew System::Windows::Forms::TabPage()); this->tabThongKe->Text = L"📊 Thống Kê & Báo Cáo"; this->tabThongKe->BackColor = Color::White;
			this->pnlDieuKhienTK = (gcnew System::Windows::Forms::Panel()); this->pnlDieuKhienTK->Dock = DockStyle::Top; this->pnlDieuKhienTK->Height = 60; this->pnlDieuKhienTK->BackColor = Color::WhiteSmoke;
			Label^ l1 = gcnew Label(); l1->Text = L"Từ ngày:"; l1->Location = Point(20, 20); l1->AutoSize = true; dtpTuNgay = gcnew DateTimePicker(); dtpTuNgay->Format = DateTimePickerFormat::Short; dtpTuNgay->Location = Point(90, 17); dtpTuNgay->Size = System::Drawing::Size(110, 30);
			Label^ l2 = gcnew Label(); l2->Text = L"Đến ngày:"; l2->Location = Point(220, 20); l2->AutoSize = true; dtpDenNgay = gcnew DateTimePicker(); dtpDenNgay->Format = DateTimePickerFormat::Short; dtpDenNgay->Location = Point(300, 17); dtpDenNgay->Size = System::Drawing::Size(110, 30);
			btnXemBieuDo = gcnew Button(); btnXemBieuDo->Text = L"Xem Báo Cáo"; btnXemBieuDo->Location = Point(440, 15); btnXemBieuDo->Size = System::Drawing::Size(140, 35); btnXemBieuDo->BackColor = Color::FromArgb(0, 123, 255); btnXemBieuDo->ForeColor = Color::White; btnXemBieuDo->FlatStyle = FlatStyle::Flat; btnXemBieuDo->Click += gcnew EventHandler(this, &QuanLyLuuChuyenForm::OnBtnXemBieuDoClick);
			pnlDieuKhienTK->Controls->Add(l1); pnlDieuKhienTK->Controls->Add(dtpTuNgay); pnlDieuKhienTK->Controls->Add(l2); pnlDieuKhienTK->Controls->Add(dtpDenNgay); pnlDieuKhienTK->Controls->Add(btnXemBieuDo);
			this->tabTKSub = (gcnew System::Windows::Forms::TabControl()); this->tabTKSub->Dock = DockStyle::Fill;
			this->tabSubCot = (gcnew System::Windows::Forms::TabPage()); this->tabSubCot->Text = L"Doanh Thu & Chi Phí (Theo Kho)"; this->tabSubCot->BackColor = Color::White;
			this->chartThongKe = (gcnew System::Windows::Forms::DataVisualization::Charting::Chart()); this->chartThongKe->Dock = DockStyle::Fill;
			ChartArea^ ca1 = (gcnew ChartArea()); ca1->AxisX->Title = L"Kho"; ca1->AxisY->Title = L"VND"; ca1->AxisX->Interval = 1; this->chartThongKe->ChartAreas->Add(ca1); this->chartThongKe->Legends->Add(gcnew Legend());
			Series^ sThu = (gcnew Series()); sThu->Name = L"Thu (Đơn Hàng)"; sThu->ChartType = SeriesChartType::Column; sThu->Color = Color::SeaGreen; sThu->IsValueShownAsLabel = true;
			Series^ sChi = (gcnew Series()); sChi->Name = L"Chi (Vận Chuyển)"; sChi->ChartType = SeriesChartType::Column; sChi->Color = Color::IndianRed; sChi->IsValueShownAsLabel = true;
			this->chartThongKe->Series->Add(sThu); this->chartThongKe->Series->Add(sChi); this->chartThongKe->Titles->Add(L"BIỂU ĐỒ TÀI CHÍNH THEO KHO"); this->tabSubCot->Controls->Add(this->chartThongKe);
			this->tabSubTron = (gcnew System::Windows::Forms::TabPage()); this->tabSubTron->Text = L"Cơ Cấu Hàng Hóa (Theo Loại)"; this->tabSubTron->BackColor = Color::White;
			this->pnlPieLayout = (gcnew System::Windows::Forms::TableLayoutPanel()); this->pnlPieLayout->Dock = DockStyle::Fill; this->pnlPieLayout->ColumnCount = 2; this->pnlPieLayout->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent, 50))); this->pnlPieLayout->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent, 50)));
			this->chartPieNhap = (gcnew System::Windows::Forms::DataVisualization::Charting::Chart()); this->chartPieNhap->Dock = DockStyle::Fill; ChartArea^ caPie1 = (gcnew ChartArea()); this->chartPieNhap->ChartAreas->Add(caPie1); Legend^ lg1 = (gcnew Legend()); lg1->Docking = Docking::Bottom; this->chartPieNhap->Legends->Add(lg1); this->chartPieNhap->Titles->Add(L"TỶ TRỌNG NHẬP HÀNG");
			Series^ sPie1 = (gcnew Series()); sPie1->ChartType = SeriesChartType::Pie; sPie1->IsValueShownAsLabel = true; sPie1->Label = L"#VALX: #PERCENT"; sPie1->LegendText = L"#VALX"; sPie1->SmartLabelStyle->Enabled = true; sPie1->SmartLabelStyle->AllowOutsidePlotArea = LabelOutsidePlotAreaStyle::Yes; this->chartPieNhap->Series->Add(sPie1);
			this->chartPieXuat = (gcnew System::Windows::Forms::DataVisualization::Charting::Chart()); this->chartPieXuat->Dock = DockStyle::Fill; ChartArea^ caPie2 = (gcnew ChartArea()); this->chartPieXuat->ChartAreas->Add(caPie2); Legend^ lg2 = (gcnew Legend()); lg2->Docking = Docking::Bottom; this->chartPieXuat->Legends->Add(lg2); this->chartPieXuat->Titles->Add(L"TỶ TRỌNG XUẤT HÀNG");
			Series^ sPie2 = (gcnew Series()); sPie2->ChartType = SeriesChartType::Pie; sPie2->IsValueShownAsLabel = true; sPie2->Label = L"#VALX: #PERCENT"; sPie2->LegendText = L"#VALX"; sPie2->SmartLabelStyle->Enabled = true; sPie2->SmartLabelStyle->AllowOutsidePlotArea = LabelOutsidePlotAreaStyle::Yes; this->chartPieXuat->Series->Add(sPie2);
			this->pnlPieLayout->Controls->Add(this->chartPieNhap, 0, 0); this->pnlPieLayout->Controls->Add(this->chartPieXuat, 1, 0); this->tabSubTron->Controls->Add(this->pnlPieLayout);
			this->tabTKSub->Controls->Add(this->tabSubCot); this->tabTKSub->Controls->Add(this->tabSubTron);
			this->tabThongKe->Controls->Add(this->tabTKSub); this->tabThongKe->Controls->Add(this->pnlDieuKhienTK);
		}

		void OnBtnXemBieuDoClick(Object^ sender, EventArgs^ e) {
			String^ fromDate = dtpTuNgay->Value.ToString("yyyy-MM-dd"); String^ toDate = dtpDenNgay->Value.ToString("yyyy-MM-dd");
			try {
				OdbcConnection^ conn = gcnew OdbcConnection(strConn); conn->Open();
				// Chart 1
				chartThongKe->Series[0]->Points->Clear(); chartThongKe->Series[1]->Points->Clear();
				System::Collections::Generic::Dictionary<String^, double>^ dictThu = gcnew System::Collections::Generic::Dictionary<String^, double>();
				System::Collections::Generic::Dictionary<String^, double>^ dictChi = gcnew System::Collections::Generic::Dictionary<String^, double>();
				System::Collections::Generic::List<String^>^ listTenKho = gcnew System::Collections::Generic::List<String^>();
				OdbcCommand^ cmdKho = gcnew OdbcCommand("SELECT MaKho, TenKho FROM Kho", conn); OdbcDataReader^ drKho = cmdKho->ExecuteReader();
				while (drKho->Read()) { String^ ten = drKho["TenKho"]->ToString(); listTenKho->Add(ten); dictThu[ten] = 0; dictChi[ten] = 0; } drKho->Close();
				String^ sqlThu = "SELECT K.TenKho, SUM(D.TongThanhToan) AS Tien FROM DonDatHang D JOIN Kho K ON D.MaKho=K.MaKho WHERE D.NgayDat BETWEEN '" + fromDate + "' AND '" + toDate + "' GROUP BY K.TenKho";
				OdbcCommand^ cmdThu = gcnew OdbcCommand(sqlThu, conn); OdbcDataReader^ drThu = cmdThu->ExecuteReader(); while (drThu->Read()) { dictThu[drThu["TenKho"]->ToString()] = Convert::ToDouble(drThu["Tien"]); } drThu->Close();
				String^ sqlChi = "SELECT K.TenKho, SUM(V.TongChiPhi) AS Tien FROM DonVanChuyen V JOIN Kho K ON V.MaKho=K.MaKho WHERE V.NgayTao BETWEEN '" + fromDate + "' AND '" + toDate + "' GROUP BY K.TenKho";
				OdbcCommand^ cmdChi = gcnew OdbcCommand(sqlChi, conn); OdbcDataReader^ drChi = cmdChi->ExecuteReader(); while (drChi->Read()) { dictChi[drChi["TenKho"]->ToString()] = Convert::ToDouble(drChi["Tien"]); } drChi->Close();
				for each (String ^ ten in listTenKho) { if (dictThu[ten] > 0 || dictChi[ten] > 0) { chartThongKe->Series[0]->Points->AddXY(ten, dictThu[ten]); chartThongKe->Series[1]->Points->AddXY(ten, dictChi[ten]); } }
				// Chart 2
				chartPieNhap->Series[0]->Points->Clear(); chartPieXuat->Series[0]->Points->Clear();
				String^ sqlPieNhap = "SELECT ISNULL(S.Loai, N'Chưa phân loại') AS NhomHang, SUM(CT.ThanhTien) AS TongTien FROM ChiTietDonDatHang CT JOIN SanPham S ON CT.MaSanPham = S.MaSanPham JOIN DonDatHang D ON CT.MaDonHang = D.MaDonHang WHERE D.NgayDat BETWEEN '" + fromDate + "' AND '" + toDate + "' GROUP BY S.Loai";
				OdbcCommand^ cmdPie1 = gcnew OdbcCommand(sqlPieNhap, conn); OdbcDataReader^ drPie1 = cmdPie1->ExecuteReader();
				while (drPie1->Read()) { String^ loai = drPie1["NhomHang"]->ToString(); if (String::IsNullOrWhiteSpace(loai)) loai = L"Chưa phân loại"; double tien = Convert::ToDouble(drPie1["TongTien"]); if (tien > 0) chartPieNhap->Series[0]->Points->AddXY(loai, tien); } drPie1->Close();
				String^ sqlPieXuat = "SELECT ISNULL(S.Loai, N'Chưa phân loại') AS NhomHang, SUM(CT.ThanhTien) AS TongTien FROM ChiTietDonVC CT JOIN SanPham S ON CT.MaSanPham = S.MaSanPham JOIN DonVanChuyen D ON CT.MaDonVC = D.MaDonVC WHERE D.NgayTao BETWEEN '" + fromDate + "' AND '" + toDate + "' GROUP BY S.Loai";
				OdbcCommand^ cmdPie2 = gcnew OdbcCommand(sqlPieXuat, conn); OdbcDataReader^ drPie2 = cmdPie2->ExecuteReader();
				while (drPie2->Read()) { String^ loai = drPie2["NhomHang"]->ToString(); if (String::IsNullOrWhiteSpace(loai)) loai = L"Chưa phân loại"; double tien = Convert::ToDouble(drPie2["TongTien"]); if (tien > 0) chartPieXuat->Series[0]->Points->AddXY(loai, tien); } drPie2->Close();
				conn->Close();
				if (chartThongKe->Series[0]->Points->Count == 0 && chartPieNhap->Series[0]->Points->Count == 0) MessageBox::Show(L"Không có dữ liệu!");
			}
			catch (Exception^ ex) { MessageBox::Show(L"Lỗi: " + ex->Message); }
		}

		void InitTabPhuongTien() {
			this->tabPhuongTien = (gcnew System::Windows::Forms::TabPage()); this->tabPhuongTien->Text = L"Danh Sách Phương Tiện"; this->tabPhuongTien->BackColor = Color::WhiteSmoke;
			this->grpThongTinXe = (gcnew System::Windows::Forms::GroupBox()); this->grpThongTinXe->Text = L"Thông tin xe"; this->grpThongTinXe->Dock = DockStyle::Top; this->grpThongTinXe->Height = 200; this->grpThongTinXe->BackColor = Color::White;
			CreateInput(grpThongTinXe, L"Mã Xe (Auto):", 30, 30, txtMaXe = gcnew TextBox()); txtMaXe->ReadOnly = true; txtMaXe->BackColor = Color::WhiteSmoke; // [READONLY]
			CreateInput(grpThongTinXe, L"Loại Xe:", 300, 30, txtLoaiXe = gcnew TextBox());
			CreateInput(grpThongTinXe, L"Số Xe:", 570, 30, txtSoXe = gcnew TextBox()); CreateInput(grpThongTinXe, L"Sức Chứa (Tấn):", 30, 90, txtSucChua = gcnew TextBox());
			CreateInput(grpThongTinXe, L"Chi Phí/Chuyến:", 300, 90, txtChiPhiXe = gcnew TextBox()); CreateInput(grpThongTinXe, L"Vận Tốc TB:", 570, 90, txtVanToc = gcnew TextBox());
			btnThemXe = CreateButton(grpThongTinXe, L"Thêm Xe", 850, 40, Color::FromArgb(0, 123, 255)); btnThemXe->Click += gcnew EventHandler(this, &QuanLyLuuChuyenForm::OnBtnAddXeClick);
			btnSuaXe = CreateButton(grpThongTinXe, L"Cập Nhật", 850, 90, Color::FromArgb(40, 167, 69)); btnSuaXe->Click += gcnew EventHandler(this, &QuanLyLuuChuyenForm::OnBtnUpdateXeClick);
			btnXoaXe = CreateButton(grpThongTinXe, L"Xóa Xe", 1000, 40, Color::FromArgb(220, 53, 69)); btnXoaXe->Click += gcnew EventHandler(this, &QuanLyLuuChuyenForm::OnBtnDeleteXeClick);
			btnLamMoiXe = CreateButton(grpThongTinXe, L"Làm Mới", 1000, 90, Color::Gray); btnLamMoiXe->Click += gcnew EventHandler(this, &QuanLyLuuChuyenForm::OnBtnRefreshXeClick);
			this->dgvPhuongTien = (gcnew System::Windows::Forms::DataGridView()); this->dgvPhuongTien->Dock = DockStyle::Fill; this->dgvPhuongTien->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill; this->dgvPhuongTien->CellClick += gcnew DataGridViewCellEventHandler(this, &QuanLyLuuChuyenForm::OnGridXeCellClick);
			this->tabPhuongTien->Controls->Add(this->dgvPhuongTien); this->tabPhuongTien->Controls->Add(this->grpThongTinXe);
		}

		void InitTabTuyen() {
			this->tabTuyen = (gcnew System::Windows::Forms::TabPage()); this->tabTuyen->Text = L"Tuyến Vận Chuyển (Kho -> ST)"; this->tabTuyen->BackColor = Color::WhiteSmoke;
			this->grpThongTinTuyen = (gcnew System::Windows::Forms::GroupBox()); this->grpThongTinTuyen->Text = L"Thiết lập Tuyến (Kho -> Kho Trung Gian -> Siêu Thị)"; this->grpThongTinTuyen->Dock = DockStyle::Top; this->grpThongTinTuyen->Height = 260; this->grpThongTinTuyen->BackColor = Color::White;
			CreateInput(grpThongTinTuyen, L"Mã Tuyến (Auto):", 30, 30, txtMaTuyen = gcnew TextBox()); txtMaTuyen->ReadOnly = true; txtMaTuyen->BackColor = Color::WhiteSmoke; // [READONLY]
			Label^ lblKho = gcnew Label(); lblKho->Text = L"1. Kho Đi (Nguồn):"; lblKho->Location = Point(30, 90); lblKho->AutoSize = true; cbKhoDi = gcnew ComboBox(); cbKhoDi->Location = Point(30, 115); cbKhoDi->Size = System::Drawing::Size(240, 30); cbKhoDi->DropDownStyle = ComboBoxStyle::DropDownList; grpThongTinTuyen->Controls->Add(lblKho); grpThongTinTuyen->Controls->Add(cbKhoDi);
			Label^ lblKhoTG = gcnew Label(); lblKhoTG->Text = L"2. Kho Trung Chuyển (Nếu có):"; lblKhoTG->Location = Point(300, 90); lblKhoTG->AutoSize = true; cbKhoTrungChuyen = gcnew ComboBox(); cbKhoTrungChuyen->Location = Point(300, 115); cbKhoTrungChuyen->Size = System::Drawing::Size(240, 30); cbKhoTrungChuyen->DropDownStyle = ComboBoxStyle::DropDownList; grpThongTinTuyen->Controls->Add(lblKhoTG); grpThongTinTuyen->Controls->Add(cbKhoTrungChuyen);
			Label^ lblST = gcnew Label(); lblST->Text = L"3. Siêu Thị Nhận (Đích):"; lblST->Location = Point(570, 90); lblST->AutoSize = true; cbSieuThiNhan = gcnew ComboBox(); cbSieuThiNhan->Location = Point(570, 115); cbSieuThiNhan->Size = System::Drawing::Size(240, 30); cbSieuThiNhan->DropDownStyle = ComboBoxStyle::DropDownList; grpThongTinTuyen->Controls->Add(lblST); grpThongTinTuyen->Controls->Add(cbSieuThiNhan);
			Label^ lblXe = gcnew Label(); lblXe->Text = L"Xe Phụ Trách:"; lblXe->Location = Point(30, 150); lblXe->AutoSize = true; cbXePhuTrach = gcnew ComboBox(); cbXePhuTrach->Location = Point(30, 175); cbXePhuTrach->Size = System::Drawing::Size(240, 30); cbXePhuTrach->DropDownStyle = ComboBoxStyle::DropDownList; grpThongTinTuyen->Controls->Add(lblXe); grpThongTinTuyen->Controls->Add(cbXePhuTrach);
			CreateInput(grpThongTinTuyen, L"Chi Phí Tuyến Cố Định (VND):", 300, 150, txtChiPhiTuyen = gcnew TextBox());
			btnThemTuyen = CreateButton(grpThongTinTuyen, L"Thêm Tuyến", 850, 60, Color::FromArgb(0, 123, 255)); btnThemTuyen->Click += gcnew EventHandler(this, &QuanLyLuuChuyenForm::OnBtnAddTuyenClick);
			btnSuaTuyen = CreateButton(grpThongTinTuyen, L"Cập Nhật", 850, 110, Color::FromArgb(40, 167, 69)); btnSuaTuyen->Click += gcnew EventHandler(this, &QuanLyLuuChuyenForm::OnBtnUpdateTuyenClick);
			btnXoaTuyen = CreateButton(grpThongTinTuyen, L"Xóa Tuyến", 1000, 60, Color::FromArgb(220, 53, 69)); btnXoaTuyen->Click += gcnew EventHandler(this, &QuanLyLuuChuyenForm::OnBtnDeleteTuyenClick);
			btnLamMoiTuyen = CreateButton(grpThongTinTuyen, L"Làm Mới", 1000, 110, Color::Gray); btnLamMoiTuyen->Click += gcnew EventHandler(this, &QuanLyLuuChuyenForm::OnBtnRefreshTuyenClick);
			this->dgvTuyen = (gcnew System::Windows::Forms::DataGridView()); this->dgvTuyen->Dock = DockStyle::Fill; this->dgvTuyen->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill; this->dgvTuyen->CellClick += gcnew DataGridViewCellEventHandler(this, &QuanLyLuuChuyenForm::OnGridTuyenCellClick);
			this->tabTuyen->Controls->Add(this->dgvTuyen); this->tabTuyen->Controls->Add(this->grpThongTinTuyen);
		}

		void InitTabTuyenNhap() {
			this->tabTuyenNhap = (gcnew System::Windows::Forms::TabPage()); this->tabTuyenNhap->Text = L"Tuyến Nhập Hàng (NCC -> Kho)"; this->tabTuyenNhap->BackColor = Color::WhiteSmoke;
			this->grpTuyenNhap = (gcnew System::Windows::Forms::GroupBox()); this->grpTuyenNhap->Text = L"Thiết lập Tuyến Nhập (Nhà CC -> Kho Trung Gian -> Kho Đích)"; this->grpTuyenNhap->Dock = DockStyle::Top; this->grpTuyenNhap->Height = 260; this->grpTuyenNhap->BackColor = Color::White;
			CreateInput(grpTuyenNhap, L"Mã Tuyến Nhập (Auto):", 30, 30, txtMaTuyenNhap = gcnew TextBox()); txtMaTuyenNhap->ReadOnly = true; txtMaTuyenNhap->BackColor = Color::WhiteSmoke; // [READONLY]
			Label^ lblNCC = gcnew Label(); lblNCC->Text = L"1. Nhà Cung Cấp (Nguồn):"; lblNCC->Location = Point(30, 90); lblNCC->AutoSize = true; cbNhaCungCap = gcnew ComboBox(); cbNhaCungCap->Location = Point(30, 115); cbNhaCungCap->Size = System::Drawing::Size(240, 30); cbNhaCungCap->DropDownStyle = ComboBoxStyle::DropDownList; grpTuyenNhap->Controls->Add(lblNCC); grpTuyenNhap->Controls->Add(cbNhaCungCap);
			Label^ lblKhoTG = gcnew Label(); lblKhoTG->Text = L"2. Kho Trung Chuyển (Nếu có):"; lblKhoTG->Location = Point(300, 90); lblKhoTG->AutoSize = true; cbKhoTGNhap = gcnew ComboBox(); cbKhoTGNhap->Location = Point(300, 115); cbKhoTGNhap->Size = System::Drawing::Size(240, 30); cbKhoTGNhap->DropDownStyle = ComboBoxStyle::DropDownList; grpTuyenNhap->Controls->Add(lblKhoTG); grpTuyenNhap->Controls->Add(cbKhoTGNhap);
			Label^ lblKhoNhan = gcnew Label(); lblKhoNhan->Text = L"3. Kho Nhận (Đích):"; lblKhoNhan->Location = Point(570, 90); lblKhoNhan->AutoSize = true; cbKhoNhan = gcnew ComboBox(); cbKhoNhan->Location = Point(570, 115); cbKhoNhan->Size = System::Drawing::Size(240, 30); cbKhoNhan->DropDownStyle = ComboBoxStyle::DropDownList; grpTuyenNhap->Controls->Add(lblKhoNhan); grpTuyenNhap->Controls->Add(cbKhoNhan);
			Label^ lblXe = gcnew Label(); lblXe->Text = L"Xe Phụ Trách:"; lblXe->Location = Point(30, 150); lblXe->AutoSize = true; cbXeNhap = gcnew ComboBox(); cbXeNhap->Location = Point(30, 175); cbXeNhap->Size = System::Drawing::Size(240, 30); cbXeNhap->DropDownStyle = ComboBoxStyle::DropDownList; grpTuyenNhap->Controls->Add(lblXe); grpTuyenNhap->Controls->Add(cbXeNhap);
			CreateInput(grpTuyenNhap, L"Chi Phí Tuyến (VND):", 300, 150, txtChiPhiNhap = gcnew TextBox());
			btnThemTuyenNhap = CreateButton(grpTuyenNhap, L"Thêm Tuyến", 850, 60, Color::FromArgb(0, 123, 255)); btnThemTuyenNhap->Click += gcnew EventHandler(this, &QuanLyLuuChuyenForm::OnBtnAddTuyenNhapClick);
			btnSuaTuyenNhap = CreateButton(grpTuyenNhap, L"Cập Nhật", 850, 110, Color::FromArgb(40, 167, 69)); btnSuaTuyenNhap->Click += gcnew EventHandler(this, &QuanLyLuuChuyenForm::OnBtnUpdateTuyenNhapClick);
			btnXoaTuyenNhap = CreateButton(grpTuyenNhap, L"Xóa Tuyến", 1000, 60, Color::FromArgb(220, 53, 69)); btnXoaTuyenNhap->Click += gcnew EventHandler(this, &QuanLyLuuChuyenForm::OnBtnDeleteTuyenNhapClick);
			btnLamMoiTuyenNhap = CreateButton(grpTuyenNhap, L"Làm Mới", 1000, 110, Color::Gray); btnLamMoiTuyenNhap->Click += gcnew EventHandler(this, &QuanLyLuuChuyenForm::OnBtnRefreshTuyenNhapClick);
			this->dgvTuyenNhap = (gcnew System::Windows::Forms::DataGridView()); this->dgvTuyenNhap->Dock = DockStyle::Fill; this->dgvTuyenNhap->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill; this->dgvTuyenNhap->CellClick += gcnew DataGridViewCellEventHandler(this, &QuanLyLuuChuyenForm::OnGridTuyenNhapCellClick);
			this->tabTuyenNhap->Controls->Add(this->dgvTuyenNhap); this->tabTuyenNhap->Controls->Add(this->grpTuyenNhap);
		}

		void CreateInput(GroupBox^ g, String^ lbl, int x, int y, TextBox^ t) { Label^ l = gcnew Label(); l->Text = lbl; l->Location = Point(x, y); l->AutoSize = true; t->Location = Point(x, y + 25); t->Size = System::Drawing::Size(240, 30); g->Controls->Add(l); g->Controls->Add(t); }
		Button^ CreateButton(GroupBox^ g, String^ txt, int x, int y, Color c) { Button^ b = gcnew Button(); b->Text = txt; b->Location = Point(x, y); b->Size = System::Drawing::Size(130, 40); b->FlatStyle = FlatStyle::Flat; b->BackColor = c; b->ForeColor = Color::White; g->Controls->Add(b); return b; }

		// LOGIC CÁC TAB KHÁC
		void LoadComboboxData() { try { OdbcConnection^ conn = gcnew OdbcConnection(strConn); conn->Open(); OdbcDataAdapter^ da; da = gcnew OdbcDataAdapter("SELECT MaKho, TenKho FROM Kho", conn); DataTable^ dtKho = gcnew DataTable(); da->Fill(dtKho); dtKho->Columns->Add("DisplayCol", String::typeid, "MaKho + ' - ' + TenKho"); cbKhoDi->DataSource = dtKho; cbKhoDi->DisplayMember = "DisplayCol"; cbKhoDi->ValueMember = "MaKho"; cbKhoTrungChuyen->BindingContext = gcnew System::Windows::Forms::BindingContext(); cbKhoTrungChuyen->DataSource = dtKho->Copy(); cbKhoTrungChuyen->DisplayMember = "DisplayCol"; cbKhoTrungChuyen->ValueMember = "MaKho"; cbKhoNhan->DataSource = dtKho->Copy(); cbKhoNhan->DisplayMember = "DisplayCol"; cbKhoNhan->ValueMember = "MaKho"; cbKhoTGNhap->BindingContext = gcnew System::Windows::Forms::BindingContext(); cbKhoTGNhap->DataSource = dtKho->Copy(); cbKhoTGNhap->DisplayMember = "DisplayCol"; cbKhoTGNhap->ValueMember = "MaKho"; da = gcnew OdbcDataAdapter("SELECT MaSieuThi, TenSieuThi FROM SieuThi", conn); DataTable^ dtST = gcnew DataTable(); da->Fill(dtST); dtST->Columns->Add("DisplayCol", String::typeid, "MaSieuThi + ' - ' + TenSieuThi"); cbSieuThiNhan->DataSource = dtST; cbSieuThiNhan->DisplayMember = "DisplayCol"; cbSieuThiNhan->ValueMember = "MaSieuThi"; da = gcnew OdbcDataAdapter("SELECT MaXe, LoaiXe FROM PhuongTien", conn); DataTable^ dtXe = gcnew DataTable(); da->Fill(dtXe); dtXe->Columns->Add("DisplayCol", String::typeid, "MaXe + ' - ' + LoaiXe"); cbXePhuTrach->DataSource = dtXe; cbXePhuTrach->DisplayMember = "DisplayCol"; cbXePhuTrach->ValueMember = "MaXe"; cbXeNhap->BindingContext = gcnew System::Windows::Forms::BindingContext(); cbXeNhap->DataSource = dtXe->Copy(); cbXeNhap->DisplayMember = "DisplayCol"; cbXeNhap->ValueMember = "MaXe"; da = gcnew OdbcDataAdapter("SELECT MaNCC, TenNCC FROM NhaCungCap", conn); DataTable^ dtNCC = gcnew DataTable(); da->Fill(dtNCC); dtNCC->Columns->Add("DisplayCol", String::typeid, "MaNCC + ' - ' + TenNCC"); cbNhaCungCap->DataSource = dtNCC; cbNhaCungCap->DisplayMember = "DisplayCol"; cbNhaCungCap->ValueMember = "MaNCC"; conn->Close(); } catch (Exception^) {} }
		void LoadDataPhuongTien() { try { OdbcConnection^ conn = gcnew OdbcConnection(strConn); conn->Open(); OdbcDataAdapter^ da = gcnew OdbcDataAdapter("SELECT MaXe, LoaiXe, SoXe, SucChuaTan, ChiPhiMoiChuyen, VanTocTB FROM PhuongTien", conn); DataTable^ dt = gcnew DataTable(); da->Fill(dt); dgvPhuongTien->DataSource = dt; conn->Close(); } catch (Exception^) {} }
		void OnBtnAddXeClick(Object^ sender, EventArgs^ e) { if (txtMaXe->Text == "") return; ExecuteSQL("INSERT INTO PhuongTien (MaXe, LoaiXe, SoXe, SucChuaTan, ChiPhiMoiChuyen, VanTocTB) VALUES ('" + txtMaXe->Text + "', N'" + txtLoaiXe->Text + "', " + (txtSoXe->Text == "" ? "0" : txtSoXe->Text) + ", " + (txtSucChua->Text == "" ? "0" : txtSucChua->Text) + ", " + (txtChiPhiXe->Text == "" ? "0" : txtChiPhiXe->Text) + ", " + (txtVanToc->Text == "" ? "0" : txtVanToc->Text) + ")"); LoadDataPhuongTien(); LoadComboboxData(); OnBtnRefreshXeClick(nullptr, nullptr); } // [UPDATE: Refresh sau khi thêm để sinh mã tiếp]
		void OnBtnUpdateXeClick(Object^ sender, EventArgs^ e) { ExecuteSQL("UPDATE PhuongTien SET LoaiXe=N'" + txtLoaiXe->Text + "', SoXe=" + txtSoXe->Text + ", SucChuaTan=" + txtSucChua->Text + ", ChiPhiMoiChuyen=" + txtChiPhiXe->Text + ", VanTocTB=" + txtVanToc->Text + " WHERE MaXe='" + txtMaXe->Text + "'"); LoadDataPhuongTien(); LoadComboboxData(); }
		void OnBtnDeleteXeClick(Object^ sender, EventArgs^ e) { if (MessageBox::Show("Xóa xe?", "Confirm", MessageBoxButtons::YesNo) == System::Windows::Forms::DialogResult::Yes) { ExecuteSQL("DELETE FROM PhuongTien WHERE MaXe='" + txtMaXe->Text + "'"); LoadDataPhuongTien(); LoadComboboxData(); OnBtnRefreshXeClick(nullptr, nullptr); } }
		void OnBtnRefreshXeClick(Object^ sender, EventArgs^ e) {
			txtMaXe->Text = GenerateAutoCode("PhuongTien", "MaXe", "XE"); // [MỚI]
			txtMaXe->ReadOnly = true;
			txtLoaiXe->Text = ""; txtSoXe->Text = ""; txtSucChua->Text = ""; txtChiPhiXe->Text = ""; txtVanToc->Text = "";
		}
		void OnGridXeCellClick(Object^ sender, DataGridViewCellEventArgs^ e) { if (e->RowIndex < 0) return; DataGridViewRow^ row = dgvPhuongTien->Rows[e->RowIndex]; txtMaXe->Text = row->Cells["MaXe"]->Value->ToString(); txtMaXe->ReadOnly = true; txtLoaiXe->Text = row->Cells["LoaiXe"]->Value->ToString(); txtSoXe->Text = row->Cells["SoXe"]->Value->ToString(); txtSucChua->Text = row->Cells["SucChuaTan"]->Value->ToString(); txtChiPhiXe->Text = row->Cells["ChiPhiMoiChuyen"]->Value->ToString(); txtVanToc->Text = row->Cells["VanTocTB"]->Value->ToString(); }

		void LoadDataTuyen() { try { OdbcConnection^ conn = gcnew OdbcConnection(strConn); conn->Open(); String^ sql = "SELECT T.MaTuyen, T.KhoDi, K1.TenKho AS TenKhoDi, T.KhoTrungChuyen, K2.TenKho AS TenKhoTG, T.SieuThiNhan, S.TenSieuThi, T.MaXe, T.ChiPhiTuyenCoDinh FROM TuyenVanChuyen T LEFT JOIN Kho K1 ON T.KhoDi = K1.MaKho LEFT JOIN Kho K2 ON T.KhoTrungChuyen = K2.MaKho LEFT JOIN SieuThi S ON T.SieuThiNhan = S.MaSieuThi"; OdbcDataAdapter^ da = gcnew OdbcDataAdapter(sql, conn); DataTable^ dt = gcnew DataTable(); da->Fill(dt); dgvTuyen->DataSource = dt; if (dgvTuyen->Columns["TenKhoDi"]) dgvTuyen->Columns["TenKhoDi"]->HeaderText = "Tên Kho Đi"; if (dgvTuyen->Columns["TenKhoTG"]) dgvTuyen->Columns["TenKhoTG"]->HeaderText = "Tên Kho TG"; if (dgvTuyen->Columns["TenSieuThi"]) dgvTuyen->Columns["TenSieuThi"]->HeaderText = "Tên ST Đích"; conn->Close(); } catch (Exception^) {} }
		void OnBtnAddTuyenClick(Object^ sender, EventArgs^ e) { if (txtMaTuyen->Text == "") return; String^ mk = cbKhoDi->SelectedValue->ToString(); String^ mtg = cbKhoTrungChuyen->SelectedValue ? cbKhoTrungChuyen->SelectedValue->ToString() : ""; String^ mst = cbSieuThiNhan->SelectedValue->ToString(); String^ mx = cbXePhuTrach->SelectedValue ? cbXePhuTrach->SelectedValue->ToString() : ""; String^ cp = txtChiPhiTuyen->Text == "" ? "0" : txtChiPhiTuyen->Text; ExecuteSQL("INSERT INTO TuyenVanChuyen (MaTuyen, KhoDi, KhoTrungChuyen, SieuThiNhan, MaXe, ChiPhiTuyenCoDinh) VALUES ('" + txtMaTuyen->Text + "','" + mk + "','" + mtg + "','" + mst + "','" + mx + "'," + cp + ")"); LoadDataTuyen(); OnBtnRefreshTuyenClick(nullptr, nullptr); }
		void OnBtnUpdateTuyenClick(Object^ sender, EventArgs^ e) { String^ mk = cbKhoDi->SelectedValue->ToString(); String^ mtg = cbKhoTrungChuyen->SelectedValue ? cbKhoTrungChuyen->SelectedValue->ToString() : ""; String^ mst = cbSieuThiNhan->SelectedValue->ToString(); String^ mx = cbXePhuTrach->SelectedValue ? cbXePhuTrach->SelectedValue->ToString() : ""; String^ cp = txtChiPhiTuyen->Text == "" ? "0" : txtChiPhiTuyen->Text; ExecuteSQL("UPDATE TuyenVanChuyen SET KhoDi='" + mk + "', KhoTrungChuyen='" + mtg + "', SieuThiNhan='" + mst + "', MaXe='" + mx + "', ChiPhiTuyenCoDinh=" + cp + " WHERE MaTuyen='" + txtMaTuyen->Text + "'"); LoadDataTuyen(); }
		void OnBtnDeleteTuyenClick(Object^ sender, EventArgs^ e) { if (MessageBox::Show("Xóa tuyến?", "Confirm", MessageBoxButtons::YesNo) == System::Windows::Forms::DialogResult::Yes) { ExecuteSQL("DELETE FROM TuyenVanChuyen WHERE MaTuyen='" + txtMaTuyen->Text + "'"); LoadDataTuyen(); OnBtnRefreshTuyenClick(nullptr, nullptr); } }
		void OnBtnRefreshTuyenClick(Object^ sender, EventArgs^ e) {
			txtMaTuyen->Text = GenerateAutoCode("TuyenVanChuyen", "MaTuyen", "TX"); // [MỚI]
			txtMaTuyen->ReadOnly = true;
			txtChiPhiTuyen->Text = ""; LoadDataTuyen();
		}
		void OnGridTuyenCellClick(Object^ sender, DataGridViewCellEventArgs^ e) { if (e->RowIndex < 0) return; DataGridViewRow^ row = dgvTuyen->Rows[e->RowIndex]; txtMaTuyen->Text = row->Cells["MaTuyen"]->Value->ToString(); txtMaTuyen->ReadOnly = true; if (row->Cells["KhoDi"]->Value->ToString() != "") cbKhoDi->SelectedValue = row->Cells["KhoDi"]->Value->ToString(); if (row->Cells["KhoTrungChuyen"]->Value->ToString() != "") cbKhoTrungChuyen->SelectedValue = row->Cells["KhoTrungChuyen"]->Value->ToString(); if (row->Cells["SieuThiNhan"]->Value->ToString() != "") cbSieuThiNhan->SelectedValue = row->Cells["SieuThiNhan"]->Value->ToString(); if (row->Cells["MaXe"]->Value->ToString() != "") cbXePhuTrach->SelectedValue = row->Cells["MaXe"]->Value->ToString(); txtChiPhiTuyen->Text = row->Cells["ChiPhiTuyenCoDinh"]->Value->ToString(); }

		void LoadDataTuyenNhap() { try { OdbcConnection^ conn = gcnew OdbcConnection(strConn); conn->Open(); String^ sql = "SELECT T.MaTuyen, T.MaNCC, N.TenNCC, T.KhoTrungChuyen, K1.TenKho AS TenKhoTG, T.MaKhoNhan, K2.TenKho AS TenKhoNhan, T.MaXe, T.ChiPhiTuyenCoDinh FROM TuyenNhapHang T LEFT JOIN NhaCungCap N ON T.MaNCC = N.MaNCC LEFT JOIN Kho K1 ON T.KhoTrungChuyen = K1.MaKho LEFT JOIN Kho K2 ON T.MaKhoNhan = K2.MaKho"; OdbcDataAdapter^ da = gcnew OdbcDataAdapter(sql, conn); DataTable^ dt = gcnew DataTable(); da->Fill(dt); dgvTuyenNhap->DataSource = dt; if (dgvTuyenNhap->Columns["MaTuyen"]) dgvTuyenNhap->Columns["MaTuyen"]->HeaderText = "Mã Tuyến"; if (dgvTuyenNhap->Columns["TenNCC"]) dgvTuyenNhap->Columns["TenNCC"]->HeaderText = "Nhà Cung Cấp"; if (dgvTuyenNhap->Columns["TenKhoTG"]) dgvTuyenNhap->Columns["TenKhoTG"]->HeaderText = "Kho Trung Gian"; if (dgvTuyenNhap->Columns["TenKhoNhan"]) dgvTuyenNhap->Columns["TenKhoNhan"]->HeaderText = "Kho Đích"; if (dgvTuyenNhap->Columns["ChiPhiTuyenCoDinh"]) dgvTuyenNhap->Columns["ChiPhiTuyenCoDinh"]->HeaderText = "Chi Phí"; conn->Close(); } catch (Exception^ ex) { MessageBox::Show("Lỗi tải Tuyến Nhập: " + ex->Message); } }
		void OnBtnAddTuyenNhapClick(Object^ sender, EventArgs^ e) { if (txtMaTuyenNhap->Text == "" || cbNhaCungCap->SelectedValue == nullptr || cbKhoNhan->SelectedValue == nullptr) { MessageBox::Show(L"Thiếu Mã, NCC hoặc Kho Nhận!"); return; } String^ mncc = cbNhaCungCap->SelectedValue->ToString(); String^ mtg = cbKhoTGNhap->SelectedValue ? cbKhoTGNhap->SelectedValue->ToString() : ""; String^ mkho = cbKhoNhan->SelectedValue->ToString(); String^ mx = cbXeNhap->SelectedValue ? cbXeNhap->SelectedValue->ToString() : ""; String^ cp = txtChiPhiNhap->Text == "" ? "0" : txtChiPhiNhap->Text; String^ sql = "INSERT INTO TuyenNhapHang (MaTuyen, MaNCC, MaKhoNhan, KhoTrungChuyen, MaXe, ChiPhiTuyenCoDinh) VALUES ('" + txtMaTuyenNhap->Text + "','" + mncc + "','" + mkho + "','" + mtg + "','" + mx + "'," + cp + ")"; if (ExecuteSQL(sql)) LoadDataTuyenNhap(); OnBtnRefreshTuyenNhapClick(nullptr, nullptr); }
		void OnBtnUpdateTuyenNhapClick(Object^ sender, EventArgs^ e) { if (txtMaTuyenNhap->Text == "") return; String^ mncc = cbNhaCungCap->SelectedValue->ToString(); String^ mtg = cbKhoTGNhap->SelectedValue ? cbKhoTGNhap->SelectedValue->ToString() : ""; String^ mkho = cbKhoNhan->SelectedValue->ToString(); String^ mx = cbXeNhap->SelectedValue ? cbXeNhap->SelectedValue->ToString() : ""; String^ cp = txtChiPhiNhap->Text == "" ? "0" : txtChiPhiNhap->Text; String^ sql = "UPDATE TuyenNhapHang SET MaNCC='" + mncc + "', MaKhoNhan='" + mkho + "', KhoTrungChuyen='" + mtg + "', MaXe='" + mx + "', ChiPhiTuyenCoDinh=" + cp + " WHERE MaTuyen='" + txtMaTuyenNhap->Text + "'"; if (ExecuteSQL(sql)) LoadDataTuyenNhap(); }
		void OnBtnDeleteTuyenNhapClick(Object^ sender, EventArgs^ e) { if (txtMaTuyenNhap->Text == "") return; if (MessageBox::Show(L"Xóa tuyến nhập này?", "Confirm", MessageBoxButtons::YesNo) == System::Windows::Forms::DialogResult::Yes) { ExecuteSQL("DELETE FROM TuyenNhapHang WHERE MaTuyen='" + txtMaTuyenNhap->Text + "'"); LoadDataTuyenNhap(); OnBtnRefreshTuyenNhapClick(nullptr, nullptr); } }
		void OnBtnRefreshTuyenNhapClick(Object^ sender, EventArgs^ e) {
			txtMaTuyenNhap->Text = GenerateAutoCode("TuyenNhapHang", "MaTuyen", "TN"); // [MỚI]
			txtMaTuyenNhap->ReadOnly = true;
			txtChiPhiNhap->Text = ""; LoadDataTuyenNhap();
		}
		void OnGridTuyenNhapCellClick(Object^ sender, DataGridViewCellEventArgs^ e) { if (e->RowIndex < 0) return; DataGridViewRow^ row = dgvTuyenNhap->Rows[e->RowIndex]; txtMaTuyenNhap->Text = row->Cells["MaTuyen"]->Value->ToString(); txtMaTuyenNhap->ReadOnly = true; if (row->Cells["MaNCC"]->Value->ToString() != "") cbNhaCungCap->SelectedValue = row->Cells["MaNCC"]->Value->ToString(); if (row->Cells["KhoTrungChuyen"]->Value->ToString() != "") cbKhoTGNhap->SelectedValue = row->Cells["KhoTrungChuyen"]->Value->ToString(); if (row->Cells["MaKhoNhan"]->Value->ToString() != "") cbKhoNhan->SelectedValue = row->Cells["MaKhoNhan"]->Value->ToString(); if (row->Cells["MaXe"]->Value->ToString() != "") cbXeNhap->SelectedValue = row->Cells["MaXe"]->Value->ToString(); txtChiPhiNhap->Text = row->Cells["ChiPhiTuyenCoDinh"]->Value->ToString(); }

		bool ExecuteSQL(String^ sql) { try { OdbcConnection^ conn = gcnew OdbcConnection(strConn); conn->Open(); OdbcCommand^ cmd = gcnew OdbcCommand(sql, conn); cmd->ExecuteNonQuery(); conn->Close(); return true; } catch (Exception^ ex) { MessageBox::Show("Lỗi SQL: " + ex->Message); return false; } }
	};
}