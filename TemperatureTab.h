#pragma once
// --- BẮT BUỘC PHẢI CÓ 3 DÒNG NÀY Ở ĐẦU FILE ---
#include <string>
#include <vector>
#include <msclr/marshal_cppstd.h>

#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include "SaoLuu.h" 
#include "QuanLyNhietDo.h"

namespace PBL2QuanLyKho {

    using namespace System;
    using namespace System::ComponentModel;
    using namespace System::Windows::Forms;
    using namespace System::Data;
    using namespace System::Drawing;
    using namespace System::Collections::Generic;
    using namespace System::Windows::Forms::DataVisualization::Charting;

    public ref class TemperatureTab : public System::Windows::Forms::Panel {
    private:
        Panel^ pnlLeft; Panel^ pnlRight;
        GroupBox^ grpKhoInfo; Label^ lblConfig; DataGridView^ dgvProducts;
        GroupBox^ grpResult; Panel^ pnlControls;
        NumericUpDown^ numStart; NumericUpDown^ numEnd;
        Button^ btnRun; Label^ lblResultTotal; DataGridView^ dgvSchedule;

        SplitContainer^ splitContainerResult; // Dùng để chia đôi màn hình
        Chart^ chartResult;
        RadioButton^ rdoAuto;
        RadioButton^ rdoFixed;
        NumericUpDown^ numFixedTemp;
        String^ CurrentMaKho;
        RadioButton^ rdoManual;       // Nút chọn chế độ "Nhập tay"
        DataGridView^ dgvManualInput; // Bảng nhập "Nhật ký nhiệt độ" ở bên trái
        SplitContainer^ splitLeft;    // Chia đôi khung bên trái (Trên: Sản phẩm, Dưới: Nhật ký)
        Button^ btnResetManual;

    public:
        TemperatureTab(String^ maKho) {
            this->CurrentMaKho = maKho;
            this->Dock = DockStyle::Fill;
            this->BackColor = Color::WhiteSmoke;
            InitializeUI();

            // Load thông tin hiển thị ngay khi mở tab
            LoadInitialDisplay();
        }

    private:
        void OnModeChanged(Object^ sender, EventArgs^ e) {
            // Nếu chọn Auto thì tắt nhập số, ngược lại thì bật
            numFixedTemp->Enabled = rdoFixed->Checked;
        }
        void InitializeUI() {
            // 1. Panel Trái (pnlLeft)
            pnlLeft = gcnew Panel(); pnlLeft->Dock = DockStyle::Left; pnlLeft->Width = 420; pnlLeft->Padding = System::Windows::Forms::Padding(5);

            // --- [MỚI] TẠO SPLIT CONTAINER CHO BÊN TRÁI ---
            splitLeft = gcnew SplitContainer();
            splitLeft->Dock = DockStyle::Fill;
            splitLeft->Orientation = Orientation::Horizontal;
            splitLeft->SplitterDistance = 200; // Chiều cao phần trên (Sản phẩm)

            // A. PHẦN TRÊN: Thông tin kho & Sản phẩm
            grpKhoInfo = gcnew GroupBox(); grpKhoInfo->Text = L"Cấu hình Kho"; grpKhoInfo->Dock = DockStyle::Top; grpKhoInfo->Height = 60;
            lblConfig = gcnew Label(); lblConfig->Dock = DockStyle::Fill; grpKhoInfo->Controls->Add(lblConfig);

            GroupBox^ grpProd = gcnew GroupBox(); grpProd->Text = L"Hàng hóa đang bảo quản"; grpProd->Dock = DockStyle::Fill;
            dgvProducts = gcnew DataGridView(); dgvProducts->Dock = DockStyle::Fill; dgvProducts->BackgroundColor = Color::White;
            dgvProducts->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill; dgvProducts->ReadOnly = true;
            dgvProducts->Columns->Add("Ten", L"Sản Phẩm");
            dgvProducts->Columns->Add("Temp", L"Chuẩn");
            dgvProducts->Columns->Add("Val", L"Giá Trị");
            grpProd->Controls->Add(dgvProducts);

            // Add vào Panel 1 của Split trái
            splitLeft->Panel1->Controls->Add(grpProd);
            splitLeft->Panel1->Controls->Add(grpKhoInfo);

            // B. [MỚI] PHẦN DƯỚI: NHẬT KÝ NHIỆT ĐỘ (NHẬP TAY)
            GroupBox^ grpManual = gcnew GroupBox();
            grpManual->Text = L"Nhật ký cài đặt nhiệt độ (Nhập tay)";
            grpManual->Dock = DockStyle::Fill;

            btnResetManual = gcnew Button();
            btnResetManual->Text = L"↺ Đặt lại (-18°C)";
            btnResetManual->Dock = DockStyle::Bottom;
            btnResetManual->Height = 35;
            btnResetManual->BackColor = Color::LightGray;
            btnResetManual->Click += gcnew EventHandler(this, &TemperatureTab::OnResetManualClick);

            dgvManualInput = gcnew DataGridView();
            dgvManualInput->Dock = DockStyle::Fill;
            dgvManualInput->BackgroundColor = Color::WhiteSmoke;
            dgvManualInput->AllowUserToAddRows = false;
            dgvManualInput->RowHeadersVisible = false;

            dgvManualInput->Columns->Add("H", L"Giờ");
            dgvManualInput->Columns[0]->ReadOnly = true;
            dgvManualInput->Columns[0]->Width = 50;

            dgvManualInput->Columns->Add("T", L"Nhiệt độ");
            dgvManualInput->Columns[1]->DefaultCellStyle->BackColor = Color::LightYellow;

            // Khởi tạo 24 giờ
            for (int i = 0; i < 24; i++) {
                dgvManualInput->Rows->Add(i + ":00", "-18");
            }

            grpManual->Controls->Add(dgvManualInput); // Grid chiếm phần còn lại
            grpManual->Controls->Add(btnResetManual); // Button nằm đáy (Dock Bottom)

            splitLeft->Panel2->Controls->Add(grpManual);

            pnlLeft->Controls->Add(splitLeft);

            // 2. Panel Phải (pnlRight) - Giữ nguyên logic cũ, chỉ thêm nút Radio
            pnlRight = gcnew Panel(); pnlRight->Dock = DockStyle::Fill; pnlRight->Padding = System::Windows::Forms::Padding(10);
            grpResult = gcnew GroupBox(); grpResult->Text = L"Kết Quả Phân Tích Chi Phí"; grpResult->Dock = DockStyle::Fill;

            pnlControls = gcnew Panel(); pnlControls->Dock = DockStyle::Top; pnlControls->Height = 90;

            // Các nút chọn giờ (Start/End) giữ nguyên...
            Label^ l1 = gcnew Label(); l1->Text = L"Từ (h):"; l1->Location = Point(10, 22); l1->AutoSize = true;
            numStart = gcnew NumericUpDown(); numStart->Minimum = 0; numStart->Maximum = 23; numStart->Value = 0; numStart->Location = Point(60, 20); numStart->Width = 50;
            Label^ l2 = gcnew Label(); l2->Text = L"Đến (h):"; l2->Location = Point(120, 22); l2->AutoSize = true;
            numEnd = gcnew NumericUpDown(); numEnd->Minimum = 0; numEnd->Maximum = 23; numEnd->Value = 23; numEnd->Location = Point(170, 20); numEnd->Width = 50;

            // --- CÁC NÚT CHỌN CHẾ ĐỘ ---
            rdoAuto = gcnew RadioButton(); rdoAuto->Text = L"Tự động (DP)"; rdoAuto->Location = Point(240, 10); rdoAuto->AutoSize = true; rdoAuto->Checked = true;
            rdoAuto->CheckedChanged += gcnew EventHandler(this, &TemperatureTab::OnModeChanged);

            rdoFixed = gcnew RadioButton(); rdoFixed->Text = L"Cố định ở:"; rdoFixed->Location = Point(240, 35); rdoFixed->AutoSize = true;
            numFixedTemp = gcnew NumericUpDown(); numFixedTemp->Location = Point(340, 33); numFixedTemp->Width = 50; numFixedTemp->Minimum = -50; numFixedTemp->Maximum = 20; numFixedTemp->Value = -20; numFixedTemp->Enabled = false;

            // [MỚI] Nút chọn chế độ Nhật ký
            rdoManual = gcnew RadioButton();
            rdoManual->Text = L"Theo nhật ký bên trái";
            rdoManual->Location = Point(240, 60); // Nằm dưới nút Cố định
            rdoManual->AutoSize = true;
            // ---------------------------

            btnRun = gcnew Button(); btnRun->Text = L"TÍNH TOÁN"; btnRun->Location = Point(440, 25); btnRun->Size = System::Drawing::Size(100, 40);
            btnRun->BackColor = Color::Blue; btnRun->ForeColor = Color::White; btnRun->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9, FontStyle::Bold));
            btnRun->Click += gcnew EventHandler(this, &TemperatureTab::OnRunOptimization);

            lblResultTotal = gcnew Label(); lblResultTotal->Text = L"Chi phí:\n0 đ"; lblResultTotal->AutoSize = true; lblResultTotal->Location = Point(560, 15);
            lblResultTotal->ForeColor = Color::Firebrick; lblResultTotal->Font = gcnew System::Drawing::Font("Segoe UI", 9, FontStyle::Bold);

            pnlControls->Controls->Add(rdoManual); // Add nút mới
            pnlControls->Controls->Add(rdoAuto); pnlControls->Controls->Add(rdoFixed); pnlControls->Controls->Add(numFixedTemp);
            pnlControls->Controls->Add(lblResultTotal); pnlControls->Controls->Add(btnRun);
            pnlControls->Controls->Add(numEnd); pnlControls->Controls->Add(l2); pnlControls->Controls->Add(numStart); pnlControls->Controls->Add(l1);

            // Phần dưới (Grid kết quả và Chart) giữ nguyên
            splitContainerResult = gcnew SplitContainer(); splitContainerResult->Dock = DockStyle::Fill; splitContainerResult->Orientation = Orientation::Horizontal; splitContainerResult->SplitterDistance = 250;

            dgvSchedule = gcnew DataGridView(); dgvSchedule->Dock = DockStyle::Fill; dgvSchedule->BackgroundColor = Color::WhiteSmoke;
            dgvSchedule->ReadOnly = true; dgvSchedule->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;
            dgvSchedule->Columns->Add("H", L"Giờ"); dgvSchedule->Columns->Add("T", L"Nhiệt Độ"); dgvSchedule->Columns->Add("D", L"Giá Điện"); dgvSchedule->Columns->Add("O", L"Vận Hành"); dgvSchedule->Columns->Add("R", L"Rủi Ro");
            splitContainerResult->Panel1->Controls->Add(dgvSchedule);
            // --- KHỞI TẠO CHART (Code mới thay thế đoạn cũ) ---
            chartResult = gcnew Chart();
            chartResult->Dock = DockStyle::Fill;

            // 1. Cấu hình ChartArea (Trục và Lưới)
            ChartArea^ chartArea = gcnew ChartArea("MainArea");
            chartArea->AxisX->Title = L"Giờ";
            chartArea->AxisX->Interval = 1;

            // ==> LÀM MỜ LƯỚI (GRID) <==
            chartArea->AxisX->MajorGrid->LineColor = Color::FromArgb(220, 220, 220); // Xám nhạt
            chartArea->AxisY->MajorGrid->LineColor = Color::FromArgb(220, 220, 220);

            chartArea->AxisY->Title = L"Chi Phí";
            chartArea->AxisY->LabelStyle->Format = "N0";

            // Trục phụ (Nhiệt độ)
            chartArea->AxisY2->Title = L"Nhiệt Độ (°C)";
            chartArea->AxisY2->Enabled = AxisEnabled::True;
            chartArea->AxisY2->MajorGrid->Enabled = false; // Tắt lưới trục phụ cho đỡ rối

            chartResult->ChartAreas->Add(chartArea);

            // Legend (Chú thích)
            Legend^ legend = gcnew Legend("MainLegend");
            legend->Docking = Docking::Top;
            legend->Alignment = StringAlignment::Center; // Căn giữa chú thích
            chartResult->Legends->Add(legend);

            // -----------------------------------------------------------
            // 2. SERIES 1: CHI PHÍ VẬN HÀNH (Cột - Vẽ Trước)
            // -----------------------------------------------------------
            Series^ sRun = gcnew Series(L"CP Vận Hành");
            sRun->ChartType = SeriesChartType::Column;
            // Màu Xanh Royal có độ trong suốt (Alpha = 180) để nhìn xuyên thấu
            sRun->Color = Color::FromArgb(180, 65, 140, 240);
            chartResult->Series->Add(sRun);

            // -----------------------------------------------------------
            // 3. SERIES 2: NHIỆT ĐỘ (Line - Vẽ Sau - Trục Phụ)
            // -----------------------------------------------------------
            Series^ sTemp = gcnew Series(L"Nhiệt Độ");
            sTemp->ChartType = SeriesChartType::Line;
            sTemp->YAxisType = AxisType::Secondary; // Gắn vào trục bên phải
            sTemp->Color = Color::Teal;             // Màu Xanh Cổ Vịt (Sang trọng)
            sTemp->BorderWidth = 3;                 // Đường dày hơn chút
            // ==> THÊM MARKER (DẤU CHẤM) <==
            sTemp->MarkerStyle = MarkerStyle::Circle;
            sTemp->MarkerSize = 8;
            sTemp->MarkerColor = Color::White;      // Chấm trắng
            sTemp->MarkerBorderColor = Color::Teal; // Viền xanh
            sTemp->MarkerBorderWidth = 2;
            chartResult->Series->Add(sTemp);

            // -----------------------------------------------------------
            // 4. SERIES 3: RỦI RO (Line - Vẽ Sau Cùng)
            // -----------------------------------------------------------
            Series^ sRisk = gcnew Series(L"CP Rủi Ro");
            sRisk->ChartType = SeriesChartType::Line;
            sRisk->Color = Color::Tomato;           // Màu Cam Đỏ (Cảnh báo nhưng không chói)
            sRisk->BorderWidth = 3;
            // ==> THÊM MARKER (HÌNH THOI) <==
            sRisk->MarkerStyle = MarkerStyle::Diamond;
            sRisk->MarkerSize = 9;
            sRisk->MarkerColor = Color::White;
            sRisk->MarkerBorderColor = Color::Tomato;
            sRisk->MarkerBorderWidth = 2;
            chartResult->Series->Add(sRisk);

            // Add Chart vào Panel
            splitContainerResult->Panel2->Controls->Add(chartResult);
            grpResult->Controls->Add(splitContainerResult); grpResult->Controls->Add(pnlControls);
            pnlRight->Controls->Add(grpResult); this->Controls->Add(pnlRight); this->Controls->Add(pnlLeft);
        }
        // Thêm/Sửa hàm này trong TemperatureTab class
        double TinhRuiRoTuBang(double nhietDoCaiDat) {
            double tongThietHai = 0;

            for (int i = 0; i < dgvProducts->Rows->Count; i++) {
                // Cột 1: Nhiệt độ chuẩn
                double nhietDoChuan = Double::Parse(dgvProducts->Rows[i]->Cells[1]->Value->ToString());

                // Cột 2: Giá trị (Cần xóa dấu phẩy ngăn cách nghìn nếu có để ép kiểu số)
                String^ strGiaTri = dgvProducts->Rows[i]->Cells[2]->Value->ToString()->Replace(",", "");
                double giaTriHang = 0;
                Double::TryParse(strGiaTri, giaTriHang);

                if (nhietDoCaiDat > nhietDoChuan) {
                    double chenhLech = nhietDoCaiDat - nhietDoChuan;
                    double tiLeHong = chenhLech * 0.1; // Hỏng 10% mỗi độ lệch
                    if (tiLeHong > 1.0) tiLeHong = 1.0;

                    tongThietHai += (giaTriHang * tiLeHong);
                }
            }
            return tongThietHai;
        }
        // Trong file TemperatureTab.h
        void LoadInitialDisplay() {
            SQLHENV hEnv; SQLHDBC hDbc; SQLHSTMT hStmt;

            // 1. Kết nối SQL
            if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) == SQL_ERROR) return;
            SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
            if (SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc) == SQL_ERROR) return;

            // Chuỗi kết nối (Unicode)
            SQLWCHAR* connStr = (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;";

            if (SQL_SUCCEEDED(SQLDriverConnect(hDbc, NULL, connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT))) {

                // --- BƯỚC QUAN TRỌNG: CHUYỂN ĐỔI CHUỖI MÀ KHÔNG CẦN MSCLR ---
                // Chuyển System::String^ (CurrentMaKho) sang std::wstring (Unicode) thủ công
                std::wstring wMaKho = L"";
                if (!String::IsNullOrEmpty(this->CurrentMaKho)) {
                    IntPtr ptr = Marshal::StringToHGlobalUni(this->CurrentMaKho);
                    wMaKho = static_cast<wchar_t*>(ptr.ToPointer());
                    Marshal::FreeHGlobal(ptr);
                }
                // -------------------------------------------------------------

                // PHẦN A: LẤY THÔNG TIN KHO
                if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
                    std::wstring sqlKho = L"SELECT NhietDoMin, NhietDoMax, CongSuatCoBan FROM Kho WHERE MaKho = ?";
                    SQLPrepare(hStmt, (SQLWCHAR*)sqlKho.c_str(), SQL_NTS);
                    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 50, 0, (SQLPOINTER)wMaKho.c_str(), 0, NULL);

                    if (SQLExecute(hStmt) == SQL_SUCCESS) {
                        if (SQLFetch(hStmt) == SQL_SUCCESS) {
                            double dMin = 0, dMax = 0, dBase = 0; // Khởi tạo = 0 để tránh lỗi E-310
                            SQLGetData(hStmt, 1, SQL_C_DOUBLE, &dMin, 0, NULL);
                            SQLGetData(hStmt, 2, SQL_C_DOUBLE, &dMax, 0, NULL);
                            SQLGetData(hStmt, 3, SQL_C_DOUBLE, &dBase, 0, NULL);
                            lblConfig->Text = String::Format(L"Dải nhiệt: {0} đến {1}°C | Công suất chuẩn: {2}kW", dMin, dMax, dBase);
                        }
                    }
                    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
                }

                // PHẦN B: LẤY SẢN PHẨM + GIÁ TRỊ (Fix Lỗi Phông & Lỗi Tính Toán)
                if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) == SQL_SUCCESS) {
                    // Câu lệnh lấy 5 cột quan trọng
                    std::wstring sqlSP = L"SELECT "
                        L"SP.TenSanPham, "      // Cột 1: Tên
                        L"SP.NhietDoThichHop, " // Cột 2: Nhiệt độ chuẩn
                        L"CT.MaLoHang, "        // Cột 3: Mã Lô
                        L"CT.SoLuongTan, "      // Cột 4: Số lượng
                        L"CT.GiaTriSanPham "    // Cột 5: GIÁ TRỊ (Quan trọng để tính tiền)
                        L"FROM TonKho TK "
                        L"JOIN ChiTietTonKho CT ON TK.MaLoHang = CT.MaLoHang "
                        L"JOIN SanPham SP ON CT.MaSanPham = SP.MaSanPham "
                        L"WHERE TK.MaKho = ? AND CT.SoLuongTan > 0 "
                        L"ORDER BY SP.NhietDoThichHop ASC";

                    SQLPrepare(hStmt, (SQLWCHAR*)sqlSP.c_str(), SQL_NTS);
                    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 50, 0, (SQLPOINTER)wMaKho.c_str(), 0, NULL);

                    if (SQLExecute(hStmt) == SQL_SUCCESS) {
                        dgvProducts->Rows->Clear();
                        dgvProducts->Columns->Clear();

                        // Thiết lập cột
                        dgvProducts->Columns->Add("Ten", L"Sản Phẩm");
                        dgvProducts->Columns->Add("Temp", L"Chuẩn (°C)");
                        dgvProducts->Columns->Add("Val", L"Giá Trị"); // Cột chứa tiền
                        dgvProducts->Columns->Add("Lo", L"Mã Lô");

                        // Dùng mảng Wide Char để hứng Tiếng Việt
                        SQLWCHAR sTen[256];
                        SQLWCHAR sMaLo[50];
                        double dTemp = 0, dSL = 0, dGiaTri = 0;

                        while (SQLFetch(hStmt) == SQL_SUCCESS) {
                            // Lấy dữ liệu Unicode chuẩn
                            SQLGetData(hStmt, 1, SQL_C_WCHAR, sTen, sizeof(sTen), NULL);
                            SQLGetData(hStmt, 2, SQL_C_DOUBLE, &dTemp, 0, NULL);
                            SQLGetData(hStmt, 3, SQL_C_WCHAR, sMaLo, sizeof(sMaLo), NULL);
                            SQLGetData(hStmt, 4, SQL_C_DOUBLE, &dSL, 0, NULL);
                            SQLGetData(hStmt, 5, SQL_C_DOUBLE, &dGiaTri, 0, NULL);

                            // Chuyển đổi hiển thị
                            String^ strTen = gcnew String((wchar_t*)sTen);
                            String^ strLo = gcnew String((wchar_t*)sMaLo);

                            // Thêm vào bảng (Format tiền có dấu phẩy: 1,000,000)
                            dgvProducts->Rows->Add(strTen, dTemp, dGiaTri.ToString("N0"), strLo);
                        }
                    }
                    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
                }
                SQLDisconnect(hDbc);
            }
            SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
            SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        }
        void OnResetManualClick(Object^ sender, EventArgs^ e) {
            // Duyệt qua tất cả các dòng và đặt lại cột Nhiệt độ (Cells[1])
            for (int i = 0; i < dgvManualInput->Rows->Count; i++) {
                dgvManualInput->Rows[i]->Cells[1]->Value = "-18";
            }
        }
        void OnRunOptimization(Object^ sender, EventArgs^ e) {
            int tStart = (int)numStart->Value;
            int tEnd = (int)numEnd->Value;

            if (tEnd < tStart) {
                MessageBox::Show(L"Thời gian kết thúc phải lớn hơn thời gian bắt đầu!");
                return;
            }

            // 1. Kết nối và khởi tạo đối tượng quản lý
            using namespace System::Runtime::InteropServices;

            IntPtr ptr = Marshal::StringToHGlobalAnsi(this->CurrentMaKho);
            std::string maKhoStd((char*)ptr.ToPointer());
            Marshal::FreeHGlobal(ptr);

            QuanLyNhietDo ql(maKhoStd);

            SQLHENV hEnv; SQLHDBC hDbc;
            SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
            SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
            SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);

            SQLWCHAR* connStr = (SQLWCHAR*)L"Driver={SQL Server};Server=LAPTOP-D0HRGU7N\\CHANGG2006;Database=PBL2;Trusted_Connection=yes;";
            SQLRETURN ret = SQLDriverConnect(hDbc, NULL, connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

            if (SQL_SUCCEEDED(ret)) {
                // Load dữ liệu KHO (minTemp, maxTemp, basePower...) và SẢN PHẨM (Giá trị, rủi ro)
                ql.LayDuLieuTuDB(hDbc);

                // --- RESET GIAO DIỆN ---
                dgvSchedule->Rows->Clear();
                chartResult->Series[L"Nhiệt Độ"]->Points->Clear();
                chartResult->Series[L"CP Vận Hành"]->Points->Clear();
                chartResult->Series[L"CP Rủi Ro"]->Points->Clear();

                double tongVanHanh = 0;
                double tongRuiRo = 0;

                // =========================================================
                // TRƯỜNG HỢP 1: TỰ ĐỘNG TỐI ƯU (Dynamic Programming)
                // =========================================================
                if (rdoAuto->Checked) {
                    ql.GiaiBaiToanToiUu(tStart, tEnd); // Chạy thuật toán DP

                    std::vector<KetQuaLichTrinh> lichTrinh = ql.GetLichTrinh();

                    for (const auto& row : lichTrinh) {
                        tongVanHanh += row.chiPhiVanHanh;
                        tongRuiRo += row.chiPhiRuiRo;

                        // Thêm vào bảng
                        dgvSchedule->Rows->Add(row.gio + ":00", row.nhietDo + " C", row.giaDien.ToString("N0"), row.chiPhiVanHanh.ToString("N0"), row.chiPhiRuiRo.ToString("N0"));

                        // Vẽ biểu đồ
                        chartResult->Series[L"Nhiệt Độ"]->Points->AddXY(row.gio, row.nhietDo);
                        chartResult->Series[L"CP Vận Hành"]->Points->AddXY(row.gio, row.chiPhiVanHanh);
                        chartResult->Series[L"CP Rủi Ro"]->Points->AddXY(row.gio, row.chiPhiRuiRo);
                    }
                }

                // =========================================================
                // TRƯỜNG HỢP 2: NHẬP TAY (Lấy từ bảng Nhật Ký bên trái)
                // =========================================================
                else if (rdoManual->Checked) {
                    for (int t = tStart; t <= tEnd; t++) {

                        String^ sTemp = dgvManualInput->Rows[t]->Cells[1]->Value->ToString();
                        double inputT = Double::Parse(sTemp);

                        // --- LOGIC TÍNH TOÁN ĐÃ SỬA ---
                        double power = ql.TinhCongSuat((int)inputT); // Lấy Công suất (kW)
                        double giaDien = ql.LayGiaDien((int)t); // Lấy Giá điện (VNĐ/kWh)

                        // [FIX] Dùng hàm TinhChiPhiRuiRo(T) (Hàm đúng trong QLNhietDo.cpp)
                        double ruiRo = ql.TinhChiPhiRuiRo((int)inputT);

                        double chiPhiVH = power * giaDien; // Chi phí Vận Hành (VNĐ/giờ)

                        tongVanHanh += chiPhiVH;
                        tongRuiRo += ruiRo;

                        // Hiển thị
                        dgvSchedule->Rows->Add(t + ":00", inputT + " C", giaDien.ToString("N0"), chiPhiVH.ToString("N0"), ruiRo.ToString("N0"));

                        chartResult->Series[L"Nhiệt Độ"]->Points->AddXY(t, inputT);
                        chartResult->Series[L"CP Vận Hành"]->Points->AddXY(t, chiPhiVH);
                        chartResult->Series[L"CP Rủi Ro"]->Points->AddXY(t, ruiRo);
                    }
                }

                // =========================================================
                // TRƯỜNG HỢP 3: CHẠY CỐ ĐỊNH 1 MỨC NHIỆT
                // =========================================================
                else {
                    double fixedT = (double)numFixedTemp->Value;

                    for (int t = tStart; t <= tEnd; t++) {
                        // --- LOGIC TÍNH TOÁN ĐÃ SỬA ---
                        double power = ql.TinhCongSuat((int)fixedT); // Lấy Công suất (kW)
                        double giaDien = ql.LayGiaDien((int)t); // Lấy Giá điện (VNĐ/kWh)

                        // [FIX] Dùng hàm TinhChiPhiRuiRo(T)
                        double ruiRo = ql.TinhChiPhiRuiRo((int)fixedT);

                        double chiPhiVH = power * giaDien; // Chi phí Vận Hành (VNĐ/giờ)

                        tongVanHanh += chiPhiVH;
                        tongRuiRo += ruiRo;

                        // Hiển thị
                        dgvSchedule->Rows->Add(t + ":00", fixedT + " C", giaDien.ToString("N0"), chiPhiVH.ToString("N0"), ruiRo.ToString("N0"));

                        chartResult->Series[L"Nhiệt Độ"]->Points->AddXY(t, fixedT);
                        chartResult->Series[L"CP Vận Hành"]->Points->AddXY(t, chiPhiVH);
                        chartResult->Series[L"CP Rủi Ro"]->Points->AddXY(t, ruiRo);
                    }
                }

                // --- HIỂN THỊ TỔNG KẾT (3 DÒNG) ---
                double tongCong = tongVanHanh + tongRuiRo;
                lblResultTotal->Text = String::Format(L"Điện: {0:N0} đ\nRủi ro: {1:N0} đ\nTỔNG: {2:N0} đ",
                    tongVanHanh, tongRuiRo, tongCong);

                chartResult->Invalidate();
            }
            else {
                MessageBox::Show(L"Không thể kết nối CSDL! Vui lòng kiểm tra lại cấu hình.");
            }

            SQLDisconnect(hDbc);
            SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
            SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        }
    };
}